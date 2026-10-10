#pragma once
#include "recomp.h"
#include <cstdint>

namespace dora3::camera {
struct Options {
    bool enabled=false,capture_mouse=false,invert_x=false,invert_y=false;
    float stick_speed=120.f,mouse_sensitivity=.15f;
};
void configure(const Options&);
bool active(); // SDL thread reads publication, never guest RAM.
void submit(float x,float y,float mx,float my,float zoom,float wheel,bool center);
void clear_input();
void reset(); // Guest session boundary, after old threads have stopped.
void observe(uint8_t*); // Guest thread; revoke availability on scene/menu cuts.
// Called instead of an ordinary root, in its existing time/pose owner.
bool dispatch(uint8_t*,recomp_context*,uint32_t root);
// UI fallback uses cumulative elapsed seconds within its Source period.
bool frame(uint8_t*,recomp_context*,double elapsed);
void clock_handoff();
}
extern "C" int dora3_modern_camera_prepare(uint8_t*);
