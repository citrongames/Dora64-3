#pragma once
#include "native_simulation_time.hpp"
#include "recomp.h"
#include <array>
#include <cstdint>
namespace dora3::native_time {
// The original script camera composes local-space operations in a fixed order.
// Its follow offset is applied once by that composition, not once per display.
// Evaluate the reviewed time-dependent equations from the start of this tick.
// There is no interpolation between completed camera poses.
class ScriptCameraTimeline {
public:
    using Update = void (*)(uint8_t*, recomp_context*);
    void advance(uint8_t* rdram, recomp_context*, const SimulationSlice&, Update);
    void reset(); // Required on scene reset, restore or a new script timeline.
private:
    std::array<std::uint8_t,4*0x1B8> controls_{};
    std::array<std::uint8_t,4*0x34> cameras_{};
    std::uint64_t tick_=0, phase_=0, denominator_=0;
    bool initialized_=false;
};
}
