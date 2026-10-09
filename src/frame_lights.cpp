#include "graphics_profile.hpp"
#include "diagnostics.hpp"
#include "frame_lights.hpp"
#include "native_game_loop.hpp"
#include "native_frame_bridge.hpp"
#include "recomp.h"
#include "guest_storage.hpp"
#include "port_settings.hpp"
#include "draw_distance.hpp"
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
    DORA3_LOG(stderr, "Dora3 frame lights: %s at %08X\n", reason, address);
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
        if (dora3::distance::task_for_display_list(display_list) != bank0 + i * bank_stride) continue;
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
#ifdef DORA3_LIVE_RT64
    // Capture policy on the guest thread that built this exact bank. The
    // parser must not inspect a later scene's live Native policy pointer.
    uint8_t* rdram=ram;
    slots[index].lights.native_ram_optional=dora3_native_frame_build_active()&&
        dora3_native_loop_gameplay_active()&&MEM_W(0,int32_t(0x801AA658))==320&&
        MEM_W(0,int32_t(0x801A48C4))==240&&MEM_W(0,int32_t(0x801A9C88))==0;
#ifdef DORA3_GRAPHICS_PROFILE
    using namespace dora3::graphics_profile;
    const auto settings=dora3::port::settings();
    context(MEM_HU(0,int32_t(0x801B36B8)),settings.frame_rate_limit,settings.frame_rate_mode,dora3::distance::multiplier());
    if(!dora3_native_frame_build_active())add(Counter::GateNotNativeBuild);
    if(!dora3_native_loop_gameplay_active())add(Counter::GateNotGameplay);
    if(MEM_W(0,int32_t(0x801AA658))!=320||MEM_W(0,int32_t(0x801A48C4))!=240)add(Counter::GateDimensions);
    if(MEM_W(0,int32_t(0x801A9C88))!=0)add(Counter::GateCPUFramebuffer);
#endif
    slots[index].lights.native_depth_address=uint32_t(MEM_W(0,int32_t(0x801AA650)))&0x1FFFFFFFu;
#endif
    slots[index].ready = true;
}
