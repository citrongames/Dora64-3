#include "native_simulation_time.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace dora3::native_time {
namespace {
thread_local const SimulationSlice* active = nullptr;
thread_local uint64_t cancellation_serial = 0, active_serial = 0;
thread_local std::vector<GravitySample>* active_gravity = nullptr;
thread_local std::vector<GravitySample>* active_drive = nullptr;
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
    advance_while(elapsed,continuous,boundary,[]{return true;});
}
bool SimulationTimeline::advance_while(Nanoseconds elapsed,
        const std::function<void(const SimulationSlice&)>& continuous,
        const std::function<void(std::uint64_t)>& boundary,
        const std::function<bool()>& current,
        const std::function<std::uint64_t(const SimulationSlice&)>& limit) {
    if (advancing_ || elapsed < 0 || !continuous || !boundary || !current)
        throw std::invalid_argument("Invalid/reentrant simulation advance");
    if (std::uint64_t(elapsed) > std::numeric_limits<std::uint64_t>::max()/60)
        throw std::overflow_error("Simulation elapsed overflow");
    std::uint64_t remaining=std::uint64_t(elapsed)*60;
    if (remaining/denominator_ + 1 > std::numeric_limits<std::uint64_t>::max()-tick_)
        throw std::overflow_error("Simulation tick overflow");
    advancing_=true;
    try {
        while (remaining) {
            if(!current()) {advancing_=false;return false;}
            auto amount=std::min(remaining,denominator_-phase_);
            if(limit) {
                const auto bounded=limit({amount,denominator_,tick_,phase_});
                if(!bounded || bounded>amount)throw std::logic_error("Invalid continuous event boundary");
                amount=bounded;
                if(!current()) {advancing_=false;return false;}
            }
            const SimulationSlice slice{amount,denominator_,tick_,phase_};
            continuous(slice);
            phase_+=amount;remaining-=amount;
            // A callback may have drained the session and loaded new resources.
            // Do not invoke even the old tick-boundary callback after revocation.
            if(!current()) {advancing_=false;return false;}
            if (phase_==denominator_) {
                phase_=0;++tick_;
                boundary(tick_);
            }
        }
    } catch (...) { advancing_=false;throw; }
    advancing_=false;
    return current();
}
SimulationScope::SimulationScope(const SimulationSlice& slice):slice_(slice),previous_(active) {
    if (active || (slice.denominator != 2*std::uint64_t(second) && slice.denominator != 3*std::uint64_t(second)) ||
        !slice.numerator || slice.numerator > slice.denominator || slice.phase >= slice.denominator ||
        slice.numerator > slice.denominator-slice.phase)
        throw std::invalid_argument("Invalid or nested continuous simulation scope");
    active=&slice_;active_serial=cancellation_serial;
    active_gravity=&gravity_;active_drive=&drive_;
}
SimulationScope::~SimulationScope() { active=previous_;active_gravity=nullptr;active_drive=nullptr; }
SimulationSuspension::SimulationSuspension():slice_(active),gravity_(active_gravity),drive_(active_drive),serial_(active_serial) {
    if(!active_simulation_slice())throw std::logic_error("Subsystem suspension outside simulation");
    active=nullptr;active_gravity=nullptr;active_drive=nullptr;
}
SimulationSuspension::~SimulationSuspension() {
    active=slice_;active_serial=serial_;active_gravity=gravity_;active_drive=drive_;
}
const SimulationSlice* active_simulation_slice() { return active_serial==cancellation_serial ? active : nullptr; }
void cancel_active_simulation() { if(active)++cancellation_serial; }
double simulation_ticks() { const auto* slice=active_simulation_slice();return slice ? slice->ticks() : 1.0; }
void publish_simulation_motion(std::uint32_t address,const KinematicStep& step) {
    if(!active_simulation_slice() || !active_gravity)
        throw std::logic_error("Motion publication outside simulation");
    if(!std::isfinite(step.velocity) || !std::isfinite(step.displacement))
        throw std::invalid_argument("Nonfinite motion publication");
    const GravitySample sample{address,float(step.velocity),float(step.displacement)};
    auto& samples=*active_gravity;
    const auto i=std::find_if(samples.begin(),samples.end(),[&](const auto& x){return x.address==address;});
    if(i==samples.end())samples.push_back(sample);else *i=sample;
}
void publish_simulation_drive(std::uint32_t address,const KinematicStep& step) {
    if(!active_simulation_slice() || !active_drive)
        throw std::logic_error("Drive publication outside simulation");
    if(!std::isfinite(step.velocity) || !std::isfinite(step.displacement))
        throw std::invalid_argument("Nonfinite drive publication");
    const GravitySample sample{address,float(step.velocity),float(step.displacement)};
    auto& samples=*active_drive;
    const auto i=std::find_if(samples.begin(),samples.end(),[&](const auto& x){return x.address==address;});
    if(i==samples.end())samples.push_back(sample);else *i=sample;
}
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

