#pragma once
#include "recomp.h"
extern "C" int dora3_native_rectangle_update(uint8_t*,recomp_context*);
extern "C" int dora3_native_zoom_update(uint8_t*,recomp_context*);
extern "C" int dora3_native_simulation_tick_end();
