#include "frame_lights.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

namespace {
struct Slot { dora3::frame::Lights lights; bool ready = false; };
std::array<Slot, 2> slots{};
std::mutex mutex;
bool enabled = false;
[[noreturn]] void invalid(const char* reason, uint32_t address) {
    std::fprintf(stderr, "Dora3 frame lights: %s at %08X\n", reason, address);
    std::abort();
}
}
namespace dora3::frame {
const void* Lights::lookup(uint32_t address) const {
    if (address == light_address || address == light_address + 8 || address == light_address + 24)
        return bytes.data() + address - light_address;
    return nullptr;
}
void enable() {
    std::scoped_lock lock(mutex);
    slots = {};
    enabled = true;
}
void reset() {
    std::scoped_lock lock(mutex);
    slots = {};
}
bool take(uint32_t display_list, Lights& output) {
    std::scoped_lock lock(mutex);
    for (unsigned i = 0; i < slots.size(); ++i) {
        if ((display_list & 0x1FFFFFFF) != ((bank0 + i * bank_stride + dl_offset) & 0x1FFFFFFF)) continue;
        if (!slots[i].ready) return false;
        output = slots[i].lights;
        slots[i].ready = false;
        return true;
    }
    return false;
}
}
extern "C" void dora3_capture_frame_lights(uint8_t* ram, uint32_t bank) {
    using namespace dora3::frame;
    std::scoped_lock lock(mutex);
    if (!enabled) return; // Diagnostic frontend has no asynchronous RT64 consumer.
    unsigned index;
    if (bank == bank0) index = 0;
    else if (bank == bank0 + bank_stride) index = 1;
    else invalid("unreviewed task bank", bank);
    if (slots[index].ready) invalid("bank republished before light snapshot consumption", bank);
    // Keep runtime word-swapped byte order: RT64's Light has the same layout.
    std::memcpy(slots[index].lights.bytes.data(), ram + light_address, light_size);
    slots[index].ready = true;
}
