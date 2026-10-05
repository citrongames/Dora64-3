#pragma once
#include "native_simulation_time.hpp"
#include "recomp.h"

// Reviewed stage AI shares the common actor pool, but its commands have an
// actor-local Original period. Numeric kind alone never identifies an overlay.
bool native_enemy_kind(int);
bool native_enemy_owned(uint8_t*,uint32_t);
void native_enemy_reset();
int native_enemy_axis(uint8_t*,recomp_context*,uint32_t);
void native_enemy_motion_end(uint8_t*,uint32_t);
void native_enemy_source_pool_end(uint8_t*);
bool native_gameplay_preview_object_pass();
void native_gameplay_preview_enemy_command_clock(uint8_t*,uint32_t);
void native_gameplay_preview_enemy_defer_birth(uint8_t*,uint32_t);
extern "C" int dora3_native_enemy_callback(uint8_t*,recomp_context*,uint32_t,uint32_t,void (*)(uint8_t*,recomp_context*));
extern "C" void dora3_native_enemy_gravity(uint8_t*,recomp_context*,unsigned);
extern "C" void dora3_native_enemy_turn(uint8_t*,recomp_context*,unsigned);
extern "C" int dora3_native_enemy_blink_skip(uint8_t*);
extern "C" void dora3_native_enemy_equation(uint8_t*,recomp_context*,uint32_t);
extern "C" int dora3_native_enemy_attachment_only();
extern "C" void dora3_native_enemy_clip_end(uint8_t*,recomp_context*);
extern "C" int dora3_native_enemy_commands_active();
extern "C" int dora3_native_enemy_model_event(uint8_t*,uint32_t,float,float,uint64_t);
extern "C" void dora3_native_enemy_model_read(uint8_t*,uint32_t);
extern "C" int dora3_native_enemy_model_zero(uint8_t*,uint32_t,float,uint64_t);
extern "C" void dora3_native_enemy_projection(uint8_t*,recomp_context*,uint32_t);
