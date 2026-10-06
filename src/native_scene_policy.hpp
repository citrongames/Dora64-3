#pragma once
#include <cstdint>
#include "native_frame_bridge.hpp"
// Scene coverage is explicit and independent of the shared queue/build loop.
struct ScenePolicy { const char* name; bool (*eligible)(uint8_t*); unsigned divider; dora3::native_time::FrameUpdate update=nullptr; };
const ScenePolicy* select_native_scene_policy(uint8_t*);
const char* native_scene_fallback_reason(uint8_t*);
// Callback identity belongs to the acquired session, rather than only its
// broad policy family. Mini overlays can reuse the same VRAM addresses.
struct SceneIdentity {
    uint32_t update=0,draw=0,module=0;
    unsigned divider=0;
    uint64_t load_generation=0;
    bool operator==(const SceneIdentity&) const = default;
};
SceneIdentity native_scene_identity(uint8_t*,const ScenePolicy&);
void native_scene34_ready();
void invalidate_native_scene_policy();
