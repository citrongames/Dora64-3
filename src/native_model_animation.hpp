#pragma once
#include "native_frame_time.hpp"
#include "native_simulation_time.hpp"
#include "recomp.h"
#include <cstddef>
#include <memory>
#include <optional>
#include <array>

namespace dora3::native_time {
struct SourceMotionSample {
    std::array<float,3> position{},velocity{},bend{};
    float yaw=0,pitch=0,oscillator=0,pitch_offset=0;
    uint64_t start_phase=0;
};
// Game-thread-owned adapter. Guest clip time/end flags remain the original
// nondestructive event interface. A frame evaluates the original source curves.
// This is not a physics adapter and does not make an unaudited scene eligible.
class ModelAnimation {
public:
    ModelAnimation();
    ~ModelAnimation();
    ModelAnimation(const ModelAnimation&) = delete;
    void hook(uint8_t*, recomp_context*, uint32_t address, Nanoseconds now);
    void advance_simulation(uint8_t*,recomp_context*);
    // Split continuous updates at a real clip boundary. A dependent action
    // then observes the end pulse before integrating the remaining interval.
    // The optional plan identifies only models actually advanced by this
    // interval, with their current rate (including newly bound clips).
    using AdvancePlan = std::function<std::optional<float>(std::uint32_t)>;
    // Reviewed owners can acquire the last Original pose/time observation.
    // Original evaluates a pose before advancing its counter; that lookahead
    // must not shorten the remaining lifetime at the ownership boundary.
    void acquire_simulation(uint8_t*,const AdvancePlan&,bool replace_timeline=true);
    std::uint64_t clip_boundary_limit(uint8_t*,const SimulationSlice&,const AdvancePlan& = {}) const;
    std::uint64_t frame_boundary_limit(uint8_t*,const SimulationSlice&,uint32_t,float,float) const;
    bool frame_event(uint8_t*,uint32_t,float,float,uint64_t,bool source_start_consumed=false);
    // Reviewed actor scalars share the registered model's allocation lifetime.
    // Decisions retain the source-tick baseline; values advance from that
    // baseline with one guest float rounding instead of accumulating substeps.
    bool scalar_positive(uint8_t*,uint32_t,uint32_t,float);
    float scalar_linear(uint8_t*,uint32_t,uint32_t,float,double);
    float scalar_previous(uint8_t*,uint32_t,uint32_t,float,double);
    // The guest keeps whole degrees in a short. Preserve its fractional
    // oscillator phase within a tick, without losing it at every truncation.
    float angle_short(uint8_t*,uint32_t,uint32_t,float);
    float angle_short_sample(uint8_t*,uint32_t,uint32_t) const;
    bool native_birth(uint32_t) const;
    float scalar_latched_turn(uint8_t*,uint32_t,uint32_t,float,float);
    // A reviewed projectile's motion sample shares its model allocation and
    // acquisition lifetime. The caller supplies geometry; this class owns
    // only the source interval and detects commands, gaps and heap resets.
    const SourceMotionSample& motion_begin(uint8_t*,uint32_t,uint32_t,uint32_t,
                                         const std::function<SourceMotionSample()>&);
    const SourceMotionSample* motion_current(uint32_t,uint32_t) const;
    void motion_end(uint8_t*,uint32_t,uint32_t);
    void bind_actor(uint8_t*,uint32_t,uint32_t,uint32_t);
    bool bound_actor_valid(uint8_t*,uint32_t,uint32_t,uint32_t) const;
    void begin_frame(uint8_t*, recomp_context*, Nanoseconds now, unsigned bank);
    void end_frame(uint8_t*);
    void reset(); // Only at a quiescent runtime boundary.
    std::size_t model_count() const;
    std::uint64_t lifetime_token(std::uint32_t) const;
    // A seek/rebind or a real flag command starts another animation run.
    std::uint64_t run_token(std::uint32_t) const;
    // Reviewed discrete owners can consume a crossing at their source end
    // without changing the raw end flag's one-advance pulse semantics.
    bool ended_this_tick(std::uint32_t) const;
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

extern "C" int dora3_native_model_advance(uint8_t*,recomp_context*);

extern "C" int dora3_native_model_event(uint8_t*,uint32_t,float,float,uint64_t);
extern "C" int dora3_native_model_start_event(uint8_t*,uint32_t,float,float,uint64_t);
extern "C" int dora3_native_model_scalar_positive(uint8_t*,uint32_t,uint32_t,float);
extern "C" float dora3_native_model_scalar_linear(uint8_t*,uint32_t,uint32_t,float,double);
extern "C" float dora3_native_model_scalar_previous(uint8_t*,uint32_t,uint32_t,float,double);
extern "C" float dora3_native_model_angle_short(uint8_t*,uint32_t,uint32_t,float);
extern "C" float dora3_native_model_latched_turn(uint8_t*,uint32_t,uint32_t,float,float);
