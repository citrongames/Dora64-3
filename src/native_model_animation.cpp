#include "recomp_context_copy.hpp"
#include "native_model_animation.hpp"
#include "native_simulation_time.hpp"
#include <limits>
#include <algorithm>
#include "native_frame_bridge.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <optional>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern "C" void dora3_s0_800393C4(uint8_t*, recomp_context*);
extern "C" void dora3_s0_8003931C(uint8_t*, recomp_context*);
extern "C" int dora3_native_loop_owns_frame_banks();
namespace dora3::native_time {
namespace {
int32_t& word(uint8_t* rdram,uint32_t a) { return MEM_W(0,int32_t(a)); }
float real(uint8_t* rdram,uint32_t a) { return std::bit_cast<float>(word(rdram,a)); }
void real(uint8_t* rdram,uint32_t a,float value) { word(rdram,a)=std::bit_cast<int32_t>(value); }
void region(uint32_t a,uint64_t size) {
    if((a&3) || a<0x80000000u || uint64_t(a)+size>0x80400000ull)
        throw std::logic_error("Model animation address outside game RDRAM");
}
struct Bone {
    uint32_t address;
    std::array<int32_t,9> transform{};
};
struct Observation {
    AnimationState before;
    int32_t expected_time;
    uint64_t step;
    unsigned divider;
    Nanoseconds origin;
    uint32_t channels,descriptors;
};
struct NativeAnimationClock {
    long double time;
    long double origin=0;
    uint64_t elapsed=0,denominator=0;
    float rate=0;
    int32_t expected_time;
    uint32_t channels,descriptors;
    float duration;
    uint8_t expected_flags;
    std::optional<uint64_t> normalization_tick{};
    std::optional<uint64_t> end_tick{};
    std::optional<SimulationSlice> last_slice{};
};
struct EventCursor {
    uint64_t sequence,tick,phase,numerator,denominator;
    bool result;
};
struct NativeScalarClock {
    uint64_t tick,denominator,start_phase,expected_phase;
    float baseline;
    int32_t expected_value;
    std::optional<double> increment{};
};
struct NativeMotionClock {
    uint64_t tick,denominator,expected_phase,open_phase;
    uint32_t physical;
    SourceMotionSample sample;
    std::array<int32_t,11> expected;
    bool open=false;
};
std::array<int32_t,11> motion_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    region(actor,0xCC);region(physical,0xDC);
    return {word(rdram,actor+0x9C),word(rdram,actor+0xA0),word(rdram,actor+0xA4),
            word(rdram,physical),word(rdram,physical+4),word(rdram,physical+8),
            word(rdram,physical+0xC),word(rdram,physical+0x58),
            MEM_H(0x34,int32_t(physical)),MEM_H(0x36,int32_t(physical)),MEM_BU(0xC9,int32_t(physical))};
}
struct Model {
    uint64_t time_command=0;
    uint64_t lifetime=0,run=0;
    std::vector<Bone> bones;
    std::optional<Observation> observation;
    std::optional<float> last_pose;
    bool sampled=false;
    bool native_pose=false;
    bool native_born=false;
    bool acquired_start=false;
    std::optional<NativeAnimationClock> native_clock;
    std::optional<SimulationSlice> source_step{},source_command_hold{};
    uint64_t source_pool=0;
    uint64_t event_sequence=0,end_sequence=0;
    long double event_from=0,event_to=0,event_distance=0;
    bool event_clamp=false;
    std::map<std::pair<uint64_t,uint32_t>,EventCursor> events;
    std::unordered_map<uint32_t,NativeScalarClock> scalars;
    std::unordered_map<uint32_t,NativeScalarClock> angles;
    std::unordered_map<uint32_t,NativeMotionClock> motions;
    struct ActorBinding {uint32_t actor,physical,model;uint64_t lifetime;};
    std::unordered_map<uint32_t,ActorBinding> bindings;
};
NativeScalarClock& scalar_clock(uint8_t* rdram,Model& entry,uint32_t address,float current,const SimulationSlice& slice) {
    region(address,4);
    const auto bits=std::bit_cast<int32_t>(current);
    if(!std::isfinite(current) || word(rdram,address)!=bits)
        throw std::logic_error("Invalid native model scalar observation");
    auto i=entry.scalars.find(address);
    if(i==entry.scalars.end() || i->second.tick!=slice.tick ||
       i->second.denominator!=slice.denominator || i->second.expected_phase!=slice.phase ||
       i->second.expected_value!=bits) {
        i=entry.scalars.insert_or_assign(address,NativeScalarClock{
            slice.tick,slice.denominator,slice.phase,slice.phase,current,bits}).first;
    }
    return i->second;
}
void restore(uint8_t* rdram,Model& entry) {
    if(!entry.sampled)return;
    for(const auto& bone:entry.bones)
        for(unsigned k=0;k<9;++k)word(rdram,bone.address+0x14+4*k)=bone.transform[k];
    entry.sampled=false;
}
std::vector<Bone> find_bones(uint8_t* rdram,uint32_t model) {
    std::vector<Bone> result;
    std::vector<uint32_t> todo{uint32_t(word(rdram,model+0x5C))};
    std::unordered_set<uint32_t> visited;
    while(!todo.empty()) {
        auto bone=todo.back();todo.pop_back();region(bone,0x84);
        if(!visited.insert(bone).second)throw std::logic_error("Cyclic/aliased model bone tree");
        result.push_back({bone,{}});
        const auto count=word(rdram,bone+0x10);
        if(count<0)throw std::logic_error("Negative model child count");
        if(count) {
            const uint32_t children=word(rdram,bone+0xC);region(children,uint64_t(count)*4);
            for(int i=0;i<count;++i)todo.push_back(uint32_t(word(rdram,children+unsigned(i)*4)));
        }
    }
    return result;
}
}
struct ModelAnimation::State {
    uint64_t next_lifetime=0,next_run=0;
    std::unordered_map<uint32_t,Model> models;
    uint64_t step=0;
    Nanoseconds origin=0;
    unsigned divider=0;
    bool frame=false;
    uint32_t source_actor=0;uint64_t source_pool=0;
};
ModelAnimation::ModelAnimation():state_(std::make_unique<State>()) {}
ModelAnimation::~ModelAnimation()=default;
void ModelAnimation::begin_source_pool(){++state_->source_pool;}
uint32_t ModelAnimation::source_command_actor(uint32_t actor) {
    const auto prior=state_->source_actor;state_->source_actor=actor;return prior;
}
std::size_t ModelAnimation::model_count() const { return state_->models.size(); }
uint64_t ModelAnimation::lifetime_token(uint32_t model) const {
    const auto owner=state_->models.find(model);
    return owner==state_->models.end()?0:owner->second.lifetime;
}
uint64_t ModelAnimation::run_token(uint32_t model) const {
    const auto owner=state_->models.find(model);
    return owner==state_->models.end()?0:owner->second.run;
}
uint64_t ModelAnimation::time_command_token(uint32_t model) const {
    const auto owner=state_->models.find(model);
    return owner==state_->models.end()?0:owner->second.time_command;
}
bool ModelAnimation::ended_this_tick(uint32_t model) const {
    const auto* slice=active_simulation_slice();const auto owner=state_->models.find(model);
    return slice && owner!=state_->models.end() && owner->second.native_clock &&
        owner->second.native_clock->end_tick==slice->tick;
}
bool ModelAnimation::command_ended(uint32_t model,const ModelEventCheckpoint& checkpoint) const {
    const auto i=state_->models.find(model);
    return i!=state_->models.end()&&i->second.run==checkpoint.run&&i->second.end_sequence!=checkpoint.ends;
}
ModelEventCheckpoint ModelAnimation::command_event_checkpoint(uint8_t* rdram,uint32_t model) const {
    const auto i=state_->models.find(model);if(i==state_->models.end())return {};
    return {i->second.run,i->second.event_distance,real(rdram,model+0x64),i->second.end_sequence};
}
SourceModelCommandSample ModelAnimation::source_command_sample(uint8_t* rdram,uint32_t model,const SimulationSlice& slice) const {
    SourceModelCommandSample result{command_event_checkpoint(rdram,model),uint8_t(MEM_BU(0x7C,int32_t(model)))};
    result.checkpoint.time=real(rdram,model+0x64);
    const auto i=state_->models.find(model);
    if(i==state_->models.end()||!i->second.native_clock)return result;
    const auto& entry=i->second;const auto& clock=*entry.native_clock;
    // A linked reader has its own birth-local clock. Project the model's
    // actual Source interval, which may differ from that reader's phase.
    const auto& observed=clock.last_slice?*clock.last_slice:slice;
    const auto end=observed.phase+observed.numerator;
    if(!observed.denominator||end>observed.denominator||clock.denominator!=observed.denominator||
       clock.expected_time!=word(rdram,model+0x64)||clock.expected_flags!=result.flags||
       (result.flags&8)||clock.channels!=uint32_t(word(rdram,model+0x60))||!clock.channels||
       clock.descriptors!=uint32_t(word(rdram,model+0x18))||
       clock.duration!=real(rdram,clock.descriptors+4))return result;
    const long double remaining=observed.denominator-end;
    auto position=[&](long double elapsed) {
        const long double value=clock.origin+static_cast<long double>(clock.rate)*elapsed/clock.denominator;
        const long double edge=std::round(value/clock.duration)*clock.duration;
        const long double tolerance=8*std::numeric_limits<long double>::epsilon()*std::max(1.0L,std::abs(value));
        return std::abs(value-edge)<=tolerance?edge:value;
    };
    const auto current=position(clock.elapsed),future=position(clock.elapsed+remaining);
    const auto start=position(static_cast<long double>(clock.elapsed)-end);
    const bool clamped=(result.flags&1)&&clock.rate>=0;
    auto cycle=std::floor(start/clock.duration);
    if(clock.rate<0&&start==clock.duration)cycle-=1;
    const bool ended=clock.normalization_tick==observed.tick||cycle!=std::floor(future/clock.duration)||
        (clamped&&future>=clock.duration);
    const bool future_end=std::floor(current/clock.duration)!=std::floor(future/clock.duration)||
        (clamped&&future>=clock.duration&&remaining>0);
    result.checkpoint.distance+=clamped?std::min(future,static_cast<long double>(clock.duration))-
        std::min(current,static_cast<long double>(clock.duration)):future-current;
    long double time=std::fmod(future,clock.duration);if(time<0)time+=clock.duration;
    if(clamped&&future>=clock.duration)time=clock.duration;
    result.checkpoint.time=float(time);
    if(future_end)++result.checkpoint.ends;
    result.flags=uint8_t((result.flags&~2u)|(ended?2u:0u));
    return result;
}
bool ModelAnimation::command_frame_event(uint8_t* rdram,uint32_t model,const ModelEventCheckpoint& checkpoint,
                                        float observed,float target,uint64_t site) const {
    const auto i=state_->models.find(model);
    if(i==state_->models.end()||i->second.run!=checkpoint.run||!i->second.native_clock||
       std::bit_cast<int32_t>(observed)!=word(rdram,model+0x64))return observed==target;
    const auto& entry=i->second;const auto& clock=*entry.native_clock;
    if(!std::isfinite(target)||target<0||target>clock.duration)return false;
    const long double start=checkpoint.time,end=start+entry.event_distance-checkpoint.distance,duration=clock.duration;
    (void)site;
    if(observed==target)return true;
    if(entry.event_clamp)return start<target&&end>=target;
    if(target==duration)return false;
    if(end>start)return std::floor((end-target)/duration)>std::floor((start-target)/duration);
    if(end<start)return std::ceil((start-target)/duration)>std::ceil((end-target)/duration);
    return false;
}
void ModelAnimation::reset() {
    if(state_->frame)throw std::logic_error("Model reset during active frame");
    *state_=State{};
}
void ModelAnimation::hook(uint8_t* rdram,recomp_context* ctx,uint32_t address,Nanoseconds now) {
    auto& s=*state_;
    const uint32_t model=uint32_t(ctx->r4);
    if(address==0x800393C4) {
        // The adapter calls this same original routine to evaluate a frame;
        // only a real game pose establishes the baseline for paused/stale models.
        if(!s.frame)if(auto i=s.models.find(model);i!=s.models.end()) {
            i->second.last_pose=real(rdram,model+0x64);
            i->second.native_pose=active_simulation_slice()!=nullptr;
        }
        return;
    }
    // Minigame4 destroys models from phase1. Retire its temporary pose before
    // the original destructor frees CPU bones; GPU vertices use the existing
    // deferred-retirement path. Other draw-time commands are not yet reviewed.
    if(s.frame && address!=0x8003B77C && address!=0x8003B744)
        throw std::logic_error("Unreviewed animation command inside native draw");
    switch(address) {
    case 0x80030850: // Before phase0; runs once per real original scene tick.
        ++s.step;s.origin=now;s.divider=uint32_t(word(rdram,0x801A39DC));return;
    case 0x8003B7D0:
        // Original resets renderer/retirement bookkeeping, not the heap or
        // bone allocations. Invalidate clocks without forgetting live owners.
        for(auto& [owner,entry]:s.models) {(void)owner;entry.observation.reset();entry.last_pose.reset();entry.native_clock.reset();entry.events.clear();entry.event_sequence=0;entry.scalars.clear();entry.angles.clear();entry.motions.clear();entry.bindings.clear();}
        return;
    case 0x80033A14: { // Region reset can coexist with other live heaps/models.
        const uint64_t end=uint64_t(model)+uint32_t(ctx->r5);
        if(uint32_t(ctx->r5)<0x20)return; // Original rejects this request.
        auto overlaps=[&](uint32_t address,uint32_t size) {
            return uint64_t(address)<end && uint64_t(address)+size>model;
        };
        for(auto i=s.models.begin();i!=s.models.end();) {
            bool invalidated=overlaps(i->first,0x84);
            for(const auto& bone:i->second.bones)invalidated|=overlaps(bone.address,0x84);
            // Resource/channel memory may be in a different resettable heap.
            for(unsigned offset:{0x00u,0x04u,0x14u,0x18u,0x1Cu,0x20u,0x24u,0x60u,0x6Cu,0x70u})
                invalidated|=overlaps(uint32_t(word(rdram,i->first+offset)),1);
            if(invalidated)i=s.models.erase(i);
            else {
                std::erase_if(i->second.scalars,[&](const auto& value){return overlaps(value.first,4);});
                std::erase_if(i->second.angles,[&](const auto& value){return overlaps(value.first,2);});
                std::erase_if(i->second.motions,[&](const auto& value){return overlaps(value.first,0xCC)||overlaps(value.second.physical,0xDC);});
                std::erase_if(i->second.bindings,[&](const auto& value){const auto& b=value.second;return overlaps(value.first,4)||overlaps(b.actor,0xCC)||overlaps(b.physical,0xDC)||overlaps(b.model,0x84);});
                ++i;
            }
        }
        return;
    }
    case 0x8003B6B4: { // Constructor has completed; same pointer is a new lifetime.
        const auto created=uint32_t(ctx->r2);region(created,0x84);
        Model fresh;fresh.lifetime=++s.next_lifetime;fresh.run=++s.next_run;fresh.bones=find_bones(rdram,created);
        fresh.native_born=active_simulation_slice()!=nullptr || dora3_native_loop_owns_frame_banks();
        s.models.insert_or_assign(created,std::move(fresh));return;
    }
    case 0x8003B77C:
        if(auto i=s.models.find(model);i!=s.models.end()) {restore(rdram,i->second);s.models.erase(i);}
        return;
    case 0x80039578: case 0x800395BC: {
        // Set/clear of an already matching flag is not a new animation run.
        // Several action handlers repeat these commands throughout their wait.
        const uint8_t bit=address==0x80039578?8:1;
        const auto command=uint32_t(ctx->r5);
        if(command<=1 && bool(MEM_BU(0x7C,int32_t(model))&bit)==bool(command))return;
        [[fallthrough]];
    }
    case 0x8003B6BC: case 0x8003B744: // New/released channels, even identical pointers.
    case 0x80039564: // An explicit seek starts a new run, even to the same time.
        if(auto i=s.models.find(model);i!=s.models.end()) {
            auto& entry=i->second;
            bool processed=false;
            if(s.source_actor&&entry.source_step&&entry.source_pool==s.source_pool)for(unsigned slot=30;slot<230;++slot) {
                const uint32_t actor=0x80171CD0u+slot*0xCC;
                if(actor>s.source_actor)break;
                if(word(rdram,actor+0x94)&&uint32_t(word(rdram,actor+0x74))==model) {processed=true;break;}
            }
            if(processed) {
                // Flag-only commands preserve the already consumed complete
                // Source advance. A preceding getter may have published its
                // time; sample the actual clock without treating that read
                // shadow as a seek. The genuine SDK then applies the command.
                if((address==0x80039578||address==0x800395BC)&&entry.native_clock) {
                    const auto saved=word(rdram,model+0x64);
                    word(rdram,model+0x64)=entry.native_clock->expected_time;
                    const auto sample=source_command_sample(rdram,model,*entry.source_step);
                    word(rdram,model+0x64)=saved;
                    real(rdram,model+0x64,sample.checkpoint.time);
                    MEM_B(0x7C,int32_t(model))=int8_t(sample.flags);
                }
                entry.source_command_hold=entry.source_step;
            }
            restore(rdram,i->second);i->second.observation.reset();i->second.native_clock.reset();i->second.events.clear();i->second.event_sequence=0;i->second.acquired_start=false;
            i->second.run=++s.next_run;
            if(address==0x80039564||address==0x8003B6BC||address==0x8003B744)i->second.time_command=i->second.run;
            if(address==0x8003B6BC || address==0x8003B744)i->second.last_pose.reset();
        }
        return;
    case 0x8003948C: break;
    default: throw std::logic_error("Unknown model animation hook");
    }
    auto i=s.models.find(model);
    if(i==s.models.end())return; // Only registered constructors establish lifetime.
    auto& entry=i->second;entry.observation.reset();
    // The simulation path advances actual guest time itself. A render frame
    // must not also extrapolate that same model using the old VI observation.
    if(active_simulation_slice())return;
    entry.native_born=false;
    entry.acquired_start=false;
    entry.native_clock.reset();
    if(s.divider!=2 && s.divider!=3)return;
    if(!word(rdram,model+0x60))return;
    const uint32_t descriptors=word(rdram,model+0x18);region(descriptors,8);
    const auto flags=MEM_BU(0x7C,int32_t(model));
    const float time=real(rdram,model+0x64),duration=real(rdram,descriptors+4);
    const float rate=std::bit_cast<float>(uint32_t(ctx->r5));
    if(!std::isfinite(time)||!std::isfinite(duration)||!std::isfinite(rate)||duration<=0)return;
    // Exact original one-step result, not the multi-wrap continuous evaluator.
    float after=time;
    if(!(flags&8)) {
        after=time+rate;
        if(after<0)after+=duration;
        else if(after>=duration)after=(flags&1)?duration:after-duration;
    }
    entry.observation=Observation{{time,duration,rate,flags,true},std::bit_cast<int32_t>(after),s.step,s.divider,s.origin,uint32_t(word(rdram,model+0x60)),descriptors};
}
void ModelAnimation::acquire_simulation(uint8_t* rdram,const AdvancePlan& plan,bool replace_timeline) {
    auto& s=*state_;
    if(s.frame || !plan)throw std::logic_error("Model simulation acquisition outside owner boundary");
    for(auto& [model,entry]:s.models) {
        // The new shared timeline can reuse tick/phase while an actor is
        // paused. Excluded models must not retain the old scalar baseline.
        const auto rate=plan(model);
        if(replace_timeline){entry.acquired_start=false;entry.source_step.reset();entry.source_command_hold.reset();}
        if(replace_timeline && entry.native_clock)entry.native_clock->end_tick.reset();
        if(replace_timeline || rate) {entry.scalars.clear();entry.angles.clear();entry.motions.clear();entry.bindings.clear();}
        if(!rate || !entry.observation || entry.native_clock || !entry.last_pose)continue;
        const auto& before=entry.observation->before;
        if(entry.observation->step!=s.step || entry.observation->divider!=unsigned(word(rdram,0x801A39DC)) ||
           entry.observation->expected_time!=word(rdram,model+0x64) || *rate!=before.increment ||
           *entry.last_pose!=before.time || !word(rdram,model+0x60) ||
           entry.observation->channels!=uint32_t(word(rdram,model+0x60)) ||
           entry.observation->descriptors!=uint32_t(word(rdram,model+0x18)) ||
           (MEM_BU(0x7C,int32_t(model))&~2u)!=(before.flags&~2u))continue;
        const auto descriptors=uint32_t(word(rdram,model+0x18));region(descriptors,8);
        if(before.duration!=real(rdram,descriptors+4))continue;
        real(rdram,model+0x64,before.time);MEM_B(0x7C,int32_t(model))=int8_t(before.flags);
        entry.acquired_start=before.time==0;
        entry.observation.reset();entry.events.clear();entry.event_sequence=0;
    }
}
std::uint64_t ModelAnimation::clip_boundary_limit(uint8_t* rdram,const SimulationSlice& slice,const AdvancePlan& plan) const {
    auto result=slice.numerator;
    for(const auto& [model,entry]:state_->models) {
        const auto intended=plan ? plan(model) : (entry.native_clock ? std::optional<float>{entry.native_clock->rate} : std::nullopt);
        if(!intended || !std::isfinite(*intended) || *intended==0)continue;
        const auto flags=MEM_BU(0x7C,int32_t(model));
        const auto channels=uint32_t(word(rdram,model+0x60));
        if(!channels || (flags&8))continue;
        const auto descriptors=uint32_t(word(rdram,model+0x18));region(descriptors,8);
        const float duration=real(rdram,descriptors+4),guest_time=real(rdram,model+0x64),rate=*intended;
        if(!std::isfinite(duration) || !std::isfinite(guest_time) || duration<=0 || guest_time<0 || guest_time>duration)continue;
        long double time=guest_time;
        if(entry.native_clock) {
            const auto& clock=*entry.native_clock;
            const bool same=clock.denominator==slice.denominator && clock.rate==rate &&
                clock.expected_time==word(rdram,model+0x64) && clock.expected_flags==flags &&
                clock.channels==channels && clock.descriptors==descriptors && clock.duration==duration;
            if(same)time=clock.time;
            else if(!plan)continue; // No caller plan: stale observations cannot establish a new rate.
        } else if(!plan)continue;
        if(rate>0 && (flags&1) && time>=duration)continue;
        // Original reverse playback reports wrapping just after zero. Expose
        // that pulse after one rational unit, so dependent actions do not wait
        // an entire variable frame after reaching the boundary.
        if(rate<0 && time==0 && !(flags&2)) {result=1;continue;}
        const long double distance=rate>0 ? duration-time : (time>0?time:duration);
        const long double units=distance*slice.denominator/std::abs(static_cast<long double>(rate));
        if(units>result)continue;
        // Split in rational source-tick units, including edges between
        // nanoseconds. The plan excludes inactive/foreign owners and uses a
        // source command's new rate before its first subsequent native step.
        const auto tolerance=16*std::numeric_limits<long double>::epsilon()*std::max(1.0L,units);
        result=std::min(result,std::max(uint64_t(1),uint64_t(std::ceil(std::max(0.0L,units-tolerance)))));
    }
    return result;
}
std::uint64_t ModelAnimation::frame_boundary_limit(uint8_t* rdram,const SimulationSlice& slice,uint32_t model,float target,float rate) const {
    const auto found=state_->models.find(model);
    if(found==state_->models.end() || !std::isfinite(target) || !std::isfinite(rate) || rate<=0)return slice.numerator;
    const auto flags=MEM_BU(0x7C,int32_t(model));const auto channels=uint32_t(word(rdram,model+0x60));
    if(!channels || (flags&8))return slice.numerator;
    const auto descriptors=uint32_t(word(rdram,model+0x18));region(descriptors,8);
    const float duration=real(rdram,descriptors+4),guest_time=real(rdram,model+0x64);
    if(!std::isfinite(duration) || duration<=0 || target<0 || target>=duration || !std::isfinite(guest_time))return slice.numerator;
    long double time=guest_time;
    if(found->second.native_clock) {
        const auto& c=*found->second.native_clock;
        if(c.denominator==slice.denominator && c.rate==rate && c.expected_time==word(rdram,model+0x64) &&
           c.expected_flags==flags && c.channels==channels && c.descriptors==descriptors && c.duration==duration)time=c.time;
    }
    if(time<0 || time>=target)return slice.numerator;
    const long double units=(static_cast<long double>(target)-time)*slice.denominator/rate;
    if(units>slice.numerator)return slice.numerator;
    const auto tolerance=16*std::numeric_limits<long double>::epsilon()*std::max(1.0L,units);
    return std::min(slice.numerator,std::max(uint64_t(1),uint64_t(std::ceil(std::max(0.0L,units-tolerance)))));
}
void ModelAnimation::advance_simulation(uint8_t* rdram,recomp_context* ctx) {
    const auto* slice=active_simulation_slice();
    if(!slice || state_->frame)throw std::logic_error("Animation simulation outside update boundary");
    const auto model=uint32_t(ctx->r4);region(model,0x84);
    auto found=state_->models.find(model);
    if(found==state_->models.end())throw std::logic_error("Unregistered model in native simulation");
    auto& entry=found->second;entry.observation.reset();
    word(rdram,0x80082724)=int32_t(model); // Original shared model selector.
    entry.source_pool=state_->source_pool;
    if(entry.source_command_hold) {
        const auto& hold=*entry.source_command_hold;
        if(hold.tick==slice->tick&&hold.denominator==slice->denominator)return;
        entry.source_command_hold.reset();
    }
    entry.source_step=*slice;
    const auto flags=MEM_BU(0x7C,int32_t(model));
    const auto channels=uint32_t(word(rdram,model+0x60));
    if((flags&8)||!channels)return; // Paused/disabled retains its end flag.
    const auto descriptors=uint32_t(word(rdram,model+0x18));region(descriptors,8);
    const auto duration=real(rdram,descriptors+4),rate=std::bit_cast<float>(uint32_t(ctx->r5));
    const auto time_bits=word(rdram,model+0x64);const auto time=std::bit_cast<float>(time_bits);
    if(!std::isfinite(duration)||duration<=0||!std::isfinite(rate)||!std::isfinite(time))throw std::logic_error("Invalid native animation state");
    auto& clock=entry.native_clock;
    if(!clock||clock->expected_time!=time_bits||clock->expected_flags!=flags||clock->channels!=channels||clock->descriptors!=descriptors||clock->duration!=duration) {
        if(clock) {entry.events.clear();entry.event_sequence=0;entry.run=++state_->next_run;}
        clock=NativeAnimationClock{time,time,0,slice->denominator,rate,time_bits,channels,descriptors,duration,flags};
        if(time<0||time>duration)clock->normalization_tick=slice->tick;
    }
    // Keep elapsed time as an integer ratio during a constant-rate run. Adding
    // hundreds of rounded fractional steps can otherwise turn exact reverse
    // clip boundaries into a spurious extra wrap (observed at 360 FPS).
    if(clock->rate!=rate || clock->denominator!=slice->denominator ||
       clock->elapsed>slice->denominator*1000000ull) {
        clock->origin=clock->time;clock->elapsed=0;
        clock->rate=rate;clock->denominator=slice->denominator;
    }
    auto position=[&](uint64_t elapsed) {
        long double value=clock->origin+static_cast<long double>(rate)*elapsed/clock->denominator;
        const long double edge=std::round(value/duration)*duration;
        const long double tolerance=8*std::numeric_limits<long double>::epsilon()*std::max(1.0L,std::abs(value));
        return std::abs(value-edge)<=tolerance ? edge : value;
    };
    const auto before=position(clock->elapsed);
    clock->elapsed+=slice->numerator;
    const auto absolute=position(clock->elapsed);
    auto before_cycle=std::floor(before/duration);
    if(rate<0 && clock->time==duration)before_cycle-=1;
    // Original also reports normalization after a seek outside the clip,
    // even when the following increment crosses no additional period. Keep
    // that pulse through its source interval so discrete owners can consume it.
    bool ended=clock->normalization_tick==slice->tick || before_cycle!=std::floor(absolute/duration);
    long double next=std::fmod(absolute,duration);
    if(next<0)next+=duration;
    if((flags&1) && rate>=0 && absolute>=duration) {next=duration;ended=true;}
    if(ended){clock->end_tick=slice->tick;++entry.end_sequence;}
    const bool clamped=(flags&1) && rate>=0;
    const auto event_from=clamped ? std::min(before,static_cast<long double>(duration)) : before;
    const auto event_to=clamped ? std::min(absolute,static_cast<long double>(duration)) : absolute;
    if(event_from!=event_to) {
        entry.event_distance+=event_to-event_from;
        ++entry.event_sequence;entry.event_from=event_from;entry.event_to=event_to;entry.event_clamp=clamped;
    }
    real(rdram,model+0x64,float(next));
    const uint8_t after=uint8_t((flags&~2u)|(ended?2u:0u));MEM_B(0x7C,int32_t(model))=int8_t(after);
    clock->time=next;clock->expected_time=word(rdram,model+0x64);clock->expected_flags=after;clock->last_slice=*slice;
}
bool ModelAnimation::scalar_positive(uint8_t* rdram,uint32_t model,uint32_t address,float current) {
    const auto* slice=active_simulation_slice();
    if(!slice)return current>0;
    const auto i=state_->models.find(model);
    if(state_->frame || i==state_->models.end())throw std::logic_error("Invalid native scalar owner");
    return scalar_clock(rdram,i->second,address,current,*slice).baseline>0;
}
float ModelAnimation::scalar_linear(uint8_t* rdram,uint32_t model,uint32_t address,float current,double delta) {
    const auto* slice=active_simulation_slice();
    if(!slice)return float(double(current)+delta);
    const auto i=state_->models.find(model);
    if(state_->frame || i==state_->models.end() || !std::isfinite(delta))throw std::logic_error("Invalid native scalar update");
    auto& clock=scalar_clock(rdram,i->second,address,current,*slice);
    const auto end=slice->phase+slice->numerator;
    const auto result=float(double(clock.baseline)+delta*double(end-clock.start_phase)/double(slice->denominator));
    clock.expected_value=std::bit_cast<int32_t>(result);clock.expected_phase=end;
    return result;
}
float ModelAnimation::angle_short(uint8_t* rdram,uint32_t model,uint32_t address,float delta) {
    if((address&1) || address<0x80000000u || uint64_t(address)+2>0x80400000ull ||
       !std::isfinite(delta) || std::abs(delta)>360)
        throw std::logic_error("Invalid native short angle");
    const int32_t current=MEM_H(0,int32_t(address));
    const auto* slice=active_simulation_slice();
    double value=double(current)+delta;
    if(slice) {
        const auto owner=state_->models.find(model);
        if(state_->frame || owner==state_->models.end())throw std::logic_error("Invalid native angle owner");
        auto& clocks=owner->second.angles;auto i=clocks.find(address);
        if(i==clocks.end() || i->second.tick!=slice->tick || i->second.denominator!=slice->denominator ||
           i->second.expected_phase!=slice->phase || i->second.expected_value!=current)
            i=clocks.insert_or_assign(address,NativeScalarClock{slice->tick,slice->denominator,slice->phase,slice->phase,float(current),current}).first;
        auto& clock=i->second;const auto end=slice->phase+slice->numerator;
        clock.increment=delta;
        value=double(clock.baseline)+double(delta)*double(end-clock.start_phase)/double(slice->denominator);
        value=std::fmod(value,360.0);if(value<0)value+=360;
        clock.expected_phase=end;clock.expected_value=int16_t(value);
    } else {value=std::fmod(value,360.0);if(value<0)value+=360;}
    MEM_H(0,int32_t(address))=int16_t(value);
    return float(value);
}
bool ModelAnimation::native_birth(uint32_t model) const {
    const auto owner=state_->models.find(model);
    if(owner==state_->models.end())throw std::logic_error("Unknown native birth owner");
    return owner->second.native_born;
}
float ModelAnimation::angle_short_sample(uint8_t* rdram,uint32_t model,uint32_t address) const {
    const auto* slice=active_simulation_slice();const auto owner=state_->models.find(model);
    if(!slice || state_->frame || owner==state_->models.end())throw std::logic_error("Invalid native angle sample");
    const auto i=owner->second.angles.find(address);
    if(i==owner->second.angles.end() || !i->second.increment || i->second.tick!=slice->tick ||
       i->second.denominator!=slice->denominator || i->second.expected_phase!=slice->phase+slice->numerator ||
       i->second.expected_value!=MEM_H(0,int32_t(address)))throw std::logic_error("Stale native angle sample");
    const auto& clock=i->second;
    double value=double(clock.baseline)+*clock.increment*double(clock.expected_phase-clock.start_phase)/clock.denominator;
    value=std::fmod(value,360.);if(value<0)value+=360.;return float(value);
}
float ModelAnimation::scalar_previous(uint8_t* rdram,uint32_t model,uint32_t address,float current,double delta) {
    const auto* slice=active_simulation_slice();if(!slice)return current;
    const auto owner=state_->models.find(model);
    if(state_->frame || owner==state_->models.end() || !std::isfinite(delta))throw std::logic_error("Invalid preceding scalar sample");
    const auto& clock=scalar_clock(rdram,owner->second,address,current,*slice);
    // Acquiring an Original update also acquires its already advanced scalar.
    // A native-born model has no such lookahead command and starts from its
    // constructor value. Applying that lag there could even exceed alpha1.
    const double lookahead=owner->second.native_born?0.:1.;
    return float(double(clock.baseline)+delta*(double(slice->phase+slice->numerator-clock.start_phase)/double(slice->denominator)-lookahead));
}
float ModelAnimation::scalar_latched_turn(uint8_t* rdram,uint32_t model,uint32_t address,float current,float delta) {
    const auto* slice=active_simulation_slice();
    if(!std::isfinite(delta) || std::abs(delta)>360)throw std::logic_error("Invalid latched angle increment");
    double value=double(current)+delta;
    if(slice) {
        const auto owner=state_->models.find(model);
        if(state_->frame || owner==state_->models.end())throw std::logic_error("Invalid latched angle owner");
        auto& clock=scalar_clock(rdram,owner->second,address,current,*slice);
        if(!clock.increment)clock.increment=delta;
        const auto end=slice->phase+slice->numerator;
        value=double(clock.baseline)+*clock.increment*double(end-clock.start_phase)/double(slice->denominator);
        value=std::fmod(value,360.);if(value<0)value+=360;
        const float result=float(value);
        clock.expected_phase=end;clock.expected_value=std::bit_cast<int32_t>(result);
        return result;
    }
    value=std::fmod(value,360.);if(value<0)value+=360;
    return float(value);
}
const SourceMotionSample& ModelAnimation::motion_begin(uint8_t* rdram,uint32_t model,uint32_t actor,uint32_t physical,const std::function<SourceMotionSample()>& initialize) {
    const auto* slice=active_simulation_slice();const auto owner=state_->models.find(model);
    if(!slice || state_->frame || owner==state_->models.end())throw std::logic_error("Invalid native motion owner");
    const auto fields=motion_fields(rdram,actor,physical);
    auto& clocks=owner->second.motions;auto i=clocks.find(actor);
    if(i==clocks.end() || i->second.tick!=slice->tick || i->second.denominator!=slice->denominator ||
       i->second.expected_phase!=slice->phase || i->second.physical!=physical || i->second.expected!=fields) {
        auto sample=initialize();sample.start_phase=slice->phase;
        i=clocks.insert_or_assign(actor,NativeMotionClock{slice->tick,slice->denominator,slice->phase,slice->phase,physical,sample,fields}).first;
    }
    i->second.open=true;i->second.open_phase=slice->phase;
    return i->second.sample;
}
void ModelAnimation::bind_actor(uint8_t* rdram,uint32_t model,uint32_t slot,uint32_t actor) {
    const auto owner=state_->models.find(model);region(slot,4);region(actor,0xCC);
    if(state_->frame || owner==state_->models.end())throw std::logic_error("Invalid captured actor owner");
    const auto physical=uint32_t(word(rdram,actor+0x98)),target_model=uint32_t(word(rdram,actor+0x74));
    region(physical,0xDC);const auto target=state_->models.find(target_model);
    if(!word(rdram,actor+0x94)||target==state_->models.end()) {owner->second.bindings.erase(slot);return;}
    owner->second.bindings.insert_or_assign(slot,Model::ActorBinding{actor,physical,target_model,target->second.lifetime});
}
bool ModelAnimation::bound_actor_valid(uint8_t* rdram,uint32_t model,uint32_t slot,uint32_t actor) const {
    const auto owner=state_->models.find(model);if(owner==state_->models.end())return false;
    const auto i=owner->second.bindings.find(slot);if(i==owner->second.bindings.end()||i->second.actor!=actor)return false;
    const auto& b=i->second;const auto target=state_->models.find(b.model);
    return word(rdram,actor+0x94)&&uint32_t(word(rdram,actor+0x98))==b.physical&&
        uint32_t(word(rdram,actor+0x74))==b.model&&target!=state_->models.end()&&target->second.lifetime==b.lifetime;
}
const SourceMotionSample* ModelAnimation::motion_current(uint32_t model,uint32_t actor) const {
    const auto* slice=active_simulation_slice();const auto owner=state_->models.find(model);
    if(!slice || owner==state_->models.end())return nullptr;
    const auto i=owner->second.motions.find(actor);
    if(i==owner->second.motions.end() || !i->second.open || i->second.tick!=slice->tick ||
       i->second.denominator!=slice->denominator || i->second.open_phase!=slice->phase)return nullptr;
    return &i->second.sample;
}
void ModelAnimation::motion_end(uint8_t* rdram,uint32_t model,uint32_t actor) {
    const auto owner=state_->models.find(model);const auto* slice=active_simulation_slice();
    if(!slice || owner==state_->models.end())return; // A hit can retire this owner.
    const auto i=owner->second.motions.find(actor);
    if(i==owner->second.motions.end() || !motion_current(model,actor))return;
    i->second.expected=motion_fields(rdram,actor,i->second.physical);
    i->second.expected_phase=slice->phase+slice->numerator;i->second.open=false;
}
bool ModelAnimation::frame_event(uint8_t* rdram,uint32_t model,float observed,float target,uint64_t site,bool source_start_consumed) {
    const auto* slice=active_simulation_slice();
    if(!slice)return observed==target;
    auto found=state_->models.find(model);
    if(found==state_->models.end())throw std::logic_error("Unregistered animation event owner");
    auto& entry=found->second;
    // A saved value can outlive a script command that seeks/rebinds the clip.
    // Such a comparison belongs to that saved value, not the new clip interval.
    if(std::bit_cast<int32_t>(observed)!=word(rdram,model+0x64))return observed==target;
    if(!std::isfinite(target))return false;
    const auto event_key=std::make_pair(site,std::bit_cast<uint32_t>(target));
    auto cursor=entry.events.find(event_key);
    if(cursor!=entry.events.end()) {
        const auto& old=cursor->second;
        if(old.sequence==entry.event_sequence && old.tick==slice->tick && old.phase==slice->phase &&
           old.numerator==slice->numerator && old.denominator==slice->denominator)return old.result;
    }
    bool result=false;
    if(cursor==entry.events.end() || cursor->second.sequence!=entry.event_sequence) {
        if(source_start_consumed && entry.acquired_start && !entry.native_clock && !entry.event_sequence)result=false;
        else if(!entry.native_clock || !entry.event_sequence)result=observed==target;
        else {
            const long double start=entry.event_from,end=entry.event_to,duration=entry.native_clock->duration;
            if(target>=0 && (target<duration || (entry.event_clamp && target==duration))) {
                if(entry.event_clamp)result=(start<target && end>=target);
                else if(end>start)result=std::floor((end-target)/duration)>std::floor((start-target)/duration);
                else if(end<start)result=std::ceil((start-target)/duration)>std::ceil((end-target)/duration);
            }
        }
    }
    entry.events.insert_or_assign(event_key,EventCursor{entry.event_sequence,slice->tick,slice->phase,slice->numerator,slice->denominator,result});
    return result;
}
std::optional<std::pair<AnimationState,unsigned>> ModelAnimation::original_clock_sample(uint8_t* rdram,std::uint32_t model) const {
    const auto& s=*state_;
    const auto found=s.models.find(model);
    if(found==s.models.end()||!found->second.observation)return std::nullopt;
    const auto& entry=found->second;const auto& observation=*entry.observation;
    if(observation.step!=s.step||observation.divider!=unsigned(word(rdram,0x801A39DC))||
       observation.expected_time!=word(rdram,model+0x64)||
       observation.channels!=std::uint32_t(word(rdram,model+0x60))||
       observation.descriptors!=std::uint32_t(word(rdram,model+0x18))||
       (MEM_BU(0x7C,std::int32_t(model))&~2u)!=(observation.before.flags&~2u)||
       word(rdram,observation.descriptors+4)!=std::bit_cast<std::int32_t>(observation.before.duration))return std::nullopt;
    return std::pair{observation.before,observation.divider};
}
void ModelAnimation::begin_frame(uint8_t* rdram,recomp_context* ctx,Nanoseconds now,unsigned bank) {
    auto& s=*state_;
    if(s.frame || bank>1 || word(rdram,0x801A9544)!=int(bank))throw std::logic_error("Invalid model frame ownership");
    s.frame=true;
    const auto previous_model=word(rdram,0x80082724);
    uint32_t temporary_model=0;int32_t saved_time=0;
    try {
        for(auto& [model,entry]:s.models) {
            if(!entry.last_pose || !word(rdram,model+0x60))continue;
            const bool advancing=entry.observation && entry.observation->step==s.step &&
                entry.observation->before.time>=0 && entry.observation->before.time<=entry.observation->before.duration;
            const bool native=entry.native_pose && entry.native_clock &&
                word(rdram,model+0x64)==entry.native_clock->expected_time &&
                MEM_BU(0x7C,int32_t(model))==entry.native_clock->expected_flags;
            float time=*entry.last_pose;
            if(native) {
                // The original player poses before advancing its clip. Rendering
                // that pose would lag by one *variable* step. Evaluate the actual
                // native clip time in the free bank, then restore game-owned bones.
                time=real(rdram,model+0x64);
                for(auto& bone:entry.bones)
                    for(unsigned k=0;k<9;++k)bone.transform[k]=word(rdram,bone.address+0x14+4*k);
                entry.sampled=true;
            }
            if(advancing) {
                const auto& o=*entry.observation;
                if(now<o.origin)throw std::logic_error("Model clock moved backwards");
                if(word(rdram,model+0x64)!=o.expected_time)throw std::logic_error("Untracked write to model animation time");
                for(auto& bone:entry.bones)
                    for(unsigned k=0;k<9;++k)bone.transform[k]=word(rdram,bone.address+0x14+4*k);
                entry.sampled=true;
                Animation animation(o.before,o.divider);
                time=animation.advance(now-o.origin).time;
            }
            temporary_model=model;saved_time=word(rdram,model+0x64);
            real(rdram,model+0x64,time);
            recomp_context pose_context;dora3::copy_recomp_context(pose_context,*ctx);pose_context.r4=int32_t(model);
            if(advancing || native)dora3_s0_800393C4(rdram,&pose_context);
            else {
                // A model not advanced this tick must keep its bone overrides,
                // but each free vertex bank still needs the last real morph.
                // Evaluate only original type10 channels; never copy busy output.
                word(rdram,0x80082724)=int32_t(model);
                const uint32_t indices=word(rdram,model+0x60),descriptors=word(rdram,model+0x18);
                const auto count=word(rdram,model+0x2C);
                if(count<0)throw std::logic_error("Negative model channel count");
                region(indices,uint64_t(count)*4);region(descriptors,uint64_t(count)*28);
                for(int j=0;j<count;++j) {
                    const auto index=word(rdram,indices+unsigned(j)*4);
                    if(index<0 || index>=count)throw std::logic_error("Model channel index out of range");
                    if(word(rdram,descriptors+unsigned(index)*28)!=10)continue;
                    dora3::copy_recomp_context(pose_context,*ctx);pose_context.r4=int32_t(indices+unsigned(j)*4);
                    dora3_s0_8003931C(rdram,&pose_context);
                }
            }
            word(rdram,model+0x64)=saved_time;temporary_model=0;
        }
        word(rdram,0x80082724)=previous_model;
    } catch(...) {
        if(temporary_model)word(rdram,temporary_model+0x64)=saved_time;
        word(rdram,0x80082724)=previous_model;
        end_frame(rdram);throw;
    }
}
void ModelAnimation::end_frame(uint8_t* rdram) {
    auto& s=*state_;
    for(auto& [model,entry]:s.models) {
        (void)model;
        restore(rdram,entry);
    }
    s.frame=false;
}
ModelAnimation& game_model_animation() { static ModelAnimation value;return value; }
ModelFrameScope::ModelFrameScope(uint8_t* ram,recomp_context* ctx,Nanoseconds now,unsigned bank):ram_(ram) {
    const auto* binding=active_frame_binding();
    if(!binding || binding->ticket.bank!=bank)throw std::logic_error("Model frame without native ticket");
    game_model_animation().begin_frame(ram,ctx,now,bank);
}
ModelFrameScope::~ModelFrameScope() { game_model_animation().end_frame(ram_); }
}
extern "C" void dora3_native_model_hook(uint8_t* ram,recomp_context* ctx,uint32_t address) {
    dora3::native_time::game_model_animation().hook(ram,ctx,address,dora3::native_time::monotonic_time());
}

