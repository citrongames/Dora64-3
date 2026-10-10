#pragma once
#include <cstdint>
namespace dora3::wide {
void begin(uint8_t* ram);
void reset();
int aspect();
int hud();
}
