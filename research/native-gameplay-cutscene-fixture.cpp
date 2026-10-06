// Gameplay-only fixtures link the real cutscene runtime for shared helpers.
// Scripted ownership is inactive; reaching either script-only root is fatal.
#include "recomp.h"
#include <cstdlib>
#ifndef DORA3_FIXTURE_REAL_LOOP
extern "C" int dora3_native_loop_cutscene_active(){return 0;}
#endif
extern "C" void dora3_s3_801E3768(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s3_801E0760(uint8_t*,recomp_context*){std::abort();}


extern "C" {
void dora3_native_minigame_source(uint8_t*,recomp_context*,uint32_t,uint32_t,unsigned){}
void dora3_native_minigame_instruction(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t,unsigned,uint32_t){}
void dora3_native_minigame_call_return(uint8_t*,recomp_context*,uint32_t,uint32_t,unsigned){}
void dora3_native_minigame_ground(uint8_t*,recomp_context*,uint32_t,unsigned){}
void dora3_native_minigame_parameter(uint8_t*,recomp_context*){}
void dora3_native_minigame_counter(uint8_t*,recomp_context*,uint32_t,double){}
void dora3_native_minigame_indirect(uint8_t*,recomp_context*,uint32_t,unsigned){}
int dora3_native_minigame_drawing(){return 0;}
double dora3_native_minigame_visual(uint8_t*,uint32_t){return 0;}
}

#include "native_minigames.hpp"
bool native_minigame_eligible(uint8_t*){return false;}
void native_minigame_begin(uint8_t*,dora3::native_time::Nanoseconds,unsigned){}
void native_minigame_update(uint8_t*,recomp_context*,dora3::native_time::Nanoseconds){}
void native_minigame_reset(uint8_t*){}
std::optional<dora3::native_time::Nanoseconds> native_minigame_render_time(){return std::nullopt;}

void native_minigame_source_completed(uint8_t*,dora3::native_time::Nanoseconds){}