extern "C" int dora3_native_model_morph_allowed(uint8_t* rdram,recomp_context*) {
    if(!dora3_native_loop_owns_frame_banks())return 1;
    const auto* frame=dora3::native_time::active_frame_binding();
    if(!frame)return 0; // Defer original update's morph writes to the free frame bank.
    if(MEM_W(0,int32_t(0x801A9544))!=int(frame->ticket.bank))
        throw std::logic_error("Morph bank does not match native frame ticket");
    return 1;
}

extern "C" int dora3_native_model_advance(uint8_t* rdram,recomp_context* ctx) {
    if(!dora3::native_time::active_simulation_slice())return 0;
    dora3::native_time::game_model_animation().advance_simulation(rdram,ctx);return 1;
}

extern "C" int dora3_native_model_event(uint8_t* ram,uint32_t model,float observed,float target,uint64_t site) {
    return dora3::native_time::game_model_animation().frame_event(ram,model,observed,target,site);
}
extern "C" int dora3_native_model_start_event(uint8_t* ram,uint32_t model,float observed,float target,uint64_t site) {
    return dora3::native_time::game_model_animation().frame_event(ram,model,observed,target,site,true);
}
extern "C" int dora3_native_model_scalar_positive(uint8_t* ram,uint32_t model,uint32_t address,float current) {
    return dora3::native_time::game_model_animation().scalar_positive(ram,model,address,current);
}
extern "C" float dora3_native_model_scalar_linear(uint8_t* ram,uint32_t model,uint32_t address,float current,double delta) {
    return dora3::native_time::game_model_animation().scalar_linear(ram,model,address,current,delta);
}
extern "C" float dora3_native_model_angle_short(uint8_t* ram,uint32_t model,uint32_t address,float delta) {
    return dora3::native_time::game_model_animation().angle_short(ram,model,address,delta);
}
extern "C" float dora3_native_model_scalar_previous(uint8_t* ram,uint32_t model,uint32_t address,float current,double delta) {
    return dora3::native_time::game_model_animation().scalar_previous(ram,model,address,current,delta);
}
extern "C" float dora3_native_model_latched_turn(uint8_t* ram,uint32_t model,uint32_t address,float current,float delta) {
    return dora3::native_time::game_model_animation().scalar_latched_turn(ram,model,address,current,delta);
}
