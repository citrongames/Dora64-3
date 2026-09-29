#pragma once
#include "librecomp/game.hpp"
#include <SDL.h>
void dora3_prepare_sdl_input();
void dora3_configure_live_input(recomp::Configuration& cfg);
void dora3_poll_live_input(SDL_Window* window);
void dora3_audio_frequency(uint32_t frequency);
void dora3_audio_queue(const int16_t* samples, size_t count);
size_t dora3_audio_remaining();
void dora3_shutdown_live_io();
