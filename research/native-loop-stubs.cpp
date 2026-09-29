// Narrow component fixtures do not activate a scene policy. The common loop has
// its own integration test using the real implementation and original functions.
#include "native_game_loop.hpp"
extern "C" void dora3_native_loop_message(uint8_t*,recomp_context*) {}
extern "C" int dora3_native_loop_before_vi(uint8_t*,recomp_context*) {return 0;}
extern "C" int dora3_native_loop_after_vi(uint8_t*,recomp_context*) {return 0;}
extern "C" int dora3_native_loop_owns_fade() {return 0;}
extern "C" void dora3_native_loop_scene_ready(uint8_t*,recomp_context*) {}
extern "C" void dora3_native_loop_transition(uint8_t*,recomp_context*) {}
extern "C" void dora3_native_loop_submit(uint8_t*,recomp_context*) {}
extern "C" void dora3_native_loop_retire(uint8_t*,recomp_context*) {}
void dora3_native_loop_reset() {}

extern "C" void dora3_native_loop_fade_command(uint8_t*,recomp_context*) {}
