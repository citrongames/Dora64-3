#pragma once
#include <cstdint>
#include "native_frame_bridge.hpp"
// Scene coverage is explicit and independent of the shared queue/build loop.
struct ScenePolicy { const char* name; bool (*eligible)(uint8_t*); unsigned divider; dora3::native_time::FrameUpdate update=nullptr; };
const ScenePolicy* select_native_scene_policy(uint8_t*);
void native_scene34_ready();
void invalidate_native_scene_policy();
