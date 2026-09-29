#include "native_frame_scheduler.hpp"
#include "recomp.h"
#include "native-scheduler-functions.h"
#include <cassert>
#include <barrier>
#include <iostream>
#include <set>
#include <thread>
#include <vector>

using namespace dora3::native_time;
// Original-only oracle; native ownership hooks are exercised by the pump check.
extern "C" int dora3_native_framebuffer_ready(uint32_t) { return 0; }
extern "C" void dora3_native_task_retired(uint8_t*, recomp_context*) {}
extern "C" void dora3_trace(uint8_t*, recomp_context*, uint32_t, uint32_t) {}
static uint32_t current_fb = 0x80380000, next_fb = current_fb;
static std::vector<int> sdk_events;
extern "C" void osViGetCurrentFramebuffer_recomp(uint8_t*, recomp_context* c) { c->r2 = int32_t(current_fb); }
extern "C" void osViGetNextFramebuffer_recomp(uint8_t*, recomp_context* c) { c->r2 = int32_t(next_fb); }
extern "C" void osSendMesg_recomp(uint8_t*, recomp_context* c) {
    assert(uint32_t(c->r4) == 0x801A99E0 && uint32_t(c->r5) == 0x80102000 && c->r6 == 1);
    sdk_events.push_back(1);
    c->r2 = 0;
}
extern "C" void osViSwapBuffer_recomp(uint8_t*, recomp_context* c) {
    next_fb = uint32_t(c->r4);
    sdk_events.push_back(2);
}
extern "C" void osViBlack_recomp(uint8_t*, recomp_context* c) {
    assert(c->r4 == 0);
    sdk_events.push_back(3);
}

static FrameTicket begin(FrameScheduler& s, Nanoseconds now) {
    const auto wake = s.request(now);
    assert(wake);
    assert(!s.request(now)); // At most one wake, including time spent in the guest queue.
    const auto ticket = s.receive(*wake, now);
    assert(ticket);
    return *ticket;
}
static void finish(FrameScheduler& s, const FrameTicket& t) {
    assert(s.complete(t, Completion::GuestTask));
    assert(s.complete(t, Completion::HostRead));
}

static void protocol() {
    FrameScheduler s;
    assert(!s.request(second) && !s.deadline()); // Default/Original issues nothing.
    assert(!s.enter(144, 0, 0, false)); // Original task drain must be certified by adapter.
    assert(s.enter(144, 0, 0, true));
    auto a = begin(s, 7'000'000);
    assert(a.bank == 0 && a.step.elapsed == 7'000'000);
    assert(!s.complete(a, Completion::GuestTask)); // Not yet published.
    assert(s.publish(a) && !s.publish(a));
    auto b = begin(s, 14'000'000);
    assert(b.bank == 1 && s.publish(b));
    assert(!s.request(second) && !s.deadline()); // Both banks held: no wake storm.
    assert(s.complete(a, Completion::GuestTask));
    assert(!s.complete(a, Completion::GuestTask));
    assert(!s.request(second)); // Guest completion alone cannot free host-read memory.
    assert(s.complete(a, Completion::HostRead));
    auto c = begin(s, second);
    assert(c.bank == 0 && c.step.elapsed == second - 14'000'000);
    assert(s.publish(c));
    assert(!s.complete(a, Completion::HostRead)); // Old ticket cannot release reused bank0.
    assert(s.complete(b, Completion::HostRead));
    assert(!s.request(2 * second)); // Host-read completion alone is also insufficient.
    assert(s.complete(b, Completion::GuestTask));
    const auto rejected = s.request(2 * second);
    assert(rejected && s.reject_enqueue(*rejected));
    const auto accepted = s.request(2 * second);
    assert(accepted && *accepted != *rejected);
    assert(!s.reject_enqueue(*rejected)); // A late failure cannot cancel a newer message.
    assert(!s.receive(*rejected, 2 * second));
    const auto d = s.receive(*accepted, 2 * second);
    assert(d && d->step.elapsed == second && s.publish(*d));
    s.leave(2 * second); // Pending tasks survive scene/mode leave.
    assert(!s.request(3 * second) && !s.drained());
    assert(!s.enter(73, 3 * second, 0, true));
    finish(s, c);
    finish(s, *d);
    assert(s.drained() && s.enter(73, 3 * second, 1, true));
    const auto old_scene_wake = s.request(4 * second);
    assert(old_scene_wake);
    s.leave(4 * second);
    assert(s.enter(165, 4 * second, 1, true));
    const auto new_scene_wake = s.request(5 * second);
    assert(new_scene_wake && !s.receive(*old_scene_wake, 5 * second));
    const auto new_frame = s.receive(*new_scene_wake, 5 * second);
    assert(new_frame && new_frame->bank == 1 && s.publish(*new_frame));
    finish(s, *new_frame);
    const auto old_rate = s.request(6 * second);
    assert(old_rate);
    s.change_rate(360, 6 * second);
    assert(!s.receive(*old_rate, 6 * second));
    assert(!s.request(6 * second));
    const auto due = s.deadline();
    assert(due);
    const auto last = begin(s, *due);
    assert(last.step.elapsed == *due - 5 * second && last.step.target_fps == 360);
    assert(s.publish(last)); finish(s, last);
    s.leave(7 * second);
    assert(s.drained() && !s.deadline());
    assert(s.stats().queue_rejections == 1);
}

