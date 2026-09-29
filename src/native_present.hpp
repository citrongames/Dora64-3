#pragma once
#include "native_frame_scheduler.hpp"
#include <array>
#include <mutex>
#include <optional>

namespace dora3::native_time {
struct PresentFrame {
    FrameTicket ticket;
    uint32_t task, framebuffer, message;
};
// Exactly two records. Registration precedes task publication. Owner outlives
// all records; clearing them requires the existing runtime reset barrier.
class PresentLedger {
public:
    void bind(const FrameTicket&, FrameScheduler&, uint32_t task, uint32_t framebuffer, uint32_t message);
    bool owns_task(uint32_t task) const;
    bool occupied() const;
    std::optional<PresentFrame> retire(uint32_t task);
    bool consumed(uint32_t message);
    bool host_read_done(const FrameTicket&);
    void reset_after_runtime_barrier();
private:
    struct Record {
        PresentFrame frame{};
        FrameScheduler* owner = nullptr;
        bool retired = false, consumed = false, host_done = false, guest_notified = false;
    };
    mutable std::mutex mutex_;
    std::array<Record,2> records_{};
    void reconcile(Record&);
};
PresentLedger& native_present_ledger();
bool native_presentation_owned();
uint64_t native_presentation_epoch();
}

// Renderer queue accepts an immutable copy and calls host_read_done after
// update_screen returns; this is a CPU-memory-read boundary, NOT GPU idle.
bool dora3_enqueue_native_present(const dora3::native_time::PresentFrame&);
