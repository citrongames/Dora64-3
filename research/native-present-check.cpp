#include "native_present.hpp"
#include "native_frame_bridge.hpp"
#include "native_frame_session.hpp"
#include "native_vi_snapshot.hpp"
#include "frame_lights.hpp"
#include "graphics_sync.hpp"
#include <variant>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
using namespace dora3::native_time;
using ultramodern::renderer::ViRegs;
extern "C" uint32_t osVirtualToPhysical(int32_t address) { return uint32_t(address)&0x1FFFFFFF; }
#include "original-vi.h"

// The native enqueue function below is extracted from the actual adapted runtime.
// Only its action queue endpoint is replaced; no renderer or device is created.
struct ScreenUpdateAction {
    ViRegs regs;
    bool cpu_changes_only=false, periodic=false;
    std::optional<FrameTicket> native_ticket;
    uint64_t native_epoch=0;
};
struct FakeQueue {
    bool accept=true;
    std::vector<ScreenUpdateAction> actions;
    bool enqueue(const ScreenUpdateAction& a) { if (!accept) return false;actions.push_back(a);return true; }
};
static struct {
    std::mutex message_mutex;
    struct : ViOracle { std::mutex regs_mutex; ViRegs update_screen_regs{}; } vi;
    FakeQueue action_queue;
} events_context;
#include "native-present-backend.h"
static unsigned screen_reads;
static bool expect_owned;
static void (*during_read)()=nullptr;
static unsigned display_refresh_rate;
static float resolution_scale;
struct FakeRenderer {
    void update_screen(bool cpu_only) {
        assert(!cpu_only);
        if (expect_owned) assert(native_present_ledger().occupied() && !game_frame_bridge().drained());
        if (during_read) during_read();
        ++screen_reads;
    }
    unsigned get_display_framerate() { return 144; }
    float get_resolution_scale() { return 1.0f; }
} renderer;
static auto* renderer_context=&renderer;
#include "native-present-worker.h"

static FrameTicket begin(FrameScheduler& s,Nanoseconds now) {
    auto w=s.request(now);assert(w);
    auto t=s.receive(*w,now);assert(t && s.publish(*t));return *t;
}
static uint32_t task(const FrameTicket& t) { return dora3::frame::bank0+t.bank*dora3::frame::bank_stride; }
static uint32_t fb(const FrameTicket& t) { return t.bank?0x803DA800u:0x803B5000u; }
static void bind(PresentLedger& l,FrameScheduler& s,const FrameTicket& t) { l.bind(t,s,task(t),fb(t),task(t)+0xC0); }
static void finish(PresentLedger& l,const FrameTicket& t) {
    assert(l.retire(task(t)));assert(l.host_read_done(t));assert(l.consumed(task(t)+0xC0));
}
static void ownership() {
    std::array<int,3> order{0,1,2};
    do {
        FrameScheduler s;PresentLedger l;assert(s.enter(60,0,0,true));
        const auto a=begin(s,second),b=begin(s,2*second);bind(l,s,a);bind(l,s,b);
        bool retired=false,consumed=false,host=false;
        for (int event:order) {
            if (event==0) { assert(l.retire(task(a)));retired=true;assert(!l.retire(task(a))); }
            if (event==1) { assert(l.consumed(task(a)+0xC0));consumed=true;assert(!l.consumed(task(a)+0xC0)); }
            if (event==2) { assert(l.host_read_done(a)==retired);host=retired; }
            assert(l.owns_task(task(a))!= (retired&&consumed&&host));
            if (!(retired&&consumed&&host)) assert(!s.request(3*second));
        }
        if (!host) assert(l.host_read_done(a)); // Rejected pre-retirement host callback cannot release it.
        assert(!l.host_read_done(a));
        const auto c=begin(s,3*second);assert(c.bank==a.bank && c.token!=a.token);bind(l,s,c);
        assert(!l.host_read_done(a) && l.owns_task(task(c))); // Stale copied ticket after bank reuse.
        s.leave(3*second);assert(!s.drained() && l.occupied());
        finish(l,b);finish(l,c);assert(s.drained() && !l.occupied());
        const auto generation=s.generation();s.reset_after_runtime_barrier(4*second);l.reset_after_runtime_barrier();
        assert(s.generation()!=generation && !l.host_read_done(c));
    } while (std::next_permutation(order.begin(),order.end()));

    FrameScheduler s;PresentLedger l;assert(s.enter(144,0,0,true));
    auto a=begin(s,second),b=begin(s,2*second);bind(l,s,a);
    try { l.bind(b,s,task(b),fb(a),task(b)+0xC0);assert(false); } catch(const std::logic_error&) {}
    try { l.bind(b,s,task(b),fb(b),task(b)+0xE0);assert(false); } catch(const std::logic_error&) {}
    bind(l,s,b);assert(l.retire(task(a)));
    std::thread renderer([&] { assert(l.host_read_done(a)); });
    std::thread game([&] { assert(l.consumed(task(a)+0xC0)); });
    renderer.join();game.join();assert(!l.owns_task(task(a)));
    finish(l,b);assert(s.drained() && !l.occupied());
}