static void original_scheduler() {
    std::vector<uint32_t> memory(0x400000 / 4);
    auto* rdram = reinterpret_cast<uint8_t*>(memory.data());
    recomp_context c{}; c.r29 = int32_t(0x803F0000);
    const auto task = int32_t(0x80101000);
    const auto ready = [&] {
        c.r4 = task;
        dora3_scheduler_framebuffer_ready(rdram, &c);
        return uint32_t(c.r2);
    };
    assert(ready() == uint32_t(task));
    next_fb = 0x803A5800;
    assert(ready() == 0); // A real VI equality gate, not a graphics-task fence.
    current_fb = next_fb;
    assert(ready() == uint32_t(task));
    MEM_W(0x50, task) = int32_t(0x801A99E0);
    MEM_W(0x54, task) = int32_t(0x80102000);
    MEM_W(0x10, task) = 1;
    MEM_W(8, task) = 0x60;
    MEM_W(12, task) = int32_t(0x80380000);
    for (int resources : {3, 2, 1}) {
        MEM_W(4, task) = resources;
        c.r5 = task;
        dora3_scheduler_task_complete(rdram, &c);
        assert(c.r2 == 0 && sdk_events.empty());
    }
    MEM_W(4, task) = 0;
    MEM_W(0, int32_t(0x8006FA9C)) = 1;
    c.r5 = task;
    dora3_scheduler_task_complete(rdram, &c);
    assert(c.r2 == 1 && (sdk_events == std::vector<int>{1, 3, 2}));
    assert(MEM_W(0, int32_t(0x8006FA9C)) == 0);
    assert(ready() == 0); // Guest completion precedes framebuffer latch on VI.
    sdk_events.clear();
    MEM_W(0x10, task) = 2; // Audio completion does not request a framebuffer swap.
    c.r5 = task;
    dora3_scheduler_task_complete(rdram, &c);
    assert((sdk_events == std::vector<int>{1}));
}

static void message_lifetime() {
    FrameScheduler scheduler;
    WakeMailbox mailbox;
    assert(scheduler.enter(60, 0, 0, true));
    const auto old = scheduler.request(second);
    assert(old && mailbox.prepare(*old));
    assert(!mailbox.receive(0x801A99E0) && mailbox.occupied()); // Original guest messages untouched.
    scheduler.leave(second);
    assert(scheduler.enter(144, second, 1, true));
    const auto newer = scheduler.request(2 * second);
    assert(newer && !mailbox.prepare(*newer)); // Old queued payload cannot be overwritten.
    assert(scheduler.reject_enqueue(*newer));
    const auto received_old = mailbox.receive(WakeMailbox::message_value);
    assert(received_old == old && !scheduler.receive(*received_old, 2 * second));
    const auto retry = scheduler.request(2 * second);
    assert(retry && mailbox.prepare(*retry));
    assert(!mailbox.reject_enqueue(*old));
    assert(mailbox.reject_enqueue(*retry)); // A full guest queue releases this token only.
    assert(scheduler.reject_enqueue(*retry));
    const auto accepted = scheduler.request(2 * second);
    assert(accepted && mailbox.prepare(*accepted));
    const auto token = mailbox.receive(WakeMailbox::message_value);
    assert(token == accepted && !mailbox.occupied());
    const auto frame = scheduler.receive(*token, 2 * second);
    assert(frame && scheduler.publish(*frame));
    finish(scheduler, *frame);
    assert(!mailbox.receive(WakeMailbox::message_value));
}

