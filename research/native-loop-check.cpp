#include "native_game_loop.hpp"
#include "native_frame_session.hpp"
#include "native_present.hpp"
#include "frame_lights.hpp"
#include "loop-functions.h"
#include <cassert>
#include <cstring>
#include <array>
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include "native_scene_policy.hpp"
using namespace dora3::native_time;
static unsigned requested=60,updates=0,native_builds=0,original_builds=0,vi_count=0,pauses=0,stage=0,resume_vi=0;
static bool initialized=false,legacy_tail_pending=true;
static unsigned white_builds=0,white_last_interval_builds=0,fade_dl_builds=0;
static bool state2_run=false;
static unsigned progress_resets=0;
static std::set<int> white_alpha;
static Nanoseconds next_vi=0,started=0;
static std::deque<uint32_t> messages,submitted;
struct Finished {};
static bool barrier_check=false;
static unsigned barrier_receive=0;
static std::array<FrameTicket,2> barrier_tickets;
static unsigned barrier_published=0;
static void barrier_consumer(uint8_t* rdram,recomp_context*,const FrameTicket& ticket,FrameScheduler& owner) {
    assert(owner.publish(ticket));barrier_tickets[barrier_published++]=ticket;
    const auto task=dora3::frame::bank0+ticket.bank*dora3::frame::bank_stride;
    native_present_ledger().bind(ticket,owner,task,ticket.bank?0x803DA800:0x803B5000,task+0xC0);
    recomp_context submit{};submit.r8=int32_t(task);dora3_native_loop_submit(rdram,&submit);
    ++MEM_W(0,int32_t(0x801AA618));
}
static void barrier_retire(uint8_t* rdram,unsigned bank) {
    const auto task=dora3::frame::bank0+bank*dora3::frame::bank_stride;
    recomp_context completion{};completion.r16=int32_t(task);MEM_W(4,completion.r16)=0;
    dora3_native_loop_retire(rdram,&completion);
    assert(native_present_ledger().retire(task));
}
static void barrier_receive_message(uint8_t* rdram,recomp_context* c) {
    assert(c->f_odd==&c->f0.u32h);c->f_odd[0]=0x12345678;
    uint32_t message=0;
    switch(barrier_receive++) {
    case 0:message=0x802F0010;break;
    case 1:message=WakeMailbox::message_value;break;
    case 2:message=dora3::frame::bank0+0xC0;break; // Message BEFORE outer retirement.
    case 3:
        assert(MEM_W(0,int32_t(0x801AA618))==1 && !game_frame_bridge().drained());
        barrier_retire(rdram,0);assert(native_present_ledger().host_read_done(barrier_tickets[0]));
        message=0x802F0020;break;
    case 4:
        barrier_retire(rdram,1);message=dora3::frame::bank0+dora3::frame::bank_stride+0xC0;break;
    case 5:
        assert(MEM_W(0,int32_t(0x801AA618))==0 && !game_frame_bridge().drained());
        assert(native_present_ledger().host_read_done(barrier_tickets[1]));message=0x802F0030;break;
    default:assert(false);
    }
    MEM_W(0,c->r5)=int32_t(message);c->r2=0;
}
static void resource_barrier_check(uint8_t* rdram,recomp_context* c) {
    auto& session=game_frame_session();auto& bridge=game_frame_bridge();
    assert(session.begin(144,0,true,barrier_consumer));
    assert(session.poll(0)==FrameSession::Phase::Native);
    for(unsigned i=0;i<2;++i) {
        assert(bridge.post_due((i+1)*second,[] {return true;}));
        assert(bridge.dispatch(WakeMailbox::message_value,(i+1)*second,rdram,c));
    }
    assert(barrier_published==2);barrier_check=true;
    const auto before=*c;const auto scratch=MEM_W(-0x10,c->r29);
    dora3_native_loop_transition(rdram,c);barrier_check=false;
    assert(barrier_receive==6 && std::memcmp(&before,c,sizeof(*c))==0 && MEM_W(-0x10,c->r29)==scratch);
    assert(session.phase()==FrameSession::Phase::Original && bridge.drained() && !native_present_ledger().occupied());
    c->r4=int32_t(game_queue);c->r5=int32_t(0x802F0040);
    for(auto value:{0x802F0010u,0x802F0020u,0x802F0030u}) {
        assert(dora3_native_loop_receive_deferred(rdram,c));assert(uint32_t(MEM_W(0,c->r5))==value);
    }
    assert(!dora3_native_loop_receive_deferred(rdram,c));
    dora3_reset_native_frame_delivery();
}
unsigned dora3_requested_native_fps() {return requested;}
extern "C" uint64_t dora3_overlay_generation(uint32_t rom) {return rom==0x635C0?1:0;}
extern "C" void do_break(uint32_t) {std::abort();}
extern "C" void dora3_trace(uint8_t* rdram,recomp_context*,uint32_t pc,uint32_t) {
    if(pc==0x801C0088)++updates;
    if(pc==0x8002FF64) {
        if(dora3_native_frame_build_active()) {
            ++native_builds;
            if(MEM_W(0,int32_t(0x801C68CC))>0) {++white_builds;white_alpha.insert(MEM_H(0,int32_t(0x801AA660)));if(MEM_W(0,int32_t(0x801C68CC))==1)++white_last_interval_builds;}
        } else ++original_builds;
    }
}
extern "C" recomp_func_t* get_function(int32_t pc) {
    if(uint32_t(pc)==0x801C0088)return dora3_scene34_update;
    if(uint32_t(pc)==0x801C012C)return dora3_scene34_draw;
    std::abort();
}
extern "C" void dora3_capture_frame_lights(uint8_t* rdram,uint32_t bank) {
    assert(bank==dora3::frame::bank0 || bank==dora3::frame::bank0+dora3::frame::bank_stride);
    const auto end=uint32_t(MEM_W(0,int32_t(0x801B1424)));
    assert(end>bank+dora3::frame::dl_offset && end<bank+dora3::frame::bank_stride);
    const int direction=MEM_B(0,int32_t(0x801A99A0));
    if(direction==-1||direction==1||direction==2) {
        uint32_t last_color=0;bool found=false;
        for(auto p=bank+dora3::frame::dl_offset;p+8<=end;p+=8)
            if(uint32_t(MEM_W(0,int32_t(p)))>>24==0xFA){last_color=uint32_t(MEM_W(4,int32_t(p)));found=true;}
        const uint32_t expected=(uint32_t(MEM_BU(0,int32_t(0x801B1404)))<<24)|
            (uint32_t(MEM_BU(0,int32_t(0x80171CB8)))<<16)|(uint32_t(MEM_BU(0,int32_t(0x801A99B4)))<<8)|
            uint32_t(MEM_HU(0,int32_t(0x801AA660)));
        assert(found&&last_color==expected);++fade_dl_builds;
    }
}
bool dora3_enqueue_native_frame_wake() {messages.push_back(WakeMailbox::message_value);return true;}
bool dora3_enqueue_native_graphics_wake() {return true;}
bool dora3_enqueue_native_present(const PresentFrame& frame) {
    assert(native_present_ledger().host_read_done(frame.ticket));return true;
}
extern "C" void dora3_scheduler_retrace(uint8_t*,recomp_context*) {std::abort();} // SP/DP separately checked by pump fixture.
extern "C" void osVirtualToPhysical_recomp(uint8_t*,recomp_context* c) {c->r2=uint32_t(c->r4)&0x1fffffff;}
extern "C" void osWritebackDCacheAll_recomp(uint8_t*,recomp_context*) {}
extern "C" void osSendMesg_recomp(uint8_t*,recomp_context* c) {
    assert(uint32_t(c->r4)==0x801A6A48);submitted.push_back(uint32_t(c->r5));c->r2=0;
}
#define EMPTY(name) extern "C" void name(uint8_t*,recomp_context*) {}
EMPTY(dora3_controller_read) EMPTY(dora3_controller_accumulate) EMPTY(dora3_controller_latch)
EMPTY(dora3_controller_reset_accumulator) EMPTY(dora3_rumble_tick) EMPTY(dora3_controller_pak_tick)
EMPTY(dora3_sound_events_tick) EMPTY(dora3_object_lifetime_tick) EMPTY(dora3_deferred_free_tick)
EMPTY(dora3_frame_timestamp) EMPTY(dora3_update_timestamp) EMPTY(dora3_update_elapsed)
extern "C" void dora3_s0_80047EF4(uint8_t*,recomp_context*) {++pauses;}
extern "C" void dora3_scene_transition_framebuffer_copy(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_scene34_controller_message(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_common_progress_reset(uint8_t*,recomp_context*) {assert(state2_run);++progress_resets;} // Counted Source command boundary; progress contents are outside this loop fixture.
extern "C" void dora3_process_pair_install(uint8_t* rdram,recomp_context* c) {
    dora3_native_loop_transition(rdram,c);
    assert(game_frame_session().phase()==FrameSession::Phase::Original && game_frame_bridge().drained());
    assert(MEM_W(0,int32_t(0x801AA618))==0 && updates==76 && native_builds>100 && stage==5);
    assert(MEM_W(0,int32_t(0x801A950C))==(state2_run?1:0x5A));
    assert(progress_resets==(state2_run?1u:0u)); // Actual transition branch invoked once.
    assert(white_last_interval_builds>0&&fade_dl_builds>white_builds);
    assert(MEM_BU(0,int32_t(0x801B1404))==255 && MEM_BU(0,int32_t(0x80171CB8))==255);
    assert(white_builds>30 && white_alpha.size()>16);
    assert(MEM_H(0,int32_t(0x801AA660))>=230); // Final legacy tick completes fade after drain.
    throw Finished{};
}
static void setup(uint8_t* rdram,recomp_context* c) {
    for(unsigned i=0;i<2;++i) {
        const auto bank=int32_t(dora3::frame::bank0+i*dora3::frame::bank_stride);
        MEM_W(0x100,bank)=int32_t(i?0x803DA800:0x803B5000);MEM_W(0x104,bank)=int32_t(i?0x803B5000:0x803DA800);
        MEM_H(0xC0,bank)=2;MEM_H(0xE0,bank)=4;
        const auto slot=int32_t(0x801A44F0+i*20);MEM_W(0,slot)=2;MEM_W(12,slot)=i;MEM_W(16,slot)=int32_t(i?0x801C012C:0x801C0088);
    }
    MEM_W(0,int32_t(0x801A39DC))=3;MEM_W(0,int32_t(0x801AA648))=1;MEM_W(0,int32_t(0x801A9500))=1;
    MEM_W(0,int32_t(0x801A48C0))=1; // Previous original frame built; its outer tail is still pending.
    MEM_W(0,int32_t(0x801A48E0))=0;MEM_W(0,int32_t(0x8017157C))=0;
    MEM_W(0,int32_t(0x801AA650))=int32_t(0x8038F800);MEM_W(0,int32_t(0x801AA658))=320;MEM_W(0,int32_t(0x801A48C4))=240;
    MEM_W(0,int32_t(0x801A950C))=34;MEM_B(34,int32_t(0x80171C50))=1;
    MEM_W(0,int32_t(0x801C68D0))=60;MEM_W(0,int32_t(0x801AEC00))=int32_t(0x80280000);
    MEM_B(3,int32_t(0x80280000))=5;MEM_H(6,int32_t(0x80280000))=16;
    MEM_W(8,int32_t(0x80280000))=64;MEM_W(12,int32_t(0x80280000))=32;
    MEM_H(0,int32_t(0x801AA660))=255;MEM_B(0,int32_t(0x801A99A0))=-1;MEM_B(0,int32_t(0x80171580))=16;
    MEM_W(0,int32_t(0x80171584))=int32_t(0x801A6A48);MEM_H(0,int32_t(0x802F0000))=1;
    recomp_context old{};old.r8=int32_t(dora3::frame::bank0+dora3::frame::bank_stride);
    dora3_native_loop_submit(rdram,&old);
    dora3_native_loop_scene_ready(rdram,c);
    // Request state2 and the last count1 interval are resident until actual
    // process installation; they must retain fractional fade presentation.
    MEM_W(0,int32_t(0x801C68D0))=1;MEM_B(34,int32_t(0x80171C50))=2;
    assert(select_native_scene_policy(rdram)!=nullptr);
    MEM_B(34,int32_t(0x80171C50))=state2_run?2:1;assert(select_native_scene_policy(rdram)!=nullptr);
    MEM_W(0,int32_t(0x801C68CC))=1;assert(select_native_scene_policy(rdram)!=nullptr);
    MEM_W(0,int32_t(0x801C68CC))=0;MEM_W(0,int32_t(0x801C68D0))=60;
    started=next_vi=monotonic_time();initialized=true;
}
extern "C" void osRecvMesg_recomp(uint8_t* rdram,recomp_context* c) {
    assert(uint32_t(c->r4)==game_queue);
    if(barrier_check) {barrier_receive_message(rdram,c);return;}
    if(!initialized)setup(rdram,c);
    for(;;) {
        const auto now=monotonic_time();assert(now-started<8*second); // Test-only watchdog.
        if(legacy_tail_pending && vi_count>=6) {
            assert(native_builds==0 && original_builds==0 && updates==1);
            recomp_context old{};old.r16=int32_t(dora3::frame::bank0+dora3::frame::bank_stride);MEM_W(4,old.r16)=0;
            dora3_native_loop_retire(rdram,&old);legacy_tail_pending=false;
        }
        if(stage==0 && native_builds>=15) {requested=73;stage=1;}
        if(stage==1 && native_builds>=35) {requested=144;stage=2;}
        if(stage==2 && native_builds>=60) {MEM_W(0,int32_t(0x801AA680))=1;resume_vi=vi_count+3;stage=3;}
        if(stage==3 && vi_count>=resume_vi) {MEM_W(0,int32_t(0x801AA680))=0;requested=0;resume_vi=vi_count+4;stage=4;}
        if(stage==4 && vi_count>=resume_vi) {requested=360;stage=5;}
        if(!submitted.empty()) {
            const auto task=submitted.front();submitted.pop_front();
            recomp_context completion{};completion.r16=int32_t(task);MEM_W(4,completion.r16)=0;
            dora3_native_loop_retire(rdram,&completion);
            // Real retirement and HostRead accounting; scheduler hardware endpoints are synthetic here.
            if(auto frame=native_present_ledger().retire(task))assert(dora3_enqueue_native_present(*frame));
            messages.push_back(task+0xC0);
        }
        if(!messages.empty()) {MEM_W(0,c->r5)=int32_t(messages.front());messages.pop_front();c->r2=0;return;}
        if(now>=next_vi) {
            next_vi+=second/60;++vi_count;++MEM_W(0,int32_t(0x801AC694));
            MEM_W(0,c->r5)=int32_t(0x802F0000);c->r2=0;return;
        }
        game_frame_bridge().post_due(now,dora3_enqueue_native_frame_wake);
        if(!messages.empty())continue;
        auto wake=next_vi;if(auto deadline=game_frame_bridge().deadline())wake=std::min(wake,*deadline);
        std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(wake)));
    }
}
int main(int argc,char** argv) {
    assert(argc==2||argc==3);state2_run=argc==3;std::vector<uint32_t> memory(0x400000/4);auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    std::ifstream in(argv[1],std::ios::binary);in.read(reinterpret_cast<char*>(rdram+0x7CD80),128);assert(in.gcount()==128);
    recomp_context c{};c.r29=int32_t(0x80300000);
    try {dora3_game_message_loop(rdram,&c);assert(false);}catch(const Finished&) {}
    assert(pauses>0 && original_builds>0 && !native_present_ledger().occupied());
    // Reset must remove the scene authorization as well as all bank ownership.
    dora3_reset_native_frame_delivery();assert(!dora3_native_loop_after_vi(rdram,&c));
    resource_barrier_check(rdram,&c);
    std::cout<<"PASS: resource barrier drains two banks with early guest completion/late SP retirement/late HostRead, preserves foreign FIFO, registers and scratch; original message loop + update + dispatcher + full frame build/submit + image draw; shared native controller; Manual 60/73/144/360, Original, pause, original white fade commands and scene selection, drain before callback/resource replacement; "
        <<white_builds<<" native white-fade builds with "<<white_alpha.size()<<" distinct alpha values; "<<white_last_interval_builds<<" last-interval builds; "<<fade_dl_builds<<" actual RDP fade-alpha comparisons; request state"<<(state2_run?2:1)<<"; "<<updates<<" discrete updates, "<<native_builds<<" native builds, "<<original_builds<<" Original builds, "<<vi_count<<" VI events; no GPU\n";
}