static void snapshots() {
    OSViMode mode{};
    mode.comRegs={0x3216,320,0x03E52239,525,0xC15,0x0C150C15,0x006C02EC,0x200,7};
    mode.fldRegs[0]={640,0x400,0x002501FF,0x000E0204,2};
    mode.fldRegs[1]={1280,0x800,0x00260200,0x000F0205,3};
    static_assert(sizeof(ViRegs)==14*sizeof(unsigned));
    for(unsigned field:{0u,1u}) for(uint32_t state:{0u,0x20u,0x40u,0x60u}) for(uint32_t address:{0x803B5000u,0x803DA800u}) {
        ViOracle oracle{};oracle.field=field;oracle.regs.VI_V_CURRENT_LINE_REG=123;
        *oracle.get_next_state()={&mode,int32_t(address),0,0,state,0xDEADBEEF,1};
        const auto copied=present_vi_snapshot(oracle.regs,mode,state,0xDEADBEEF,address,field);
        oracle.update_vi(); // Exact pinned runtime method, independently extracted.
        assert(std::memcmp(&copied,&oracle.regs,sizeof(copied))==0 && copied.VI_V_CURRENT_LINE_REG==123);
    }
    try { present_vi_snapshot({},mode,0,0,0,2);assert(false); } catch(const std::invalid_argument&) {}

    FrameScheduler s;PresentLedger l;assert(s.enter(60,0,0,true));auto t=begin(s,second);bind(l,s,t);
    auto frame=l.retire(task(t));assert(frame);
    try { dora3_enqueue_native_present(*frame);assert(false); } catch(const std::logic_error&) {}
    auto& vi=events_context.vi;vi.field=1;vi.regs.VI_V_CURRENT_LINE_REG=99;
    // Deliberately wrong next framebuffer: enqueue must use this task's framebuffer.
    *vi.get_next_state()={&mode,int32_t(0x803DA800),0,0,0,0x12345678,1};
    const int original_state=vi.cur_state;
    assert(dora3_enqueue_native_present(*frame));
    assert(vi.cur_state==original_state && vi.get_next_state()->framebuffer==int32_t(0x803DA800));
    auto& queued=events_context.action_queue.actions.back();
    assert(queued.regs.VI_ORIGIN_REG==(fb(t)&0x1FFFFFFF)+1280 && queued.native_ticket->token==t.token);
    assert(!queued.periodic && !queued.cpu_changes_only && queued.regs.VI_STATUS_REG==0x12345678);
    mode.fldRegs[1].origin=0;vi.get_next_state()->control=0;assert(queued.regs.VI_STATUS_REG==0x12345678);
    assert(queued.regs.VI_ORIGIN_REG==(fb(t)&0x1FFFFFFF)+1280);
    assert(!s.drained());assert(l.consumed(task(t)+0xC0));assert(!s.drained());
    assert(l.host_read_done(*queued.native_ticket) && s.drained());
    events_context.action_queue.accept=false;assert(!dora3_enqueue_native_present(*frame));
}
static std::optional<PresentFrame> submitted;
static void submit(uint8_t*,recomp_context*,const FrameTicket& t,FrameScheduler& s) {
    assert(s.publish(t));auto& ledger=native_present_ledger();bind(ledger,s,t);
    submitted=ledger.retire(task(t));assert(submitted);
    assert(ledger.consumed(task(t)+0xC0)); // Only renderer acknowledgement remains.
}
static void worker() {
    auto& bridge=game_frame_bridge();auto& permit=dora3::sync::periodic_present;
    const auto old_epoch=native_presentation_epoch();
    ScreenUpdateAction old{{},false,true,{},old_epoch};
    assert(permit.acquire());assert(bridge.enter(60,0,0,true,submit));
    process_action(old);assert(screen_reads==0); // Native took ownership after enqueue.
    assert(permit.acquire());permit.complete();
    recomp_context context{};assert(bridge.post_due(second,[] {return true;}));
    bridge.dispatch(WakeMailbox::message_value,second,nullptr,&context);
    bridge.leave(second);assert(native_presentation_owned());
    expect_owned=true;process_action(ScreenUpdateAction{{},false,false,submitted->ticket});
    assert(screen_reads==1 && bridge.drained() && !native_presentation_owned());
    expect_owned=false;
    assert(permit.acquire());process_action(old);assert(screen_reads==1); // Late old request after drain still drops.
    assert(permit.acquire());old.native_epoch=native_presentation_epoch();process_action(old);
    assert(screen_reads==2 && display_refresh_rate==144 && resolution_scale==1.0f);
    // Exercise the actual runtime worker lease, not just the gate in isolation.
    during_read=[] {
        auto& session=game_frame_session();assert(session.begin(60,0,true,submit));
        assert(session.poll(second)==FrameSession::Phase::Acquiring);
        assert(!game_frame_bridge().active() && native_presentation_owned());
    };
    assert(permit.acquire());process_action(old);during_read=nullptr;
    assert(screen_reads==3 && game_frame_session().phase()==FrameSession::Phase::Acquiring);
    assert(game_frame_session().poll(2*second)==FrameSession::Phase::Native);
    game_frame_session().leave(3*second);assert(!presentation_gate().reserved());
    assert(permit.acquire());permit.complete();
}
int main() {
    ownership();snapshots();worker();
    std::cout<<"PASS: all six completion orderings, early/duplicate/stale callbacks, two-bank backpressure, concurrent host/game completion; exact runtime VI oracle (16 combinations), copied framebuffer/mode, no VI state advance, enqueue rejection, real worker acknowledges after reads and drops late periodic actions\n";
}
