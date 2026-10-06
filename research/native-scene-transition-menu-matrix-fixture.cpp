// The room-barrier fixture has no open menu or leased preview actors.
// Actual menu matrix arithmetic is checked by native-ui-check; reaching it in
// this resource/drain scope is an unseeded boundary and must fail explicitly.
#include "recomp.h"
#include <cstdio>
#include <cstdlib>
extern "C" void dora3_s0_8002DA98(uint8_t*,recomp_context*) {
    std::fputs("Unexpected menu matrix in room-barrier fixture\n",stderr);
    std::abort();
}