extern "C" float dora3_native_gravity_f32(float velocity,float gravity,float terminal,std::uint32_t address) {
    using namespace dora3::native_time;
    const auto* step=active_simulation_slice();
    if(!step)return velocity-gravity;
    if(!std::isfinite(velocity)||!std::isfinite(gravity)||!std::isfinite(terminal))
        throw std::invalid_argument("Nonfinite native gravity");
    const double h=step->ticks(),a=-double(gravity),v=std::max(double(velocity),double(terminal));
    const float next=float(std::max(double(terminal),v+a*h));
    // Continuous kick/drift extension: x' = max(terminal, v + a/2).
    // Integrate the linear part only until the terminal drift is reached.
    // This is partition independent, including the crossing of the speed cap.
    const double start=v+0.5*a;
    double displacement;
    if(a==0)displacement=std::max(start,double(terminal))*h;
    else {
        const double crossing=std::clamp((double(terminal)-start)/a,0.0,h);
        if(a<0)displacement=start*crossing+0.5*a*crossing*crossing+double(terminal)*(h-crossing);
        else displacement=double(terminal)*crossing+start*(h-crossing)+0.5*a*(h*h-crossing*crossing);
    }
    auto& samples=*active_gravity;
    const GravitySample sample{address,next,float(displacement)};
    const auto it=std::find_if(samples.begin(),samples.end(),[&](const auto& x){return x.address==address;});
    if(it==samples.end())samples.push_back(sample);else *it=sample;
    return next;
}
extern "C" float dora3_native_gravity_displacement_f32(float velocity,std::uint32_t address) {
    using namespace dora3::native_time;
    if(!active_simulation_slice())return velocity;
    const auto& samples=*active_gravity;
    const auto it=std::find_if(samples.begin(),samples.end(),[&](const auto& x){return x.address==address;});
    // A contact/impulse may replace velocity after gravity. Never apply a
    // displacement calculated for the old velocity to that new motion.
    if(it!=samples.end() && std::bit_cast<std::uint32_t>(velocity)==std::bit_cast<std::uint32_t>(it->velocity))
        return it->displacement;
    return dora3_native_rate_f32(velocity);
}

extern "C" double dora3_native_approach_coefficient_f64(double alpha) {
    const auto* step=dora3::native_time::active_simulation_slice();
    if(!step || step->numerator==step->denominator)return alpha;
    if(!std::isfinite(alpha)||alpha<0||alpha>1)throw std::invalid_argument("Invalid native approach coefficient");
    if(alpha==1)return 1;
    return -std::expm1(step->ticks()*std::log1p(-alpha));
}
extern "C" double dora3_native_retention_f64(double retention) {
    const auto* step=dora3::native_time::active_simulation_slice();
    if(!step || step->numerator==step->denominator)return retention;
    if(!std::isfinite(retention)||retention<0||retention>1)throw std::invalid_argument("Invalid native retention");
    return std::pow(retention,step->ticks());
}
extern "C" double dora3_native_tick_end_approach_f64(double current,double target,double alpha) {
    const auto* step=dora3::native_time::active_simulation_slice();
    if(!step || step->ends_tick())return current;
    if(!std::isfinite(current)||!std::isfinite(target)||!std::isfinite(alpha)||alpha<0||alpha>1)
        throw std::invalid_argument("Invalid source-tick approach threshold");
    const auto remaining=double(step->denominator-step->phase-step->numerator)/step->denominator;
    return target+(current-target)*std::pow(1.0-alpha,remaining);
}
extern "C" int dora3_native_simulation_tick_end() {
    const auto* step=dora3::native_time::active_simulation_slice();
    return !step || step->ends_tick();
}

