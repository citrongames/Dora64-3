#pragma once
#include "native_frame_time.hpp"
#include "recomp.h"
#include <cstddef>
#include <memory>

namespace dora3::native_time {
// Game-thread-owned adapter. Guest clip time/end flags remain the original
// nondestructive event interface. A frame evaluates the original source curves.
// This is not a physics adapter and does not make an unaudited scene eligible.
class ModelAnimation {
public:
    ModelAnimation();
    ~ModelAnimation();
    ModelAnimation(const ModelAnimation&) = delete;
    void hook(uint8_t*, recomp_context*, uint32_t address, Nanoseconds now);
    void begin_frame(uint8_t*, recomp_context*, Nanoseconds now, unsigned bank);
    void end_frame(uint8_t*);
    void reset(); // Only at a quiescent runtime boundary.
    std::size_t model_count() const;
private:
    struct State;
    std::unique_ptr<State> state_;
};
ModelAnimation& game_model_animation();
class ModelFrameScope {
public:
    ModelFrameScope(uint8_t* ram, recomp_context* ctx, Nanoseconds now, unsigned bank);
    ~ModelFrameScope();
    ModelFrameScope(const ModelFrameScope&) = delete;
private:
    uint8_t* ram_;
};
}
extern "C" void dora3_native_model_hook(uint8_t*, recomp_context*, uint32_t);
