#include "native_frame_time.hpp"
#include "recomp.h"
#include "native-time-functions.h"
#include <cassert>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

using namespace dora3::native_time;
extern "C" void dora3_trace(uint8_t*, recomp_context*, uint32_t, uint32_t) {}

static Nanoseconds frame_time(std::uint64_t frame, unsigned fps) {
    return (frame * second + fps - 1) / fps;
}

static void check_clock() {
    for (unsigned fps : {60u, 61u, 73u, 90u, 120u, 144u, 165u, 240u, 359u, 360u}) {
        ManualClock clock;
        clock.configure(Mode::Manual, fps, 0);
        SceneClock twenty(3), thirty(2);
        Nanoseconds elapsed = 0;
        for (unsigned i = 1; i <= fps * 60; ++i) {
            const auto at = frame_time(i, fps);
            assert(clock.deadline() == at);
            assert(!clock.take(at - 1));
            auto frame = clock.take(at);
            assert(frame && frame->target_fps == fps);
            elapsed += frame->elapsed;
            twenty.advance(frame->elapsed);
            thirty.advance(frame->elapsed);
            assert(!clock.take(at));
        }
        assert(elapsed == 60 * second);
        assert(twenty.progress().whole == 1200 && twenty.progress().numerator == 0);
        assert(thirty.progress().whole == 1800 && thirty.progress().numerator == 0);
    }
    ManualClock c;
    for (auto mode : {Mode::Original, Mode::Display}) {
        c.configure(mode, 144, 0);
        assert(!c.active() && !c.deadline() && !c.take(10 * second));
    }
    c.configure(Mode::Manual, 60, 0);
    assert(c.take(20'000'000)->elapsed == 20'000'000);
    c.configure(Mode::Manual, 144, 25'000'000);
    auto changed = c.take(*c.deadline());
    assert(changed && changed->elapsed == 11'944'445); // 5 ms before reconfiguration retained.
    assert(c.take(second)->elapsed == second - 31'944'445); // Stalls do not discard game time.
    assert(!c.take(second)); // No burst of stale display frames.
    c.resume(10 * second);
    assert(c.take(*c.deadline())->elapsed == 6'944'445); // Explicit suspension is excluded.
    c.configure(Mode::Original, 144, 11 * second);
    assert(!c.take(12 * second) && !c.deadline());
    c.configure(Mode::Manual, 73, 12 * second);
    assert(c.take(*c.deadline())->elapsed == frame_time(1, 73));
    c.resume(13 * second);
    assert(!c.take(12 * second)); // Backwards source clock rebases, never emits negative dt.
    SceneClock long_run(3);
    const auto p = long_run.advance(99 * 3600 * second);
    assert(p.whole == 99 * 3600 * 20 && p.numerator == 0);
}

static FadeState original_fade(uint8_t* rdram, recomp_context& c, FadeState state) {
    MEM_H(0, int32_t(0x801AA660)) = state.alpha;
    MEM_B(0, int32_t(0x801A99A0)) = state.direction;
    MEM_B(0, int32_t(0x80171580)) = state.step;
    dora3_fade_tick(rdram, &c);
    return {MEM_HU(0, int32_t(0x801AA660)), MEM_B(0, int32_t(0x801A99A0)), state.step};
}

static void check_fades(uint8_t* rdram, recomp_context& c) {
    // Compare against the actual generated ROM body, not a second copy of our formula.
    for (unsigned divider : {2u, 3u}) {
        for (int direction : {-2, -1, 0, 1, 2})
            for (int alpha = 0; alpha <= 255; ++alpha)
                for (unsigned step = 0; step <= 255; ++step) {
                    FadeState start{alpha, direction, step};
                    Fade fade(start, divider);
                    assert(fade.advance(0) == start);
                    assert(fade.advance(frame_time(divider, 60)) == original_fade(rdram, c, start));
                }
        for (unsigned fps : {60u, 73u, 144u, 165u, 360u}) {
            for (unsigned step : {0u, 1u, 7u, 16u, 96u, 255u}) {
                for (int direction : {-1, 1}) {
                    FadeState start{direction == 1 ? 0 : 255, direction, step};
                    Fade fade(start, divider);
                    FadeState reference = start;
                    std::uint64_t ticks = 0;
                    Nanoseconds previous = 0;
                    int distinct = 0, previous_alpha = start.alpha;
                    for (unsigned i = 1; i <= fps * 2; ++i) {
                        const auto now = frame_time(i, fps);
                        const auto current = fade.advance(now - previous);
                        const auto due = std::uint64_t(now) * 60 / (divider * second);
                        while (ticks < due) { reference = original_fade(rdram, c, reference); ++ticks; }
                        assert(current.direction == reference.direction);
                        assert(current.alpha >= 0 && current.alpha <= 255);
                        if (std::uint64_t(now) * 60 % (divider * second) == 0)
                            assert(current == reference);
                        if (current.alpha != previous_alpha) ++distinct;
                        previous_alpha = current.alpha;
                        previous = now;
                    }
                    if (step == 16 && fps == 60 && divider == 3) assert(distinct > 16);
                }
            }
        }
    }
    // New identical command must restart timing; no detection by parameter equality.
    Fade first({0, 1, 16}, 3);
    assert(first.advance(100'000'000).alpha == 32);
    Fade restarted({0, 1, 16}, 3);
    assert(restarted.advance(25'000'000).alpha == 8);
}

