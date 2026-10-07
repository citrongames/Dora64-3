// Standalone motion/gravity checks acquire SimulationScope only, never a live
// GameplayPreview, motion callback, projectile owner or model allocation.
// Preserve those inactive-owner boundaries explicitly. The full player and
// scene checks link the actual GameplayPreview implementation and exercise
// flight and allocation-scoped projectile gravity separately.
#include "native_simulation_time.hpp"
#include "recomp.h"
#ifdef DORA3_FIXTURE_INACTIVE_PROJECTILE
extern "C" int dora3_native_preview_hook(uint8_t*,recomp_context*,uint32_t){return 0;}
extern "C" float dora3_native_projectile_gravity_f32(uint8_t*,recomp_context* ctx){
    return dora3_native_gravity_f32(ctx->f0.fl,ctx->f2.fl,-50.f,uint32_t(ctx->r4)+0x14);
}
#endif
#ifdef DORA3_FIXTURE_INACTIVE_FLIGHT
extern "C" int dora3_native_player_flight_hook(uint8_t*,recomp_context*,uint32_t){return 0;}
#endif

// No UI observation owner in this scalar/player-motion leaf fixture.
extern "C" void dora3_native_ui_domain(uint8_t*,unsigned domain){if(domain!=16384u)std::abort();}
