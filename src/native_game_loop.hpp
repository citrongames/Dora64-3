#pragma once
#include <cstdint>
#include "recomp.h"
// All hooks run on scheduled guest threads; reset requires runtime quiescence.
extern "C" {
void dora3_native_loop_message(uint8_t*,recomp_context*);
int dora3_native_loop_before_vi(uint8_t*,recomp_context*);
int dora3_native_loop_after_vi(uint8_t*,recomp_context*);
int dora3_native_loop_owns_fade();
int dora3_native_loop_gameplay_active();
int dora3_native_loop_owns_frame_banks();
void dora3_native_loop_fade_command(uint8_t*,recomp_context*);
void dora3_native_loop_scene_ready(uint8_t*,recomp_context*);
int dora3_native_loop_receive_deferred(uint8_t*,recomp_context*);
void dora3_native_loop_resource_barrier(uint8_t*,recomp_context*);
void dora3_native_loop_transition(uint8_t*,recomp_context*);
void dora3_native_loop_submit(uint8_t*,recomp_context*);
void dora3_native_loop_retire(uint8_t*,recomp_context*);
}
void dora3_native_loop_reset();
unsigned dora3_requested_native_fps(); // Frontend snapshot; 0 = Original/Display.
unsigned dora3_native_presentation_fps(); // Thread-safe target query for present/VSync.