static void concurrent_completions() {
    FrameScheduler scheduler;
    assert(scheduler.enter(360, 0, 0, true));
    std::barrier rendezvous(3);
    FrameTicket ticket{};
    auto worker = [&](Completion kind) {
        for (int i = 1; i <= 2000; ++i) {
            rendezvous.arrive_and_wait();
            assert(scheduler.complete(ticket, kind));
            rendezvous.arrive_and_wait();
        }
    };
    std::thread guest(worker, Completion::GuestTask), host(worker, Completion::HostRead);
    for (int i = 1; i <= 2000; ++i) {
        ticket = begin(scheduler, i * 3'000'000LL);
        assert(scheduler.publish(ticket));
        rendezvous.arrive_and_wait();
        rendezvous.arrive_and_wait();
        assert(scheduler.drained());
    }
    guest.join(); host.join();
    scheduler.leave(7 * second);
    assert(scheduler.stats().frames == 2000 && scheduler.drained());
}

static void native_fade_display_lists() {
    std::vector<uint32_t> memory(0x400000 / 4);
    auto* rdram = reinterpret_cast<uint8_t*>(memory.data());
    recomp_context c{}; c.r29 = int32_t(0x803F0000);
    MEM_B(0, int32_t(0x801B1404)) = 255;
    MEM_B(0, int32_t(0x80171CB8)) = 255;
    MEM_B(0, int32_t(0x801A99B4)) = 255;
    MEM_H(0, int32_t(0x801A6C58)) = 320;
    MEM_H(0, int32_t(0x801A48B6)) = 240;
    for (unsigned fps : {60u, 73u, 144u, 165u, 360u}) {
        FrameScheduler scheduler;
        WakeMailbox mailbox;
        Fade fade({0, 1, 16}, 3);
        assert(scheduler.enter(fps, 0, 0, true));
        std::set<unsigned> distinct_alpha;
        unsigned built = 0;
        for (;;) {
            const auto now = scheduler.deadline();
            assert(now);
            const auto wake = scheduler.request(*now);
            assert(wake && mailbox.prepare(*wake));
            const auto message = mailbox.receive(WakeMailbox::message_value);
            assert(message);
            const auto frame = scheduler.receive(*message, *now);
            assert(frame);
            const auto state = fade.advance(frame->step.elapsed);
            const auto buffer = int32_t(0x80200000 + frame->bank * 0x1000);
            const auto other = int32_t(0x80200000 + (frame->bank ^ 1) * 0x1000);
            const auto other_before = MEM_W(0x34, other);
            MEM_W(0, int32_t(0x801B1424)) = buffer;
            MEM_H(0, int32_t(0x801AA660)) = state.alpha;
            MEM_B(0, int32_t(0x801A99A0)) = state.direction;
            dora3_fade_draw_dispatch(rdram, &c); // Original guest code builds a fresh 2D list.
            assert(MEM_W(0, int32_t(0x801B1424)) == buffer + 0x40);
            assert(uint32_t(MEM_W(0x30, buffer)) == 0xFA000000);
            const auto rgba = uint32_t(MEM_W(0x34, buffer));
            assert((rgba >> 8) == 0xFFFFFF && (rgba & 255) == unsigned(state.alpha));
            assert(MEM_W(0x34, other) == other_before);
            distinct_alpha.insert(rgba & 255);
            assert(scheduler.publish(*frame));
            finish(scheduler, *frame); // Synthetic completion endpoints; no actual RSP/GPU.
            ++built;
            if (state.direction == 2) {
                assert(*now >= 800'000'000 && *now < 800'000'000 + (second + fps - 1) / fps);
                break;
            }
            assert(*now < 800'000'000);
        }
        assert(built == (fps * 4 + 4) / 5 && distinct_alpha.size() > 16);
        assert(scheduler.drained());
    }
}

int main() {
    protocol(); original_scheduler(); message_lifetime(); concurrent_completions(); native_fade_display_lists();
    std::cout << "PASS: one queued wake; immutable token behind 32-bit message; two banks; publication and independent guest/host completion; stale scene/rate tokens; enqueue rejection; elapsed-time conservation under backpressure; Original bypass; concurrent completion order; original ROM VI gate and completion-before-swap order; native 60/73/144/165/360 scheduling builds original 2D fade display lists with extra alpha phases and unchanged duration. No game/window/GPU.\n";
}
