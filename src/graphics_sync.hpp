#pragma once
#include <atomic>
#include <cstdint>

namespace dora3::sync {
// One outstanding periodic presentation request, including its processing time.
// VI/AI messages and graphics tasks do not pass through this gate.
class PeriodicPresent {
    std::atomic<bool> pending{false};
    std::atomic<uint64_t> accepted{0}, skipped{0};
public:
    bool acquire() {
        if (pending.exchange(true, std::memory_order_acq_rel)) {
            skipped.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        accepted.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    void complete() { pending.store(false, std::memory_order_release); }
    uint64_t submissions() const { return accepted.load(std::memory_order_relaxed); }
    uint64_t coalesced() const { return skipped.load(std::memory_order_relaxed); }
};
inline PeriodicPresent periodic_present;
}
extern "C" void dora3_begin_cpu_fb_copy();
extern "C" void dora3_end_cpu_fb_copy();
