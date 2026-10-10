#pragma once
#include "port_settings.hpp"
#include <array>
namespace dora3::port {
struct PhysicalGamepad {
    std::array<float,SDL_CONTROLLER_BUTTON_MAX+SDL_CONTROLLER_AXIS_MAX*2> pad{};
    bool connected=false;
    SDL_JoystickID instance=-1;
    std::string name;
};
struct PhysicalInput {
    std::array<unsigned char,SDL_NUM_SCANCODES> keys{};
    std::array<PhysicalGamepad,PlayerCount> gamepads{};
    bool focused=false,background_gamepad=false;
};
// SDL thread publishes state; render thread consumes a copy, never SDL handles.
void publish_physical(const PhysicalInput& input);
PhysicalInput physical();
bool menu_open();
void set_menu_open(bool open);
bool capture_active();
bool ui_input_blocked();
void begin_capture(Action action,bool gamepad,int player=0);
int menu_gamepad();
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
