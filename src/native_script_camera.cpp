#include "native_script_camera.hpp"
#include <cstring>
#include <stdexcept>
namespace dora3::native_time {
void ScriptCameraTimeline::reset() { initialized_=false;tick_=phase_=denominator_=0; }
void ScriptCameraTimeline::advance(uint8_t* rdram,recomp_context* ctx,
        const SimulationSlice& slice,Update update) {
    if(!rdram || !ctx || !update || active_simulation_slice())
        throw std::invalid_argument("Invalid script camera invocation");
    // Validate before modifying memory or bookkeeping.
    { SimulationScope validate(slice); }
    if(initialized_ && (slice.tick!=tick_ || slice.phase!=phase_ || slice.denominator!=denominator_))
        throw std::logic_error("Non-contiguous script camera time; explicit reset required");
    if(!initialized_ && slice.phase!=0)
        throw std::logic_error("Script camera must begin at a tick boundary");
    constexpr auto control_offset=0x2183D0, camera_offset=0x1A99F8;
    if(slice.phase==0) {
        std::memcpy(controls_.data(),rdram+control_offset,controls_.size());
        std::memcpy(cameras_.data(),rdram+camera_offset,cameras_.size());
    } else {
        std::memcpy(rdram+control_offset,controls_.data(),controls_.size());
        std::memcpy(rdram+camera_offset,cameras_.data(),cameras_.size());
    }
    const SimulationSlice cumulative{slice.phase+slice.numerator,slice.denominator,slice.tick,0};
    { SimulationScope scope(cumulative);update(rdram,ctx); }
    initialized_=true;denominator_=slice.denominator;
    tick_=slice.tick+(slice.ends_tick()?1:0);
    phase_=slice.ends_tick()?0:slice.phase+slice.numerator;
}
}
