#pragma once
#include <cstdint>
// Scene coverage is explicit and independent of the shared queue/build loop.
struct ScenePolicy { const char* name; bool (*eligible)(uint8_t*); unsigned divider; };
const ScenePolicy* select_native_scene_policy(uint8_t*);
void native_scene34_ready();
void invalidate_native_scene_policy();
