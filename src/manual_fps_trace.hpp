#pragma once
#include "recomp.h"
#include <cstdint>
namespace dora3::manual_trace {
#ifdef DORA3_CRASH_LOG
bool gate(unsigned channel,unsigned slot=0,std::uint64_t interval_us=0,unsigned limit=12000);
void log(const char*,...);
void settings(int mode,int fps,int aspect,int hud,const char* reason);
void frame(std::uint8_t*,const char* stage);
void scope(std::uint8_t*,recomp_context*,unsigned role,unsigned origin,unsigned aspect);
void display_list(std::uint8_t*,std::uint32_t begin,std::uint32_t end);
#else
inline bool gate(unsigned,unsigned=0,std::uint64_t=0,unsigned=12000){return false;}
inline void settings(int,int,int,int,const char*){}
inline void frame(std::uint8_t*,const char*){}
inline void scope(std::uint8_t*,recomp_context*,unsigned,unsigned,unsigned){}
inline void display_list(std::uint8_t*,std::uint32_t,std::uint32_t){}
#endif
}
