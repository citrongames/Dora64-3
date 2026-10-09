#pragma once
#include "librecomp/game.hpp"
void dora3_configure_live_graphics(recomp::Configuration& cfg);
void dora3_drain_live_graphics();

// Guest boundary: no pending Original task; internally takes RDRAM exclusion.
void dora3_materialize_native_framebuffers();
// The CPU framebuffer-copy hook already owns that exclusion.
void dora3_materialize_native_framebuffers_locked();
