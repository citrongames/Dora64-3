#include "diagnostics.hpp"
#include "native_ui.hpp"
#include "game_reset.hpp"
#include "cutscene_input.hpp"
#include <cstdio>
#include "native_simulation_time.hpp"
#include "native_effects.hpp"
#include "native_frame_bridge.hpp"
#include "native_model_animation.hpp"
#include "native_minigames.hpp"
#include "native_texture_animation.hpp"
#include "native_gameplay_preview.hpp"
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
void dora3_s0_8002DA98(uint8_t*,recomp_context*);
void dora3_s0_8002F064(uint8_t*,recomp_context*);
int dora3_native_loop_gameplay_active();
int dora3_overlay_loaded(uint32_t);
int dora3_native_loop_owns_fade();
void dora3_s2_menu_801BBCEC(uint8_t*,recomp_context*);
void dora3_s2_menu_801BBEB0(uint8_t*,recomp_context*);
void dora3_s2_menu_801BC17C(uint8_t*,recomp_context*);
void dora3_s2_menu_801BC4C0(uint8_t*,recomp_context*);
void dora3_s2_menu_801C14B8(uint8_t*,recomp_context*);
void dora3_lib_80057A80(uint8_t*,recomp_context*);
void dora3_s3_801DB7C8(uint8_t*,recomp_context*);
void dora3_s3_801D6EAC(uint8_t*,recomp_context*);
void dora3_s3_801D8FF0(uint8_t*,recomp_context*);
void dora3_s3_801D7294(uint8_t*,recomp_context*);
void dora3_s3_801D5864(uint8_t*,recomp_context*);
void dora3_s3_801D8578(uint8_t*,recomp_context*);
void dora3_s3_801DB720(uint8_t*,recomp_context*);
void dora3_s3_801D9B00(uint8_t*,recomp_context*);
void dora3_s3_801D8A24(uint8_t*,recomp_context*);
void dora3_s3_801D9C94(uint8_t*,recomp_context*);
void dora3_s3_801DA06C(uint8_t*,recomp_context*);
void dora3_s3_801DAB58(uint8_t*,recomp_context*);
void dora3_s3_801DA588(uint8_t*,recomp_context*);
void dora3_s3_801DAF50(uint8_t*,recomp_context*);
void dora3_s3_801DB3D0(uint8_t*,recomp_context*);
}
namespace {
using namespace dora3::native_time;
constexpr uint32_t image_size=0x800000,processes=0x801A44F0,panels=0x801B8AE0;
int32_t& word(uint8_t* rdram,uint32_t a){return MEM_W(0,int32_t(a));}
float real(uint8_t* rdram,uint32_t a){return std::bit_cast<float>(word(rdram,a));}
void real(uint8_t* rdram,uint32_t a,float f){word(rdram,a)=std::bit_cast<int32_t>(f);}
enum class Profile { None,Game,Title,Files,Intro,Selection,Image,StageMap,AuxiliaryViews,AuxiliaryWorldView };
Profile profile(uint8_t* ram) {
    // Overlay-qualified callback pairs, not overlapping virtual addresses alone.
    if(word(ram,processes)!=2||word(ram,processes+20)!=2||word(ram,processes+12)!=0||word(ram,processes+32)!=1)return Profile::None;
    const uint32_t update=uint32_t(word(ram,processes+16)),draw=uint32_t(word(ram,processes+36));
    for(unsigned i=2;i<8;++i)if(word(ram,processes+i*20)!=0&&word(ram,processes+i*20)!=5)return Profile::None;
    if(dora3_overlay_loaded(0x70A50)&&update==0x801BB730&&draw==0x801BB848)return Profile::Game;
    if(dora3_overlay_loaded(0x59E80)&&update==0x801B59D8&&draw==0x801B5A70)return Profile::AuxiliaryViews;
    if(dora3_overlay_loaded(0x70A50)&&update==0x801F9B58&&draw==0x801F9C88)return Profile::AuxiliaryWorldView;
    if(!dora3_overlay_loaded(0x635C0))return Profile::None;
    if(update==0x801BB52C&&draw==0x801BB648)return Profile::StageMap;
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
enum Domain : unsigned { Panels=1,Rectangle=2,Title=4,Intro=8,SelectionActor=16,SelectionRow=32,SelectionExit=64,MapCamera=128,MapMarker=256,MapHint=512,GameMenuSpin=1024,GameMenuGhost=2048,GameMenuCarousel=4096,AuxiliaryCamera=8192,OrdinaryCamera=16384 };
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
    std::array<Guard,13> guards{};
    uint32_t bitmap_start=0,background_start=0;
    int bitmap_dx=0,bitmap_dy=0;
    struct Drift {uint32_t address;float velocity,displacement;};
    std::vector<Drift> drifts;
    struct ActorDrawRestore {
        uint32_t actor,active,model;
        std::array<int32_t,19> fields;
        uint8_t dirty;
    };
    std::vector<ActorDrawRestore> actors;
    std::array<float,4> map_rectangle{};
    bool map_rectangle_valid=false;
} state;
void publish(uint8_t* ram,uint32_t a,int32_t value){state.fields.push_back({a,word(ram,a)});word(ram,a)=value;}
// Menu indices612..648 are preserved in the relocated actor table, outside
// the ordinary common range30..599. Their draw-derived matrix/world coordinates must be
// restored together with the borrowed local pose before the next Source update.
bool guest_pointer(uint32_t p,unsigned bytes) {
    return !(p&3)&&p>=0x80000000u&&uint64_t(p)+bytes<=0x80800000ull;
}
void lease_menu_actor(uint8_t* ram,uint32_t actor) {
    if(std::any_of(state.actors.begin(),state.actors.end(),[&](const auto& v){return v.actor==actor;}))return;
    uint8_t* rdram=ram;
    UiState::ActorDrawRestore v{actor,uint32_t(word(ram,actor+0x94)),uint32_t(word(ram,actor+0x74)),{},MEM_BU(0x6A,int32_t(actor))};
    for(unsigned i=0;i<16;++i)v.fields[i]=word(ram,actor+0x24+4*i);
    for(unsigned i=0;i<3;++i)v.fields[16+i]=word(ram,actor+0x9C+4*i);
    state.actors.push_back(v);
}
void restore_menu_actors(uint8_t* ram) {
    uint8_t* rdram=ram;
    for(const auto& actor:state.actors) {
        if(uint32_t(word(ram,actor.actor+0x94))!=actor.active||uint32_t(word(ram,actor.actor+0x74))!=actor.model)continue;
        for(unsigned i=0;i<16;++i)word(ram,actor.actor+0x24+4*i)=actor.fields[i];
        for(unsigned i=0;i<3;++i)word(ram,actor.actor+0x9C+4*i)=actor.fields[16+i];
        MEM_B(0x6A,int32_t(actor.actor))=int8_t(actor.dirty);
    }
}
void build_menu_matrices(uint8_t* ram,recomp_context* ctx) {
    if(state.actors.empty())return;
    const auto stack=uint32_t(ctx->r29)-0x800;
    if(!guest_pointer(stack,0xA00))throw std::logic_error("Invalid Native menu math stack");
    struct StackLease {
        uint8_t* ram;uint32_t stack;std::array<uint32_t,0xA00/4> saved;
        StackLease(uint8_t* image,uint32_t address):ram(image),stack(address) {
            std::memcpy(saved.data(),ram+(stack&0xFFFFFF),sizeof(saved));
        }
        ~StackLease(){std::memcpy(ram+(stack&0xFFFFFF),saved.data(),sizeof(saved));}
    } scratch(ram,stack);
    for(const auto& actor:state.actors) {
        recomp_context math;dora3::copy_recomp_context(math,*ctx);math.r4=int32_t(actor.actor);
        dora3_s0_8002DA98(ram,&math);
    }
}
void publish_menu_float(uint8_t* ram,uint32_t actor,unsigned offset,float value) {
    lease_menu_actor(ram,actor);publish(ram,actor+offset,std::bit_cast<int32_t>(value));
}
double rom_double(uint8_t* ram,uint32_t address) {
    uint8_t* rdram=ram;return std::bit_cast<double>(uint64_t(LD(int32_t(address),0)));
}
double normalized_angle(double degrees) {
    const double value=std::fmod(degrees,360.);return value<0?value+360.:value;
}
void evaluate_game_menus(uint8_t* ram,double h) {
    uint8_t* rdram=ram;
    const int menu=word(ram,0x8021D484);
    if(menu==1||menu==2) {
        // F1598/F4948 advance the highlighted item/weapon model's yaw by4/10.
        constexpr uint32_t selected=0x804D2130;
        if((state.domains&GameMenuSpin)&&word(ram,selected+0x94))publish_menu_float(ram,selected,0x10,float(normalized_angle(real(ram,selected+0x10)+(menu==1?4.:10.)*h)));
        // F1A38/F433C: preview copies rotate by1 and fade from Source count/20.
        // The short timer, show/hide flag and clip commands remain Source-owned.
        if(!(state.domains&GameMenuGhost))return;
        for(unsigned i=0;i<unsigned(menu==1?18:14);++i) {
            const uint32_t actor=0x804D12D8+i*0xCC;
            if(!word(ram,actor+0x94))continue;
            publish_menu_float(ram,actor,0x10,float(normalized_angle(real(ram,actor+0x10)+h)));
            const uint32_t driver=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74));
            if(!guest_pointer(driver,0x36)||!guest_pointer(model,0x84)||!game_model_animation().lifetime_token(model))continue;
            const int count=MEM_H(0x34,int32_t(driver));
            if(count>0)publish(ram,model+0x78,std::bit_cast<int32_t>(float(std::max(0.,double(count)-h)/20.)));
        }
    } else if(menu==3&&(state.domains&GameMenuCarousel)) {
        // F6870 stores trunc(shortBF70*ROM retention) before deriving the six
        // character positions on both circles. Extend that exact scalar step;
        // never blend stored XYZ poses or advance input/selection/short counters.
        const int phase=MEM_H(0,int32_t(0x8021BF70));
        const double retention=rom_double(ram,0x80216EF8);
        if(!std::isfinite(retention)||retention<0||retention>1)return;
        const int next=int(double(phase)*retention);
        const double shown=phase&&next&&double(next)/phase>0?
            double(phase)*std::pow(double(next)/phase,h):double(phase)*(1-h);
        const int selected=MEM_B(0,int32_t(0x8021BFF4));
        if(selected<0||selected>=6)return;
        for(unsigned group=0;group<2;++group)for(unsigned i=0;i<6;++i) {
            const uint32_t actor=(group?0x804CFFB8u:0x804CFAF0u)+i*0xCC;
            const uint32_t driver=uint32_t(word(ram,actor+0x98));
            if(!word(ram,actor+0x94)||!guest_pointer(driver,0x36))continue;
            const double angle=normalized_angle(double(MEM_H(0x34,int32_t(driver)))-selected*60.+shown);
            const double old_angle=normalized_angle(double(MEM_H(0x34,int32_t(driver)))-selected*60.+phase);
            const double coefficient=rom_double(ram,group?0x80216F08:0x80216F00);
            const float radians=float(double(float(angle))*coefficient);
            const float old_radians=float(double(float(old_angle))*coefficient);
            // Anchor the authored scalar equation at its actual Source result.
            // h=0 preserves Source XYZ bit-for-bit despite SDK trig rounding.
            publish_menu_float(ram,actor,0,float(double(real(ram,actor))+120.*(std::sin(radians)-std::sin(old_radians))));
            publish_menu_float(ram,actor,8,float(double(real(ram,actor+8))+120.*(std::cos(radians)-std::cos(old_radians))));
        }
    }
}
// Geometry is borrowed for this frame, just like menu actor poses. Source
// continues to own held-input selection, scene commands and every timer.
void publish_camera_geometry(uint8_t* ram,recomp_context* ctx,uint8_t* scratch,unsigned count) {
    for(unsigned i=0;i<count;++i) {
        const auto camera=0x801A99F8u+i*0x34;
        recomp_context math;dora3::copy_recomp_context(math,*ctx);math.r4=int32_t(camera);
        math.r29=int32_t(0x803FD000);dora3_s0_8002F064(scratch,&math);
        for(unsigned offset=0;offset<0x34;offset+=4)publish(ram,camera+offset,word(scratch,camera+offset));
    }
}
void evaluate_auxiliary_cameras(uint8_t* ram,recomp_context* ctx) {
    auto* scratch=reinterpret_cast<uint8_t*>(state.working.data());std::memcpy(scratch,ram,image_size);
    uint8_t* rdram=ram;
    const bool four=state.owner==Profile::AuxiliaryViews;
    const uint16_t held=MEM_H(0,int32_t(four?0x801A9504u:0x801A9AD0u));
    const auto camera=0x801A99F8u;
    const double angle_step=(four?2.:4.)*state.h;
    auto turn=[&](uint32_t at,double delta){real(scratch,at,float(normalized_angle(double(real(scratch,at))+delta)));};
    // 5788's two yaw and pitch branches are independent, including both held
    // directions.96E0 uses the selected-controller held snapshot copied by300C4.
    if(held&(four?2:1))turn(camera+0x1C,angle_step);
    if(held&(four?1:2))turn(camera+0x1C,-angle_step);
    if(held&8)turn(camera+0x18,angle_step);
    if(held&4)turn(camera+0x18,-angle_step);
    const float floor=four?50.f:20.f;
    if(held&0x400)real(scratch,camera+0x30,float(double(real(scratch,camera+0x30))+floor*state.h));
    if((held&0x800)&&(!four||!(held&0x400)))real(scratch,camera+0x30,std::max(floor,float(double(real(scratch,camera+0x30))-floor*state.h)));
    if(four) {
        turn(camera+0x34+0x1C,8.*state.h);turn(camera+0x68+0x1C,-8.*state.h);
        const auto distance=camera+0x9C+0x30;
        const float current=real(scratch,distance);
        // >2000 ->50 is a Source cut, not a velocity. It must occur once in
        // the real callback, without predicting that reset in extra frames.
        if(current<=2000.f)real(scratch,distance,float(double(current)+10.*state.h));
    }
    publish_camera_geometry(ram,ctx,scratch,four?4:1);
}
bool ordinary_camera_observed(uint8_t* ram) {
    return (state.ram==ram&&state.owner==Profile::Game&&(state.domains&OrdinaryCamera))||
        (!observation.running&&observation.ram==ram&&observation.owner==Profile::Game&&(observation.mask&OrdinaryCamera));
}
bool ordinary_camera_scene_stable(uint8_t* ram) {
    uint8_t* rdram=ram;
    return word(ram,0x80218AD8)==0&&word(ram,0x8021D484)==0&&
        word(ram,0x802170EC)==0&&word(ram,0x802170F0)==0&&MEM_BU(0,int32_t(0x80217118))==0&&
        word(ram,0x801AA648)&&word(ram,0x801A9500)&&ordinary_camera_observed(ram);
}
void evaluate_ordinary_camera_frame(uint8_t* ram,recomp_context* ctx) {
    uint8_t* rdram=ram;const unsigned mode=MEM_BU(0,int32_t(0x801B361A));
    using Function=void (*)(uint8_t*,recomp_context*);
    constexpr std::array<Function,16> updates{dora3_s3_801DB7C8,dora3_s3_801DB7C8,dora3_s3_801D6EAC,dora3_s3_801D8FF0,dora3_s3_801D7294,dora3_s3_801D5864,dora3_s3_801D8578,dora3_s3_801DB720,dora3_s3_801D9B00,dora3_s3_801D8A24,dora3_s3_801D9C94,dora3_s3_801DA06C,dora3_s3_801DAB58,dora3_s3_801DA588,dora3_s3_801DAF50,dora3_s3_801DB3D0};
    if(mode>=updates.size())return;
    auto* scratch=reinterpret_cast<uint8_t*>(state.working.data());std::memcpy(scratch,ram,image_size);
    recomp_context local;dora3::copy_recomp_context(local,*ctx);
    local.r29=int32_t(0x803FD000);local.r4=int32_t(0x801A99F8);
    constexpr uint64_t denominator=3000000000ull;
    // Only the reached camera equation runs in the private image. DBD58's
    // input/mode-change prefix, world/player dispatch and resource commands are
    // absent. The real Source handler remains authoritative at the next tick.
    SimulationScope time({uint64_t(state.h*denominator),denominator,state.epoch,0});
    updates[mode](scratch,&local);
    publish_camera_geometry(ram,ctx,scratch,2);
}
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
void evaluate_stage_map(uint8_t* ram,double h) {
    uint8_t* rdram=ram;
    const int old=word(ram,0x801B369C),next=word(ram,0x801C6390),step=word(ram,0x801C638C);
    // Source has24 rows at2940, stride152; the area/marker fields are at+84.
    // The -1 sentinel at3780 precedes the unrelated bitmap resource table.
    if(old<0||old>=24||next<0||next>=24||step<0)return;
    const uint32_t from=0x801C29C4+152*unsigned(old),to=0x801C29C4+152*unsigned(next);
    const int a=word(ram,from),b=word(ram,to);
    for(unsigned j=0;j<4;++j)state.map_rectangle[j]=float(word(ram,std::array<uint32_t,4>{0x801C6370,0x801C6380,0x801C6364,0x801C6368}[j]));
    state.map_rectangle_valid=true;
    if((state.domains&MapCamera)&&step>0&&step<8&&a>=0&&a<4&&b>=0&&b<4) {
        for(unsigned j=0;j<2;++j) {
            // Four short pairs at3B24 include (0,240) for the fourth area.
            // 3B30 also names the easing table, but its used indices are1..7:
            // index0 overlaps that fourth anchor and is never a float coefficient.
            const int start=MEM_H(int32_t(4*a+2*j),int32_t(0x801C3B24));
            const int end=MEM_H(int32_t(4*b+2*j),int32_t(0x801C3B24));
            // BAF64 decrements the counter. The next authored endpoint is
            // step-1, including the exact current-area anchor at zero.
            const float ka=real(ram,0x801C3B30+4*unsigned(step));
            const float kb=step==1?1.f:real(ram,0x801C3B30+4*unsigned(step-1));
            const int source=int(float(end)+float(start-end)*ka),next=int(float(end)+float(start-end)*kb);
            state.map_rectangle[j]=float(double(source)+(next-source)*h);
            publish(ram,j?0x801C6380:0x801C6370,int32_t(state.map_rectangle[j]));
        }
    }
    if(state.domains&MapMarker) {
        const int phase=step-(a==b?3:9);
        if(phase>0&&phase<=3) {
            const auto table=a==b?0x801C3B54u:0x801C3B64u;
            const float ka=real(ram,table+4*unsigned(phase)),kb=phase==1?1.f:real(ram,table+4*unsigned(phase-1));
            for(unsigned j=0;j<2;++j) {
                const int start=MEM_H(int32_t(4+2*j),int32_t(from)),end=MEM_H(int32_t(4+2*j),int32_t(to));
                const int source=int(float(end)+float(start-end)*ka),next=int(float(end)+float(start-end)*kb);
                state.map_rectangle[2+j]=float(double(source)+(next-source)*h);
                publish(ram,j?0x801C6368:0x801C6364,int32_t(state.map_rectangle[2+j]));
            }
        }
    }
    if((state.domains&MapHint)&&word(ram,0x801C636C)==0) {
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
    if(word(ram,0x804B255C))for(uint32_t a:{0x804B24CCu,0x804B24D0u})publish_menu_float(ram,0x804B24C8,a-0x804B24C8,std::bit_cast<float>(word(scratch,a)));
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
    const bool game_menu=p==Profile::Game&&word(ram,0x8021D484)>=1&&word(ram,0x8021D484)<=3&&
        !word(ram,0x802170EC)&&!word(ram,0x802170F0);
    uint8_t* rdram=ram;
    const int fade_direction=MEM_B(0,int32_t(0x801A99A0));
    // A fade is independent of the scene-exit timer. Room/interior holds,
    // paused story actions and the last count1 interval still draw this resident
    // callback pair. Keep its fade moving until ACTUAL cleanup/process install
    // drains the banks (B9FDC, BA2B8/BA254, 280DC), not at request time.
    // Eligible gameplay/cutscene policies retain priority; this fallback grants
    // presentation of the fade, never a new gameplay simulation owner.
    const bool game_fade=p==Profile::Game&&(fade_direction==-1||fade_direction==1);
    // Player control/physics eligibility does not own the camera. A stable
    // world whose actual Source callback reached one of the15 ordinary roots
    // can continue its camera equation even while the player domain is blocked.
    const bool game_camera=p==Profile::Game&&ordinary_camera_scene_stable(ram);
    return (game_menu||game_fade||game_camera||(p!=Profile::None&&p!=Profile::Game&&p!=Profile::Image))&&
        (divider==2||divider==3)&&word(ram,0x801A9C88)==0;
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
    // Only the three fields mutated by stage-map drawing. Camera/marker
    // equations and selection/input never enter these snapshots; two fields share
    // A858's entry/exit, so the boundary visits every matching guard.
    state.guards[6].pc=0x801B96F8;state.guards[6].address=0x801C6384;state.guards[6].size=4;
    state.guards[7].pc=0x801BA858;state.guards[7].address=0x801C6378;state.guards[7].size=4;
    state.guards[8].pc=0x801BA858;state.guards[8].address=0x801C6388;state.guards[8].size=4;
    // Item side arrows advance their two byte oscillator in the draw callback.
    state.guards[9].pc=0x801F0CB8;state.guards[9].address=0x8021BFCB;state.guards[9].size=1;
    state.guards[10].pc=0x801F0CB8;state.guards[10].address=0x8021BE42;state.guards[10].size=1;
    // The -94 choice token has a separate horizontal oscillator. The old
    // 8D38..8D40 guard covers only the -96 token, not this choice arrow.
    // Snapshot both disjoint words so additional builds cannot advance its
    // offset or reverse its direction; input/selected answer stays Source-owned.
    state.guards[11].pc=0x801B7DCC;state.guards[11].address=0x801B8D08;state.guards[11].size=4;
    state.guards[12].pc=0x801B7DCC;state.guards[12].address=0x801B8D88;state.guards[12].size=4;
}
UiFrameScope::UiFrameScope(uint8_t* ram,recomp_context* ctx,Nanoseconds now):ram_(ram) {
    uint8_t* rdram=ram;
    if(state.frame)throw std::logic_error("Nested UI frame");
    if(state.ram!=ram||state.owner==Profile::None||profile(ram)!=state.owner){ram_=nullptr;return;}
    state.frame=true;state.fields.clear();state.actors.clear();state.map_rectangle_valid=false;
    state.first_frame=state.frame_epoch!=state.epoch;state.frame_epoch=state.epoch;
    for(auto& g:state.guards){g.call=0;if(state.first_frame)g.originals.clear();}
    state.h=std::clamp(double(std::max<Nanoseconds>(0,now-state.source_time))*60.0/(state.divider*1e9),0.0,1.0);
    try {
        if(state.owner==Profile::Game) {
            evaluate_game_menus(ram,state.h);
            if(state.h>0&&!dora3_native_loop_gameplay_active()&&ordinary_camera_scene_stable(ram))evaluate_ordinary_camera_frame(ram,ctx);
            if(state.domains&Panels)evaluate_panels(ram,state.h);
            if((state.domains&Rectangle)&&state.h>0&&MEM_H(0,int32_t(0x8021811A))==1) {
                auto* scratch=reinterpret_cast<uint8_t*>(state.working.data());std::memcpy(scratch,ram,image_size);
                constexpr uint64_t denominator=3000000000ull;
                SimulationScope time({uint64_t(state.h*denominator),denominator,state.epoch,0});
                dora3_native_rectangle_update(scratch,ctx);
                for(uint32_t a:{0x8021811Cu,0x80217194u,0x802171E4u,0x80217198u})publish(ram,a,word(scratch,a));
            }
        } else if((state.owner==Profile::AuxiliaryViews||state.owner==Profile::AuxiliaryWorldView)&&
                  (state.domains&AuxiliaryCamera)&&state.h>0)evaluate_auxiliary_cameras(ram,ctx);
        else if(state.owner==Profile::StageMap)evaluate_stage_map(ram,state.h);
        else if(state.owner==Profile::Title&&(state.domains&Title)&&state.h>0)evaluate_title(ram,ctx);
        else if(state.owner==Profile::Selection&&(state.domains&SelectionActor)&&state.h>0)evaluate_selection(ram,ctx);
        else if(state.owner==Profile::Intro&&(state.domains&Intro)) {
            const int mode=word(ram,0x801C68DC);const int alpha=word(ram,0x801C69A8);
            if(mode==0||mode==2)publish(ram,0x801C69A8,int32_t(std::clamp(double(alpha)+(mode==0?10:-10)*state.h,0.0,255.0)));
        }
        build_menu_matrices(ram,ctx);
    } catch(...) {restore_menu_actors(ram);for(auto i=state.fields.rbegin();i!=state.fields.rend();++i)word(ram,i->address)=i->source;state.fields.clear();state.actors.clear();state.frame=false;throw;}
}
UiFrameScope::~UiFrameScope(){if(!ram_)return;
    restore_menu_actors(ram_);
    state.actors.clear();for(auto& g:state.guards)if(g.entered){if(g.restoring)std::memcpy(ram_+((g.address&0xFFFFFF)^(g.size==1?3u:0u)),g.bytes.data(),g.size);g.entered=false;}for(auto i=state.fields.rbegin();i!=state.fields.rend();++i)word(ram_,i->address)=i->source;state.fields.clear();state.frame=false;}
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
    if(!state.frame&&!state.equations&&!active_simulation_slice()) {
        if(dora3::cutscene_input_source_completed(ram)) {
            DORA3_LOG_TEXT("Cutscene Start: retained request after skip-thread initialization\n",stderr);
            dora3_capture_controller_state(ram,"early-start-retained");
        }
        dora3_capture_controller_state(ram,"source-complete");
    }
    observation.running=false;
    if(!dora3_native_loop_owns_fade())return;
    if(state.frame)throw std::logic_error("Source update inside UI frame");
    const auto completed=dora3::native_time::monotonic_time();
    // SDK model observations previously started at phase0 entry, while menu
    // and minigame equations started after their callbacks. Real callback CPU
    // time then changed the relative body/model/camera phase every Source tick.
    // Commit all borrowed visual owners at the same completed Source boundary.
    game_model_animation().resume_frame_clock(ram,completed);
    native_minigame_source_completed(ram,completed);
    dora3::native_time::native_texture_source_completed(ram,completed);
    if(state.ram!=ram)return;
    ++state.epoch;state.source_time=completed;
    state.domains=observation.ram==ram&&observation.owner==state.owner?observation.mask:0;
}
extern "C" void dora3_native_ui_draw_boundary(uint8_t* ram,uint32_t pc,unsigned end) {
    if(!state.frame||state.ram!=ram)return;
    if(pc==0x801BA0CC&&state.owner==Profile::StageMap&&state.map_rectangle_valid) {
        if(!end)state.background_start=uint32_t(word(ram,0x801B1424));
        else {
            const auto finish=uint32_t(word(ram,0x801B1424));
            const int dx=int(std::lround((state.map_rectangle[0]-word(ram,0x801C6370))*32.));
            const int dy=int(std::lround((state.map_rectangle[1]-word(ram,0x801C6380))*32.));
            if(guest_pointer(state.background_start,8)&&finish>=state.background_start&&
               finish-state.background_start<=0x10000&&guest_pointer(finish,0))
                for(auto at=state.background_start;at+8<=finish;at+=8)
                    if(uint32_t(word(ram,at))==0xE1000000u) {
                        const uint32_t uv=uint32_t(word(ram,at+4));
                        word(ram,at+4)=int32_t((uint32_t(uint16_t(int16_t(uv>>16)+dx))<<16)|uint16_t(int16_t(uv)+dy));
                    }
            state.background_start=0;
        }
    }
    for(auto& g:state.guards)if(g.pc==pc) {
        if(!end) {
            if(g.entered)throw std::logic_error("Nested UI drawing");
            g.entered=true;
            g.restoring=!state.first_frame;g.epoch=state.epoch;
            auto* data=ram+((g.address&0xFFFFFF)^(g.size==1?3u:0u));
            if(g.restoring){
                g.bytes.assign(data,data+g.size);
                if(g.call<g.originals.size())std::memcpy(data,g.originals[g.call].data(),g.size);
            } else g.originals.emplace_back(data,data+g.size);
            ++g.call;
        } else {
            if(!g.entered)throw std::logic_error("Unbalanced UI drawing");
            if(g.restoring)std::memcpy(ram+((g.address&0xFFFFFF)^(g.size==1?3u:0u)),g.bytes.data(),g.size);
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

extern "C" void dora3_native_ui_charge_rectangle(uint8_t* ram,recomp_context* ctx) {
    if(!state.frame||state.ram!=ram||state.owner!=Profile::Game||word(ram,0x8021D484)!=0)return;
    const auto fraction=native_gameplay_preview_source_fraction();
    if(!fraction)return;
    uint8_t* rdram=ram;
    const int count=word(ram,0x8021D698),weapon=word(ram,0x801B36E0);
    // C261C increments only a held B charge, caps50, and resets on release.
    // Preserve its decisions/rumble/cue count; only the new rectangle borrows
    // a fractional scalar, before integer pixel rounding in the old HUD.
    if(count<=0||count>=50||!(MEM_HU(0,int32_t(0x8021D644))&0x4000)||weapon<0||weapon>=32||
       MEM_BU(0,int32_t(0x801FA5D9+12*unsigned(weapon)))!=2)return;
    const double progress=std::min(50.,double(count)+std::clamp(*fraction,0.,1.));
    const uint32_t right=uint32_t((187.+66.*progress/50.)*4.);
    const uint32_t source=uint32_t(ctx->r4);
    ctx->r4=int32_t((source&~0x00FFF000u)|((right&0xFFFu)<<12));
}

extern "C" int dora3_native_ui_map_cursor_rectangle(uint8_t* ram,recomp_context* ctx) {
    if(!state.frame||state.ram!=ram||state.owner!=Profile::StageMap||!state.map_rectangle_valid)return 0;
    // BAA14's delay slot converts an integer Y a second time. Supply the
    // authored fractional X/Y directly to the original scaled texture builder.
    // Bob direction/turning points and sprite commands remain Source-owned.
    const double bob=real(ram,0x801C6378);
    const double rate=word(ram,0x801C6388)?rom_double(ram,0x801C5330):-rom_double(ram,0x801C5338);
    const double shown_bob=std::clamp(bob+rate*state.h,-1.,1.);
    ctx->f12.fl=state.map_rectangle[2]-state.map_rectangle[0]-16.f;
    ctx->f14.fl=float(state.map_rectangle[3]-state.map_rectangle[1]-16.+shown_bob);
    return 1;
}

// The old bitmap builder rounds/clips/chunks integer texels. Keep its input
// integral, then translate the emitted rectangles as one rigid sprite. This
// preserves chunk adjacency and load bounds. At end, make each render tile
// exclusive of the extra loaded row/column and cancel RT64's fractional-top
// texel bias. The Source builder alone was safe only at integral positions.
extern "C" int dora3_native_ui_map_bitmap_begin(uint8_t* ram,recomp_context* c,unsigned node) {
    if(!state.frame||state.ram!=ram||state.owner!=Profile::StageMap||!state.map_rectangle_valid)return 0;
    if(state.bitmap_start)throw std::logic_error("Nested stage-map bitmap");
    if(node) {
        c->f12.fl+=float(word(ram,0x801C6370))-state.map_rectangle[0];
        c->f14.fl+=float(word(ram,0x801C6380))-state.map_rectangle[1];
    }
    const float x=c->f12.fl,y=c->f14.fl;
    c->f12.fl=std::floor(x);c->f14.fl=std::floor(y);
    state.bitmap_dx=int(std::lround((x-c->f12.fl)*4.));
    state.bitmap_dy=int(std::lround((y-c->f14.fl)*4.));
    state.bitmap_start=uint32_t(word(ram,0x801B1424));
    return 1;
}
extern "C" void dora3_native_ui_map_bitmap_end(uint8_t* ram) {
    if(!state.bitmap_start)return;
    const auto finish=uint32_t(word(ram,0x801B1424));
    if(guest_pointer(state.bitmap_start,8)&&finish>=state.bitmap_start&&
       finish-state.bitmap_start<=0x10000&&guest_pointer(finish,0)) {
        uint32_t render_tile=0,render_bounds=0;
        for(auto at=state.bitmap_start;at+8<=finish;at+=8) {
            const auto command=uint32_t(word(ram,at));
            if(command>>24==0xF5&&((uint32_t(word(ram,at+4))>>24)&7)==0)render_tile=at;
            if(command==0xF2000000u)render_bounds=at;
            if(command>>24!=0xE4)continue;
            for(unsigned offset:{0u,4u}) {
                const uint32_t packed=uint32_t(word(ram,at+offset));
                const auto x=((packed>>12)&0xFFF)+state.bitmap_dx;
                const auto y=(packed&0xFFF)+state.bitmap_dy;
                word(ram,at+offset)=int32_t((packed&0xFF000000u)|((x&0xFFF)<<12)|(y&0xFFF));
            }
            // Both reviewed map callers set scaleX=scaleY=1. Each SDK chunk
            // ends in F2 local bounds, E4, E1, F1(1,1). Do not rewrite loaders
            // or guess the footprint of an unrelated/scaled command sequence.
            if(!render_tile||!render_bounds||at+24>finish||
               uint32_t(word(ram,at+8))!=0xE1000000u||
               uint32_t(word(ram,at+16))!=0xF1000000u||
               uint32_t(word(ram,at+20))!=0x04000400u)continue;
            const auto bounds=uint32_t(word(ram,render_bounds+4));
            const unsigned width=(bounds>>12)&0xFFF,rows=bounds&0xFFF;
            const auto lower=uint32_t(word(ram,at)),upper=uint32_t(word(ram,at+4));
            const unsigned top=upper&0xFFF,bottom=lower&0xFFF;
            if(width<4||rows<4||(width&3)||(rows&3)||bottom<top||
               (bottom-top!=rows&&bottom-top+1!=rows))continue;
            // F4's inclusive extra row can contain bytes outside the BMP.
            // Clamp the render tile to actual texels, including filter taps;
            // this is a sampling bound, not a crop of the sprite geometry.
            word(ram,render_tile+4)=int32_t(uint32_t(word(ram,render_tile+4))|0x00080200u);
            word(ram,render_bounds+4)=int32_t((bounds&0xFF000000u)|((width-4)<<12)|(rows-4));
            // SDK subtracts one quarter pixel from its final strip. With a
            // fractional top, ceil-based texture span can lose a whole texel.
            // An exact rows-pixel extent keeps the UV slope at1 for every phase.
            word(ram,at)=int32_t((lower&0xFFFFF000u)|((top+rows)&0xFFF));
            // Pinned RT64 drawRect adds (dtdy>>5)/32 when uly has any fraction.
            // E1 uses signed10.5: cancel that whole-texel bias for rigid motion.
            const auto uv=uint32_t(word(ram,at+12));
            const int t=int16_t(uv)-(top&3?32:0);
            word(ram,at+12)=int32_t((uv&0xFFFF0000u)|uint16_t(t));
        }
    }
    state.bitmap_start=0;
}
