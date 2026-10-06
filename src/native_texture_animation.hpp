#pragma once
#include "native_frame_time.hpp"
#include "recomp.h"
namespace dora3::native_time {
void native_texture_source_completed(uint8_t*,Nanoseconds);
void native_texture_reset();
class TextureFrameScope {
    uint8_t* ram_=nullptr;
public:
    TextureFrameScope(uint8_t*,Nanoseconds,unsigned);
    ~TextureFrameScope();
    TextureFrameScope(const TextureFrameScope&)=delete;
};
}
extern "C" void dora3_native_texture_bind(uint8_t*,recomp_context*,unsigned);
extern "C" void dora3_native_texture_builder(uint8_t*,recomp_context*,unsigned);
extern "C" void dora3_native_texture_submit_guard(uint8_t*,recomp_context*);
