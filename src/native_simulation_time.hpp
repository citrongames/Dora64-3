#pragma once
#include "native_frame_time.hpp"
#include <cstdint>
#include <functional>
#include <vector>

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
    // Stop catch-up immediately when an update replaces its scene or mode.
    // The owner must keep this timeline alive until the call returns, even if
    // the callback resets the current session. Unconsumed old-scene time must
    // never run callbacks against the replacement scene's allocations.
    // Optional event limit splits a slice in the same rational tick units.
    // It must return 1..slice.numerator and must not advance any subsystem.
    bool advance_while(Nanoseconds elapsed,
                 const std::function<void(const SimulationSlice&)>& continuous,
                 const std::function<void(std::uint64_t)>& boundary,
                 const std::function<bool()>& current,
                 const std::function<std::uint64_t(const SimulationSlice&)>& limit = {});
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
struct GravitySample { std::uint32_t address; float velocity, displacement; };
class SimulationScope {
public:
    explicit SimulationScope(const SimulationSlice&);
    ~SimulationScope();
    SimulationScope(const SimulationScope&) = delete;
private:
    std::vector<GravitySample> gravity_,drive_;
    SimulationSlice slice_;
    const SimulationSlice* previous_;
};
// Explicit subsystem boundary: a camera evaluates cumulative tick time while
// its caller retains the current movement slice and paired displacement cache.
class SimulationSuspension {
public:
    SimulationSuspension();
    ~SimulationSuspension();
    SimulationSuspension(const SimulationSuspension&)=delete;
private:
    const SimulationSlice* slice_;
    std::vector<GravitySample>* gravity_;
    std::vector<GravitySample>* drive_;
    std::uint64_t serial_;
    const SimulationSlice* command_slice_;
    std::uint64_t command_serial_;
};
const SimulationSlice* active_simulation_slice();
// Read-only animation-command ownership, including a suspended Source callback.
// This is an actor-local interval, not the world timestamp for sound/fade commands.
const SimulationSlice* command_simulation_slice();
// Resource replacement revokes the current scope before guest code resumes.
// Its RAII owner still performs cleanup; a new scene cannot inherit old dt.
void cancel_active_simulation();
double simulation_ticks();

// These quantities are expressed per original scene tick, NOT per rendered frame.
// For an unclamped constant acceleration this is the continuous extension of
// the original kick-then-drift recurrence v += a; x += v. At integer tick times
// it agrees with that recurrence, independently of the rendering partition.
struct KinematicStep { double velocity, displacement; };
// A reviewed owner may calculate velocity from its exact Source baseline.
// Publish its paired displacement to the same live collision interval.
void publish_simulation_motion(std::uint32_t address,const KinematicStep&);
void publish_simulation_drive(std::uint32_t address,const KinematicStep&);
KinematicStep legacy_kinematic_step(double velocity, double acceleration, double ticks);
double approach_value(double current, double target, double speed, double ticks);
double decay_value(double current, double target, double retention, double ticks);
} // namespace dora3::native_time

extern "C" float dora3_native_rate_f32(float);
extern "C" double dora3_native_rate_f64(double);

extern "C" int dora3_native_simulation_active();
extern "C" int dora3_native_simulation_tick_start();
extern "C" float dora3_native_displacement_f32(float, std::uint32_t);
extern "C" float dora3_native_angular_displacement_f32(float, std::uint32_t);

extern "C" float dora3_native_gravity_f32(float, float, float, std::uint32_t);
extern "C" float dora3_native_gravity_displacement_f32(float, std::uint32_t);

extern "C" double dora3_native_approach_coefficient_f64(double);
extern "C" double dora3_native_retention_f64(double);
// Value at the containing source-tick endpoint, after a partial approach.
// Reviewed discrete thresholds use this value while the visible value keeps
// advancing continuously; deletion still commits at the endpoint itself.
extern "C" double dora3_native_tick_end_approach_f64(double,double,double);
extern "C" int dora3_native_simulation_tick_end();

extern "C" double dora3_native_capped_approach_f64(double,double,double);

extern "C" float dora3_native_drive_f32(float,float,float,float,float,std::uint32_t);
extern "C" void dora3_native_project_drive_f32(float,float,std::uint32_t,std::uint32_t,std::uint32_t);
