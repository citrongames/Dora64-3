// UI/process/effect-only fixtures have an empty real model registry.
// Any pose/morph path is an explicit unseeded boundary, never a silent stub.
#include "native_frame_bridge.hpp"
#include "native_gameplay_preview.hpp"
#include <cstdlib>
namespace dora3::native_time {
const FrameBinding* active_frame_binding(){return nullptr;}
}
#ifndef DORA3_UI_CHARGE_FRACTION_EXTERNAL
std::optional<double> native_gameplay_preview_source_fraction(){return std::nullopt;}
#endif
extern "C" int dora3_native_loop_owns_frame_banks(){return 0;}
extern "C" void dora3_s0_800393C4(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s0_8003931C(uint8_t*,recomp_context*){std::abort();}

#ifdef DORA3_UI_MATRICES_INACTIVE
extern "C" void dora3_s0_8002DA98(uint8_t*,recomp_context*){std::abort();}
#endif
