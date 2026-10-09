#include "cutscene_input.hpp"
#include "recomp.h"
#include "native_frame_bridge.hpp"
#include "native_simulation_time.hpp"
#include <optional>

extern "C" int dora3_overlay_loaded(uint32_t);
extern "C" uint64_t dora3_overlay_generation(uint32_t);
namespace {
constexpr uint32_t scene_rom=0x70A50,script_mode=0x80218AD8,script_buffer=0x80218B08;
constexpr uint32_t start_requested=0x80218B00,skip_thread=0x802183C0;
constexpr uint32_t pressed_buttons=0x801A48EC,source_tick=0x801AC6B4;
constexpr uint16_t start_button=0x1000;
struct EarlyStart {
    uint8_t* ram;
    uint32_t buffer,command,tick;
    uint64_t generation;
};
std::optional<EarlyStart> early_start;
uint32_t word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,int32_t(address)));}
uint16_t half(uint8_t* rdram,uint32_t address){return MEM_HU(0,int32_t(address));}
bool pointer(uint32_t address,unsigned bytes,unsigned alignment) {
    return !(address&(alignment-1))&&address>=0x80000000u&&uint64_t(address)+bytes<=0x80800000ull;
}
bool source_boundary() {
    return !dora3::native_time::active_frame_binding()&&!dora3::native_time::active_simulation_slice();
}
}
namespace dora3 {
void reset_cutscene_input(){early_start.reset();}
void cutscene_input_before_script(uint8_t* ram) {
    early_start.reset();
    if(!ram||!source_boundary()||!dora3_overlay_loaded(scene_rom)||word(ram,script_mode)!=1||
       !(half(ram,pressed_buttons)&start_button)||half(ram,start_requested)!=0||
       half(ram,skip_thread+4)!=0||half(ram,skip_thread+6)!=0||half(ram,skip_thread+10)!=0)return;
    const auto buffer=word(ram,script_buffer),command=word(ram,skip_thread);
    if(!pointer(buffer,32,4)||!pointer(command,8,2))return;
    const auto initial_offset=word(ram,buffer+7*4);
    // E3CE8 builds record7 from header[7]. All30 scripts350..379 begin
    // their skip thread with pause(7), separator(1), then the skip program.
    // Match that INITIAL command, not an arbitrary later script pause.
    if(initial_offset<32||uint64_t(buffer)+initial_offset!=command||
       half(ram,command)!=3||half(ram,command+2)!=7||half(ram,command+4)!=1)return;
    early_start=EarlyStart{ram,buffer,command,word(ram,source_tick),dora3_overlay_generation(scene_rom)};
}
bool cutscene_input_source_completed(uint8_t* rdram) {
    if(!early_start)return false;
    const auto request=*early_start;early_start.reset();
    if(!rdram||rdram!=request.ram||!source_boundary()||!dora3_overlay_loaded(scene_rom)||
       dora3_overlay_generation(scene_rom)!=request.generation||word(rdram,script_mode)!=1||
       word(rdram,script_buffer)!=request.buffer||word(rdram,source_tick)!=request.tick||
       word(rdram,skip_thread)!=request.command+6||half(rdram,skip_thread+4)!=0||
       half(rdram,skip_thread+6)!=0||half(rdram,skip_thread+10)!=1||half(rdram,start_requested)!=1)return false;
    // E4484 accepts Start before dispatching script commands: it sets8B00
    // and clears record7+A. On the first update, pause(7) immediately sets
    // that same flag again.8B00 then rejects further Start edges forever.
    // Retain ONLY that accepted request after its exact initialization;
    // the unchanged interpreter executes the skip program next Source tick.
    // Buttons, its one-shot flag, waits and later pause commands stay Original.
    MEM_H(10,int32_t(skip_thread))=0;
    return true;
}
}
