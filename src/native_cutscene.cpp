#include "manual_fps_trace.hpp"
#include "native_cutscene.hpp"
#include "native_frame_bridge.hpp"
#include "native_simulation_time.hpp"
#include "recomp_context_copy.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

extern "C" {
int dora3_native_loop_cutscene_active();
int dora3_overlay_loaded(uint32_t);
void dora3_s3_801E3768(uint8_t*,recomp_context*);
void dora3_s3_801E0760(uint8_t*,recomp_context*);
void dora3_s3_801BBDDC(uint8_t*,recomp_context*);
void dora3_s0_8002F064(uint8_t*,recomp_context*);
}
namespace {
constexpr uint32_t image_size=0x800000, cameras=0x801A99F8;
constexpr uint32_t camera_controls=0x802183D0, participant_controls=0x80218B10;
constexpr std::array<uint32_t,3> origins{0x800DAE54,0x800FE878,0x800DD3F0};
int32_t word(uint8_t* rdram,uint32_t address){return MEM_W(0,int32_t(address));}
bool pointer(uint32_t address,unsigned size){return !(address&3)&&address>=0x80000000u&&uint64_t(address)+size<=0x80800000ull;}
struct JumpStep {uint32_t address;float velocity,displacement;};
thread_local bool equations_active=false;
thread_local std::vector<JumpStep> jumps;
struct EquationScope {
    EquationScope(){if(equations_active)throw std::logic_error("Nested cutscene equations");equations_active=true;jumps.clear();}
    ~EquationScope(){equations_active=false;jumps.clear();}
};
// These fixed roots have no interpreter, resource, input, RNG or audio calls.
// Geometry scratch, flags, timers and blink writes stay in the isolated image.
// Publication contains reviewed transforms and the model's visual alpha only.
void evaluate_scene(uint8_t* rdram,recomp_context* ctx) {
    using namespace dora3::native_time;
    const auto* active=active_simulation_slice();
    if(!active)throw std::logic_error("Cutscene equations require explicit time");
    EquationScope equations;
    // Every equation evaluation begins at a Source-period anchor. The original
    // timer preludes run once in this private image before motion, including
    // duration0's immediate channel clear; guest timers never publish.
    dora3_s3_801E3768(rdram,ctx);
    dora3_s3_801E0760(rdram,ctx);
    // BB080 uses the first scripted target for the shared world origin.
    ctx->f12.fl=std::bit_cast<float>(word(rdram,camera_controls+0x38));
    ctx->f14.fl=std::bit_cast<float>(word(rdram,camera_controls+0x3C));
    ctx->r6=word(rdram,camera_controls+0x40);
    dora3_s3_801BBDDC(rdram,ctx);
    for(unsigned i=0;i<4;++i) {
        const auto control=camera_controls+i*0x1B8;
        const auto mode=MEM_H(0x36,int32_t(control));
        if(MEM_H(0x34,int32_t(control))&&(mode==0||mode==2)) {
            ctx->r4=int32_t(cameras+i*0x34);dora3_s0_8002F064(rdram,ctx);
        }
    }
}
}
extern "C" int dora3_native_cutscene_equations_active(){return equations_active;}
extern "C" float dora3_native_cutscene_gravity_f32(float velocity,float gravity,uint32_t address) {
    const double h=dora3::native_time::simulation_ticks();
    if(!equations_active)return velocity-gravity;
    // Fractional kick/drift, anchored to the unchanged Original period. At1
    // the actual float subtraction and -48 clamp are exactly Original.
    const float next=h==1?std::max(velocity-gravity,-48.0f):float(std::max(double(velocity)-gravity*h,-48.0));
    const float drift=h==1?next:float(h*std::max(double(velocity)-0.5*gravity*(h+1),-48.0));
    jumps.push_back({address,next,drift});return next;
}
extern "C" float dora3_native_cutscene_query_velocity(uint32_t physics,float velocity) {
    if(!equations_active)return velocity;
    for(const auto& jump:jumps)if(jump.address==physics+0x14&&std::bit_cast<uint32_t>(jump.velocity)==std::bit_cast<uint32_t>(velocity))return jump.displacement;
    return velocity;
}
extern "C" float dora3_native_cutscene_axis_drift(uint8_t* rdram,uint32_t control,uint32_t actor,uint32_t axis,float velocity,uint32_t acceleration) {
    using namespace dora3::native_time;
    if(equations_active&&axis==4&&MEM_H(0x9C,int32_t(control))) {
        const auto address=uint32_t(word(rdram,actor+0x98))+0x14;
        const double h=simulation_ticks();const float a=std::bit_cast<float>(acceleration);
        for(const auto& jump:jumps)if(jump.address==address&&
            std::bit_cast<uint32_t>(float(jump.velocity+dora3_native_rate_f32(a)))==std::bit_cast<uint32_t>(velocity))
            return float(jump.displacement+0.5*a*h*(h+1));
    }
    return dora3_native_displacement_f32(velocity,acceleration);
}
namespace dora3::native_time {
struct CutsceneTimeline::State {
    // Source geometry/ROM/heap reads use an exact immutable8MiB anchor.
    // A separate working image prevents every geometry/model/timer scratch
    // write from escaping. No pointers into these images are kept by guest code.
    std::vector<uint32_t> anchor,working,pending;
    std::vector<int32_t> controls;
    bool motion_pending=false,current_interval=false;
    struct Lease {uint32_t control,actor,physics,model;int32_t identity;};
    std::vector<Lease> leases;
    struct Field {uint32_t address;int32_t source,published;int lease;};
    std::vector<Field> fields;
    Nanoseconds source_time=0,shown_time=0;
    unsigned divider=3;
    bool open=false,in_source=false;
    uint8_t* ram=nullptr;
    bool valid(uint8_t* rdram,const Lease& lease) const {
        return MEM_H(4,int32_t(lease.control))&&uint32_t(word(rdram,lease.control))==lease.actor&&
            word(rdram,lease.actor+0x94)==lease.identity&&uint32_t(word(rdram,lease.actor+0x98))==lease.physics&&
            uint32_t(word(rdram,lease.actor+0x74))==lease.model;
    }
    void restore(uint8_t* rdram) {
        if(rdram!=ram){fields.clear();return;}
        for(const auto& field:fields)
            if((field.lease<0||valid(rdram,leases[unsigned(field.lease)]))&&word(rdram,field.address)==field.published)
                MEM_W(0,int32_t(field.address))=field.source;
        fields.clear();
    }
    void capture(uint8_t* rdram,Nanoseconds now) {
        anchor.resize(image_size/4);working.resize(image_size/4);
        if(motion_pending) {
            auto* before=reinterpret_cast<uint8_t*>(pending.data());
            // A later Source lifecycle operation may replace a participant.
            // Never publish transforms from a different allocation/command.
            for(unsigned i=0;i<60&&motion_pending;++i) {
                const auto control=participant_controls+i*0xD4;
                if(MEM_H(4,int32_t(control))==0)continue;
                const auto actor=uint32_t(word(rdram,control));
                if(word(before,control)!=int32_t(actor)||!pointer(actor,0xCC)) {motion_pending=false;break;}
                for(unsigned off:{0x74u,0x94u,0x98u})
                    if(word(before,actor+off)!=word(rdram,actor+off))motion_pending=false;
            }
        }
        if(motion_pending)anchor.swap(pending);
        else std::memcpy(anchor.data(),rdram,image_size);
        current_interval=motion_pending;motion_pending=false;
        controls.clear();
        for(auto region:{std::pair{camera_controls,4u*0x1B8},std::pair{participant_controls,60u*0xD4}})
            for(unsigned off=0;off<region.second;off+=4)controls.push_back(word(rdram,region.first+off));
        fields.clear();leases.clear();source_time=shown_time=now;ram=rdram;open=true;in_source=false;
        auto own=[&](uint32_t address,unsigned bytes,int lease=-1) {
            for(unsigned off=0;off<bytes;off+=4) {
                const auto value=word(rdram,address+off);
                if(std::none_of(fields.begin(),fields.end(),[&](const Field& f){return f.address==address+off;}))
                    fields.push_back({address+off,value,value,lease});
            }
        };
        for(auto address:origins)own(address,4);
        for(unsigned i=0;i<4;++i) {
            const auto control=camera_controls+i*0x1B8;
            if(!MEM_H(0x34,int32_t(control)))continue;
            own(cameras+i*0x34,0x34);own(control+0x38,0x3C);
        }
        for(unsigned i=0;i<60;++i) {
            const auto control=participant_controls+i*0xD4;
            if(!MEM_H(4,int32_t(control)))continue;
            const auto actor=uint32_t(word(rdram,control));
            if(!pointer(actor,0xCC))throw std::logic_error("Invalid scripted participant");
            const auto model=uint32_t(word(rdram,actor+0x74));
            const auto lease=int(leases.size());
            leases.push_back({control,actor,uint32_t(word(rdram,actor+0x98)),model,word(rdram,actor+0x94)});
            own(control+0x18,0x24,lease);
            own(actor+0xC,0x18,lease);own(actor+0x9C,0xC,lease);
            // E89CC selects model group0. Its visual mode/alpha may change
            // continuously; physical marker2A and completion flag remain Source.
            if(std::bit_cast<float>(word(rdram,control+0xC8))!=-1.0f&&pointer(model,0x7C))own(model+0x74,8,lease);
        }
        std::sort(fields.begin(),fields.end(),[](const Field& a,const Field& b){return a.address<b.address;});
    }
    bool changed(uint8_t* rdram) const {
        if(rdram!=ram)return true;
        for(const auto& lease:leases)if(!valid(rdram,lease))return true;
        for(const auto& field:fields)if(word(rdram,field.address)!=field.published)return true;
        unsigned control_index=0;
        // The complete controller arrays cover cuts, rates, flags, durations,
        // actor replacement and follow target. Equal-valued commands are
        // bracketed by the actual BB080 Source hooks rather than guessed here.
        for(auto region:{std::pair{camera_controls,4u*0x1B8},std::pair{participant_controls,60u*0xD4}})
            for(unsigned off=0;off<region.second;off+=4) {
                const auto address=region.first+off;
                const auto it=std::lower_bound(fields.begin(),fields.end(),address,[](const Field& f,uint32_t value){return f.address<value;});
                if(word(rdram,address)!=(it==fields.end()||it->address!=address?controls[control_index]:it->published))return true;
                ++control_index;
            }
        return false;
    }
};
CutsceneTimeline::CutsceneTimeline():state_(std::make_unique<State>()){}
CutsceneTimeline::~CutsceneTimeline()=default;
void CutsceneTimeline::begin(uint8_t* ram,Nanoseconds now,unsigned divider) {
    if(!ram||(divider!=2&&divider!=3)||now<0)throw std::invalid_argument("Invalid cutscene acquisition");
    reset(ram);state_->divider=divider;state_->capture(ram,now);
}
void CutsceneTimeline::advance(uint8_t* rdram,recomp_context* ctx,Nanoseconds now,Evaluate evaluate) {
    auto& state=*state_;
    if(!state.open||state.in_source)return;
    if(!rdram||!ctx||!evaluate||now<state.shown_time||active_simulation_slice())
        throw std::invalid_argument("Invalid cutscene advance");
    if(now==state.shown_time)return; // Full-RAM/context no-op at duplicate timestamps.
    if(state.changed(rdram)) {
#ifdef DORA3_CRASH_LOG
        if(dora3::manual_trace::gate(12,0,0,12000)) {
            dora3::manual_trace::log("SCRIPT_REBASE now=%lld source=%lld shown=%lld current_interval=%d fields=%zu leases=%zu",(long long)now,(long long)state.source_time,(long long)state.shown_time,int(state.current_interval),state.fields.size(),state.leases.size());
            unsigned mismatches=0;
            for(const auto& f:state.fields)if(word(rdram,f.address)!=f.published&&mismatches++<8)
                dora3::manual_trace::log("SCRIPT_MISMATCH address=%08X source=%08X published=%08X actual=%08X lease=%d",f.address,uint32_t(f.source),uint32_t(f.published),uint32_t(word(rdram,f.address)),f.lease);
        }
#endif
        // A real command/replacement invalidates the entire coupled equation.
        // Preserve changed words and restore only our still-owned sample words.
        state.restore(rdram);state.capture(rdram,now);return;
    }
    std::memcpy(state.working.data(),state.anchor.data(),image_size);
    auto* image=reinterpret_cast<uint8_t*>(state.working.data());
    recomp_context context;dora3::copy_recomp_context(context,*ctx);
    const auto elapsed=uint64_t(now-state.source_time);
    if(elapsed>UINT64_MAX/60)throw std::overflow_error("Cutscene elapsed overflow");
    const auto denominator=uint64_t(state.divider)*second;
    // Replay only the reached command interval. In script350 the chair/Nobita
    // move in one/two-tick bursts (opcode65 at ROM16778E onward). Starting
    // from the completed pose predicts the *next* timer clear and loses those
    // intermediate poses. SDK model samples already use the current interval.
    auto remaining=state.current_interval?std::min(elapsed*60,denominator):elapsed*60;uint64_t tick=0;
    while(remaining) {
        // A30Hz endpoint is rounded upward to integer nanoseconds. Its
        // sub-nanosecond residue must not enter the next timer/composition
        // prelude: some camera geometry contains instantaneous base changes.
        // This discards less than1ns only at an exact whole-period boundary;
        // delayed updates still evaluate all elapsed whole periods normally.
        if(tick&&remaining<60)break;
        const auto amount=std::min(remaining,denominator);
        SimulationScope explicit_time({amount,denominator,tick,0});evaluate(image,&context);
        remaining-=amount;++tick;
    }
    // Publish one coherent participant→camera→origin pose. Registers, stack,
    // heap, geometry results, interpreter records and event flags never publish.
    for(auto& field:state.fields) {
        field.published=word(image,field.address);
        MEM_W(0,int32_t(field.address))=field.published;
    }
#ifdef DORA3_CRASH_LOG
    if(dora3::manual_trace::gate(13,0,0,12000))
        dora3::manual_trace::log("SCRIPT_SAMPLE now=%lld source=%lld previous=%lld elapsed=%llu denominator=%llu current=%d fields=%zu",(long long)now,(long long)state.source_time,(long long)state.shown_time,(unsigned long long)elapsed,(unsigned long long)denominator,int(state.current_interval),state.fields.size());
#endif
    state.shown_time=now;
}
void CutsceneTimeline::before_source(uint8_t* rdram) {
    auto& state=*state_;if(!state.open)return;
    state.restore(rdram);state.in_source=true;
}
void CutsceneTimeline::after_source(uint8_t* rdram,Nanoseconds now) {
    auto& state=*state_;if(!state.open)return;
    state.capture(rdram,now);
}
void CutsceneTimeline::motion_source(uint8_t* ram) {
    auto& s=*state_;
    if(!s.open||!s.in_source||s.ram!=ram||equations_active)return;
    s.pending.resize(image_size/4);std::memcpy(s.pending.data(),ram,image_size);
    s.motion_pending=true;
}
void CutsceneTimeline::source_completed(uint8_t* ram,Nanoseconds now) {
    auto& s=*state_;
    if(s.open&&!s.in_source&&s.ram==ram&&now>=s.source_time)
        s.source_time=s.shown_time=now;
}
void CutsceneTimeline::reset(uint8_t* ram) {
    auto& state=*state_;
    if(ram&&state.open&&ram==state.ram)state.restore(ram);
    state.fields.clear();state.leases.clear();state.open=state.in_source=false;state.motion_pending=state.current_interval=false;state.ram=nullptr;
}
std::optional<Nanoseconds> CutsceneTimeline::render_time() const {
    return state_->open?std::optional{state_->shown_time}:std::nullopt;
}
}
namespace dora3::native_time {
// E8F24/2DA98 rebuild local draw poses and matrices; E8ECC may also write
// billboard angles. Those borrowed draw writes must not be mistaken for a
// new script command by CutsceneTimeline::changed on the next Native frame.
// Keep the published world pose, script state and animation/event ownership.
struct CutsceneFrameScope::State {
    uint8_t* ram=nullptr;
    struct Pose {
        uint32_t control,actor,physics;
        int32_t identity;
        std::array<int32_t,25> fields;
        std::array<int32_t,8> components;
        uint8_t dirty;
    };
    std::vector<Pose> poses;
};
CutsceneFrameScope::CutsceneFrameScope(uint8_t* rdram):state_(std::make_unique<State>()) {
    if(!dora3_native_loop_cutscene_active()||!native_cutscene_eligible(rdram))return;
    state_->ram=rdram;
    for(unsigned i=0;i<60;++i) {
        const uint32_t control=participant_controls+i*0xD4;
        if(!MEM_H(4,int32_t(control)))continue;
        const uint32_t actor=uint32_t(word(rdram,control));
        State::Pose pose{control,actor,uint32_t(word(rdram,actor+0x98)),word(rdram,actor+0x94),{},{},MEM_BU(0x6A,int32_t(actor))};
        for(unsigned j=0;j<25;++j)pose.fields[j]=word(rdram,actor+4*j);
        for(unsigned j=0;j<8;++j)pose.components[j]=word(rdram,actor+0x74+4*j);
        state_->poses.push_back(pose);
    }
}
CutsceneFrameScope::~CutsceneFrameScope() {
    if(!state_->ram)return;
    uint8_t* rdram=state_->ram;
    for(const auto& pose:state_->poses) {
        if(!MEM_H(4,int32_t(pose.control))||uint32_t(word(rdram,pose.control))!=pose.actor||
           word(rdram,pose.actor+0x94)!=pose.identity||uint32_t(word(rdram,pose.actor+0x98))!=pose.physics)continue;
        bool same=true;
        for(unsigned j=0;j<8;++j)same=same&&word(rdram,pose.actor+0x74+4*j)==pose.components[j];
        if(!same)continue;
        for(unsigned j=0;j<25;++j)MEM_W(0,int32_t(pose.actor+4*j))=pose.fields[j];
        MEM_B(0x6A,int32_t(pose.actor))=int8_t(pose.dirty);
    }
}
}
namespace {dora3::native_time::CutsceneTimeline scene;}
bool native_cutscene_eligible(uint8_t* rdram) {
    if(!rdram||!dora3_overlay_loaded(0x70A50)||word(rdram,0x80218AD8)!=1)return false;
    for(unsigned i=0;i<8;++i) {
        const auto slot=0x801A44F0u+i*20;
        if(i<2) {
            if(word(rdram,slot)!=2||word(rdram,slot+12)!=int(i)||uint32_t(word(rdram,slot+16))!=(i?0x801BB848u:0x801BB730u))return false;
        } else if(word(rdram,slot)!=0&&word(rdram,slot)!=5)return false;
    }
    if(word(rdram,0x802170EC)||MEM_BU(0,int32_t(0x80217118))||word(rdram,0x802170F0)||
       word(rdram,0x8021D484)||word(rdram,0x8021D494)||word(rdram,0x801A9C88)||
       !word(rdram,0x801AA648)||!word(rdram,0x801A9500))return false;
    for(unsigned i=0;i<4;++i) {
        const auto control=camera_controls+i*0x1B8;
        if(!MEM_H(0x34,int32_t(control)))continue;
        const auto mode=MEM_H(0x36,int32_t(control));
        if(mode<0||mode>3)return false;
        if(mode>=2) {const auto target=MEM_H(0x10A,int32_t(control));if(target<0||target>=60)return false;}
    }
    for(unsigned i=0;i<60;++i) {
        const auto control=participant_controls+i*0xD4;
        if(!MEM_H(4,int32_t(control)))continue;
        const auto actor=uint32_t(word(rdram,control));
        if(!pointer(actor,0xCC)||!pointer(uint32_t(word(rdram,actor+0x98)),0xF4))return false;
    }
    return true;
}
void native_cutscene_begin(uint8_t* ram,dora3::native_time::Nanoseconds now,unsigned divider){scene.begin(ram,now,divider);}
void native_cutscene_update(uint8_t* ram,recomp_context* ctx,dora3::native_time::Nanoseconds now) {
    if(dora3_native_loop_cutscene_active()&&native_cutscene_eligible(ram))scene.advance(ram,ctx,now,evaluate_scene);
}
void native_cutscene_source_completed(uint8_t* ram,dora3::native_time::Nanoseconds now){scene.source_completed(ram,now);}
void native_cutscene_reset(uint8_t* ram){scene.reset(ram);}
std::optional<dora3::native_time::Nanoseconds> native_cutscene_render_time(){return scene.render_time();}
void native_cutscene_evaluate(uint8_t* ram,recomp_context* ctx){evaluate_scene(ram,ctx);}
extern "C" void dora3_native_cutscene_source(uint8_t* ram,recomp_context* ctx,unsigned phase) {
    using namespace dora3::native_time;
#ifdef DORA3_CRASH_LOG
    if(dora3::manual_trace::gate(17,phase,0,12000))
        dora3::manual_trace::log("SCRIPT_SOURCE phase=%u active=%d eligible=%d scripted=%d",phase,dora3_native_loop_cutscene_active(),int(native_cutscene_eligible(ram)),word(ram,0x80218AD8));
#endif
    if(!dora3_native_loop_cutscene_active())return;
    if(phase==0) {
        native_cutscene_update(ram,ctx,monotonic_time());scene.before_source(ram);
    } else if(phase==1) {
        if(native_cutscene_eligible(ram))scene.after_source(ram,monotonic_time());
        else scene.reset(); // Original completion already owns the new pose.
    } else if(phase==2)scene.motion_source(ram);
    else throw std::invalid_argument("Invalid cutscene Source boundary");
}
