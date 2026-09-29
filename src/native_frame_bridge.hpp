#pragma once
#include "native_frame_scheduler.hpp"
#include "recomp.h"

namespace dora3::native_time {
// True means accepted by the host queue, not delivered to the guest queue.
// False or an exception means nothing was enqueued. Sender must not reenter post_due.
using WakeSender = bool (*)();
using FrameConsumer = void (*)(uint8_t*, recomp_context*, const FrameTicket&, FrameScheduler&);

// enter/change_rate/leave/dispatch belong to the scheduled game thread.
// post_due may run on one host producer, concurrently with dispatch/completions.
// A consumer must publish before submission and retain its ticket until BOTH
// actual guest completion and the reviewed host-memory boundary are established.
class FrameBridge {
public:
    bool enter(unsigned fps, Nanoseconds now, unsigned bank, bool safe, FrameConsumer consumer);
    void change_rate(unsigned fps, Nanoseconds now);
    void leave(Nanoseconds now);
    // Reset coordinator only, after the verified runtime quiescence barrier.
    // Excludes post_due and leaves production disabled before queues are cleared.
    void reset_after_runtime_barrier(Nanoseconds now);
    bool post_due(Nanoseconds now, WakeSender sender);
    bool dispatch(uint32_t message, Nanoseconds now, uint8_t* ram, recomp_context* context);
    std::optional<Nanoseconds> deadline() const;
    uint64_t generation() const { return scheduler_.generation(); }
    bool active() const { return scheduler_.active(); }
    bool drained() const { return scheduler_.drained(); }
    SchedulerStats stats() const { return scheduler_.stats(); }
private:
    FrameScheduler scheduler_;
    WakeMailbox mailbox_;
    std::mutex producer_mutex_;
    FrameConsumer consumer_ = nullptr;
};

struct FrameBinding { FrameTicket ticket; FrameScheduler* owner; };
const FrameBinding* active_frame_binding(); // Scheduled game thread only.
FrameBridge& game_frame_bridge();
Nanoseconds monotonic_time();
} // namespace dora3::native_time

// Implemented in the pinned, build-local mesgqueue adaptation. Retains the wake
// if the guest queue is full; reports an initial host allocation failure.
bool dora3_enqueue_native_frame_wake();
void dora3_reset_native_frame_delivery();
extern "C" int dora3_native_game_message(uint8_t*, recomp_context*);
extern "C" int dora3_native_frame_build_active();
