#pragma once
#include <array>
#include <cstdint>
#include <limits>
namespace dora3::minigame {
// Exact update/draw pairs and module ROM identity. VRAM alone is ambiguous.
// Keep the incomplete block out of production Native eligibility. Remove the
// guard only after all nine games/common paths are implemented, before replay.
inline constexpr bool implementation_ready=true;
struct Definition {std::uint32_t rom,update,draw;};
inline constexpr std::array definitions{
    Definition{0x1961B0, 0x801CACAC, 0x801CB160}, // .minigame_1
    Definition{0x19F510, 0x801C7CBC, 0x801C73F4}, // .minigame_2
    Definition{0x1A4600, 0x801C7CA4, 0x801C764C}, // .minigame_3
    Definition{0x1A9E50, 0x801C94D0, 0x801C90C4}, // .minigame_4
    Definition{0x1B0B90, 0x801C67A4, 0x801C6A30}, // .minigame_5
    Definition{0x1B4660, 0x801C69F8, 0x801C7048}, // .minigame_6
    Definition{0x1B8AE0, 0x801C6844, 0x801C6B90}, // .minigame_7
    Definition{0x1BC460, 0x801C7EF4, 0x801C8270}, // .minigame_8
    Definition{0x1C16B0, 0x801C7508, 0x801C7864}, // .minigame_9
    Definition{0x18BDF0, 0x801BD5B8, 0x801BD7A8}, // .minigame_common
    Definition{0x18BDF0, 0x801BE430, 0x801BE990}, // .minigame_common
    Definition{0x18BDF0, 0x801BF8CC, 0x801C1BDC}, // .minigame_common
};
struct MotionBounds {std::uint32_t rom,site;double minimum,maximum;bool clamp;};
inline constexpr double unlimited=std::numeric_limits<double>::infinity();
inline constexpr std::array motion_bounds{
    MotionBounds{0x19F510,0x801C4688,-40,40,true},
    MotionBounds{0x19F510,0x801C46B4,-40,40,true},
    MotionBounds{0x19F510,0x801C46E0,-40,40,true},
    MotionBounds{0x19F510,0x801C470C,-40,40,true},
    MotionBounds{0x19F510,0x801C5150,300,unlimited,true},
    MotionBounds{0x19F510,0x801C5154,10,unlimited,true},
    MotionBounds{0x19F510,0x801C54A0,-unlimited,300,true},
    MotionBounds{0x19F510,0x801C6AA8,-unlimited,40,true},
    MotionBounds{0x19F510,0x801C6B74,0,unlimited,true},
    MotionBounds{0x1A4600,0x801C4C18,1300,unlimited,true},
    MotionBounds{0x1A4600,0x801C4C48,30,unlimited,true},
    MotionBounds{0x1A4600,0x801C54B4,-unlimited,1,false},
    MotionBounds{0x1A4600,0x801C54B8,-unlimited,1,false},
    MotionBounds{0x1A4600,0x801C54BC,-unlimited,1,false},
    MotionBounds{0x1A4600,0x801C68E0,-250,250,true},
    MotionBounds{0x1A4600,0x801C7DC0,0,unlimited,true},
    // C5E1C stops retreat after Source worldX drops below80; preserve its
    // final Source overshoot while preventing another fractional retreat.
    MotionBounds{0x1A4600,0x801C5EE4,80,unlimited,false},
    MotionBounds{0x1B4660,0x801C5F1C,-unlimited,200,false},
    MotionBounds{0x1B8AE0,0x801C4F68,300,unlimited,false},
    MotionBounds{0x1C16B0,0x801C5610,400,unlimited,false},
    MotionBounds{0x1A9E50,0x801C4038,25,unlimited,false},
    MotionBounds{0x1A9E50,0x801C4070,400,unlimited,false},
    MotionBounds{0x1A9E50,0x801C40A4,-unlimited,60,true},
    MotionBounds{0x1BC460,0x801C42F0,-50,unlimited,true},
    MotionBounds{0x1BC460,0x801C6E1C,0,unlimited,true},
};
struct ProjectedPosition {std::uint32_t rom,site;};
inline constexpr std::array projected_positions{
    ProjectedPosition{0x1961B0,0x801C97DC},ProjectedPosition{0x1961B0,0x801C97E4},
    ProjectedPosition{0x1A9E50,0x801C50E0},ProjectedPosition{0x1A9E50,0x801C50E8},
    ProjectedPosition{0x1A9E50,0x801C8908},ProjectedPosition{0x1A9E50,0x801C8928},
};

}
