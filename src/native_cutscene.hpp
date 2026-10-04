#pragma once
#include "native_frame_time.hpp"
#include "recomp.h"
#include <memory>
#include <optional>

namespace dora3::native_time {
// Source owns the interpreter, waits, input, allocations and transitions.
// Native evaluates the reviewed motion/camera equations in an isolated guest
// image and publishes transforms/visual alpha. Source commands read Source state.
class CutsceneTimeline {
public:
    using Evaluate = void (*)(uint8_t*,recomp_context*);
    CutsceneTimeline();
    ~CutsceneTimeline();
    CutsceneTimeline(const CutsceneTimeline&)=delete;
    void begin(uint8_t*,Nanoseconds,unsigned);
    void advance(uint8_t*,recomp_context*,Nanoseconds,Evaluate);
    void before_source(uint8_t*);
    void after_source(uint8_t*,Nanoseconds);
    void reset(uint8_t* =nullptr);
    std::optional<Nanoseconds> render_time() const;
private:
    struct State;
    std::unique_ptr<State> state_;
};
}

bool native_cutscene_eligible(uint8_t*);
void native_cutscene_begin(uint8_t*,dora3::native_time::Nanoseconds,unsigned);
void native_cutscene_update(uint8_t*,recomp_context*,dora3::native_time::Nanoseconds);
void native_cutscene_reset(uint8_t* =nullptr);
std::optional<dora3::native_time::Nanoseconds> native_cutscene_render_time();
void native_cutscene_evaluate(uint8_t*,recomp_context*);
extern "C" void dora3_native_cutscene_source(uint8_t*,recomp_context*,unsigned);
