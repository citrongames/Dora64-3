#include "native_graphics_pump.hpp"
#include "native_frame_bridge.hpp"
#include "native_present.hpp"
#include "ultramodern/ultramodern.hpp"
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
using namespace dora3::native_time;
extern "C" void dora3_scheduler_thread(uint8_t*, recomp_context*);
extern "C" void dora3_graphics_task_submit(uint8_t*, recomp_context*);
extern "C" bool dora3_interrupts_enabled() { return true; }
extern "C" void switch_error(const char*, uint32_t, uint32_t) { assert(false); }
extern "C" void pause_self(uint8_t*) { assert(false); }
namespace ultramodern {
bool is_game_thread() { return true; }
PTR(OSThread) this_thread() { return 0; }
bool thread_queue_empty(uint8_t*, PTR(PTR(OSThread))) { return true; }
void thread_queue_insert(uint8_t*, PTR(PTR(OSThread)), PTR(OSThread)) { assert(false); }
PTR(OSThread) thread_queue_pop(uint8_t*, PTR(PTR(OSThread))) { assert(false); return 0; }
void schedule_running_thread(uint8_t*, PTR(OSThread)) { assert(false); }
void run_next_thread_and_wait(uint8_t*) { assert(false); }
void check_running_queue(uint8_t*) {}
}
constexpr int32_t sc = int32_t(0x801A69D0), command = sc+0x78, events = sc+0x40;
constexpr int32_t graphics = int32_t(0x8010E310), audio_task = int32_t(0x80230000);
constexpr int32_t clients = int32_t(0x80210000), client_q0 = clients+0x100, client_q1 = clients+0x140;
constexpr int32_t storage = int32_t(0x80220000), result = storage+0x700;
static std::vector<uint32_t> launches, sends;
static unsigned yields, broadcasts, swaps, cache_flushes, captures;
static uint32_t current_fb, next_fb;
static bool fail_command_read;
static std::vector<PresentFrame> presents;
static uint8_t* checked_ram;
static bool fast_completion=false;
static int tail_offset=0;
extern "C" void dora3_native_guest_completion(uint8_t*,recomp_context*);
bool dora3_enqueue_native_present(const PresentFrame& frame) {
    auto* rdram=checked_ram;
    assert(swaps && next_fb==frame.framebuffer); // Must follow original final read/swap.
    if (fast_completion) assert(MEM_W(tail_offset,graphics)!=0x123BAD); // Caller must have finished timing write.
    presents.push_back(frame);
    if (fast_completion) {
        assert(native_present_ledger().host_read_done(frame.ticket));
        assert(game_frame_bridge().drained());
        // Model immediate bank reuse. Any later old scheduler write corrupts this.
        MEM_W(tail_offset,graphics)=0x7654321;
    }
    return true; // Delay host read acknowledgement until the test explicitly finishes it.
}
extern "C" void dora3_native_guest_completion(uint8_t*,recomp_context*);
struct StopLoop {};
struct InjectedFailure {};
extern "C" void dora3_trace(uint8_t*, recomp_context*, uint32_t address, uint32_t) {
    if(address == 0x80026A28 && dora3_native_scheduler_pumping()) {
        std::thread other([] { assert(!dora3_native_scheduler_pumping()); });
        other.join();
    }
}
extern "C" void osRecvMesg_recomp(uint8_t* rdram, recomp_context* c) {
    if (int32_t(c->r4)==command && fail_command_read) throw InjectedFailure{};
    const int value = osRecvMesg(rdram, int32_t(c->r4), int32_t(c->r5), 0);
    if (value == -1 && int32_t(c->r4)==events && c->r6==1) throw StopLoop{};
    assert(c->r6 == 0 || value == 0);
    c->r2 = value;
}
extern "C" void osSendMesg_recomp(uint8_t* rdram, recomp_context* c) {
    if (uint32_t(c->r5)==uint32_t(sc)) ++broadcasts;
    if (int32_t(c->r4)==command || int32_t(c->r4)==events) sends.push_back(uint32_t(c->r4));
    c->r2 = osSendMesg(rdram, int32_t(c->r4), int32_t(c->r5), int32_t(c->r6));
    assert(c->r2==0);
}
extern "C" void osGetCount_recomp(uint8_t* rdram, recomp_context* c) {
    if (fast_completion && swaps && (MEM_W(4,graphics)&3)==0) {
        assert(presents.empty()); // This SDK call is BEFORE the caller's final task accesses.
        assert(osRecvMesg(rdram,int32_t(game_queue),result,0)==0);
        recomp_context game{};game.r29=int32_t(0x803D0000);MEM_W(0x10,game.r29)=MEM_W(0,result);
        dora3_native_guest_completion(rdram,&game);
        assert(!game_frame_bridge().drained());
    }
    c->r2=12345;
}
extern "C" void osSpTaskLoad_recomp(uint8_t*, recomp_context*) {}
extern "C" void osSpTaskStartGo_recomp(uint8_t*, recomp_context* c) { launches.push_back(uint32_t(c->r4)-0x10); }
extern "C" void osWritebackDCacheAll_recomp(uint8_t*, recomp_context*) { ++cache_flushes; }
extern "C" void osSpTaskYield_recomp(uint8_t*, recomp_context*) { ++yields; }
extern "C" void osSpTaskYielded_recomp(uint8_t*, recomp_context* c) { c->r2=1; }
extern "C" void osViGetCurrentFramebuffer_recomp(uint8_t*, recomp_context* c) { c->r2=int32_t(current_fb); }
extern "C" void osViGetNextFramebuffer_recomp(uint8_t*, recomp_context* c) { c->r2=int32_t(next_fb); }
extern "C" void osViSwapBuffer_recomp(uint8_t*, recomp_context* c) { ++swaps;next_fb=uint32_t(c->r4); }
extern "C" void osViBlack_recomp(uint8_t*, recomp_context*) {}
extern "C" void dora3_dp_set_buffer(uint8_t*, recomp_context*) { assert(false); }
extern "C" void dora3_s0_closure_8005DEC0(uint8_t*, recomp_context*) {}
extern "C" void dora3_s0_closure_80059BD0(uint8_t*, recomp_context*) { assert(false); }
extern "C" void dora3_s0_80047EF4(uint8_t*, recomp_context*) { assert(false); }
extern "C" void dora3_rumble_ports_reset(uint8_t*, recomp_context*) { assert(false); }
extern "C" void dora3_capture_frame_lights(uint8_t*, uint32_t bank) { assert(bank==uint32_t(graphics)); ++captures; }
static void send_event(uint8_t* rdram, uint32_t message) {
    assert(osSendMesg(rdram, events, int32_t(message), 0)==0);
}
static void run_scheduler(uint8_t* rdram) {
    recomp_context c{};c.r29=int32_t(0x803F0000);c.r4=sc;
    try {dora3_scheduler_thread(rdram,&c);assert(false);} catch(const StopLoop&) {}
    assert(!dora3_native_scheduler_pumping());
}
static void native_consumer(uint8_t*,recomp_context*,const FrameTicket&,FrameScheduler&);
static void initialize(uint8_t* rdram) {
    // No actual renderer/guest threads in this harness: model its reset barrier.
    native_present_ledger().reset_after_runtime_barrier();
    game_frame_bridge().reset_after_runtime_barrier(0);
    presents.clear();checked_ram=rdram;fast_completion=false;tail_offset=0;
    assert(game_frame_bridge().enter(60,0,0,true,native_consumer));
    std::memset(rdram + (uint32_t(sc)&0x1FFFFFFF),0,0x284);
    osCreateMesgQueue(rdram,events,storage,16);
    osCreateMesgQueue(rdram,command,storage+0x100,16);
    osCreateMesgQueue(rdram,int32_t(game_queue),storage+0x200,8);
    osCreateMesgQueue(rdram,client_q0,storage+0x300,8);
    osCreateMesgQueue(rdram,client_q1,storage+0x400,8);
    MEM_W(0x260,sc)=clients;MEM_W(0,clients)=clients+8;MEM_W(4,clients)=client_q0;
    MEM_W(8,clients)=0;MEM_W(12,clients)=client_q1;
    MEM_W(0x27C,sc)=77;MEM_W(0,int32_t(0x801A48C8))=12;MEM_W(0,int32_t(0x801AC694))=13;
    MEM_W(0,int32_t(0x801AA680))=0;MEM_W(0,int32_t(0x80171584))=command;
    MEM_W(0,int32_t(0x80171CC0))=graphics;
    MEM_W(0,int32_t(0x801B1608))=graphics+0x108;
    MEM_W(0,int32_t(0x801B1424))=graphics+0x12328+16;
    MEM_W(0x100,graphics)=int32_t(0x803B5000);
    MEM_H(0xC0,graphics)=2;MEM_H(0xE0,graphics)=4;
    current_fb=next_fb=0x803B5000;
    sends.clear();launches.clear();yields=broadcasts=swaps=cache_flushes=captures=0;
}
static void counters_unchanged(uint8_t* rdram) {
    assert(MEM_W(0x27C,sc)==77 && MEM_W(0,int32_t(0x801A48C8))==12 && MEM_W(0,int32_t(0x801AC694))==13);
    assert(broadcasts==0);
}
static void native_consumer(uint8_t* rdram,recomp_context* c,const FrameTicket& t,FrameScheduler& s) {
    assert(dora3_native_frame_build_active() && s.publish(t));
    c->r4=0;c->r5=0x40; // Real graphics_frame_build requests the type2 (+C0) completion.
    dora3_graphics_task_submit(rdram,c);
    // Bank completions are deliberately NOT synthesized here.
}
static void publish_native(uint8_t* rdram) {
    auto& b=game_frame_bridge();recomp_context c{};c.r29=int32_t(0x803E0000);
    assert(b.post_due(second,[] {return true;}));
    assert(b.dispatch(WakeMailbox::message_value,second,rdram,&c));
    assert(!dora3_native_frame_build_active() && !b.drained());
    assert((sends==std::vector<uint32_t>{uint32_t(command)}));
    // Wake uses retained external delivery, not another blocking guest send.
    assert(captures==1);
}
static void task(uint8_t* rdram,int32_t address,int type,int flags) {
    MEM_W(0,address)=0;MEM_W(8,address)=flags;MEM_W(0x10,address)=type;
    MEM_W(0x50,address)=int32_t(game_queue);MEM_W(0x54,address)=address+0xE0;
    MEM_W(0xC,address)=int32_t(current_fb);
    assert(osSendMesg(rdram,command,address,0)==0);
}
int main(int argc,char** argv) {
    assert(argc==2);
    std::vector<uint8_t> ram(8*1024*1024);auto* rdram=ram.data();
    std::ifstream seed(argv[1],std::ios::binary);
    seed.read(reinterpret_cast<char*>(rdram+0x7CD60),28);assert(seed.gcount()==28);
    initialize(rdram);
    recomp_context c{};c.r29=int32_t(0x803E0000);
    dora3_graphics_task_submit(rdram,&c); // Original path never wakes scheduler early.
    assert((sends==std::vector<uint32_t>{uint32_t(command)}));
    assert(osRecvMesg(rdram,events,result,0)==-1);

    initialize(rdram);publish_native(rdram);next_fb+=0x25800;run_scheduler(rdram); // Native ignores the old VI gate.
    assert((launches==std::vector<uint32_t>{uint32_t(graphics)}));counters_unchanged(rdram);
    assert(MEM_W(0x274,sc)==graphics && MEM_W(0x278,sc)==graphics);
    send_event(rdram,graphics_pump_message);run_scheduler(rdram);assert(launches.size()==1);counters_unchanged(rdram);
    send_event(rdram,667);run_scheduler(rdram);assert(swaps==0 && presents.empty());
    assert(osRecvMesg(rdram,int32_t(game_queue),result,0)==-1);
    send_event(rdram,668);run_scheduler(rdram);assert(swaps==1);
    assert(osRecvMesg(rdram,int32_t(game_queue),result,0)==0 && MEM_W(0,result)==graphics+0xC0);
    assert(presents.size()==1 && !game_frame_bridge().drained());
    MEM_W(0x10,c.r29)=MEM_W(0,result);
    dora3_native_guest_completion(rdram,&c);
    assert(!game_frame_bridge().drained()); // Scheduler+game done; host still reads it.
    assert(native_present_ledger().host_read_done(presents[0].ticket));
    assert(game_frame_bridge().drained() && !native_present_ledger().occupied());
    assert(!native_present_ledger().host_read_done(presents[0].ticket));
    counters_unchanged(rdram);
    send_event(rdram,666);run_scheduler(rdram);
    assert(broadcasts==2 && MEM_W(0x27C,sc)==78 && MEM_W(0,int32_t(0x801A48C8))==13 && MEM_W(0,int32_t(0x801AC694))==14);

    for (bool dp_first:{false,true}) {
        initialize(rdram);publish_native(rdram);run_scheduler(rdram);
        send_event(rdram,dp_first?668:667);run_scheduler(rdram);assert(presents.empty());
        tail_offset=dp_first?0x58:0x60;MEM_W(tail_offset,graphics)=0x123BAD;
        MEM_W(0x5C,graphics)=100;MEM_W(0x64,graphics)=100;
        fast_completion=true;
        send_event(rdram,dp_first?667:668);run_scheduler(rdram);
        assert(presents.size()==1 && game_frame_bridge().drained());
        assert(MEM_W(tail_offset,graphics)==0x7654321);
    }

    initialize(rdram);
    for (int i=0;i<16;++i) send_event(rdram,900); // Fill scheduler event queue.
    publish_native(rdram); // Must return with the wake retained externally.
    run_scheduler(rdram);assert(launches.size()==1);counters_unchanged(rdram);

    initialize(rdram);task(rdram,graphics,1,0x63);task(rdram,audio_task,2,2);
    send_event(rdram,graphics_pump_message);run_scheduler(rdram);
    assert((launches==std::vector<uint32_t>{uint32_t(audio_task)}) && cache_flushes==1);
    send_event(rdram,667);run_scheduler(rdram);
    assert(launches.size()==2 && launches[1]==uint32_t(graphics));counters_unchanged(rdram);

    initialize(rdram);task(rdram,graphics,1,0x63);send_event(rdram,graphics_pump_message);run_scheduler(rdram);
    task(rdram,audio_task,2,2);send_event(rdram,graphics_pump_message);run_scheduler(rdram);
    assert(yields==1 && launches.size()==1 && (MEM_W(4,graphics)&0x10));counters_unchanged(rdram);

    initialize(rdram);task(rdram,graphics,1,0x63);next_fb+=0x25800;
    send_event(rdram,graphics_pump_message);run_scheduler(rdram);assert(launches.empty());
    next_fb=current_fb;send_event(rdram,graphics_pump_message);run_scheduler(rdram);assert(launches.size()==1);
    counters_unchanged(rdram); // Framebuffer readiness is intentionally still enforced.

    initialize(rdram);task(rdram,graphics,1,0x63);MEM_W(0,int32_t(0x801AA680))=1;
    send_event(rdram,graphics_pump_message);run_scheduler(rdram);assert(launches.empty());counters_unchanged(rdram);
    MEM_W(0,int32_t(0x801AA680))=0;send_event(rdram,graphics_pump_message);run_scheduler(rdram);assert(launches.size()==1);

    initialize(rdram);task(rdram,graphics,1,0x63);send_event(rdram,graphics_pump_message);
    game_frame_bridge().leave(second);run_scheduler(rdram);
    assert(launches.empty());counters_unchanged(rdram); // Late wake in Original.
    send_event(rdram,666);run_scheduler(rdram);assert(launches.size()==1 && broadcasts==2);

    initialize(rdram);c={};c.r29=int32_t(0x803E0000);c.r17=sc;MEM_W(0x10,c.r29)=int32_t(graphics_pump_message);
    auto saved=c;assert(dora3_native_scheduler_message(rdram,&c));assert(std::memcmp(&c,&saved,sizeof(c))==0);
    fail_command_read=true;
    try {dora3_native_scheduler_message(rdram,&c);assert(false);}catch(const InjectedFailure&) {}
    assert(!dora3_native_scheduler_pumping() && std::memcmp(&c,&saved,sizeof(c))==0);
    native_present_ledger().reset_after_runtime_barrier();
    game_frame_bridge().reset_after_runtime_barrier(second);
    std::cout<<"PASS: immediate game+host completion and bank reuse after both SP/DP caller tails (SP-first and DP-first); registered native framebuffer bypass, present after swap, type2 consumption and delayed host acknowledgement; original submit -> native-only wake -> original scheduler including full event queue; no synthetic VI/client broadcast; audio priority/yield, SP/DP completion, framebuffer gate, pause, late Original wake and context/thread-local isolation preserved\n";
}
