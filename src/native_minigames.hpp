#pragma once
#include "native_frame_time.hpp"
#include "recomp.h"
#include <cstdint>
#include <optional>

bool native_minigame_eligible(std::uint8_t*);
// Exact loaded ROM module, including the shared selection/results module.
std::uint32_t native_minigame_module(std::uint8_t*);
void native_minigame_begin(std::uint8_t*,dora3::native_time::Nanoseconds,unsigned);
void native_minigame_update(std::uint8_t*,recomp_context*,dora3::native_time::Nanoseconds);
void native_minigame_reset(std::uint8_t* =nullptr);
std::optional<dora3::native_time::Nanoseconds> native_minigame_render_time();
extern "C" void dora3_native_minigame_source(std::uint8_t*,recomp_context*,std::uint32_t,std::uint32_t,unsigned);
extern "C" void dora3_native_minigame_instruction(std::uint8_t*,recomp_context*,std::uint32_t,std::uint32_t,std::uint32_t,unsigned,std::uint32_t);
extern "C" void dora3_native_minigame_call_return(std::uint8_t*,recomp_context*,std::uint32_t,std::uint32_t,unsigned);

namespace dora3::native_time {
// SDK draw helpers recompute actor matrices/world coordinates. Their temporary
// derivatives belong to this frame; input/physics must see the Source values.
class MinigameFrameScope {
public:
    explicit MinigameFrameScope(std::uint8_t*);
    ~MinigameFrameScope();
    MinigameFrameScope(const MinigameFrameScope&)=delete;
private:
    std::uint8_t* ram_=nullptr;
};
}
extern "C" int dora3_native_minigame_drawing();
extern "C" double dora3_native_minigame_visual(std::uint8_t*,std::uint32_t);

extern "C" void dora3_native_minigame_ground(std::uint8_t*,recomp_context*,std::uint32_t,unsigned);
