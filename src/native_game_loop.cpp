#include "native_game_loop.hpp"
#include "native_model_animation.hpp"
#include "native_frame_session.hpp"
#include "frame_lights.hpp"
#include "native_scene_policy.hpp"
#include <array>
#include <cstdio>
#include <mutex>
#include <optional>
#include <stdexcept>
extern "C" void dora3_graphics_frame_build(uint8_t*,recomp_context*);
using namespace dora3::native_time;
namespace {
using Phase=FrameSession::Phase;
constexpr uint32_t count=0x801AA618,next_bank=0x801A9544,built=0x801A48C0,deferred=0x801B1430;
int32_t& word(uint8_t* rdram,uint32_t address) { return MEM_W(0,int32_t(address)); }
// All original and native submissions use the same two banks. Game's count can
// reach zero before the outer SP/DP caller finishes; track that boundary too.
std::mutex task_mutex;
std::array<bool,2> pending{};
std::optional<unsigned> task_bank(uint32_t address) {
    for(unsigned i=0;i<2;++i) if(address==dora3::frame::bank0+i*dora3::frame::bank_stride)return i;
    return std::nullopt;
}
bool legacy_drained() { std::lock_guard lock(task_mutex);return !pending[0]&&!pending[1]; }
const ScenePolicy* active_policy=nullptr;
const ScenePolicy* entry_policy=nullptr;
std::optional<FadeTimeline> fade;
FadeState read_fade(uint8_t* rdram) {
    return {MEM_H(0,int32_t(0x801AA660)),MEM_B(0,int32_t(0x801A99A0)),MEM_BU(0,int32_t(0x80171580))};
}
void write_fade(uint8_t* rdram,FadeState state) {
    MEM_H(0,int32_t(0x801AA660))=state.alpha;MEM_B(0,int32_t(0x801A99A0))=state.direction;
}
unsigned applied_fps=0;
uint64_t frames=0;
bool common_eligible(uint8_t* rdram) {
    return word(rdram,0x801AA680)!=1 && word(rdram,0x801B1400)==0 && word(rdram,0x801A48E0)==0;
}
void finish_session(Phase before,Phase after) {
    if(before!=Phase::Original && after==Phase::Original) {
        std::fprintf(stderr,"NATIVE LOOP drained frames=%llu; Original resumes\n",static_cast<unsigned long long>(frames));
        active_policy=nullptr;fade.reset();applied_fps=0;
    }
}
void build_frame(uint8_t*,recomp_context*,const FrameTicket&,FrameScheduler&);
void try_enter(uint8_t* rdram,Nanoseconds now,unsigned fps) {
    if(!entry_policy || word(rdram,count)!=0 || !legacy_drained())return;
    const auto bank=word(rdram,next_bank);if(bank<0 || bank>1)throw std::logic_error("Invalid original next bank");
    if(!game_frame_session().begin(fps,unsigned(bank),true,build_frame))return;
    active_policy=entry_policy;entry_policy=nullptr;applied_fps=fps;frames=0;
    fade.emplace(read_fade(rdram),active_policy->divider,now);
    game_frame_session().poll(now);
    std::fprintf(stderr,"NATIVE LOOP acquire policy=%s fps=%u original_hz=%u\n",active_policy->name,fps,60/active_policy->divider);
}
void synchronize(uint8_t* rdram,Nanoseconds now) {
    auto& session=game_frame_session();const auto before=session.phase();
    const unsigned fps=dora3_requested_native_fps();
    if(before==Phase::Acquiring || before==Phase::Native) {
        if(!fps || !common_eligible(rdram) || !active_policy || !active_policy->eligible(rdram)) {
            if(before==Phase::Native && fade)write_fade(rdram,fade->sample(read_fade(rdram),now));
            std::fprintf(stderr,"NATIVE LOOP leave (mode/pause/scene boundary)\n");session.leave(now);
        } else if(fps!=applied_fps) { session.change_rate(fps,now);applied_fps=fps; }
    }
    const auto after=session.poll(now);
    if(before==Phase::Acquiring && after==Phase::Native)fade.emplace(read_fade(rdram),active_policy->divider,now);
    finish_session(before,after);
    if(entry_policy) {
        if(!fps || !common_eligible(rdram) || !entry_policy->eligible(rdram))entry_policy=nullptr;
        else try_enter(rdram,now,fps);
    }
}
void build_frame(uint8_t* rdram,recomp_context* ctx,const FrameTicket& ticket,FrameScheduler& owner) {
    // Eligibility was checked immediately before dispatch on this guest thread.
    // No yield occurs between that check, reservation and entry here.
    if(!active_policy || !fade || game_frame_session().phase()!=Phase::Native ||
       !common_eligible(rdram) || !active_policy->eligible(rdram) ||
       word(rdram,count)<0 || word(rdram,count)>=2 || word(rdram,next_bank)!=int(ticket.bank))
        throw std::logic_error("Native loop lost safe build boundary");
    write_fade(rdram,fade->sample(read_fade(rdram),monotonic_time()));
    if(!owner.publish(ticket))throw std::logic_error("Native frame publication failed");
    ctx->r4=int32_t(dora3::frame::bank0+ticket.bank*dora3::frame::bank_stride);
    {
        ModelFrameScope models(rdram,ctx,monotonic_time(),ticket.bank);
        dora3_graphics_frame_build(rdram,ctx);
    }
    // Same ordering as the original loop: increment after blocking submit;
    // queued completions are consumed only after returning to receive.
    ++word(rdram,count);word(rdram,next_bank)=ticket.bank^1;
    word(rdram,built)=1;word(rdram,deferred)=0;++frames;
}
}
extern "C" void dora3_native_loop_message(uint8_t* rdram,recomp_context*) { synchronize(rdram,monotonic_time()); }
extern "C" int dora3_native_loop_before_vi(uint8_t* rdram,recomp_context*) {
    const auto phase=game_frame_session().phase();
    if(entry_policy || phase==Phase::Acquiring || phase==Phase::Draining)return 1;
    if(phase==Phase::Native)word(rdram,deferred)=0;
    return 0; // Original VI, input, sound and discrete update frequency preserved.
}
extern "C" int dora3_native_loop_after_vi(uint8_t* rdram,recomp_context*) {
    auto& session=game_frame_session();
    if(entry_policy || session.phase()!=Phase::Original)return 1;
    const auto fps=dora3_requested_native_fps();
    if(!fps || !common_eligible(rdram) || word(rdram,built)!=0 || word(rdram,deferred)!=0 || word(rdram,0x801AC694)!=0)return 0;
    entry_policy=select_native_scene_policy(rdram);
    if(!entry_policy)return 0;
    // Freeze original update/build at this fresh discrete boundary, even if the
    // prior task's game message arrived before its outer scheduler caller ended.
    try_enter(rdram,monotonic_time(),fps);
    return 1;
}