static void check_animation(uint8_t* rdram, recomp_context& c) {
    const auto object = int32_t(0x80100000), animation = int32_t(0x80101000);
    MEM_W(0x18, object) = animation;
    MEM_W(0x60, object) = 1;
    MEM_W(0, int32_t(0x80082724)) = object;
    const auto put_float = [&](int offset, int32_t address, float value) {
        MEM_W(offset, address) = std::bit_cast<int32_t>(value);
    };
    const auto get_float = [&](int offset, int32_t address) {
        return std::bit_cast<float>(MEM_W(offset, address));
    };
    put_float(4, animation, 10.0f);
    const auto call = [&](float delta) {
        c.r4 = object;
        uint32_t bits;
        std::memcpy(&bits, &delta, sizeof(bits));
        c.r5 = int32_t(bits);
        dora3_s0_8003948C(rdram, &c);
    };
    MEM_B(0x7c, object) = 0;
    put_float(0x64, object, 2);
    call(0.25f);
    assert(get_float(0x64, object) == 2.25f);
    MEM_B(0x7c, object) = 8;
    call(0.25f);
    assert(get_float(0x64, object) == 2.25f); // Paused animation does not advance.
    MEM_B(0x7c, object) = 0;
    put_float(0x64, object, 9.75f);
    call(0.5f);
    assert(get_float(0x64, object) == 0.25f && (MEM_BU(0x7c, object) & 2));
    call(0.25f);
    assert(!(MEM_BU(0x7c, object) & 2)); // Completion is a pulse, not a sticky event.
    MEM_B(0x7c, object) = 1;
    put_float(0x64, object, 9.75f);
    call(0.5f);
    assert(get_float(0x64, object) == 10 && (MEM_BU(0x7c, object) & 2));
    MEM_B(0x7c, object) = 0;
    put_float(0x64, object, 0.25f);
    call(-0.5f);
    assert(get_float(0x64, object) == 9.75f && (MEM_BU(0x7c, object) & 2));
    // A stall cannot be forwarded as one oversized delta: original wraps only once.
    put_float(0x64, object, 0);
    call(25.0f);
    assert(get_float(0x64, object) == 15.0f);

    // Native version agrees on original-sized steps within the verified domain.
    for (unsigned divider : {2u, 3u})
        for (unsigned flags : {0u, 1u, 2u, 3u, 8u, 10u})
            for (bool enabled : {false, true})
                for (float initial : {0.0f, 0.25f, 9.0f, 9.75f})
                    for (float speed : {-0.5f, 0.0f, 0.25f, 1.0f}) {
                        Animation native({initial, 10, speed, uint8_t(flags), enabled}, divider);
                        MEM_W(0x60, object) = enabled;
                        MEM_B(0x7c, object) = flags;
                        put_float(0x64, object, initial);
                        call(speed);
                        const auto state = native.advance(frame_time(divider, 60));
                        assert(std::abs(state.time - get_float(0x64, object)) < 1e-5f);
                        assert(state.flags == MEM_BU(0x7c, object));
                    }
    // Same clip time after the same elapsed time at unrelated display rates.
    for (unsigned fps : {60u, 73u, 144u, 165u, 360u}) {
        Animation native({0, 10, 1, 0, true}, 3);
        Nanoseconds before = 0;
        AnimationState state{};
        for (unsigned i = 1; i <= fps * 2; ++i) {
            const auto at = frame_time(i, fps);
            state = native.advance(at - before);
            before = at;
        }
        assert(state.time == 0 && native.consume_completions() == 4);
        assert(native.consume_completions() == 0);
    }
    Animation crossing({9.75f, 10, 1, 0, true}, 3);
    assert(crossing.advance(25'000'000).flags & 2);
    assert(!(crossing.advance(25'000'000).flags & 2));
    assert(crossing.consume_completions() == 1); // Survives later native updates.
    Animation stalled({0, 10, 1, 0, true}, 3);
    assert(stalled.advance(1'250'000'000).time == 5);
    assert(stalled.consume_completions() == 2); // All crossed loops retained.
    Animation reverse({0.25f, 10, -1, 0, true}, 3);
    assert(reverse.advance(1'250'000'000).time == 5.25f);
    assert(reverse.consume_completions() == 3);
    Animation clamped({9.75f, 10, 1, 1, true}, 3);
    assert(clamped.advance(second).time == 10);
    assert(clamped.consume_completions() == 1);
    assert(clamped.advance(second).flags & 2);
    assert(clamped.consume_completions() == 0);
}

int main() {
    check_clock();
    std::vector<uint32_t> memory(0x400000 / 4);
    auto* rdram = reinterpret_cast<uint8_t*>(memory.data());
    recomp_context context{};
    context.r29 = int32_t(0x803F0000);
    check_fades(rdram, context);
    check_animation(rdram, context);
    std::cout << "PASS: Manual 60..360 deadlines, Original/Display bypass, 20/30 Hz time conservation, mode changes, stalls/resume; 655360 original fade comparisons and fractional fades; 384 original animation comparisons, rate-independent native clip time, pause, clamp, multiple wraps and completion retention. No game/window/GPU.\n";
}
