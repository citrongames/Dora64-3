// Gameplay-only fixtures link the real cutscene runtime for shared helpers.
// Scripted ownership is inactive; reaching either script-only root is fatal.
#include "recomp.h"
#include <cstdlib>
#ifndef DORA3_FIXTURE_REAL_LOOP
extern "C" int dora3_native_loop_cutscene_active(){return 0;}
#endif
extern "C" void dora3_s3_801E3768(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801E0760(uint8_t*,recomp_context*){std::abort();}
