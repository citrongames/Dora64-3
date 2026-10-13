#pragma once
#include "native_frame_time.hpp"
#include <cstdint>
#include <memory>
#include <array>
#include <vector>
#include <functional>

namespace dora3::native_time {
// A typed expression records reviewed Original arithmetic. Commands, integer
// state and resources stay in the guest update; this ledger publishes only
// explicitly owned floating-point motion fields between those updates.
class MinigameEquations {
public:
    using Expression=std::uint32_t;
    enum class Operation : std::uint8_t {
        Add,Subtract,Multiply,Divide,Negate,Absolute,SquareRoot,
        Sine,Cosine,AngleWrap,AngleDelta,RoundFloat,Heading
    };
    enum class FieldRole : std::uint8_t {Scratch,Motion,Angle,Derived,Command};
    struct Lifetime {
        std::uint32_t actor=0,identity=0,model=0,model_slot=0,resources=0,driver=0;
        std::uint64_t allocation=0,scene=0;
        std::array<std::uint32_t,7> components{};
        std::array<std::uint8_t,7> types{};
        std::array<std::uint64_t,7> component_allocations{};
    };
    MinigameEquations();
    ~MinigameEquations();
    MinigameEquations(const MinigameEquations&)=delete;
    void begin_source(std::uint8_t*,std::uint32_t module,std::uint64_t scene,
                      Nanoseconds,unsigned divider);
    Expression constant(double);
    // External time leaves resolve a registered clip's real Source equation,
    // never a previously drawn pose. Unsupported/unowned clocks return NaN.
    Expression time_operand(double source_value,std::function<double(double)>);
    Expression load_float(std::uint32_t,float);
    Expression operation(Operation,Expression,Expression=0);
    void bind_snapshot(Expression,std::uint32_t snapshot,std::uint32_t field);
    void store_float(std::uint32_t,Expression,float,FieldRole,const Lifetime&);
    // Explicit reviewed x'=x+rate publication. The rate retains its recorded
    // acceleration/heading dependencies; no ready pose pair is interpolated.
    void store_motion_from_rate(std::uint32_t,Expression,float,const Lifetime&);
    // C'=C+gain*shortest(target-C). The four reviewed camera helpers choose
    // the next pure90-degree gain branch from completed C/target equations;
    // that coefficient stays fixed for the interval. The target is lifted once;
    // moving target equations stay continuous across their0/360 seam.
    void store_circular_approach(std::uint32_t,Expression target,float source_result,
                                 double gain,const Lifetime&);
    // A raw setter/zero store cancels a previously recorded recurrence.
    // Bounds mirror reviewed pre/post arithmetic conditions. Preconditions
    // stop a completed step without suppressing Source's intentional overshoot.
    void constrain_float(std::uint32_t,double minimum,double maximum,bool clamp);
    std::vector<std::uint32_t> changed_fields() const;
    void command_write(std::uint32_t,unsigned bytes=4);
    std::vector<std::uint32_t> changed_local_actors() const;
    void end_source(std::uint8_t*,Nanoseconds);
    // Refresh a Source-owned copy after a pure Native geometry query. The
    // Source value/foreign-write guard remain exact across frame borrowing.
    bool publish_derived_float(std::uint8_t*,std::uint32_t,float);
    void sample(std::uint8_t*,Nanoseconds);
    // Rebase a completed Source observation at actual session activation.
    // It retains the equation and lifetime; it never advances a command.
    void resume_after_handoff(std::uint8_t*,Nanoseconds);
    void restore_source(std::uint8_t*);
    void reset(std::uint8_t* =nullptr);
    bool recording() const;
    std::uint32_t module() const;
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
