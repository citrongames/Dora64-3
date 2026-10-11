#pragma once
#include <filesystem>
namespace dora3::android {
struct Paths {std::filesystem::path data,user,rom;};
Paths prepare();
void shader_loading(bool);
}
