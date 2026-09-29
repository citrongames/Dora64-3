#include "native_frame_time.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace dora3::native_time {
namespace {
void check_divider(unsigned divider) {
    if (divider != 2 && divider != 3)
        throw std::invalid_argument("Unverified Dora3 scene retrace divider");
}
Nanoseconds frame_offset(std::uint64_t index, unsigned fps) {
    // Avoid index * second overflow; round up so a frame is never due early.
    const auto seconds = index / fps;
    if (seconds > std::uint64_t(std::numeric_limits<Nanoseconds>::max() / second - 1))
        throw std::overflow_error("Native frame clock range");
    return Nanoseconds(seconds) * second + Nanoseconds(((index % fps) * second + fps - 1) / fps);
}
}

double FrameStep::scene_ticks(unsigned divider) const {
    check_divider(divider);
    return double(elapsed) * 60.0 / (double(second) * divider);
}

void ManualClock::configure(Mode mode, unsigned fps, Nanoseconds now) {
    if (mode != Mode::Manual) fps = 0;
    if (mode == Mode::Manual && (fps < 60 || fps > 360))
        throw std::invalid_argument("Manual FPS outside the existing slider range");
    if (now < 0) throw std::invalid_argument("Negative monotonic time");
    if (fps == fps_) return;
    const bool was_active = active();
    fps_ = fps;
    origin_ = now;
    next_index_ = 1;
    if (!was_active || !active()) previous_ = now;
    // Changing the slider changes deadlines, not accumulated simulation time.
    next_ = active() ? now + frame_offset(1, fps_) : now;
}

void ManualClock::resume(Nanoseconds now) {
    if (now < 0) throw std::invalid_argument("Negative monotonic time");
    origin_ = previous_ = now;
    next_index_ = 1;
    next_ = active() ? now + frame_offset(1, fps_) : now;
}

void ManualClock::schedule_after(Nanoseconds now) {
    const auto elapsed = std::uint64_t(now - origin_);
    next_index_ = (elapsed / second) * fps_ + ((elapsed % second) * fps_) / second + 1;
    next_ = origin_ + frame_offset(next_index_, fps_);
}

std::optional<FrameStep> ManualClock::take(Nanoseconds now) {
    if (!active()) return std::nullopt; // Original remains outside the native clock.
    if (now < previous_) { resume(now); return std::nullopt; }
    if (now < next_) return std::nullopt;
    FrameStep step{now - previous_, fps_};
    previous_ = now;
    schedule_after(now); // Coalesce missed presentation deadlines; retain elapsed time.
    return step;
}

std::optional<Nanoseconds> ManualClock::deadline() const {
    if (!active()) return std::nullopt;
    return next_;
}

double TickProgress::fractional() const { return double(numerator) / double(denominator); }

SceneClock::SceneClock(unsigned divider) : denominator_(std::uint64_t(divider) * second) {
    check_divider(divider);
}

TickProgress SceneClock::advance(Nanoseconds elapsed) {
    if (elapsed < 0) throw std::invalid_argument("Negative simulation interval");
    // Splitting first keeps even very long suspension intervals out of ns*60 overflow.
    const auto ns = std::uint64_t(elapsed);
    whole_ += (ns / denominator_) * 60;
    remainder_ += (ns % denominator_) * 60;
    whole_ += remainder_ / denominator_;
    remainder_ %= denominator_;
    return progress();
}

TickProgress SceneClock::progress() const { return {whole_, remainder_, denominator_}; }

Fade::Fade(FadeState initial, unsigned divider) : initial_(initial), time_(divider) {
    if (initial.alpha < 0 || initial.alpha > 255 || initial.step > 255 ||
        initial.direction < -2 || initial.direction > 2)
        throw std::invalid_argument("Invalid Dora3 fade state");
}

FadeState Fade::advance(Nanoseconds elapsed) {
    const auto time = time_.advance(elapsed);
    auto state = initial_;
    if (state.direction != -1 && state.direction != 1) return state;
    const unsigned distance = state.direction > 0 ? 255 - state.alpha : state.alpha;
    if (state.step == 0 && distance != 0) return state;
    const unsigned ticks = state.step == 0 ? 1 : std::max(1u, (distance + state.step - 1) / state.step);
    if (time.whole >= ticks) {
        state.alpha = state.direction > 0 ? 255 : 0;
        state.direction *= 2;
        return state;
    }
    const unsigned consumed = std::min<std::uint64_t>(distance, time.whole * state.step);
    const unsigned partial_step = std::min(state.step, distance - consumed);
    const auto partial = (time.numerator * partial_step) / time.denominator;
    state.alpha += state.direction * int(consumed + partial);
    return state;
}

FadeTimeline::FadeTimeline(FadeState initial, unsigned divider, Nanoseconds now)
    : fade_(initial, divider), last_(initial), divider_(divider), previous_(now) {
    if (now < 0) throw std::invalid_argument("Negative fade origin");
}
FadeState FadeTimeline::sample(FadeState observed, Nanoseconds now) {
    if (now < previous_) throw std::invalid_argument("Fade clock moved backwards");
    if (observed != last_) {
        if (!pending_command_) throw std::logic_error("Untracked write to native fade");
        fade_ = Fade(observed, divider_);
    }
    pending_command_ = false;
    last_ = fade_.advance(now - previous_);
    previous_ = now;
    return last_;
}
FadeState FadeTimeline::before_command(FadeState observed, Nanoseconds now) {
    const auto value = sample(observed, now);
    pending_command_ = true;
    return value;
}

Animation::Animation(AnimationState initial, unsigned divider)
    : initial_(initial), current_(initial), time_(divider) {
    if (!std::isfinite(initial.time) || !std::isfinite(initial.duration) ||
        !std::isfinite(initial.increment) || initial.duration <= 0 ||
        initial.time < 0 || initial.time > initial.duration)
        throw std::invalid_argument("Invalid Dora3 animation state");
}

AnimationState Animation::advance(Nanoseconds elapsed) {
    if (elapsed < 0) throw std::invalid_argument("Negative animation interval");
    if (elapsed == 0 || !initial_.enabled || (initial_.flags & 8)) return current_;
    const auto time = time_.advance(elapsed);
    const double units = double(time.whole) + time.fractional();
    const double position = double(initial_.time) + units * initial_.increment;
    const double duration = initial_.duration;
    double value = position;
    double cycles = 0;
    bool finished = false;
    if (position < 0) {
        cycles = std::ceil(-position / duration);
        value = position + cycles * duration;
    }
    else if (position >= duration) {
        if (initial_.flags & 1) {
            value = duration;
            cycles = 1;
            finished = true;
        }
        else {
            cycles = std::floor(position / duration);
            value = position - cycles * duration;
        }
    }
    if (!std::isfinite(cycles) || cycles >= double(std::numeric_limits<std::uint64_t>::max()))
        throw std::overflow_error("Animation completion counter range");
    const auto count = std::uint64_t(cycles);
    const auto new_completions = count - crossings_;
    pending_ += new_completions;
    crossings_ = count;
    current_.time = float(value);
    current_.flags = (initial_.flags & ~2u) | ((finished || new_completions != 0) ? 2u : 0u);
    return current_;
}

std::uint64_t Animation::consume_completions() {
    const auto result = pending_;
    pending_ = 0;
    return result;
}
} // namespace dora3::native_time
