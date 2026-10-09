#pragma once
#include "recomp.h"
#include <cstdint>
namespace dora3::native_time {
inline constexpr uint32_t graphics_pump_message = 0xFFFFFF04u;
}
bool dora3_enqueue_native_graphics_wake();
// Guest-thread safe point: deliver pending events and respect guest priorities.
// No blocking receive, no synthetic completion and no guest register writes.
#ifdef DORA3_LIVE_RT64
void dora3_native_scheduler_checkpoint(uint8_t*);
#else
inline void dora3_native_scheduler_checkpoint(uint8_t*) {}
#endif
extern "C" int dora3_native_scheduler_message(uint8_t*, recomp_context*);
extern "C" int dora3_native_scheduler_pumping();
extern "C" void dora3_native_task_submitted(uint8_t*, recomp_context*);
