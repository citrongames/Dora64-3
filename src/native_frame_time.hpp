#pragma once
#include <cstdint>
#include <optional>

namespace dora3::native_time {
using Nanoseconds = std::int64_t;
inline constexpr Nanoseconds second = 1'000'000'000;
enum class Mode { Original, Display, Manual };

struct FrameStep {
    Nanoseconds elapsed;
    unsigned target_fps;
    // Units of the original scene tick, not a universal 20 Hz assumption.
    double scene_ticks(unsigned retrace_divider) const;
};

// Owned by the game thread. Call take() only when a graphics bank is available.
// This clock never changes VI/AI, emits guest messages, or accesses guest RAM.
class ManualClock {
public:
    void configure(Mode mode, unsigned fps, Nanoseconds now);
    void resume(Nanoseconds now); // Explicit suspension; no implicit elapsed-time cap.
    std::optional<FrameStep> take(Nanoseconds now);
    std::optional<Nanoseconds> deadline() const;
    bool active() const { return fps_ != 0; }
private:
    unsigned fps_ = 0;
    Nanoseconds origin_ = 0, previous_ = 0, next_ = 0;
    std::uint64_t next_index_ = 1;
    void schedule_after(Nanoseconds now);
};

struct TickProgress {
    std::uint64_t whole;
    std::uint64_t numerator, denominator;
    double fractional() const;
};

// Scene-owned elapsed time. Reset explicitly when a scene/clock domain changes.
// Carries integer remainders: Manual 73/144/165 must not lose legacy events.
class SceneClock {
public:
    explicit SceneClock(unsigned retrace_divider);
    TickProgress advance(Nanoseconds elapsed);
    TickProgress progress() const;
private:
    std::uint64_t whole_ = 0, remainder_ = 0, denominator_;
};

struct FadeState {
    int alpha;
    int direction; // -1/+1 running; -2/+2 complete; 0 inactive.
    unsigned step;
    bool operator==(const FadeState&) const = default;
};

// Time-based version of the verified resident fade (80028948).
// Original tick boundaries and completion time match the original routine.
// Restart on every guest fade command, even if parameters repeat.
class Fade {
public:
    Fade(FadeState initial, unsigned retrace_divider);
    FadeState advance(Nanoseconds elapsed);
private:
    FadeState initial_;
    SceneClock time_;
};

struct AnimationState {
    float time;
    float duration;
    float increment; // Original units per scene tick; may be negative.
    std::uint8_t flags; // +0x7C: bit0 forward clamp, bit1 end pulse, bit3 pause.
    bool enabled; // Original +0x60 gate.
};

// Explicit per-clip lifetime: recreate on clip change, seek, rate change or reuse.
// Do not key this state by a raw recycled guest pointer without an owner generation.
class Animation {
public:
    Animation(AnimationState initial, unsigned retrace_divider);
    AnimationState advance(Nanoseconds elapsed);
    std::uint64_t consume_completions();
private:
    AnimationState initial_, current_;
    SceneClock time_;
    std::uint64_t crossings_ = 0, pending_ = 0;
};
} // namespace dora3::native_time
