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
