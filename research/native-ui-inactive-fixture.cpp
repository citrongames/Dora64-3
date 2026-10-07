// Pure process/effect checks have no loaded UI scene or acquired Native loop.
// Link the actual UI observer; any unexpected entry into menu math is fatal.
#include "recomp.h"
#include "native_frame_time.hpp"
#include <cstdlib>
extern "C" int dora3_overlay_loaded(uint32_t){return 0;}
extern "C" int dora3_native_loop_owns_fade(){return 0;}
extern "C" void dora3_lib_80057A80(uint8_t*,recomp_context*){std::abort();}

namespace dora3::native_time {
Nanoseconds monotonic_time(){std::abort();}
}

// Synthetic loaded-module epoch; actual reload behavior is checked separately.
extern "C" uint64_t dora3_overlay_generation(uint32_t rom){return dora3_overlay_loaded(rom)?1:0;}

extern "C" int dora3_native_loop_gameplay_active(){return 0;}
// No loaded overlay in these two leaf fixtures. Actual camera math is proved
// by the connected UI check; reaching any of these boundaries is a failure.
extern "C" void dora3_s0_8002F064(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DB7C8(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D6EAC(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D8FF0(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D7294(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D5864(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D8578(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DB720(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D9B00(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D8A24(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801D9C94(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DA06C(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DAB58(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DA588(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DAF50(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801DB3D0(uint8_t*,recomp_context*){std::abort();}
