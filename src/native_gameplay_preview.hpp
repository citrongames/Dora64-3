#pragma once
#include "native_frame_time.hpp"
#include "native_simulation_time.hpp"
#include "recomp.h"
#include <optional>
bool native_gameplay_preview_eligible(uint8_t*);
void native_gameplay_preview_begin(uint8_t*,dora3::native_time::Nanoseconds,unsigned);
void native_gameplay_preview_reset(uint8_t* =nullptr);
void native_gameplay_preview_update(uint8_t*,recomp_context*,dora3::native_time::Nanoseconds);
std::optional<dora3::native_time::SimulationSlice> native_gameplay_preview_slice();
// Active guest command's rational interval start; null outside real gameplay.
std::optional<dora3::native_time::SourceTimestamp> native_gameplay_preview_command_time();
std::optional<dora3::native_time::Nanoseconds> native_gameplay_preview_render_time();
extern "C" int dora3_native_preview_hook(uint8_t*,recomp_context*,uint32_t);
extern "C" void dora3_native_projectile_scope_end();
extern "C" void dora3_native_object_scope_end();

// True only inside the actual Native common-pool pass, never in Original AI.
bool native_gameplay_preview_object_pass();

// Fraction of the current authored gameplay period after the last Source input.
std::optional<double> native_gameplay_preview_source_fraction();
