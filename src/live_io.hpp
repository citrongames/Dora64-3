#pragma once
#include "librecomp/game.hpp"
#include <SDL.h>
void dora3_prepare_sdl_input();
void dora3_configure_live_input(recomp::Configuration& cfg);
void dora3_poll_live_input(SDL_Window* window);
void dora3_audio_frequency(uint32_t frequency);
void dora3_audio_queue(const int16_t* samples, size_t count);
size_t dora3_audio_remaining();
// Only after guest threads and queued RSP work have stopped.
void dora3_reset_live_io();
void dora3_shutdown_live_io();

// Read-only diagnostic snapshot; uses the existing input mutex, never SDL calls.
struct Dora3LiveInputSnapshot { uint16_t buttons;float x,y;bool enabled; };
Dora3LiveInputSnapshot dora3_live_input_snapshot();
