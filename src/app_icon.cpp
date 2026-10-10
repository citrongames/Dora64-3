#include "app_icon.hpp"
#include "dora3_icon.h"
#include "stb/stb_image.h"
#include <SDL.h>
#include <cstdio>

void dora3_set_window_icon(SDL_Window* window) {
    int width=0,height=0;
    auto* pixels=stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(dora3_icon_png),
        static_cast<int>(dora3_icon_png_size),&width,&height,nullptr,4);
    if(!pixels) {
        std::fprintf(stderr,"Could not decode application icon: %s\n",stbi_failure_reason());
        return;
    }
    auto* surface=SDL_CreateRGBSurfaceWithFormatFrom(
        pixels,width,height,32,width*4,SDL_PIXELFORMAT_RGBA32);
    if(surface) {
        SDL_SetWindowIcon(window,surface);
        SDL_FreeSurface(surface);
    } else {
        std::fprintf(stderr,"Could not create application icon: %s\n",SDL_GetError());
    }
    stbi_image_free(pixels);
}
