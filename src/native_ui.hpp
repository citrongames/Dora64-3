#pragma once
#include "native_frame_time.hpp"
#include "recomp.h"
#include <cstdint>

namespace dora3::native_time {
// UI equations borrow a frame. Guest command state remains at Source cadence;
// temporary panel/menu coordinates cannot escape into the next input update.
bool native_ui_menu_eligible(uint8_t*);
bool native_ui_game_eligible(uint8_t*);
void native_ui_begin(uint8_t*,Nanoseconds,unsigned);
void native_ui_reset();
class UiFrameScope {
public:
    UiFrameScope(uint8_t*,recomp_context*,Nanoseconds);
    ~UiFrameScope();
    UiFrameScope(const UiFrameScope&)=delete;
private:
    uint8_t* ram_;
};
}
extern "C" void dora3_native_ui_source(uint8_t*,recomp_context*);
extern "C" void dora3_native_ui_source_begin(uint8_t*,recomp_context*);
extern "C" void dora3_native_ui_domain(uint8_t*,unsigned);
extern "C" void dora3_native_ui_draw_boundary(uint8_t*,uint32_t,unsigned);
extern "C" int dora3_native_ui_command_once(uint32_t);
extern "C" int dora3_native_ui_equations_active();
extern "C" float dora3_native_ui_velocity_f32(float,float,uint32_t);
extern "C" float dora3_native_ui_displacement_f32(float,uint32_t);
extern "C" float dora3_native_ui_rate_f32(float);
extern "C" double dora3_native_ui_rate_f64(double);
extern "C" double dora3_native_ui_selection_y_f64(double,float,double);

// C9EBC charge fill uses RDP10.2 coordinates; counter/input remain Source-owned.
extern "C" void dora3_native_ui_charge_rectangle(uint8_t*,recomp_context*);

extern "C" int dora3_native_ui_map_cursor_rectangle(uint8_t*,recomp_context*);
