#include "native_game_loop.hpp"
#include "native_process_time.hpp"
#include "native_camera_timeline.hpp"
#include "native_gameplay_preview.hpp"
#include "native_cutscene.hpp"
#include "native_present.hpp"
#include "recomp_context_copy.hpp"
#include <deque>
#include "native_model_animation.hpp"
#include "native_frame_session.hpp"
#include "frame_lights.hpp"
#include "native_scene_policy.hpp"
#include <array>
#include <cstdio>
#include <mutex>
#include <optional>
#include <stdexcept>
extern "C" void osRecvMesg_recomp(uint8_t*,recomp_context*);
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
// Scheduled game-thread FIFO: resource waits may consume completions ahead of VI,
// but must replay every other message in its original order.
std::deque<uint32_t> deferred_messages;
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
std::optional<Nanoseconds> render_time() {
    if(active_policy&&active_policy->update==native_cutscene_update)return native_cutscene_render_time();
    return native_gameplay_preview_render_time();
}
unsigned applied_fps=0;
uint64_t frames=0;
bool common_eligible(uint8_t* rdram) {
    return word(rdram,0x801AA680)!=1 && word(rdram,0x801B1400)==0 && word(rdram,0x801A48E0)==0;
}
void finish_session(uint8_t* ram,Phase before,Phase after) {
    if(before!=Phase::Original && after==Phase::Original) {
        std::fprintf(stderr,"NATIVE LOOP drained frames=%llu; Original resumes\n",static_cast<unsigned long long>(frames));
        active_policy=nullptr;fade.reset();applied_fps=0;dora3_native_camera_dispatch_reset();native_gameplay_preview_reset(ram);native_cutscene_reset(ram);
    }
}
void build_frame(uint8_t*,recomp_context*,const FrameTicket&,FrameScheduler&);
void update_frame(uint8_t*,recomp_context*,Nanoseconds);
void try_enter(uint8_t* rdram,Nanoseconds now,unsigned fps) {
    if(!entry_policy || word(rdram,count)!=0 || !legacy_drained())return;
    const auto bank=word(rdram,next_bank);if(bank<0 || bank>1)throw std::logic_error("Invalid original next bank");
    if(!game_frame_session().begin(fps,unsigned(bank),true,build_frame,update_frame))return;
    active_policy=entry_policy;entry_policy=nullptr;applied_fps=fps;frames=0;
    if(active_policy->update==native_gameplay_preview_update)native_gameplay_preview_begin(rdram,now,active_policy->divider);
    if(active_policy->update==native_cutscene_update)native_cutscene_begin(rdram,now,active_policy->divider);
    fade.emplace(read_fade(rdram),active_policy->divider,now);
    game_frame_session().poll(now);
    std::fprintf(stderr,"NATIVE LOOP acquire policy=%s fps=%u original_hz=%u\n",active_policy->name,fps,60/active_policy->divider);
}
void synchronize(uint8_t* rdram,Nanoseconds now) {
    auto& session=game_frame_session();const auto before=session.phase();
    const unsigned fps=dora3_requested_native_fps();
    if(before==Phase::Acquiring || before==Phase::Native) {
        if(!fps || !common_eligible(rdram) || !active_policy || !active_policy->eligible(rdram)) {
            if(before==Phase::Native && fade)write_fade(rdram,fade->sample(read_fade(rdram),render_time().value_or(now)));
            std::fprintf(stderr,"NATIVE LOOP leave (mode/pause/scene boundary)\n");session.leave(now);
        } else if(fps!=applied_fps) { session.change_rate(fps,now);applied_fps=fps; }
    }
    const auto after=session.poll(now);
    if(before==Phase::Acquiring && after==Phase::Native)fade.emplace(read_fade(rdram),active_policy->divider,now);
    finish_session(rdram,before,after);
    if(entry_policy) {
        if(!fps || !common_eligible(rdram) || !entry_policy->eligible(rdram))entry_policy=nullptr;
        else try_enter(rdram,now,fps);
    }
}
void update_frame(uint8_t* rdram,recomp_context* ctx,Nanoseconds now) {
    synchronize(rdram,now);
    if(game_frame_session().phase()!=Phase::Native || !active_policy)return;
    if(active_policy->update)active_policy->update(rdram,ctx,now);
    // A world callback may have replaced its scene, divider, or pause state.
    // This runs with no reserved bank and no native build binding.
    synchronize(rdram,now);
}
void build_frame(uint8_t* rdram,recomp_context* ctx,const FrameTicket& ticket,FrameScheduler& owner) {
    // Eligibility was checked immediately before dispatch on this guest thread.
    // No yield occurs between that check, reservation and entry here.
    if(!active_policy || !fade || game_frame_session().phase()!=Phase::Native ||
       !common_eligible(rdram) || !active_policy->eligible(rdram) ||
       word(rdram,count)<0 || word(rdram,count)>=2 || word(rdram,next_bank)!=int(ticket.bank))
        throw std::logic_error("Native loop lost safe build boundary");
    write_fade(rdram,fade->sample(read_fade(rdram),render_time().value_or(monotonic_time())));
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
        write_fade(rdram,fade->before_command(read_fade(rdram),native_gameplay_preview_command_time().value_or(SourceTimestamp{monotonic_time(),0})));
}
extern "C" int dora3_native_loop_owns_frame_banks() { return game_frame_session().phase()!=Phase::Original; }
extern "C" int dora3_native_loop_owns_fade() { return game_frame_session().phase()==Phase::Native && active_policy; }
extern "C" void dora3_native_loop_scene_ready(uint8_t*,recomp_context*) { native_scene34_ready(); }
extern "C" int dora3_native_loop_receive_deferred(uint8_t* rdram,recomp_context* ctx) {
    if(deferred_messages.empty())return 0;
    if(uint32_t(ctx->r4)!=game_queue)throw std::logic_error("Deferred receive on wrong queue");
    MEM_W(0,ctx->r5)=int32_t(deferred_messages.front());deferred_messages.pop_front();
    ctx->r6=1;ctx->r2=0;return 1; // Original JAL delay slot sets blocking receive.
}
extern "C" void dora3_native_loop_resource_barrier(uint8_t* rdram,recomp_context* ctx) {
    auto& session=game_frame_session();
    if(!entry_policy && session.phase()==Phase::Original)return;
    if(active_frame_binding())throw std::logic_error("Resource replacement inside an unpublished native frame");
    const auto fade_stamp=native_gameplay_preview_command_time();
    cancel_active_simulation();
    const auto now=monotonic_time();const auto before=session.phase();
    if(before==Phase::Native && fade) {
        if(fade_stamp)write_fade(rdram,fade->sample(read_fade(rdram),*fade_stamp));
        else write_fade(rdram,fade->sample(read_fade(rdram),now));
    }
    entry_policy=nullptr;session.leave(now);
    // This is a scheduled guest receive, never a host wait holding the running
    // guest thread. Other guest threads may finish SP/DP while we are blocked.
    recomp_context receive;dora3::copy_recomp_context(receive,*ctx);
    const auto scratch=ctx->r29-0x10;
    const auto saved_scratch=MEM_W(0,scratch);
    try {
        while(session.poll(monotonic_time())!=Phase::Original) {
            receive.r4=int32_t(game_queue);receive.r5=scratch;receive.r6=1;
            osRecvMesg_recomp(rdram,&receive);
            if(receive.r2!=0)throw std::logic_error("Blocking resource receive failed");
            const auto message=uint32_t(MEM_W(0,scratch));
            if(message==WakeMailbox::message_value) {
                // leave() invalidated the token; consume its mailbox ownership.
                game_frame_bridge().dispatch(message,monotonic_time(),rdram,&receive);
            } else if(native_present_ledger().awaiting_message(message)) {
                if(word(rdram,count)<=0)throw std::logic_error("Native completion without outstanding task");
                native_present_ledger().consumed(message);--word(rdram,count);
            } else deferred_messages.push_back(message);
        }
    } catch(...) {MEM_W(0,scratch)=saved_scratch;throw;}
    MEM_W(0,scratch)=saved_scratch;
    if(word(rdram,count)!=0 || !legacy_drained())throw std::logic_error("Resource barrier missed task retirement");
    finish_session(rdram,before,session.phase());
}
extern "C" void dora3_native_loop_transition(uint8_t* rdram,recomp_context* ctx) {
    dora3_native_loop_resource_barrier(rdram,ctx);
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
    dora3_native_process_time_reset();dora3_native_camera_dispatch_reset();native_gameplay_preview_reset();native_cutscene_reset();
    std::lock_guard lock(task_mutex);pending={};invalidate_native_scene_policy();active_policy=nullptr;entry_policy=nullptr;
    fade.reset();applied_fps=0;frames=0;deferred_messages.clear();game_model_animation().reset();
}

extern "C" int dora3_native_loop_gameplay_active() {return active_policy && active_policy->update==native_gameplay_preview_update && game_frame_session().phase()==FrameSession::Phase::Native;}

extern "C" int dora3_native_loop_cutscene_active() {return active_policy&&active_policy->update==native_cutscene_update&&game_frame_session().phase()==FrameSession::Phase::Native;}
