#pragma once
#include <cstdint>
namespace dora3 {
// Scheduled Source thread only; cold boot discards any unfinished request.
void cutscene_input_before_script(uint8_t* ram);
bool cutscene_input_source_completed(uint8_t* ram);
void reset_cutscene_input();
}