// Fractional iterate of e -> e - clamp(alpha*e, -limit, limit).
// The phase coordinate maps every full original update to phase - 1,
// including the step which crosses from capped motion into exponential decay.
// Unlike scaling the already-clamped delta, subdivisions compose exactly for
// a fixed target. No history/cache is needed when the target changes.
extern "C" double dora3_native_capped_approach_f64(double error,double alpha,double limit) {
    const auto* slice=dora3::native_time::active_simulation_slice();
    if(!slice || slice->numerator==slice->denominator)return error*alpha;
    if(!std::isfinite(error)||!std::isfinite(alpha)||!std::isfinite(limit)||alpha<=0||alpha>=1||limit<=0)
        throw std::invalid_argument("Invalid capped approach");
    const double e=std::abs(error),h=slice->ticks(),threshold=limit/alpha,k=-std::log1p(-alpha);
    if(e<=threshold)return error*(-std::expm1(-k*h));
    const double bands=std::ceil((e-threshold)/limit);
    const double residual=e-bands*limit;
    const double phase=bands+std::log(residual/threshold)/k-h;
    const double next_bands=std::max(0.0,std::ceil(phase));
    const double next=next_bands*limit+threshold*std::exp(k*(phase-next_bands));
    return std::copysign(std::clamp(e-next,0.0,limit),error);
}

namespace {
double bounded_drift(double v,double a,double duration,double low,double high) {
    // Integrate the legacy kick/drift extension, splitting exactly at clamps.
    // The same expression serves acceleration, braking and slope rollback.
    double points[4]{0,duration,0,0};unsigned count=2;
    if(a!=0)for(double bound:{low,high}) {
        const double t=(bound-v-0.5*a)/a;
        if(t>0 && t<duration)points[count++]=t;
    }
    // Only the two endpoints and at most two clamp crossings exist.
    // A bounded insertion sort keeps that small range explicit even under
    // GCC15 O2, whose generic sort's 16-element path diagnoses this array.
    for(unsigned i=1;i<count;++i) {
        const auto value=points[i];unsigned j=i;
        while(j && value<points[j-1]) {points[j]=points[j-1];--j;}
        points[j]=value;
    }
    double result=0;
    for(unsigned i=1;i<count;++i) {
        const auto x=points[i-1],y=points[i];
        result+=(y-x)*0.5*(std::clamp(v+a*(x+0.5),low,high)+std::clamp(v+a*(y+0.5),low,high));
    }
    return result;
}
}
extern "C" float dora3_native_drive_f32(float velocity,float target,float amount,float slope,float maximum,std::uint32_t address) {
    using namespace dora3::native_time;
    const auto* slice=active_simulation_slice();
    if(!slice)throw std::logic_error("Native drive called outside simulation");
    if(!std::isfinite(velocity)||!std::isfinite(target)||!std::isfinite(amount)||!std::isfinite(slope)||!std::isfinite(maximum)||amount<0||maximum<-30)
        throw std::invalid_argument("Invalid native drive parameters");
    double v=std::clamp(double(velocity),-30.0,double(maximum));
    const double t=std::clamp(double(target),-30.0,double(maximum));
    double remaining=slice->ticks(),distance=0;
    // At most one target crossing, followed by arrival at a speed bound.
    for(unsigned segment=0;remaining>0;++segment) {
        if(segment>3)throw std::logic_error("Nonconvergent native drive");
        double a=(v<=t?double(amount):-double(amount))+slope;
        if((v==t && a>=0)||(v<=-30 && a<0)||(v>=maximum && a>0))a=0;
        double low=-30,high=maximum,duration=remaining;
        if(v<t && a>0)high=t;
        if(v>t && a<0)low=t;
        if(a!=0)duration=std::min(duration,((a>0?high:low)-v)/a);
        if(duration<=0)throw std::logic_error("Empty native drive segment");
        distance+=bounded_drift(v,a,duration,low,high);
        v=std::clamp(v+a*duration,low,high);remaining-=duration;
    }
    const GravitySample sample{address,float(v),float(distance)};
    auto& samples=*active_drive;
    const auto it=std::find_if(samples.begin(),samples.end(),[&](const auto& x){return x.address==address;});
    if(it==samples.end())samples.push_back(sample);else *it=sample;
    return sample.velocity;
}
extern "C" void dora3_native_project_drive_f32(float velocity,float direction,std::uint32_t speed_bits,std::uint32_t source,std::uint32_t destination) {
    using namespace dora3::native_time;
    if(!active_simulation_slice())return;
    const auto& speeds=*active_drive;
    const auto found=std::find_if(speeds.begin(),speeds.end(),[&](const auto& x){return x.address==source && std::bit_cast<uint32_t>(x.velocity)==speed_bits;});
    if(found==speeds.end())return;
    const GravitySample sample{destination,velocity,found->displacement*direction};
    auto& samples=*active_gravity;
    const auto it=std::find_if(samples.begin(),samples.end(),[&](const auto& x){return x.address==destination;});
    if(it==samples.end())samples.push_back(sample);else *it=sample;
}
