#pragma once
#include <cstdint>
#include "recomp.h"
namespace dora3::retirement {
// Enabled before guest threads; reset only after graphics/guest shutdown barrier.
void enable();
void reset();
void trace(uint8_t* ram, recomp_context* context, uint32_t address);
void parsed(uint32_t display_list);
struct Stats { uint64_t held_ticks, spilled, released; uint32_t spill_pending; };
Stats stats();
}
