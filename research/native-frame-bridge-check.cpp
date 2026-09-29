#include "native_frame_bridge.hpp"
#include "native_present.hpp"
#include <stdexcept>
bool dora3_enqueue_native_present(const dora3::native_time::PresentFrame&) { throw std::logic_error("Unexpected graphics task in bridge-only check"); }
#include "ultramodern/ultramodern.hpp"
#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

using namespace dora3::native_time;
extern "C" void dora3_game_message_loop(uint8_t*, recomp_context*);
void dequeue_external_messages(uint8_t*);
static bool interrupts = true;
extern "C" bool dora3_interrupts_enabled() { return interrupts; }
namespace ultramodern {
bool is_game_thread() { return true; }
PTR(OSThread) this_thread() { return 0; }
bool thread_queue_empty(uint8_t*, PTR(PTR(OSThread))) { return true; }
void thread_queue_insert(uint8_t*, PTR(PTR(OSThread)), PTR(OSThread)) { assert(false); }
PTR(OSThread) thread_queue_pop(uint8_t*, PTR(PTR(OSThread))) { assert(false); return 0; }
void schedule_running_thread(uint8_t*, PTR(OSThread)) { assert(false); }
void run_next_thread_and_wait(uint8_t*) { assert(false); }
void check_running_queue(uint8_t*) {}
}
static constexpr int32_t queue = int32_t(game_queue), buffer = int32_t(0x80220000), result = buffer + 0x100;
static constexpr int32_t vi_message = int32_t(0x80220120), task_message = int32_t(0x8010E3D0);
static FrameScheduler completion_owner;
static int updates, builds, retires, lifetimes, pauses, native_frames, receive_index;
static bool loop_test;
struct StopLoop {};
extern "C" void dora3_trace(uint8_t*, recomp_context*, uint32_t, uint32_t) {}
extern "C" void dora3_s0_80047EF4(uint8_t*, recomp_context*) { ++pauses; }
extern "C" void dora3_object_lifetime_tick(uint8_t*, recomp_context*) { ++lifetimes; }
extern "C" void dora3_deferred_free_tick(uint8_t*, recomp_context*) { ++retires; }
extern "C" void dora3_game_retrace_update(uint8_t*, recomp_context*) { ++updates; }
extern "C" void dora3_graphics_frame_build(uint8_t*, recomp_context*) { ++builds; }
extern "C" void dora3_scene_transition_framebuffer_copy(uint8_t*, recomp_context*) { assert(false); }

static void consumer(uint8_t*, recomp_context* c, const FrameTicket& t, FrameScheduler& s) {
    ++native_frames;
    assert(t.step.elapsed > 0);
    assert(s.publish(t));
    assert(s.complete(t, Completion::HostRead));
    assert(s.complete(t, Completion::GuestTask));
    // A real generated callback clobbers registers too. They must not leak back
    // into the original loop's saved selector or stack pointer.
    *c = {};
}
static bool reject_sender() { return false; }
static bool retain_sender() { return true; }
static bool throw_sender() { throw std::bad_alloc{}; }
static FrameBridge* synchronous_bridge;
static uint8_t* synchronous_ram;
static recomp_context* synchronous_context;
static bool synchronous_sender() {
    // Model a receiver that consumes the queue entry before enqueue returns.
    assert(synchronous_bridge->dispatch(WakeMailbox::message_value, 5 * second,
        synchronous_ram, synchronous_context));
    return true;
}
static uint32_t read_message(uint8_t* rdram) {
    assert(osRecvMesg(rdram, queue, result, OS_MESG_NOBLOCK) == 0);
    return uint32_t(MEM_W(0, result));
}

static void transport(uint8_t* rdram) {
    osCreateMesgQueue(rdram, queue, buffer, 1);
    assert(osSendMesg(rdram, queue, 123, OS_MESG_NOBLOCK) == 0);
    assert(dora3_enqueue_native_frame_wake());
    // All three runtime receive paths must retain the sole native wake while
    // the guest queue is full. These are actual pinned runtime queue bodies.
    dequeue_external_messages(rdram);
    ultramodern::wait_for_external_message_timed(rdram, 0);
    ultramodern::wait_for_external_message(rdram);
    assert(read_message(rdram) == 123);
    assert(read_message(rdram) == WakeMailbox::message_value);
    assert(osRecvMesg(rdram, queue, result, OS_MESG_NOBLOCK) == -1);
    assert(dora3_enqueue_native_frame_wake());
    interrupts = false;
    dequeue_external_messages(rdram);
    assert(osRecvMesg(rdram, queue, result, OS_MESG_NOBLOCK) == -1);
    interrupts = true;
    assert(read_message(rdram) == WakeMailbox::message_value);
}

