// Actual common session/loop/scene-policy/visual owners; synthetic scene clocks.
#include "native_game_loop.hpp"
#include "native_frame_session.hpp"
#include "native_scene_policy.hpp"
#include "native_present.hpp"
#include "native_model_animation.hpp"
#include "native_gameplay_preview.hpp"
#include "native_cutscene.hpp"
#include "native_minigames.hpp"
#include "native_ui.hpp"
#include "frame_lights.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <cstring>
#include <deque>
#include <iostream>
#include <thread>
#include <vector>
using namespace dora3::native_time;
namespace {
enum class Family {World,Scripted,Mini,Menu,Unknown};
Family family=Family::World;
unsigned fps=60;
uint32_t mini_rom=0x1961B0;
uint64_t load_generation=1;
struct ClockProbe {unsigned begins=0,updates=0,resets=0;Nanoseconds origin=0,last=0;} world,scripted,mini;
std::deque<FrameTicket> tickets;
unsigned builds=0;
void begin(ClockProbe& p,Nanoseconds now){++p.begins;p.origin=p.last=now;}
void update(ClockProbe& p,Nanoseconds now){assert(p.begins&&now>=p.last);++p.updates;p.last=now;}
void reset(ClockProbe& p){++p.resets;p.origin=p.last=0;}
ClockProbe& selected(){return family==Family::World?world:family==Family::Scripted?scripted:mini;}
int32_t& W(uint8_t* rdram,uint32_t a){return MEM_W(0,int32_t(a));}
void set_family(uint8_t* ram,Family value,unsigned divider){
    family=value;W(ram,0x801A39DC)=int(divider);W(ram,0x801AA648)=W(ram,0x801A9500)=1;
    W(ram,0x801AA680)=W(ram,0x801B1400)=W(ram,0x801A48E0)=0;
    W(ram,0x801A48C0)=W(ram,0x801B1430)=W(ram,0x801AC694)=0;
    for(unsigned i=0;i<8;++i){auto slot=0x801A44F0+i*20;W(ram,slot)=i<2?2:0;W(ram,slot+12)=int(i);}
    W(ram,0x801A4500)=int32_t(value==Family::Mini?0x801CACAC:value==Family::Menu?0x801BCEF8:value==Family::Unknown?0x801B1234:0x801BB730);
    W(ram,0x801A4514)=int32_t(value==Family::Mini?0x801CB160:value==Family::Menu?0x801BD078:value==Family::Unknown?0x801B123C:0x801BB848);
}
void request_frame(uint8_t* ram,recomp_context& ctx){
    auto& bridge=game_frame_bridge();const auto deadline=bridge.deadline();assert(deadline);
    std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(*deadline)));
    const auto now=monotonic_time();assert(bridge.post_due(now,[]{return true;}));
    assert(bridge.dispatch(WakeMailbox::message_value,now,ram,&ctx));
}
void retire(uint8_t* ram,recomp_context& ctx,bool host=true){
    assert(!tickets.empty());const auto ticket=tickets.front();
    const auto task=dora3::frame::bank0+ticket.bank*dora3::frame::bank_stride;
    recomp_context c{};c.r16=int32_t(task);W(ram,task+4)=0;
    dora3_native_loop_retire(ram,&c);assert(native_present_ledger().retire(task));
    assert(native_present_ledger().consumed(task+0xC0));assert(W(ram,0x801AA618)>0);--W(ram,0x801AA618);
    if(host){assert(native_present_ledger().host_read_done(ticket));tickets.pop_front();}
    dora3_native_loop_message(ram,&ctx);
}
void release_host(uint8_t* ram,recomp_context& ctx){
    assert(!tickets.empty());assert(native_present_ledger().host_read_done(tickets.front()));tickets.pop_front();
    dora3_native_loop_message(ram,&ctx);
}
void clean(uint8_t* ram){
    assert(tickets.empty());dora3_reset_native_frame_delivery();std::memset(ram,0,0x400000);
    world=scripted=mini=ClockProbe{};builds=0;
}
}
unsigned dora3_requested_native_fps(){return fps;}
extern "C" int dora3_overlay_loaded(uint32_t rom){return rom==0x70A50||rom==0x635C0||rom==mini_rom;}
extern "C" uint64_t dora3_overlay_generation(uint32_t rom){return dora3_overlay_loaded(rom)?load_generation:0;}
bool native_gameplay_preview_eligible(uint8_t*){return family==Family::World;}
void native_gameplay_preview_begin(uint8_t*,Nanoseconds now,unsigned){begin(world,now);}
void native_gameplay_preview_update(uint8_t*,recomp_context*,Nanoseconds now){update(world,now);}
void native_gameplay_preview_reset(uint8_t*){reset(world);}
std::optional<Nanoseconds> native_gameplay_preview_render_time(){return world.begins&&world.last?std::optional{world.last}:std::nullopt;}
std::optional<SourceTimestamp> native_gameplay_preview_command_time(){return std::nullopt;}
bool native_cutscene_eligible(uint8_t*){return family==Family::Scripted;}
void native_cutscene_begin(uint8_t*,Nanoseconds now,unsigned){begin(scripted,now);}
void native_cutscene_update(uint8_t*,recomp_context*,Nanoseconds now){update(scripted,now);}
void native_cutscene_reset(uint8_t*){reset(scripted);}
std::optional<Nanoseconds> native_cutscene_render_time(){return scripted.begins&&scripted.last?std::optional{scripted.last}:std::nullopt;}
bool native_minigame_eligible(uint8_t*){return family==Family::Mini;}
uint32_t native_minigame_module(uint8_t*){return mini_rom;}
void native_minigame_begin(uint8_t*,Nanoseconds now,unsigned){begin(mini,now);}
void native_minigame_update(uint8_t*,recomp_context*,Nanoseconds now){update(mini,now);}
void native_minigame_reset(uint8_t*){reset(mini);}
std::optional<Nanoseconds> native_minigame_render_time(){return mini.begins&&mini.last?std::optional{mini.last}:std::nullopt;}
namespace dora3::native_time {
MinigameFrameScope::MinigameFrameScope(uint8_t*){}
MinigameFrameScope::~MinigameFrameScope()=default;
}
void dora3_native_process_time_reset(){}
void dora3_native_camera_dispatch_reset(){}
bool dora3_enqueue_native_frame_wake(){return true;}
bool dora3_enqueue_native_present(const PresentFrame&){return true;}
extern "C" void dora3_graphics_frame_build(uint8_t* ram,recomp_context* ctx){
    const auto* binding=active_frame_binding();assert(binding&&binding->owner->published(binding->ticket));
    const auto task=uint32_t(ctx->r4);const auto ticket=binding->ticket;
    assert(task==dora3::frame::bank0+ticket.bank*dora3::frame::bank_stride);
    native_present_ledger().bind(ticket,*binding->owner,task,ticket.bank?0x803DA800:0x803B5000,task+0xC0);
    recomp_context submit{};submit.r8=int32_t(task);dora3_native_loop_submit(ram,&submit);
    tickets.push_back(ticket);++builds;
}
extern "C" void osRecvMesg_recomp(uint8_t*,recomp_context*){std::abort();} // Queue/FIFO proof is the real loop fixture.
extern "C" void dora3_s0_800393C4(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s0_8003931C(uint8_t*,recomp_context*){std::abort();}
#define BOUNDARY(name) extern "C" void name(uint8_t*,recomp_context*){std::abort();}
BOUNDARY(dora3_s2_menu_801BBCEC) BOUNDARY(dora3_s2_menu_801BBEB0)
BOUNDARY(dora3_s2_menu_801BC17C) BOUNDARY(dora3_s2_menu_801BC4C0)
BOUNDARY(dora3_s2_menu_801C14B8) BOUNDARY(dora3_lib_80057A80)
extern "C" int dora3_native_rectangle_update(uint8_t*,recomp_context*){std::abort();}
int main(){
    std::vector<uint32_t> memory(0x400000/4);auto* ram=reinterpret_cast<uint8_t*>(memory.data());
    recomp_context ctx{};ctx.r29=int32_t(0x803F0000);
    unsigned cases=0;
    for(auto kind:{Family::World,Family::Scripted,Family::Mini})for(unsigned divider:{2u,3u})for(unsigned rate:{60u,73u,120u,144u,165u,360u}){
        clean(ram);fps=rate;++load_generation;set_family(ram,kind,divider);auto& probe=selected();
        assert(presentation_gate().begin_read());
        assert(dora3_native_loop_after_vi(ram,&ctx)==1);
        assert(game_frame_session().phase()==FrameSession::Phase::Acquiring&&probe.begins==0);
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
        dora3_native_loop_message(ram,&ctx);assert(probe.begins==0);
        const auto released=monotonic_time();presentation_gate().end_read();dora3_native_loop_message(ram,&ctx);
        assert(game_frame_session().phase()==FrameSession::Phase::Native&&probe.begins==1&&probe.origin>=released);
        request_frame(ram,ctx);assert(builds==1&&probe.updates==1&&probe.last>=probe.origin);
        retire(ram,ctx,false);assert(!game_frame_bridge().drained());
        fps=0;dora3_native_loop_message(ram,&ctx);assert(game_frame_session().phase()==FrameSession::Phase::Draining);
        assert(probe.origin&&dora3_native_loop_before_vi(ram,&ctx)==1);
        release_host(ram,ctx);assert(game_frame_session().phase()==FrameSession::Phase::Original&&!probe.origin);
        assert(W(ram,0x801AA618)==0&&!native_present_ledger().occupied());++cases;
    }
    // Old payload survives rate changes, but cannot consume the new clock.
    clean(ram);fps=60;set_family(ram,Family::World,3);assert(dora3_native_loop_after_vi(ram,&ctx));
    const auto due=*game_frame_bridge().deadline();std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(due)));
    assert(game_frame_bridge().post_due(monotonic_time(),[]{return true;}));const auto original=world.origin;
    fps=144;dora3_native_loop_message(ram,&ctx);
    assert(game_frame_bridge().dispatch(WakeMailbox::message_value,monotonic_time(),ram,&ctx));
    assert(world.origin==original&&world.begins==1&&world.updates==0&&builds==0);
    request_frame(ram,ctx);assert(world.updates==1);retire(ram,ctx);
    fps=0;dora3_native_loop_message(ram,&ctx);assert(game_frame_session().phase()==FrameSession::Phase::Original);++cases;
    // Same callbacks and same ROM, newer verified load: old wake is stale.
    clean(ram);fps=360;set_family(ram,Family::Mini,3);assert(dora3_native_loop_after_vi(ram,&ctx));
    const auto next=*game_frame_bridge().deadline();std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(next)));
    assert(game_frame_bridge().post_due(monotonic_time(),[]{return true;}));++load_generation;
    dora3_native_loop_message(ram,&ctx);assert(game_frame_session().phase()==FrameSession::Phase::Original);
    assert(game_frame_bridge().dispatch(WakeMailbox::message_value,monotonic_time(),ram,&ctx));assert(builds==0);++cases;
    // Cancel Acquiring before activation; neither old nor replacement clocks run.
    clean(ram);fps=73;set_family(ram,Family::World,2);assert(presentation_gate().begin_read());
    assert(dora3_native_loop_after_vi(ram,&ctx));set_family(ram,Family::Scripted,2);dora3_native_loop_message(ram,&ctx);
    assert(game_frame_session().phase()==FrameSession::Phase::Original&&world.begins==0&&scripted.begins==0);
    presentation_gate().end_read();assert(dora3_native_loop_after_vi(ram,&ctx));assert(scripted.begins==1);
    fps=0;dora3_native_loop_message(ram,&ctx);++cases;
    // Unsupported callback keeps Original, and cannot inherit a former session.
    clean(ram);fps=120;set_family(ram,Family::Unknown,3);
    assert(!dora3_native_loop_after_vi(ram,&ctx)&&game_frame_session().phase()==FrameSession::Phase::Original);++cases;
    for(auto kind:{Family::World,Family::Menu,Family::Mini,Family::Scripted}) {
        clean(ram);fps=360;set_family(ram,kind,3);assert(dora3_native_loop_after_vi(ram,&ctx));
        request_frame(ram,ctx);request_frame(ram,ctx);assert(tickets.size()==2&&W(ram,0x801AA618)==2);
        fps=0;dora3_native_loop_message(ram,&ctx);assert(game_frame_session().phase()==FrameSession::Phase::Draining);
        retire(ram,ctx);assert(game_frame_session().phase()==FrameSession::Phase::Draining);
        retire(ram,ctx);assert(game_frame_session().phase()==FrameSession::Phase::Original&&!native_present_ledger().occupied());++cases;
    }
    clean(ram);fps=120;set_family(ram,Family::Mini,3);assert(dora3_native_loop_after_vi(ram,&ctx));
    mini_rom=0x1A0500;dora3_native_loop_message(ram,&ctx);
    assert(game_frame_session().phase()==FrameSession::Phase::Original&&mini.begins==1);++cases;
    clean(ram);fps=73;set_family(ram,Family::World,3);assert(dora3_native_loop_after_vi(ram,&ctx));
    W(ram,0x801A39DC)=2;dora3_native_loop_message(ram,&ctx);
    assert(game_frame_session().phase()==FrameSession::Phase::Original);
    assert(dora3_native_loop_after_vi(ram,&ctx)&&world.begins==2);fps=0;dora3_native_loop_message(ram,&ctx);++cases;
    for(uint32_t disabled:{0x801AA648u,0x801A9500u}) {
        clean(ram);fps=360;set_family(ram,Family::Menu,3);W(ram,disabled)=0;
        assert(!dora3_native_loop_after_vi(ram,&ctx)&&game_frame_session().phase()==FrameSession::Phase::Original&&builds==0);
        W(ram,disabled)=1;assert(dora3_native_loop_after_vi(ram,&ctx));request_frame(ram,ctx);
        W(ram,disabled)=0;dora3_native_loop_message(ram,&ctx);
        assert(game_frame_session().phase()==FrameSession::Phase::Draining&&tickets.size()==1);
        retire(ram,ctx);assert(game_frame_session().phase()==FrameSession::Phase::Original&&!native_present_ledger().occupied());++cases;
    }
    // Quiescent reset removes retained mailbox and old generation, no clocks restart.
    clean(ram);fps=360;set_family(ram,Family::World,3);assert(dora3_native_loop_after_vi(ram,&ctx));
    const auto reset_due=*game_frame_bridge().deadline();std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(reset_due)));
    assert(game_frame_bridge().post_due(monotonic_time(),[]{return true;}));const auto epoch=game_frame_bridge().generation();
    dora3_reset_native_frame_delivery();assert(game_frame_bridge().generation()>epoch);
    assert(game_frame_bridge().dispatch(WakeMailbox::message_value,monotonic_time(),ram,&ctx)&&builds==0);
    assert(game_frame_session().phase()==FrameSession::Phase::Original&&world.begins==1&&!world.origin);++cases;
    std::cout<<"PASS: "<<cases<<" actual common-loop integration schedules, actual activation clock origin after held Original read, six Manual rates and both Source dividers, fractional modes, two completion boundaries, rate/reload/callback cancellation, unsupported fallback and quiescent mailbox reset; synthetic scene-clock/queue endpoints; no game/GPU.\n";
}
