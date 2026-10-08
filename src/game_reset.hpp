#pragma once
#include "recomp.h"
namespace dora3 {
// UI publishes intent only; guest state is touched at the message-loop boundary.
void request_game_reset();
}
extern "C" void dora3_game_reset_poll(uint8_t*, recomp_context*);
