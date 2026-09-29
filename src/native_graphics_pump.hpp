#pragma once
#include "recomp.h"
#include <cstdint>
namespace dora3::native_time {
inline constexpr uint32_t graphics_pump_message = 0xFFFFFF04u;
}
bool dora3_enqueue_native_graphics_wake();
extern "C" int dora3_native_scheduler_message(uint8_t*, recomp_context*);
extern "C" int dora3_native_scheduler_pumping();
extern "C" void dora3_native_task_submitted(uint8_t*, recomp_context*);