extern "C" void dora3_native_loop_fade_command(uint8_t* rdram,recomp_context*) {
    if(game_frame_session().phase()==Phase::Native && active_policy && fade)
        write_fade(rdram,fade->before_command(read_fade(rdram),monotonic_time()));
}
extern "C" int dora3_native_loop_owns_frame_banks() { return game_frame_session().phase()!=Phase::Original; }
extern "C" int dora3_native_loop_owns_fade() { return game_frame_session().phase()==Phase::Native && active_policy; }
extern "C" void dora3_native_loop_scene_ready(uint8_t*,recomp_context*) { native_scene34_ready(); }
extern "C" void dora3_native_loop_transition(uint8_t*,recomp_context*) {
    // Common process-pair replacement invalidates overlay-specific eligibility.
    // The policy must leave BEFORE executing such an update, not after freeing RAM.
    if(entry_policy || game_frame_session().phase()!=Phase::Original)throw std::logic_error("Unreviewed transition while native frames own memory");
    invalidate_native_scene_policy();
}
extern "C" void dora3_native_loop_submit(uint8_t*,recomp_context* ctx) {
    if(auto bank=task_bank(uint32_t(ctx->r8))) { std::lock_guard lock(task_mutex);pending[*bank]=true; }
}
extern "C" void dora3_native_loop_retire(uint8_t* rdram,recomp_context* ctx) {
    if((uint32_t(MEM_W(4,ctx->r16))&3)!=0)return;
    if(auto bank=task_bank(uint32_t(ctx->r16))) { std::lock_guard lock(task_mutex);pending[*bank]=false; }
}
void dora3_native_loop_reset() {
    std::lock_guard lock(task_mutex);pending={};invalidate_native_scene_policy();active_policy=nullptr;entry_policy=nullptr;
    fade.reset();applied_fps=0;frames=0;game_model_animation().reset();
}
