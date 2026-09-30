#include "recomp_context_copy.hpp"
#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <optional>
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
};
struct Model {
    std::vector<Bone> bones;
    std::optional<Observation> observation;
    std::optional<float> last_pose;
    bool sampled=false;
};
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
    std::unordered_map<uint32_t,Model> models;
    uint64_t step=0;
    Nanoseconds origin=0;
    unsigned divider=0;
    bool frame=false;
};
ModelAnimation::ModelAnimation():state_(std::make_unique<State>()) {}
ModelAnimation::~ModelAnimation()=default;
std::size_t ModelAnimation::model_count() const { return state_->models.size(); }
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
        if(!s.frame)if(auto i=s.models.find(model);i!=s.models.end())
            i->second.last_pose=real(rdram,model+0x64);
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
        for(auto& [owner,entry]:s.models) {(void)owner;entry.observation.reset();entry.last_pose.reset();}
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
            if(invalidated)i=s.models.erase(i);else ++i;
        }
        return;
    }
    case 0x8003B6B4: { // Constructor has completed; same pointer is a new lifetime.
        const auto created=uint32_t(ctx->r2);region(created,0x84);
        s.models.insert_or_assign(created,Model{find_bones(rdram,created),{},{},false});return;
    }
    case 0x8003B77C:
        if(auto i=s.models.find(model);i!=s.models.end()) {restore(rdram,i->second);s.models.erase(i);}
        return;
    case 0x8003B6BC: case 0x8003B744: // New/released channels, even identical pointers.
    case 0x80039564: case 0x80039578: case 0x800395BC: // Seek/pause/clamp are commands.
        if(auto i=s.models.find(model);i!=s.models.end()) {
            restore(rdram,i->second);i->second.observation.reset();
            if(address==0x8003B6BC || address==0x8003B744)i->second.last_pose.reset();
        }
        return;
    case 0x8003948C: break;
    default: throw std::logic_error("Unknown model animation hook");
    }
    auto i=s.models.find(model);
    if(i==s.models.end())return; // Only registered constructors establish lifetime.
    auto& entry=i->second;entry.observation.reset();
    if(s.divider!=2 && s.divider!=3)return;
    if(!word(rdram,model+0x60))return;
    const uint32_t descriptors=word(rdram,model+0x18);region(descriptors,8);
    const auto flags=MEM_BU(0x7C,int32_t(model));
    const float time=real(rdram,model+0x64),duration=real(rdram,descriptors+4);
    const float rate=std::bit_cast<float>(uint32_t(ctx->r5));
    if(!std::isfinite(time)||!std::isfinite(duration)||!std::isfinite(rate)||duration<=0||time<0||time>duration)return;
    // Exact original one-step result, not the multi-wrap continuous evaluator.
    float after=time;
    if(!(flags&8)) {
        after=time+rate;
        if(after<0)after+=duration;
        else if(after>=duration)after=(flags&1)?duration:after-duration;
    }
    entry.observation=Observation{{time,duration,rate,flags,true},std::bit_cast<int32_t>(after),s.step,s.divider,s.origin};
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
            const bool advancing=entry.observation && entry.observation->step==s.step;
            float time=*entry.last_pose;
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
            if(advancing)dora3_s0_800393C4(rdram,&pose_context);
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
