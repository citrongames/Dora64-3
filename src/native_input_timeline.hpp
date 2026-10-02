#pragma once
#include "recomp.h"
#include "native_frame_time.hpp"
#include <array>
#include <cstdint>

// Game-thread input ownership. SI continues to sample on real VI; native
// updates consume edges once, independently of the original update cadence.
void native_input_begin(uint8_t*,dora3::native_time::Nanoseconds =0);
void native_input_reset();
void native_input_end(uint8_t*);
dora3::native_time::Nanoseconds native_input_next_sample(dora3::native_time::Nanoseconds,dora3::native_time::Nanoseconds);
extern "C" void dora3_native_input_sample(uint8_t*,recomp_context*);
class NativeInputScope {
public:
    explicit NativeInputScope(uint8_t*);
    NativeInputScope(uint8_t*,dora3::native_time::Nanoseconds);
    ~NativeInputScope();
    NativeInputScope(const NativeInputScope&)=delete;
private:
    uint8_t* ram_;
    uint64_t epoch_;
    std::array<int16_t,36> words_{};
    std::array<int8_t,8> sticks_{};
};