// Gameplay remains deliberately disabled in the image-loop fixture.
// The player fixture executes all of these Original contact bodies.
extern "C" void dora3_s3_801C0624(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801B922C(uint8_t*,recomp_context*) {std::abort();}
// This scene34 loop fixture has no gameplay NPCs. Actual B91D4 is
// executed by the player/world and pending-story fixtures; reaching it
// here is a strict unexpected-boundary failure, never a no-op.
extern "C" void dora3_s3_core_801B91D4(uint8_t*,recomp_context*) {std::abort();}
// The image-scene loop fixture never enters sceneFD NPC dialogue. Those
// real quest/request callbacks run in the player/world scene fixtures;
// reaching them here must fail instead of hiding an unexpected boundary.
extern "C" void dora3_s1_801B4FBC(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801BA0C4(uint8_t*,recomp_context*) {std::abort();}

// This loop-only scene has no NPCs. Both real angle helpers execute in
// player/world proof; an unexpected visit here must fail immediately.
extern "C" void dora3_s3_demo_801E8590(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_80031A0C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801DCC1C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801DCC50(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801DC880(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801DCDF0(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801DD8FC(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801C8B28(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801E8BDC(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_demo_801E88C4(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_demo_801E8838(uint8_t*,recomp_context*) {std::abort();}
extern "C" int dora3_overlay_loaded(uint32_t rom) {return rom==0x635C0;}
extern "C" void dora3_s3_801D10D8(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_menu_8002A3B0(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_8002EA4C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_8002A2E8(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_8002A328(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801C05F0(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801CFA94(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801D0088(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_8002A1B8(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_80028C74(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801CB2F8(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801CB3A8(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801CB870(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_lib_80057A80(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_lib_80055BD0(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801D05A8(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C120C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C2798(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C554C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C1A3C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C1ADC(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801C0D88(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801C0E3C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801C0EC0(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C8118(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801BBDDC(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801C1664(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801DBD58(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s0_8002F064(uint8_t*,recomp_context*) {std::abort();}

// This image-loop scope never enters gameplay; the reward and scene scopes
// execute the real gravity/probe body. Any unexpected activation is fatal.
extern "C" void dora3_s3_801E810C(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_core_801EBCC0(uint8_t*,recomp_context*) {std::abort();}
extern "C" void dora3_s3_801E89CC(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s3_801E3768(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s3_801E0760(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s2_menu_801BBCEC(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s2_menu_801BBEB0(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s2_menu_801BC17C(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s2_menu_801BC4C0(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s2_menu_801C14B8(uint8_t*,recomp_context*) {std::abort();}

extern "C" void dora3_s3_core_801EBAD4(uint8_t*,recomp_context*) {std::abort();} // No gameplay pickups in the image-loop scope.
// Boss1Bironn's real geometry executes in the enemy closure. The image-loop
// scope contains no such actor; reaching this dependency is a fatal failure.
extern "C" void dora3_s33_remaining_8021FED8(uint8_t*,recomp_context*) {std::abort();}

// Image-loop fixture never admits minigame terrain or transform owners. The
// minigame connected fixture executes their actual SDK bodies instead.
extern "C" void dora3_s0_80040104(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s0_800327A4(uint8_t*,recomp_context*){std::abort();}
extern "C" void dora3_s0_8002DA98(uint8_t*,recomp_context*){std::abort();}
