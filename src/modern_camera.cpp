#include "modern_camera.hpp"
#include "native_simulation_time.hpp"
#include "recomp_context_copy.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <mutex>

extern "C" {
int dora3_overlay_loaded(uint32_t);
uint64_t dora3_overlay_generation(uint32_t);
void dora3_s0_80040104(uint8_t*,recomp_context*);
void dora3_s0_8002F064(uint8_t*,recomp_context*);
void dora3_s3_801D55EC(uint8_t*,recomp_context*);
}
namespace dora3::camera {
namespace {
constexpr uint32_t Camera=0x801A99F8;
constexpr float Rad=.01745329251994329577f;
struct Vec {float x,y,z;Vec operator+(Vec b)const{return{x+b.x,y+b.y,z+b.z};}Vec operator-(Vec b)const{return{x-b.x,y-b.y,z-b.z};}Vec operator*(float s)const{return{x*s,y*s,z*s};}};
struct Input {float x=0,y=0,mx=0,my=0,zoom=0,wheel=0;bool center=false;};
std::mutex input_mutex;
Options options;
Input pending;
std::atomic_bool available{false};
std::atomic<int64_t> last_available{0};
std::atomic_uint revision{0};
struct Orbit {
    bool initialized=false;
    unsigned revision=0;
    uint64_t generation=0;
    uint32_t player=0;
    unsigned mode=0;
    float yaw=0,pitch=20,distance=300,actual=300,height=35,shake=0;
    Vec focus{}; // Absolute world position; original target is origin-relative.
    double frame_elapsed=0,center_remaining=0;
    float center_yaw=0;
} orbit;
float preferred_distance=300.f;
int64_t now_ms(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
int32_t& word(uint8_t* rdram,uint32_t a){return MEM_W(0,int32_t(a));}
float real(uint8_t* ram,uint32_t a){return std::bit_cast<float>(word(ram,a));}
void real(uint8_t* ram,uint32_t a,float v){word(ram,a)=std::bit_cast<int32_t>(v);}
Vec vector(uint8_t* ram,uint32_t a){return{real(ram,a),real(ram,a+4),real(ram,a+8)};}
bool finite(Vec v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
bool pointer(uint32_t p,unsigned size){return !(p&3)&&p>=0x80000000&&uint64_t(p)+size<=0x80800000;}
float wrap(float v){v=std::fmod(v,360.f);return v<0?v+360.f:v;}
float blend(float rate,double dt){return float(-std::expm1(-rate*dt));}
bool eligible(uint8_t* rdram) {
    if(!rdram||!dora3_overlay_loaded(0x70A50))return false;
    // Match the actual world owner, not VRAM shared with menu/minigame overlays.
    for(unsigned i=0;i<8;++i) {
        const auto p=0x801A44F0+i*20;
        if(i<2) {if(word(rdram,p)!=2||word(rdram,p+12)!=int(i)||uint32_t(word(rdram,p+16))!=(i?0x801BB848u:0x801BB730u))return false;}
        else if(word(rdram,p)!=0&&word(rdram,p)!=5)return false;
    }
    const auto mode=MEM_BU(0,int32_t(0x801B361A));
    if(mode<3||mode>5)return false;
    // Original world/room/script ownership; dialogue also releases free input.
    if(word(rdram,0x80218AD8)||word(rdram,0x8021D484)||word(rdram,0x8021D494)||
       word(rdram,0x802170EC)||word(rdram,0x802170F0)||MEM_BU(0,int32_t(0x80217118))||
       MEM_B(0,int32_t(0x8021710D))||MEM_BU(0,int32_t(0x801B36EC))||word(rdram,0x801A9C88)||
       !word(rdram,0x801AA648)||!word(rdram,0x801A9500))return false;
    const uint32_t actor=word(rdram,0x8021D49C),control=word(rdram,0x8021D6C0);
    return pointer(actor,0xCC)&&pointer(control,0x58)&&word(rdram,actor+0x94)!=0&&word(rdram,actor+0x94)!=66&&word(rdram,actor+0x94)!=67;
}
void revoke(){available=false;orbit.initialized=false;orbit.frame_elapsed=0;clear_input();}
Vec origin(uint8_t* ram){return{real(ram,0x800DAE54),real(ram,0x800FE878),real(ram,0x800DD3F0)};}
void call(uint8_t* ram,const recomp_context& ctx,recomp_func_t* fn,uint32_t a0) {
    recomp_context local;copy_recomp_context(local,ctx);local.r4=int32_t(a0);local.r29=ADD32(ctx.r29,-0x200);fn(ram,&local);
}
// 80040104 queries the original room grid and publishes two surface records
// (height, plane normal/flags) at DACB8/DACD4 and inside/outside bits at DACF0.
// Keep these query temporaries private to the camera; player collision owns its
// own most recent result. Original camera also uses this query through D5378.
struct QueryScope {
    uint8_t* ram;std::array<int32_t,15> saved;
    explicit QueryScope(uint8_t* p):ram(p){for(unsigned i=0;i<saved.size();++i)saved[i]=word(ram,0x800DACB8+4*i);}
    ~QueryScope(){for(unsigned i=0;i<saved.size();++i)word(ram,0x800DACB8+4*i)=saved[i];}
};
bool clear_point(uint8_t* ram,const recomp_context& ctx,Vec point) {
    recomp_context q;copy_recomp_context(q,ctx);q.r29=ADD32(ctx.r29,-0x200);
    q.f12.d=point.x;q.f14.d=point.y;
    uint8_t* rdram=ram;SD(std::bit_cast<uint64_t>(double(point.z)),0x10,q.r29);
    dora3_s0_80040104(ram,&q);
    const float floor=real(ram,0x800DACB8),ceiling=real(ram,0x800DACD4);
    return !(word(ram,0x800DACF0)&3)&&std::isfinite(floor)&&std::isfinite(ceiling)&&
        point.y>=floor+12.f&&point.y<=ceiling-12.f;
}
float clear_distance(uint8_t* ram,const recomp_context& ctx,Vec focus,Vec ray,float wanted) {
    if(!word(ram,0x800DAE64))return wanted; // Original no-room geometry case.
    QueryScope preserve(ram);
    float previous=0;
    // Probe the whole segment, not only the far endpoint (which could lie on
    // the other side of a wall). Original recenter grows this distance by20.
    for(float d=std::min(10.f,wanted);;d=std::min(d+10.f,wanted)) {
        if(!clear_point(ram,ctx,focus+ray*d)) {
            float high=d,low=previous;
            for(unsigned i=0;i<5;++i){float mid=(low+high)*.5f;if(clear_point(ram,ctx,focus+ray*mid))low=mid;else high=mid;}
            return std::max(1.f,low-12.f);
        }
        previous=d;if(d>=wanted)return wanted;
    }
}
void retire_recenter(uint8_t* rdram) {
    // Do not leave the Original five-tick Z turn holding control+1 forever
    // after taking over its root. Release only a turn we actually found.
    const unsigned mode=MEM_BU(0,int32_t(0x801B361A));
    const uint32_t a=mode==3?0x802181F2u:mode==4?0x80218260u:0x80218228u;
    const bool held=MEM_H(0,int32_t(a))!=0;
    if(held)MEM_H(0,int32_t(a))=0;
    if(held){uint32_t control=word(rdram,0x8021D6C0);if(pointer(control,0x58))MEM_B(1,int32_t(control))=0;}
}
bool update(uint8_t* rdram,recomp_context* ctx,double seconds,bool source,bool frame_update) {
    Options config;Input input;
    {std::lock_guard lock(input_mutex);config=options;}
    if(!config.enabled||!eligible(rdram)){revoke();return false;}
    const unsigned mode=MEM_BU(0,int32_t(0x801B361A));
    const uint32_t player=word(rdram,0x8021D49C);
    const auto generation=dora3_overlay_generation(0x70A50);
    const Vec world=origin(rdram),position=vector(rdram,player+0x9C);
    if(!finite(world)||!finite(position)){revoke();return false;}
    const bool acquire=!orbit.initialized||orbit.player!=player||orbit.generation!=generation||orbit.revision!=revision.load();
    if(acquire) {
        orbit={};orbit.initialized=true;orbit.player=player;orbit.generation=generation;orbit.revision=revision;
        orbit.yaw=real(rdram,Camera+0x1C);orbit.pitch=std::clamp(real(rdram,Camera+0x18),0.f,80.f);
        orbit.focus=vector(rdram,Camera)+world;orbit.distance=preferred_distance;
        orbit.actual=std::clamp(real(rdram,Camera+0x30),1.f,600.f);
        if(!finite(orbit.focus)||!std::isfinite(orbit.yaw)||!std::isfinite(orbit.pitch)||!std::isfinite(orbit.actual)){revoke();return false;}
        orbit.height=std::clamp(orbit.focus.y-position.y,20.f,100.f);
    }
    if(!frame_update)retire_recenter(rdram);
    orbit.mode=mode;
    double dt=seconds;
    if(frame_update){dt=std::max(0.,seconds-orbit.frame_elapsed);orbit.frame_elapsed=seconds;}
    else if(source){dt=std::max(0.,seconds-orbit.frame_elapsed);orbit.frame_elapsed=0;}
    else orbit.frame_elapsed=0;
    {std::lock_guard lock(input_mutex);input=pending;pending.mx=pending.my=pending.wheel=0;pending.center=false;}
    last_available=now_ms();available=true;
    const float sx=config.invert_x?-1.f:1.f,sy=config.invert_y?-1.f:1.f;
    orbit.yaw=wrap(orbit.yaw-sx*(input.x*config.stick_speed*float(dt)+input.mx*config.mouse_sensitivity));
    orbit.pitch=std::clamp(orbit.pitch+sy*(input.y*config.stick_speed*float(dt)+input.my*config.mouse_sensitivity),0.f,80.f);
    // Z is a camera command here; unlike Dora64 it is not crouch.
    if(input.x!=0||input.mx!=0)orbit.center_remaining=0;
    if(input.center){orbit.center_yaw=wrap(real(rdram,player+0x10)-180.f);orbit.center_remaining=5.*word(rdram,0x801A39DC)/60.;}
    if(orbit.center_remaining>0){
        const auto step=std::min(dt,orbit.center_remaining);
        orbit.yaw=wrap(orbit.yaw+std::remainder(orbit.center_yaw-orbit.yaw,360.f)*float(step/orbit.center_remaining));
        orbit.center_remaining-=step;
    }
    orbit.distance=std::clamp(orbit.distance+input.zoom*240.f*float(dt)-input.wheel*30.f,80.f,600.f);
    preferred_distance=orbit.distance;
    const Vec wanted=position+Vec{0,orbit.height,0};
    orbit.focus=orbit.focus+(wanted-orbit.focus)*blend(14.f,dt);
    // Same elevation/yaw convention as resident8002F064 (55BD0=cos,57A80=sin).
    const float cp=std::cos(orbit.pitch*Rad);
    const Vec ray{std::sin(orbit.yaw*Rad)*cp,std::sin(orbit.pitch*Rad),std::cos(orbit.yaw*Rad)*cp};
    const float allowed=clear_distance(rdram,*ctx,orbit.focus,ray,orbit.distance);
    orbit.actual=allowed<orbit.actual?allowed:orbit.actual+(allowed-orbit.actual)*blend(8.f,dt);
    const auto relative=orbit.focus-world;
    real(rdram,Camera,relative.x);real(rdram,Camera+4,relative.y);real(rdram,Camera+8,relative.z);
    real(rdram,Camera+0x18,orbit.pitch);real(rdram,Camera+0x1C,orbit.yaw);real(rdram,Camera+0x20,0);
    real(rdram,Camera+0x30,orbit.actual);
    // Preserve Source shake commands/counters exactly once; Native uses the
    // same command boundary predicate as the existing ordinary camera roots.
    const auto* slice=native_time::active_simulation_slice();
    if(!frame_update&&(source||(slice&&slice->ends_tick()))) {
        // Source also reaches this root during Native handoff/room loading.
        // Suspension requires an active slice; Original is already unscaled.
        if(slice) {
            native_time::SimulationSuspension boundary;
            call(rdram,*ctx,dora3_s3_801D55EC,Camera);
        } else {
            call(rdram,*ctx,dora3_s3_801D55EC,Camera);
        }
        orbit.shake=real(rdram,Camera+4)-relative.y;
    }
    real(rdram,Camera+4,relative.y+orbit.shake);
    call(rdram,*ctx,dora3_s0_8002F064,Camera);
    return true;
}
}
void configure(const Options& value){std::lock_guard lock(input_mutex);if(options.enabled!=value.enabled){++revision;pending={};available=false;}options=value;}
bool active(){return available.load()&&now_ms()-last_available.load()<150;}
void submit(float x,float y,float mx,float my,float zoom,float wheel,bool center){std::lock_guard lock(input_mutex);if(!active()){pending={};return;}pending.x=x;pending.y=y;pending.mx+=mx;pending.my+=my;pending.zoom=zoom;pending.wheel+=wheel;pending.center|=center;}
void clear_input(){std::lock_guard lock(input_mutex);pending={};}
void reset(){orbit={};preferred_distance=300;available=false;last_available=0;clear_input();}
void clock_handoff(){orbit.frame_elapsed=0;}
void observe(uint8_t* ram){bool enabled;{std::lock_guard lock(input_mutex);enabled=options.enabled;}if(!enabled||!eligible(ram))revoke();}
bool dispatch(uint8_t* ram,recomp_context* ctx,uint32_t root){
    if(root!=0x801D8FF0&&root!=0x801D7294&&root!=0x801D5864){revoke();return false;}
    const auto* slice=native_time::active_simulation_slice();
    return update(ram,ctx,slice?slice->seconds():double(word(ram,0x801A39DC))/60.,!slice,false);
}
bool frame(uint8_t* ram,recomp_context* ctx,double elapsed){return update(ram,ctx,elapsed,false,true);}
}
extern "C" int dora3_modern_camera_prepare(uint8_t* ram){
    dora3::camera::observe(ram);
    // Eligibility is evaluated before Original's R/C-up/C-down prefix even on
    // the first frame after enabling. Leave automatic swimming mode changes.
    using namespace dora3::camera;
    std::lock_guard lock(input_mutex);return options.enabled&&eligible(ram);
}