static void lifecycle(uint8_t* rdram, recomp_context* c) {
    FrameBridge b;
    assert(!b.enter(60, 0, 0, true, nullptr));
    assert(!b.enter(60, 0, 0, false, consumer));
    assert(!b.post_due(second, retain_sender));
    assert(b.enter(60, 0, 0, true, consumer));
    assert(!b.post_due(second, reject_sender));
    assert(b.stats().queue_rejections == 1);
    assert(b.post_due(second, retain_sender));
    b.change_rate(144, second);
    assert(!b.post_due(2 * second, retain_sender)); // Old queued payload must drain first.
    assert(b.dispatch(WakeMailbox::message_value, 2 * second, rdram, c));
    assert(native_frames == 0);
    assert(b.post_due(2 * second, retain_sender));
    const auto before = *c;
    assert(b.dispatch(WakeMailbox::message_value, 2 * second, rdram, c));
    assert(std::memcmp(&before, c, sizeof(*c)) == 0 && native_frames == 1);
    assert(b.post_due(3 * second, retain_sender));
    b.leave(3 * second);
    assert(b.enter(73, 3 * second, 1, true, consumer));
    assert(!b.post_due(4 * second, retain_sender));
    assert(b.dispatch(WakeMailbox::message_value, 4 * second, rdram, c));
    assert(native_frames == 1); // Late wake cannot run a new scene's callback.
    assert(b.post_due(4 * second, retain_sender));
    b.leave(4 * second);
    assert(b.dispatch(WakeMailbox::message_value, 4 * second, rdram, c));
    assert(b.dispatch(WakeMailbox::message_value, 4 * second, rdram, c));
    assert(!b.dispatch(uint32_t(vi_message), 4 * second, rdram, c));
    assert(native_frames == 1 && b.drained());
    assert(b.enter(60, 4 * second, 0, true, consumer));
    try { b.post_due(5 * second, throw_sender); assert(false); }
    catch (const std::bad_alloc&) {}
    synchronous_bridge = &b;
    synchronous_ram = rdram;
    synchronous_context = c;
    assert(b.post_due(5 * second, synchronous_sender));
    assert(native_frames == 2 && b.drained());
    b.leave(5 * second);
}

extern "C" void osRecvMesg_recomp(uint8_t* rdram, recomp_context* c) {
    assert(loop_test && uint32_t(c->r4) == game_queue && c->r6 == 1);
    auto& b = game_frame_bridge();
    // At each iteration inspect the effects of the PREVIOUS message, then
    // supply the next one through the real external/guest message queue.
    switch (receive_index++) {
    case 0:
        assert(b.enter(60, monotonic_time() - second, 0, true, consumer));
        assert(b.post_due(monotonic_time(), dora3_enqueue_native_frame_wake));
        break;
    case 1:
        assert(native_frames == 1 && lifetimes == 0 && retires == 0 && builds == 0 && updates == 0);
        assert(MEM_W(0, int32_t(0x801AA618)) == 0);
        assert(osSendMesg(rdram, queue, vi_message, OS_MESG_NOBLOCK) == 0);
        break;
    case 2: {
        assert(lifetimes == 1 && retires == 1 && builds == 1 && updates == 1);
        assert(MEM_W(0, int32_t(0x801AA618)) == 1);
        assert(completion_owner.enter(60,0,0,true));
        const auto wake=completion_owner.request(second);assert(wake);
        const auto ticket=completion_owner.receive(*wake,second);assert(ticket && completion_owner.publish(*ticket));
        native_present_ledger().bind(*ticket,completion_owner,0x8010E310,0x803B5000,uint32_t(task_message));
        assert(native_present_ledger().retire(0x8010E310));
        assert(native_present_ledger().host_read_done(*ticket) && !completion_owner.drained());
        MEM_W(0,int32_t(0x801AA680))=1; // Completion must survive the Original pause gate.
        assert(osSendMesg(rdram, queue, task_message, OS_MESG_NOBLOCK) == 0);
        break;
    }
    case 3:
        assert(completion_owner.drained() && !native_present_ledger().occupied() && pauses==0);
        MEM_W(0,int32_t(0x801AA680))=0;
        assert(MEM_W(0, int32_t(0x801AA618)) == 0);
        b.leave(monotonic_time());
        assert(dora3_enqueue_native_frame_wake()); // Stale/empty mailbox in Original.
        break;
    case 4:
        assert(native_frames == 1 && lifetimes == 1 && retires == 1 && builds == 1 && updates == 1);
        assert(b.enter(60, monotonic_time() - second, 0, true, consumer));
        assert(b.post_due(monotonic_time(), dora3_enqueue_native_frame_wake));
        MEM_W(0, int32_t(0x801AA680)) = 1;
        break;
    case 5:
        assert(native_frames == 1 && pauses == 0 && b.drained());
        assert(!b.post_due(monotonic_time() + second, dora3_enqueue_native_frame_wake));
        assert(osSendMesg(rdram, queue, vi_message, OS_MESG_NOBLOCK) == 0);
        break;
    case 6:
        assert(pauses == 1 && updates == 1 && lifetimes == 1 && retires == 1);
        MEM_H(0,vi_message+8)=2;
        assert(osSendMesg(rdram,queue,vi_message+8,OS_MESG_NOBLOCK)==0);
        break;
    case 7:
        assert(pauses==2 && MEM_W(0,int32_t(0x801AA618))==0); // Unregistered type2 keeps Original pause behavior.
        MEM_W(0, int32_t(0x801AA680)) = 0;
        throw StopLoop{};
    default: assert(false);
    }
    c->r2 = osRecvMesg(rdram, int32_t(c->r4), int32_t(c->r5), OS_MESG_NOBLOCK);
    assert(c->r2 == 0);
}

