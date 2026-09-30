#include "native_simulation_time.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace dora3::native_time {
namespace {
thread_local const SimulationSlice* active = nullptr;
void valid_ticks(double ticks) {
    if (!std::isfinite(ticks) || ticks < 0 || ticks > 1)
        throw std::invalid_argument("Simulation step must stay inside one scene tick");
}
}
double SimulationSlice::ticks() const { return double(numerator) / double(denominator); }
double SimulationSlice::seconds() const { return double(numerator) / (60.0 * double(second)); }
SimulationTimeline::SimulationTimeline(unsigned divider):divider_(divider),denominator_(std::uint64_t(divider)*second) {
    if (divider != 2 && divider != 3) throw std::invalid_argument("Unreviewed scene tick divider");
}
void SimulationTimeline::advance(Nanoseconds elapsed,
        const std::function<void(const SimulationSlice&)>& continuous,
        const std::function<void(std::uint64_t)>& boundary) {
    if (advancing_ || elapsed < 0 || !continuous || !boundary)
        throw std::invalid_argument("Invalid/reentrant simulation advance");
    if (std::uint64_t(elapsed) > std::numeric_limits<std::uint64_t>::max()/60)
        throw std::overflow_error("Simulation elapsed overflow");
    std::uint64_t remaining=std::uint64_t(elapsed)*60;
    if (remaining/denominator_ + 1 > std::numeric_limits<std::uint64_t>::max()-tick_)
        throw std::overflow_error("Simulation tick overflow");
    advancing_=true;
    try {
        while (remaining) {
            const auto amount=std::min(remaining,denominator_-phase_);
            const SimulationSlice slice{amount,denominator_,tick_,phase_};
            continuous(slice);
            phase_+=amount;remaining-=amount;
            if (phase_==denominator_) {
                phase_=0;++tick_;
                boundary(tick_);
            }
        }
    } catch (...) { advancing_=false;throw; }
    advancing_=false;
}
SimulationScope::SimulationScope(const SimulationSlice& slice):slice_(slice),previous_(active) {
    if (active || (slice.denominator != 2*std::uint64_t(second) && slice.denominator != 3*std::uint64_t(second)) ||
        !slice.numerator || slice.numerator > slice.denominator || slice.phase >= slice.denominator ||
        slice.numerator > slice.denominator-slice.phase)
        throw std::invalid_argument("Invalid or nested continuous simulation scope");
    active=&slice_;
}
SimulationScope::~SimulationScope() { active=previous_; }
const SimulationSlice* active_simulation_slice() { return active; }
double simulation_ticks() { return active ? active->ticks() : 1.0; }
KinematicStep legacy_kinematic_step(double velocity,double acceleration,double ticks) {
    valid_ticks(ticks);
    if (!std::isfinite(velocity) || !std::isfinite(acceleration))
        throw std::invalid_argument("Nonfinite kinematic input");
    const auto next=velocity+acceleration*ticks;
    return {next,next*ticks+0.5*acceleration*ticks*(1.0-ticks)};
}
double approach_value(double current,double target,double speed,double ticks) {
    valid_ticks(ticks);
    if (!std::isfinite(current) || !std::isfinite(target) || !std::isfinite(speed) || speed<0)
        throw std::invalid_argument("Invalid approach input");
    const double delta=target-current, amount=speed*ticks;
    if (std::abs(delta)<=amount)return target;
    return current+std::copysign(amount,delta);
}
double decay_value(double current,double target,double retention,double ticks) {
    valid_ticks(ticks);
    if (!std::isfinite(current) || !std::isfinite(target) || !std::isfinite(retention) || retention<0 || retention>1)
        throw std::invalid_argument("Invalid decay input");
    if (!ticks)return current;
    return target+(current-target)*std::pow(retention,ticks);
}
}
extern "C" float dora3_native_rate_f32(float value) {
    const auto* step=dora3::native_time::active_simulation_slice();
    return step ? float(double(value)*step->ticks()) : value;
}
extern "C" double dora3_native_rate_f64(double value) {
    const auto* step=dora3::native_time::active_simulation_slice();
    return step ? value*step->ticks() : value;
}

extern "C" int dora3_native_simulation_active() { return dora3::native_time::active_simulation_slice()!=nullptr; }
extern "C" int dora3_native_simulation_tick_start() {
    const auto* step=dora3::native_time::active_simulation_slice();
    return !step || step->phase==0;
}
extern "C" float dora3_native_displacement_f32(float velocity,std::uint32_t acceleration_bits) {
    const auto* step=dora3::native_time::active_simulation_slice();
    if (!step)return velocity;
    const double h=step->ticks(),a=std::bit_cast<float>(acceleration_bits);
    return float(double(velocity)*h+0.5*a*h*(1.0-h));
}
