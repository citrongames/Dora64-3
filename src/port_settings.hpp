#pragma once
#include <array>
#include <filesystem>
#include <string>
#include <SDL.h>
#include "port_cheats.hpp"
#include "modern_camera.hpp"

namespace dora3::port {
enum Action { A, B, Z, Start, L, R, DUp, DDown, DLeft, DRight,
              CUp, CDown, CLeft, CRight, StickUp, StickDown, StickLeft, StickRight, CameraZoomIn, CameraZoomOut, ActionCount };
inline constexpr std::array<const char*, ActionCount> action_names{
    "A", "B", "Z", "Start", "L", "R", "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right",
    "C Up", "C Down", "C Left", "C Right", "Stick Up", "Stick Down", "Stick Left", "Stick Right", "Camera zoom in", "Camera zoom out"};
// SDL button IDs, or 100 + axis * 2 + direction (0 negative, 1 positive).
constexpr int pad_axis(int axis, bool positive) { return 100 + axis * 2 + int(positive); }
struct Settings {
    std::string language="ja";
    int volume=100, resolution=0, filtering=0; // 0 original, 1 2x, 2 4x, 3 window scale; 0 N64, 1 bilinear
    int window_width=1280, window_height=720;
    int display_index=0, display_refresh_rate=0;
    int frame_rate_mode=0, frame_rate_limit=144; // Original, Display, Manual (Dora64)
    bool fullscreen=false, show_fps=false, autosave=true;
    int aspect_ratio=0, hud_layout=0; // Original/Expand and Original/Full, independently.
    float draw_distance=1.0f;
    float deadzone=0.25f;
    bool background_gamepad_input=true;
    std::array<int, ActionCount> keys{}, pad{};
    CheatSettings cheats{};
    dora3::camera::Options camera{};
};
Settings defaults();
void initialize(const std::filesystem::path& directory);
Settings settings();
// Independent setting groups avoid lost binding updates from another thread.
void set_general(const Settings& value);
void set_cheats(const CheatSettings& value);
void set_deadzone(float value);
void set_background_gamepad_input(bool value);
void set_fullscreen(bool value);
void correct_display_index(int expected, int value);
void bind(Action action, bool gamepad, int code);
void reset_bindings();
void profile(bool modern);
std::string save_error();
std::string pad_name(int code);
bool valid_pad(int code);
bool valid_key(int code);
}
