// These math roots cannot be reached in gameplay/process/effect-only fixtures.
// Real menu math and full files draw are exercised by native-ui-check instead.
#include "recomp.h"
#include <cstdlib>
extern "C" void dora3_s2_menu_801BBCEC(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s2_menu_801BBEB0(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s2_menu_801BC17C(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s2_menu_801BC4C0(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s2_menu_801C14B8(uint8_t*,recomp_context*){std::abort();}
