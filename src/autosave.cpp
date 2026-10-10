#include "autosave.hpp"
#include "recomp_context_copy.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>

extern "C" {
uint64_t dora3_overlay_generation(uint32_t);
void dora3_s0_menu_80043800(uint8_t*,recomp_context*);
void dora3_s0_menu_80043F1C(uint8_t*,recomp_context*);
}
namespace {
std::atomic_bool enabled{true};
// Positive deadline: saved; negative deadline: EEPROM write failed.
std::atomic<int64_t> notification_deadline{0};
// Only the original game thread accesses visit state. Native visual frames do
// not call these hooks. Reset runs after the previous guest threads stop.
int pending_slot=-1;
uint64_t pending_generation=0;
int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
int32_t word(uint8_t* rdram,uint32_t address) { return MEM_W(0,int32_t(address)); }
bool map_active(uint8_t* rdram) {
    constexpr uint32_t processes=0x801A44F0;
    // 800280DC installs both callbacks with state3 (80028040), which the
    // dispatcher promotes to state2 on the next matching phase. At map entry
    // they are still deferred; the draw phase may also lag the first update.
    const int update_state=word(rdram,processes),draw_state=word(rdram,processes+20);
    return (update_state==2 || update_state==3) && (draw_state==2 || draw_state==3) &&
        word(rdram,processes+12)==0 && word(rdram,processes+32)==1 &&
        uint32_t(word(rdram,processes+16))==0x801BB52C &&
        uint32_t(word(rdram,processes+36))==0x801BB648;
}
bool slot_valid(uint8_t* rdram,int slot) {
    // RAM progress can be newer than its checksum; require the occupied marker,
    // not a stale packed-slot checksum. Original packing regenerates it below.
    return slot>=0 && slot<3 && MEM_BU(0,int32_t(0x80087A28+slot*0x200))==1 &&
        MEM_BU(0,int32_t(0x801B3763))==0; // Never save a service/debug preset.
}
}
void dora3::autosave::configure(bool value) { enabled.store(value); }
void dora3::autosave::reset() {
    pending_slot=-1;pending_generation=0;notification_deadline.store(0);
}
dora3::autosave::Notification dora3::autosave::notification() {
    const auto deadline=notification_deadline.load();
    if(deadline>now_ms())return Notification::Saved;
    if(deadline<0 && -deadline>now_ms())return Notification::Failed;
    return Notification::None;
}
extern "C" void dora3_autosave_map_begin(uint8_t* rdram,recomp_context*) {
    pending_slot=-1;pending_generation=0;
    const auto generation=dora3_overlay_generation(0x635C0);
    const int slot=word(rdram,0x801B3698);
    if(generation && map_active(rdram) && slot_valid(rdram,slot)) {
        pending_slot=slot;pending_generation=generation;
    }
}
extern "C" void dora3_autosave_map_ready(uint8_t* rdram,recomp_context* ctx) {
    if(pending_slot<0)return;
    const int slot=pending_slot;
    // Consume even when disabled: toggling the option or moving on this map
    // must not create another save. A new map initialization arms the next visit.
    pending_slot=-1;
    if(!enabled.load() || !pending_generation ||
       pending_generation!=dora3_overlay_generation(0x635C0) ||
       !map_active(rdram) || word(rdram,0x801B3698)!=slot || !slot_valid(rdram,slot) ||
       word(rdram,0x801C636C)!=0 || word(rdram,0x801C63A0)!=0 || word(rdram,0x801C638C)!=0)return;
    // Same pair as the map's Yes confirmation (801BAB90), with a private CPU
    // register image so the input handler resumes with its original registers.
    recomp_context save{};dora3::copy_recomp_context(save,*ctx);
    save.r4=slot;dora3_s0_menu_80043800(rdram,&save);
    save.r4=slot;dora3_s0_menu_80043F1C(rdram,&save);
    const int64_t deadline=now_ms()+2000;
    notification_deadline.store(int32_t(save.r2)==0?deadline:-deadline);
    // Success means accepted by the ordinary EEPROM runtime. Its existing
    // asynchronous disk writer retains backup/error reporting responsibility.
}
