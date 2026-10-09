#pragma once
#include <cstdint>
#include "recomp.h"
namespace dora3::distance {
float configure(float value);
float multiplier();
}
extern "C" {
float dora3_distance_squared(float original);
float dora3_distance_lifetime(float original);
float dora3_distance_far(float original);
float dora3_distance_spawn_limit(uint8_t*,float original,float distance);
void dora3_distance_spawn_begin(uint8_t*,recomp_context*);
void dora3_distance_spawn_end(uint8_t*,recomp_context*);
void dora3_distance_spawn_result(uint8_t*,recomp_context*);
}
