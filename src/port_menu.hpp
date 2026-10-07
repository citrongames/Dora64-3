#pragma once
#include "port_settings.hpp"
#include <array>
namespace dora3::port {
struct PhysicalInput {
    std::array<unsigned char,SDL_NUM_SCANCODES> keys{};
    std::array<float,SDL_CONTROLLER_BUTTON_MAX+SDL_CONTROLLER_AXIS_MAX*2> pad{};
    bool focused=false,connected=false,background_gamepad=false;
    std::string controller_name;
};
// SDL thread publishes state; render thread consumes a copy, never SDL handles.
void publish_physical(const PhysicalInput& input);
PhysicalInput physical();
bool menu_open();
void set_menu_open(bool open);
bool capture_active();
bool ui_input_blocked();
void begin_capture(Action action,bool gamepad);
void cancel_capture();
std::string capture_label();
// Called on the SDL thread before translating input for the N64.
// Optional game sample masks only controls still held across menu/focus changes.
bool process_port_input(const PhysicalInput& input,PhysicalInput* game_input=nullptr);
void overlay_input();
void draw_overlay();
void record_output_frame();
void request_exit();
bool take_exit_request();
}
