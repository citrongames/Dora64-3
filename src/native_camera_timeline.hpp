#pragma once
#include "native_simulation_time.hpp"
#include "recomp.h"
#include <array>
#include <cstdint>
namespace dora3::native_time {
// Evaluate coupled camera equations from the current original tick's initial
// state. Targets/geometry are read at the current native sample; completed poses
// are never blended. Only reviewed camera-owned bytes are restored; geometric
// eye XYZ/up-vector fields written by 8002F064 are deliberately excluded.
class OrdinaryCameraTimeline {
public:
    using Update=void (*)(uint8_t*,recomp_context*);
    void advance(uint8_t*,recomp_context*,const SimulationSlice&,uint32_t root,Update);
    void reset();
private:
    std::array<uint8_t,256> initial_{},published_{};
    uint64_t tick_=0,phase_=0,denominator_=0;
    uint32_t root_=0,camera_=0;
    bool initialized_=false,anchored_=false;
};
}

extern "C" void dora3_native_camera_dispatch(uint8_t*,recomp_context*,uint32_t,recomp_func_t*);
void dora3_native_camera_dispatch_reset();