static FrameScheduler* held_scheduler;
static std::optional<FrameTicket> held_ticket;
static void hold_consumer(uint8_t*, recomp_context*, const FrameTicket& t, FrameScheduler& s) {
    assert(s.publish(t));
    held_scheduler = &s;
    held_ticket = t;
}
static void reset_delivery(uint8_t* rdram, recomp_context* c) {
    auto& b = game_frame_bridge();
    assert(b.enter(60, 0, 0, true, hold_consumer));
    assert(b.post_due(second, dora3_enqueue_native_frame_wake));
    assert(b.dispatch(read_message(rdram), second, rdram, c));
    assert(held_ticket && !b.drained());
    assert(b.post_due(2 * second, dora3_enqueue_native_frame_wake));
    // The harness has no renderer or running guest threads. In the real app the
    // pinned caller establishes both barriers before invoking this queue reset.
    ultramodern::reset_message_queues_for_game_reset();
    assert(b.drained() && !b.deadline());
    assert(!b.post_due(3 * second, dora3_enqueue_native_frame_wake));
    assert(osRecvMesg(rdram, queue, result, OS_MESG_NOBLOCK) == -1);
    assert(!held_scheduler->complete(*held_ticket, Completion::GuestTask));
    assert(!held_scheduler->complete(*held_ticket, Completion::HostRead));
    assert(b.enter(60, 3 * second, 0, true, consumer));
    assert(b.post_due(4 * second, dora3_enqueue_native_frame_wake));
    assert(b.dispatch(read_message(rdram), 4 * second, rdram, c));
    assert(b.drained());
    b.leave(4 * second);
}

static void concurrent(uint8_t* rdram, recomp_context* c) {
    FrameBridge b;
    native_frames = 0;
    assert(b.enter(360, 0, 0, true, consumer));
    std::atomic<unsigned> accepted{};
    std::thread producer([&] {
        for (unsigned i = 1; i <= 2000; ++i) {
            while (!b.post_due(Nanoseconds(i) * second, dora3_enqueue_native_frame_wake)) std::this_thread::yield();
            accepted.store(i, std::memory_order_release);
        }
    });
    for (unsigned i = 1; i <= 2000; ++i) {
        while (accepted.load(std::memory_order_acquire) < i) std::this_thread::yield();
        assert(b.dispatch(read_message(rdram), Nanoseconds(i) * second, rdram, c));
    }
    producer.join();
    assert(native_frames == 2000 && b.drained());
    b.leave(2001 * second);
}
int main() {
    std::vector<uint8_t> memory(8 * 1024 * 1024);
    auto* rdram = memory.data();
    recomp_context ctx{};
    ctx.r29 = int32_t(0x803F0000);
    transport(rdram);
    lifecycle(rdram, &ctx);
    native_frames = 0;
    MEM_H(0, vi_message) = 1;
    MEM_H(0, task_message) = 2;
    loop_test = true;
    try { dora3_game_message_loop(rdram, &ctx); assert(false); }
    catch (const StopLoop&) {}
    loop_test = false;
    reset_delivery(rdram, &ctx);
    concurrent(rdram, &ctx);
    std::cout << "PASS: registered type2 drains during pause while unregistered type2 remains Original; real runtime queue full/requeue/interrupt gating; original game loop native/VI/task/pause dispatch; context preserved; stale scene/rate wakes; quiescent reset; 2000 concurrent deliveries\n";
}
