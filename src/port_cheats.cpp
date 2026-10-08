#include "port_cheats.hpp"
#include "recomp.h"
#include <atomic>

extern "C" int dora3_overlay_loaded(uint32_t);
namespace {
constexpr uint32_t common_rom=0x5E400;
constexpr uint32_t selected_character=0x801B374A, status_records=0x801B361B;
constexpr uint32_t character_parameters=0x801B8370, item_counters=0x801B3700;
constexpr unsigned character_count=6, parameter_stride=0x1C, health_offset=0x18;
// ROM name table8020EF88 and the menu's shared getter prove these exact IDs.
constexpr std::array<unsigned,7> item_ids{1,2,3,4,5,8,11};
std::atomic_uint32_t options{0};

unsigned maximum_health(uint8_t* rdram,uint32_t& status) {
    if(!rdram||!dora3_overlay_loaded(common_rom))return 0;
    const unsigned slot=MEM_BU(0,int32_t(selected_character));
    if(slot>=character_count)return 0;
    status=status_records+slot*2;
    // Availability is a separate progression flag. Original health changes
    // do not gate on it; refill only the selected HP byte without changing it.
    const uint32_t maximum_address=character_parameters+slot*parameter_stride+health_offset;
    // MEM_BU expects a sign-extended guest address. An unsigned offset mixed
    // with an int32_t base would promote the sum and address RAM+4GiB.
    const unsigned maximum=MEM_BU(0,int32_t(maximum_address));
    return maximum>0&&maximum<=127?maximum:0; // Original HP reads are signed bytes.
}
bool frozen_item(uint32_t flags,unsigned id) {
    for(unsigned i=0;i<item_ids.size();++i)
        if(id==item_ids[i])return (flags&(1u<<(i+1)))!=0;
    return false;
}
bool refill_item(uint8_t* rdram,recomp_context* ctx) {
    const auto flags=options.load(std::memory_order_relaxed);
    if(!flags||!rdram||!frozen_item(flags,uint32_t(ctx->r4))||!dora3_overlay_loaded(common_rom))return false;
    MEM_B(0,int32_t(item_counters+uint32_t(ctx->r4)))=99;
    return true;
}
}
namespace dora3::port {
void configure_cheats(const CheatSettings& settings) {
    uint32_t flags=0;
    for(unsigned i=0;i<settings.size();++i)if(settings[i])flags|=1u<<i;
    options.store(flags,std::memory_order_relaxed);
}
bool cheats_enabled(){return options.load(std::memory_order_relaxed)!=0;}
}

// Called only on the guest game thread, before Original or Native dispatch.
// No guest pointer is cached, so cold boot/reset/load/character switches need
// no cheat-specific session state. The UI only publishes atomic options.
extern "C" void dora3_cheats_tick(uint8_t* rdram) {
    const auto flags=options.load(std::memory_order_relaxed);
    if(!flags||!rdram||!dora3_overlay_loaded(common_rom))return;
    if(flags&1u) {
        uint32_t status=0;const unsigned maximum=maximum_health(rdram,status);
        if(maximum)MEM_B(1,int32_t(status))=maximum;
    }
    for(unsigned i=0;i<item_ids.size();++i)
        if(flags&(1u<<(i+1)))MEM_B(0,int32_t(item_counters+item_ids[i]))=99;
}
extern "C" void dora3_cheats_health_store(uint8_t* rdram,recomp_context* ctx) {
    if(!(options.load(std::memory_order_relaxed)&1u))return;
    uint32_t status=0;const unsigned maximum=maximum_health(rdram,status);
    //801C0758 stores v0 to the current status byte BEFORE its lethal check.
    if(maximum&&uint32_t(ctx->r3)==status)ctx->r2=maximum;
}
extern "C" void dora3_cheats_counter_store(uint8_t* rdram,recomp_context* ctx) {
    const auto flags=options.load(std::memory_order_relaxed);
    const auto id=uint32_t(ctx->r4);
    if(!flags||!rdram||!frozen_item(flags,id)||!dora3_overlay_loaded(common_rom))return;
    //801B5130 stores v0 at3700(at); a0 remains the Original item ID.
    // Keep the leaf's original register writes, clamp and return sequence.
    if(uint32_t(ctx->r1)+0x3700u==item_counters+id)ctx->r2=99;
}
extern "C" void dora3_cheats_counter_get(uint8_t* rdram,recomp_context* ctx) {
    // Also covers reads following a bulk save/new-game state restore before
    // the next message boundary. The Original getter itself still returns the count.
    refill_item(rdram,ctx);
}
