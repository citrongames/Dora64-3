#pragma once
#include <cstdint>
#include <limits>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace dora3::native_time {
// Serializes admission, not the read itself. Guest threads never wait for the
// renderer while holding the runtime's cooperative guest scheduling token.
class PresentationGate {
public:
    std::optional<uint64_t> reserve() {
        std::lock_guard lock(mutex_);
        if (reservation_) return std::nullopt;
        if (serial_==std::numeric_limits<uint64_t>::max()) throw std::overflow_error("Presentation generation exhausted");
        reservation_=++serial_;
        return reservation_;
    }
    bool ready(uint64_t token) const {
        std::lock_guard lock(mutex_);
        return reservation_==token && readers_==0;
    }
    bool release(uint64_t token) {
        std::lock_guard lock(mutex_);
        if (reservation_!=token) return false;
        reservation_.reset();return true;
    }
    bool reserved() const { std::lock_guard lock(mutex_);return reservation_.has_value(); }
    bool begin_read() {
        std::lock_guard lock(mutex_);
        if (reservation_) return false;
        ++readers_;return true;
    }
    void end_read() {
        std::lock_guard lock(mutex_);
        if (!readers_) std::terminate();
        --readers_;
    }
    void reset_after_runtime_barrier() {
        std::lock_guard lock(mutex_);
        if (readers_) throw std::logic_error("Presentation reset before host read barrier");
        reservation_.reset(); // Serial never rolls back across resets.
    }
private:
    mutable std::mutex mutex_;
    uint64_t serial_=0;
    unsigned readers_=0;
    std::optional<uint64_t> reservation_;
};
inline PresentationGate& presentation_gate() { static PresentationGate gate;return gate; }
class OriginalReadLease {
public:
    explicit OriginalReadLease(PresentationGate& gate,bool needed=true):gate_(needed?&gate:nullptr),valid_(!needed||gate.begin_read()) {}
    ~OriginalReadLease() { if (gate_ && valid_) gate_->end_read(); }
    OriginalReadLease(const OriginalReadLease&)=delete;
    OriginalReadLease& operator=(const OriginalReadLease&)=delete;
    explicit operator bool() const { return valid_; }
private:
    PresentationGate* gate_;
    bool valid_;
};
}
