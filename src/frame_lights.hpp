#pragma once
#include <array>
#include <cstdint>

namespace dora3::frame {
inline constexpr uint32_t bank0 = 0x8010E310, bank_stride = 0x31728;
inline constexpr uint32_t dl_offset = 0x12328;
inline constexpr uint32_t light_address = 0x6FB70, light_size = 0x28;
struct Lights {
    std::array<uint8_t, light_size> bytes{};
    bool native_ram_optional=false;
    uint32_t native_depth_address=0;
    // RT64 reads a 16-byte Light even for the ambient entry.
    const void* lookup(uint32_t physical_address) const;
};
// Enable once before starting guest execution. Reset only at the graphics barrier.
void enable();
void reset();
bool take(uint32_t display_list, Lights& output);
}
extern "C" void dora3_capture_frame_lights(uint8_t* ram, uint32_t bank);
