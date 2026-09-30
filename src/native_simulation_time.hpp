#pragma once
#include "native_frame_time.hpp"
#include <cstdint>
#include <functional>

namespace dora3::native_time {
struct SimulationSlice {
    // Exact rational original-tick units; denominator = divider * 1e9.
    std::uint64_t numerator, denominator, tick, phase;
    double ticks() const;
    double seconds() const;
    bool ends_tick() const { return phase + numerator == denominator; }
};

// The continuous callback performs real state updates. The boundary callback
// performs discrete commands once after each completed original game tick.
// No frames are interpolated or predicted, and no elapsed time is discarded.
// The caller establishes initial discrete state before the first advance.
class SimulationTimeline {
public:
    explicit SimulationTimeline(unsigned divider);
    void advance(Nanoseconds elapsed,
                 const std::function<void(const SimulationSlice&)>& continuous,
                 const std::function<void(std::uint64_t)>& boundary);
    std::uint64_t ticks() const { return tick_; }
    std::uint64_t phase() const { return phase_; }
    unsigned divider() const { return divider_; }
private:
    unsigned divider_;
    std::uint64_t denominator_, tick_ = 0, phase_ = 0;
    bool advancing_ = false;
};

// Explicitly entered by a reviewed continuous update closure. Outside it every
// patched instruction retains its original operand. Rendering does not create
// a simulation scope implicitly; activating a scene requires its whole contract.
class SimulationScope {
public:
    explicit SimulationScope(const SimulationSlice&);
    ~SimulationScope();
    SimulationScope(const SimulationScope&) = delete;
private:
    SimulationSlice slice_;
    const SimulationSlice* previous_;
};
const SimulationSlice* active_simulation_slice();
double simulation_ticks();

// These quantities are expressed per original scene tick, NOT per rendered frame.
// For an unclamped constant acceleration this is the continuous extension of
// the original kick-then-drift recurrence v += a; x += v. At integer tick times
// it agrees with that recurrence, independently of the rendering partition.
struct KinematicStep { double velocity, displacement; };
KinematicStep legacy_kinematic_step(double velocity, double acceleration, double ticks);
double approach_value(double current, double target, double speed, double ticks);
double decay_value(double current, double target, double retention, double ticks);
} // namespace dora3::native_time

extern "C" float dora3_native_rate_f32(float);
extern "C" double dora3_native_rate_f64(double);

extern "C" int dora3_native_simulation_active();
extern "C" int dora3_native_simulation_tick_start();
extern "C" float dora3_native_displacement_f32(float, std::uint32_t);
