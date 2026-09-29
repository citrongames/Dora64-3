#pragma once
#include "native_frame_time.hpp"
#include <array>
#include <mutex>
#include <optional>

namespace dora3::native_time {
inline constexpr std::uint32_t game_queue = 0x801A99E0u;
struct WakeToken {
    std::uint64_t epoch = 0, serial = 0;
    bool operator==(const WakeToken&) const = default;
};
struct FrameTicket {
    WakeToken token;
    unsigned bank;
    FrameStep step;
};
enum class Completion { GuestTask, HostRead };
struct SchedulerStats {
    std::uint64_t wakes = 0, frames = 0, queue_rejections = 0;
    std::uint64_t stale_wakes = 0, stale_completions = 0;
};

// OSMesg is 32-bit. Keep one immutable token until that queued message is read,
// even across a scene/rate change. Reusing a mutable payload would create an ABA
// bug: an old message could then execute the new scene's frame request.
// The game-queue adapter MUST intercept message_value before guest pointer reads.
class WakeMailbox {
public:
    static constexpr std::uint32_t message_value = 0xFFFFFF03u;
    bool prepare(WakeToken token);
    bool reject_enqueue(WakeToken token);
    std::optional<WakeToken> receive(std::uint32_t message);
    bool occupied() const;
private:
    mutable std::mutex mutex_;
    std::optional<WakeToken> queued_;
};

// Protocol for the future game-queue adapter; never sends VI/AI or changes guest RAM.
// No thread/callback runs under this object's lock. A runtime sender must carry
// the immutable WakeToken through enqueue/receive; a pointer to mutable pending
// state cannot distinguish an old queued wake from a new scene's wake.
class FrameScheduler {
public:
    // Caller must establish a legacy tick boundary and drain ORIGINAL tasks too.
    // Our bank ledger can only prove completion of tasks that we issued.
    bool enter(unsigned fps, Nanoseconds now, unsigned next_bank, bool original_boundary_drained);
    void change_rate(unsigned fps, Nanoseconds now);
    void leave(Nanoseconds now); // Cancel new wakes; keep outstanding task ownership.
    // ONLY after guest threads stop and the runtime host-read reset barrier.
    // Normal mode/scene changes must use leave, which retains owned banks.
    void reset_after_runtime_barrier(Nanoseconds now);
    std::optional<WakeToken> request(Nanoseconds now);
    bool reject_enqueue(WakeToken token); // Only on an actual failed enqueue.
    std::optional<FrameTicket> receive(WakeToken token, Nanoseconds now);
    bool publish(const FrameTicket& ticket);
    bool complete(const FrameTicket& ticket, Completion completion);
    std::optional<Nanoseconds> deadline() const;
    bool active() const;
    bool drained() const;
    SchedulerStats stats() const;
private:
    enum class Phase { Free, Building, Published };
    struct Bank {
        Phase phase = Phase::Free;
        WakeToken token{};
        bool guest_done = false, host_done = false;
    };
    mutable std::mutex mutex_;
    ManualClock clock_;
    std::array<Bank, 2> banks_{};
    std::optional<WakeToken> pending_;
    std::uint64_t epoch_ = 0, serial_ = 0;
    unsigned next_bank_ = 0;
    SchedulerStats stats_{};
    bool drained_locked() const;
    void invalidate_locked();
};
} // namespace dora3::native_time
