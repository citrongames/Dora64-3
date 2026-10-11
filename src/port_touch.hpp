#pragma once
#include <filesystem>
#include <SDL.h>
#include "port_touch_state.hpp"

namespace dora3::touch {
void initialize(const std::filesystem::path& directory);
void process_event(const SDL_Event& event);
void update();
void cancel();
// Normalized port-menu bounds, published by the render thread.
void set_port_menu_bounds(Rect bounds, bool canDismiss);
bool is_editing();
GameInput read_input();
CameraInput take_camera();
void draw();
void draw_settings();
}
