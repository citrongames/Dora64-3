#include "graphics_profile.hpp"
#include "recomp_context_copy.hpp"
#include "native_frame_bridge.hpp"
#include "native_graphics_pump.hpp"
#include "native_present.hpp"
#include "native_frame_session.hpp"
#include "native_game_loop.hpp"
#include <chrono>

namespace {
thread_local const dora3::native_time::FrameBinding* building_native_frame = nullptr;
struct NativeBuildScope {
    const dora3::native_time::FrameBinding* previous = building_native_frame;
    explicit NativeBuildScope(const dora3::native_time::FrameBinding& binding) { building_native_frame = &binding; }
    ~NativeBuildScope() { building_native_frame = previous; }
};
}
extern "C" int dora3_native_frame_build_active() { return building_native_frame != nullptr; }

namespace dora3::native_time {
const FrameBinding* active_frame_binding() { return building_native_frame; }
bool FrameBridge::enter(unsigned fps, Nanoseconds now, unsigned bank, bool safe, FrameConsumer consumer, FrameUpdate update) {
    if (!consumer || !scheduler_.enter(fps, now, bank, safe)) return false;
    consumer_ = consumer;
    update_ = update;
    return true;
}
void FrameBridge::change_rate(unsigned fps, Nanoseconds now) {
    scheduler_.change_rate(fps, now);
}
void FrameBridge::leave(Nanoseconds now) {
    scheduler_.leave(now);
    consumer_ = nullptr;
    update_ = nullptr;
    // Keep the mailbox: an old enqueued message still owns its payload.
}
void FrameBridge::reset_after_runtime_barrier(Nanoseconds now) {
    std::lock_guard producer_lock(producer_mutex_);
    scheduler_.reset_after_runtime_barrier(now);
    mailbox_.receive(WakeMailbox::message_value);
    consumer_ = nullptr;
    update_ = nullptr;
}
bool FrameBridge::post_due(Nanoseconds now, WakeSender sender) {
    if (!sender) return false;
    std::lock_guard producer_lock(producer_mutex_);
    if (mailbox_.occupied()) return false;
    const auto wake = scheduler_.request(now);
    if (!wake) return false;
    if (!mailbox_.prepare(*wake)) {
        scheduler_.reject_enqueue(*wake);
        return false;
    }
    // No mailbox/scheduler lock across enqueue. The scheduled game thread can
    // receive this message before the sender returns to the producer.
    try {
        dora3::graphics_profile::mark(dora3::graphics_profile::Mark::WakePost,wake->serial,wake->epoch);
        if (sender()) return true;
    } catch (...) {
        // Sender exceptions must mean no enqueue, just like a false result.
        mailbox_.reject_enqueue(*wake);
        scheduler_.reject_enqueue(*wake);
        throw;
    }
    mailbox_.reject_enqueue(*wake);
    scheduler_.reject_enqueue(*wake);
    return false;
}
bool FrameBridge::dispatch(uint32_t message, Nanoseconds now, uint8_t* ram, recomp_context* context) {
    if (message != WakeMailbox::message_value) return false;
    const auto wake = mailbox_.receive(message);
    if (!wake) return true; // Reserved value is NEVER a guest pointer, even in Original.
    dora3::graphics_profile::mark(dora3::graphics_profile::Mark::WakeReceive,wake->serial,wake->epoch);
    // Host completions may have arrived after the receive that delivered this
    // wake. Process them before a long Native update can delay the scheduler.
    dora3_native_scheduler_checkpoint(ram);
    // Keep the queued request pending during update: no new wake can overtake
    // it. Unlike a Building bank, this token can be invalidated by a resource
    // barrier without waiting for a frame that has not yet been submitted.
    if (update_ && scheduler_.ready(*wake,now)) {
        recomp_context update_context;dora3::copy_recomp_context(update_context,*context);
        update_(ram,&update_context,now);
    }
    // The update is a coherent boundary: no borrowed model pose or unpublished
    // frame buffer is active. Let already-ready scheduler work run before draw.
    dora3_native_scheduler_checkpoint(ram);
    // Update may have changed scene/rate/session. Never publish its old wake.
    const auto ticket = scheduler_.receive(*wake, now);
    if (ticket && consumer_) {
        dora3::graphics_profile::mark(dora3::graphics_profile::Mark::FrameTicket,ticket->token.serial,ticket->bank);
        // This hook is not a MIPS call site. Keep all registers, notably s0/s1
        // used by the original message loop, intact across the extra frame.
        recomp_context frame_context;dora3::copy_recomp_context(frame_context,*context);
        const FrameBinding binding{*ticket,&scheduler_};
        NativeBuildScope scope(binding);
        consumer_(ram, &frame_context, *ticket, scheduler_);
    }
    return true;
}
std::optional<Nanoseconds> FrameBridge::deadline() const {
    if (mailbox_.occupied()) return std::nullopt;
    return scheduler_.deadline();
}
FrameBridge& game_frame_bridge() {
    static FrameBridge bridge;
    return bridge;
}
Nanoseconds monotonic_time() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
} // namespace dora3::native_time

void dora3_reset_native_frame_delivery() {
    using namespace dora3::native_time;
    native_present_ledger().reset_after_runtime_barrier();
    game_frame_bridge().reset_after_runtime_barrier(monotonic_time());
    game_frame_session().reset_after_runtime_barrier();
    dora3_native_loop_reset();
}

extern "C" int dora3_native_game_message(uint8_t* rdram, recomp_context* ctx) {
    using namespace dora3::native_time;
    const auto message = uint32_t(MEM_W(0x10, ctx->r29));
    if (message != WakeMailbox::message_value) return 0;
    // Consume reserved messages even while the original game pause gate is set.
    // Leaving invalidates their tokens before any scene callback can run.
    const auto now = monotonic_time();
    auto& bridge = game_frame_bridge();
    if (MEM_W(0, int32_t(0x801AA680)) == 1) bridge.leave(now);
    return bridge.dispatch(message, now, rdram, ctx) ? 1 : 0;
}
