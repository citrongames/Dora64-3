#pragma once
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>
namespace dora3::localization {
// Reserved inside every Source/Native RAM image; never part of the game heap.
inline constexpr uint32_t arena_begin=0x807F8000u;
struct Language {std::string code,name;};
void initialize(const std::filesystem::path& root,std::span<const char> rom);
const std::vector<Language>& languages();
std::string system_language();
std::string select(const std::string& code);
std::string current_language();
void reset();
void initialize_memory(uint8_t* ram);
}
