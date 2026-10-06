#include "native_ui.hpp"
#include "native_simulation_time.hpp"
#include "native_effects.hpp"
#include "native_frame_bridge.hpp"
#include "recomp_context_copy.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <climits>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <vector>

extern "C" {
int dora3_overlay_loaded(uint32_t);
int dora3_native_loop_owns_fade();
void dora3_s2_menu_801BBCEC(uint8_t*,recomp_context*);
void dora3_s2_menu_801BBEB0(uint8_t*,recomp_context*);
void dora3_s2_menu_801BC17C(uint8_t*,recomp_context*);
void dora3_s2_menu_801BC4C0(uint8_t*,recomp_context*);
void dora3_s2_menu_801C14B8(uint8_t*,recomp_context*);
void dora3_lib_80057A80(uint8_t*,recomp_context*);
}
namespace {
using namespace dora3::native_time;
constexpr uint32_t image_size=0x400000,processes=0x801A44F0,panels=0x801B8AE0;
int32_t& word(uint8_t* rdram,uint32_t a){return MEM_W(0,int32_t(a));}
float real(uint8_t* rdram,uint32_t a){return std::bit_cast<float>(word(rdram,a));}
void real(uint8_t* rdram,uint32_t a,float f){word(rdram,a)=std::bit_cast<int32_t>(f);}
enum class Profile { None,Game,Title,Files,Intro,Selection,Image,Pak };
Profile profile(uint8_t* ram) {
    // Overlay-qualified callback pairs, not overlapping virtual addresses alone.
    if(word(ram,processes)!=2||word(ram,processes+20)!=2||word(ram,processes+12)!=0||word(ram,processes+32)!=1)return Profile::None;
    const uint32_t update=uint32_t(word(ram,processes+16)),draw=uint32_t(word(ram,processes+36));
    for(unsigned i=2;i<8;++i)if(word(ram,processes+i*20)!=0&&word(ram,processes+i*20)!=5)return Profile::None;
    if(dora3_overlay_loaded(0x70A50)&&update==0x801BB730&&draw==0x801BB848)return Profile::Game;
    if(!dora3_overlay_loaded(0x635C0))return Profile::None;
    if(update==0x801BB52C&&draw==0x801BB648)return Profile::Pak;
    if(update==0x801BCEF8&&draw==0x801BD078)return Profile::Title;
    if(update==0x801BFAA4&&draw==0x801BFC44)return Profile::Files;
    if(update==0x801C0BBC&&draw==0x801C0C88)return Profile::Intro;
    if(update==0x801C24A4&&draw==0x801C267C)return Profile::Selection;
    if(update==0x801C0088&&draw==0x801C012C)return Profile::Image;
    return Profile::None;
}
struct Field {uint32_t address;int32_t source;};
struct Guard {
    uint32_t pc=0,address=0,size=0;
    uint64_t epoch=~uint64_t(0);
    bool restoring=false,entered=false;
    std::vector<uint8_t> bytes;
    std::vector<std::vector<uint8_t>> originals;
    unsigned call=0;
};
enum Domain : unsigned { Panels=1,Rectangle=2,Title=4,Intro=8,SelectionActor=16,SelectionRow=32,SelectionExit=64,PakCursor=128,PakSize=256,PakHint=512 };
struct Observation {uint8_t* ram=nullptr;Profile owner=Profile::None;unsigned mask=0;bool running=false;} observation;
struct UiState {
    Profile owner=Profile::None;
    uint8_t* ram=nullptr;
    Nanoseconds source_time=0;
    unsigned divider=3;
    uint64_t epoch=0;
    bool frame=false,equations=false,first_frame=false;
    uint64_t frame_epoch=~uint64_t(0);
    double h=0;
    unsigned domains=0;
    std::vector<uint32_t> working;
    std::vector<Field> fields;
    std::array<Guard,9> guards{};
    struct Drift {uint32_t address;float velocity,displacement;};
    std::vector<Drift> drifts;
} state;
void publish(uint8_t* ram,uint32_t a,int32_t value){state.fields.push_back({a,word(ram,a)});word(ram,a)=value;}
void evaluate_panels(uint8_t* ram,double h) {
    uint8_t* rdram=ram;
    for(unsigned i=0;i<8;++i) {
        const uint32_t p=panels+i*0x44;
        const int duration=MEM_H(0x40,int32_t(p)),elapsed=MEM_H(0x42,int32_t(p));
        if(!MEM_H(0,int32_t(p))||duration<=0||elapsed>=duration)continue;
        for(unsigned j=0;j<5;++j) {
            const int32_t start=word(ram,p+4+j*4),target=word(ram,p+0x18+j*4);
            // Original divides the signed delta first, then multiplies time.
            // Keep that quantized rate; float time is not a lerp of two poses.
            const int32_t delta=std::bit_cast<int32_t>(uint32_t(target)-uint32_t(start));
            const int32_t rate=delta/duration;
            const double value=double(start)+double(rate)*(double(elapsed)+h);
            publish(ram,p+0x2C+j*4,int32_t(std::clamp(value,double(INT32_MIN),double(INT32_MAX))));
        }
    }
}
// The ROM stores easing coefficients, not successive poses. Evaluate that
// authored scalar curve at Source step+h, retaining signed-short endpoints
// and truncation to integer pixel coordinates. Device/FSM operations stay Source.
float curve(uint8_t* ram,uint32_t table,unsigned step,double h,unsigned last) {
    const float a=real(ram,table+4*step);
    const float b=step==last?0.0f:real(ram,table+4*(step+1));
    return h==0?a:h==1?b:float(double(a)+(double(b)-a)*h);
}
void evaluate_pak(uint8_t* ram,double h) {
    uint8_t* rdram=ram;
    const int old=word(ram,0x801B369C),next=word(ram,0x801C6390),step=word(ram,0x801C638C);
    // The menu owns 32 descriptors. Reject foreign/uninitialized selectors.
    if(old<0||old>=32||next<0||next>=32||step<0)return;
    const uint32_t from=0x801C29C4+152*unsigned(old),to=0x801C29C4+152*unsigned(next);
    const int a=word(ram,from),b=word(ram,to);
    if((state.domains&PakCursor)&&step<8&&a>=0&&a<3&&b>=0&&b<3) {
        const float k=curve(ram,0x801C3B30,unsigned(step),h,7);
        for(unsigned j=0;j<2;++j) {
            const int start=MEM_H(int32_t(4*a+2*j),int32_t(0x801C3B24));
            const int end=MEM_H(int32_t(4*b+2*j),int32_t(0x801C3B24));
            publish(ram,j?0x801C6380:0x801C6370,int32_t(float(end)+float(start-end)*k));
        }
    }
    if(state.domains&PakSize) {
        const int phase=step-(a==b?3:9);
        if(phase>=0&&phase<3) {
            const float k=curve(ram,a==b?0x801C3B54:0x801C3B64,unsigned(phase),h,3);
            for(unsigned j=0;j<2;++j) {
                const int start=MEM_H(int32_t(4+2*j),int32_t(from)),end=MEM_H(int32_t(4+2*j),int32_t(to));
                publish(ram,j?0x801C6368:0x801C6364,int32_t(float(end)+float(start-end)*k));
            }
        }
    }
    if((state.domains&PakHint)&&word(ram,0x801C636C)==0) {
        const int v=word(ram,0x801C63A8);
        if(!word(ram,0x801C6398)&&v<6)publish(ram,0x801C63A8,int32_t(double(v)+h));
        else if(word(ram,0x801C6398)&&v>=0)publish(ram,0x801C63A8,int32_t(double(v)-h));
    }
}
void evaluate_title(uint8_t* ram,recomp_context* ctx) {
    auto* scratch=reinterpret_cast<uint8_t*>(state.working.data());std::memcpy(scratch,ram,image_size);
    recomp_context local;dora3::copy_recomp_context(local,*ctx);local.r29=int32_t(0x803FD000);
    const int mode=word(scratch,0x801C647C);
    using Function=void (*)(uint8_t*,recomp_context*);
    const std::array<Function,4> functions{dora3_s2_menu_801BBCEC,dora3_s2_menu_801BBEB0,dora3_s2_menu_801BC17C,dora3_s2_menu_801BC4C0};
    if(mode<0||mode>=int(functions.size()))return;
    state.equations=true;state.drifts.clear();
    try {for(unsigned i=0;i<8;++i){local.r4=i;functions[mode](scratch,&local);}}
    catch(...){state.equations=false;state.drifts.clear();throw;}
    state.equations=false;state.drifts.clear();
    for(unsigned i=0;i<8;++i)for(unsigned offset:{0u,4u,8u,12u}) {
        const uint32_t a=0x801C6488+i*0x34+offset;publish(ram,a,word(scratch,a));
    }
}
void evaluate_selection(uint8_t* ram,recomp_context* ctx) {
    auto* scratch=reinterpret_cast<uint8_t*>(state.working.data());std::memcpy(scratch,ram,image_size);
    recomp_context local;dora3::copy_recomp_context(local,*ctx);local.r29=int32_t(0x803FD000);
    const float phase=real(scratch,0x801C69CC);
    const double angle=phase>=360?0:double(phase)+10*state.h;
    uint8_t* rdram=scratch;
    const double radians=std::bit_cast<double>(uint64_t(LD(int32_t(0x801C6350),0)));
    local.f12.fl=float(angle*radians);dora3_lib_80057A80(scratch,&local);
    real(scratch,0x801C69F4,local.f0.fl*10);
    state.equations=true;
    try {dora3_s2_menu_801C14B8(scratch,&local);}catch(...){state.equations=false;throw;}
    state.equations=false;
    // This reviewed menu actor lives in the resident fixed object array.
    // Its command flags and phase/counter writes remain private.
    if(word(ram,0x8017255C))for(uint32_t a:{0x801724CCu,0x801724D0u})publish(ram,a,word(scratch,a));
    const int y=word(ram,0x801C69C4);
    if((state.domains&SelectionExit)&&y>=105)publish(ram,0x801C69C4,int32_t(double(y)-state.h));
    if(state.domains&SelectionRow) {
        const int line=word(ram,0x801C69E0);
        if(line>20)publish(ram,0x801C69E0,int32_t(double(line)-state.h));
    }
}
}
extern "C" int dora3_native_ui_equations_active(){return state.equations;}
extern "C" float dora3_native_ui_rate_f32(float value){return state.equations?float(double(value)*state.h):value;}
extern "C" double dora3_native_ui_rate_f64(double value){return state.equations?value*state.h:value;}
extern "C" double dora3_native_ui_selection_y_f64(double value,float,double gain){return state.equations?value-gain*(1-state.h):value;}
extern "C" float dora3_native_ui_velocity_f32(float velocity,float acceleration,uint32_t a) {
    if(!state.equations)return velocity+acceleration;
    const float endpoint=(a-0x801C6488)%0x34==0x1C?std::min(velocity+acceleration,30.0f):velocity+acceleration;
    const double effective=double(endpoint)-velocity;
    const float next=state.h==1?endpoint:float(double(velocity)+effective*state.h);
    const float drift=state.h==1?next:float(double(velocity)*state.h+0.5*effective*state.h*(state.h+1));
    state.drifts.push_back({a,next,drift});return next;
}
extern "C" float dora3_native_ui_displacement_f32(float value,uint32_t a) {
    if(!state.equations)return value;
    for(const auto& d:state.drifts)if(d.address==a&&std::bit_cast<uint32_t>(d.velocity)==std::bit_cast<uint32_t>(value))return d.displacement;
    return float(double(value)*state.h);
}
namespace dora3::native_time {
bool native_ui_menu_eligible(uint8_t* ram) {
    const auto p=profile(ram);const int divider=word(ram,0x801A39DC);
    return p!=Profile::None&&p!=Profile::Game&&p!=Profile::Image&&(divider==2||divider==3)&&word(ram,0x801A9C88)==0;
}
bool native_ui_game_eligible(uint8_t* ram){return profile(ram)==Profile::Game&&(word(ram,0x801A39DC)==2||word(ram,0x801A39DC)==3)&&word(ram,0x801A9C88)==0;}
void native_ui_reset(){if(state.frame)throw std::logic_error("UI reset during frame");state=UiState{};observation={};}
void native_ui_begin(uint8_t* ram,Nanoseconds now,unsigned divider) {
    const auto completed=observation;native_ui_reset();observation=completed;
    state.owner=profile(ram);state.ram=ram;state.divider=divider;state.source_time=now;
    if(observation.ram==ram&&observation.owner==state.owner&&!observation.running)state.domains=observation.mask;
    state.working.resize(image_size/4);
    state.guards[0].pc=0x801B6E1C;state.guards[0].address=0x801B8D98;state.guards[0].size=8*0x24;
    state.guards[1].pc=0x801B7DCC;state.guards[1].address=0x801B8D38;state.guards[1].size=12;
    state.guards[2].pc=0x801BDEFC;state.guards[2].address=0x801C673C;state.guards[2].size=0x18C;
    state.guards[3].pc=0x801C22C4;state.guards[3].address=0x801C69EC;state.guards[3].size=4;
    state.guards[4].pc=0x801B63B8;state.guards[4].address=0x801B8D10;state.guards[4].size=0x30;
    // Files root also advances decoration6840, prompt6700, arrow68C0 and
    // highlight681C outside the row helper. Preserve its complete draw state.
    state.guards[5].pc=0x801BFC44;state.guards[5].address=0x801C6700;state.guards[5].size=0x1C8;
    // Only the three fields mutated by Pak drawing. Cursor/size equations and
    // save/device state never enter these snapshots; two disjoint fields share
    // A858's entry/exit, so the boundary visits every matching guard.
    state.guards[6].pc=0x801B96F8;state.guards[6].address=0x801C6384;state.guards[6].size=4;
    state.guards[7].pc=0x801BA858;state.guards[7].address=0x801C6378;state.guards[7].size=4;
    state.guards[8].pc=0x801BA858;state.guards[8].address=0x801C6388;state.guards[8].size=4;
}
UiFrameScope::UiFrameScope(uint8_t* ram,recomp_context* ctx,Nanoseconds now):ram_(ram) {
    uint8_t* rdram=ram;
    if(state.frame)throw std::logic_error("Nested UI frame");
    if(state.ram!=ram||state.owner==Profile::None||profile(ram)!=state.owner){ram_=nullptr;return;}
    state.frame=true;state.fields.clear();
    state.first_frame=state.frame_epoch!=state.epoch;state.frame_epoch=state.epoch;
    for(auto& g:state.guards){g.call=0;if(state.first_frame)g.originals.clear();}
    state.h=std::clamp(double(std::max<Nanoseconds>(0,now-state.source_time))*60.0/(state.divider*1e9),0.0,1.0);
    try {
        if(state.owner==Profile::Game) {
            if(state.domains&Panels)evaluate_panels(ram,state.h);
            if((state.domains&Rectangle)&&state.h>0&&MEM_H(0,int32_t(0x8021811A))==1) {
                auto* scratch=reinterpret_cast<uint8_t*>(state.working.data());std::memcpy(scratch,ram,image_size);
                constexpr uint64_t denominator=3000000000ull;
                SimulationScope time({uint64_t(state.h*denominator),denominator,state.epoch,0});
                dora3_native_rectangle_update(scratch,ctx);
                for(uint32_t a:{0x8021811Cu,0x80217194u,0x802171E4u,0x80217198u})publish(ram,a,word(scratch,a));
            }
        } else if(state.owner==Profile::Pak)evaluate_pak(ram,state.h);
        else if(state.owner==Profile::Title&&(state.domains&Title)&&state.h>0)evaluate_title(ram,ctx);
        else if(state.owner==Profile::Selection&&(state.domains&SelectionActor)&&state.h>0)evaluate_selection(ram,ctx);
        else if(state.owner==Profile::Intro&&(state.domains&Intro)) {
            const int mode=word(ram,0x801C68DC);const int alpha=word(ram,0x801C69A8);
            if(mode==0||mode==2)publish(ram,0x801C69A8,int32_t(std::clamp(double(alpha)+(mode==0?10:-10)*state.h,0.0,255.0)));
        }
    } catch(...) {for(auto i=state.fields.rbegin();i!=state.fields.rend();++i)word(ram,i->address)=i->source;state.fields.clear();state.frame=false;throw;}
}
UiFrameScope::~UiFrameScope(){if(!ram_)return;for(auto& g:state.guards)if(g.entered){if(g.restoring)std::memcpy(ram_+(g.address&0xFFFFFF),g.bytes.data(),g.size);g.entered=false;}for(auto i=state.fields.rbegin();i!=state.fields.rend();++i)word(ram_,i->address)=i->source;state.fields.clear();state.frame=false;}
}
extern "C" void dora3_native_ui_source_begin(uint8_t* ram,recomp_context* ctx) {
    if(ctx->r4!=0||state.frame)return;
    observation={ram,profile(ram),0,true};
}
extern "C" void dora3_native_ui_domain(uint8_t* ram,unsigned domain) {
    if(observation.running&&observation.ram==ram&&!state.frame&&!state.equations)observation.mask|=domain;
}
extern "C" void dora3_native_ui_source(uint8_t* ram,recomp_context* ctx) {
    if(ctx->r21!=0)return;
    observation.running=false;
    if(!dora3_native_loop_owns_fade()||state.ram!=ram)return;
    if(state.frame)throw std::logic_error("Source update inside UI frame");
    ++state.epoch;state.source_time=dora3::native_time::monotonic_time();
    state.domains=observation.ram==ram&&observation.owner==state.owner?observation.mask:0;
}
extern "C" void dora3_native_ui_draw_boundary(uint8_t* ram,uint32_t pc,unsigned end) {
    if(!state.frame||state.ram!=ram)return;
    for(auto& g:state.guards)if(g.pc==pc) {
        if(!end) {
            if(g.entered)throw std::logic_error("Nested UI drawing");
            g.entered=true;
            g.restoring=!state.first_frame;g.epoch=state.epoch;
            auto* data=ram+(g.address&0xFFFFFF);
            if(g.restoring){
                g.bytes.assign(data,data+g.size);
                if(g.call<g.originals.size())std::memcpy(data,g.originals[g.call].data(),g.size);
            } else g.originals.emplace_back(data,data+g.size);
            ++g.call;
        } else {
            if(!g.entered)throw std::logic_error("Unbalanced UI drawing");
            if(g.restoring)std::memcpy(ram+(g.address&0xFFFFFF),g.bytes.data(),g.size);
            g.entered=false;
        }
    }
}
extern "C" int dora3_native_ui_command_once(uint32_t pc) {
    if(!state.frame||pc!=0x801B63B8)return 1;
    // Preserve the Original call count even if one Source frame draws several
    // views. Only additional Native builds suppress the repeated request.
    return state.first_frame;
}
