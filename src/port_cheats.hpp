#pragma once
#include <array>
#include <cstdint>

namespace dora3::port {
enum Cheat { InfiniteHealth, FullRecoveryFruit, RevivalCapsule, EnergyStone,
             GasSpray, MysteriousFruit, SilverKey, GoldKey, CheatCount };
using CheatSettings=std::array<bool,CheatCount>;
inline constexpr std::array<const char*,CheatCount> cheat_keys{
    "infinite_health", "full_recovery_fruit", "revival_capsule", "energy_stone",
    "gas_spray", "mysterious_fruit", "silver_key", "gold_key"};
void configure_cheats(const CheatSettings& settings);
bool cheats_enabled();
}
