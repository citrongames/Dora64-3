#pragma once
#include "port_settings.hpp"
#include <vector>
namespace dora3::port {
struct OutputMode {
    int width=0,height=0,refresh_rate=0;
    bool operator==(const OutputMode&) const = default;
};
struct DisplayInfo { std::string name; OutputMode desktop; std::vector<OutputMode> modes; };
// The render/UI thread reads snapshots and requests refresh; only SDL thread enumerates/applies.
std::vector<DisplayInfo> displays();
std::vector<OutputMode> window_modes(const DisplayInfo& display);
std::string output_mode_label(const OutputMode& mode,bool include_refresh);
void request_display_refresh();
void refresh_displays();
void update_display_window(SDL_Window* window);
void publish_display_rate(unsigned rate);
unsigned display_rate();
unsigned target_frame_rate(const Settings& settings,unsigned rate);
}
