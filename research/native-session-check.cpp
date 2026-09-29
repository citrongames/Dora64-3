#include "native_frame_session.hpp"
#include "native_frame_producer.hpp"
#include "native_present.hpp"
#include <cassert>
#include <condition_variable>
#include <iostream>
#include <latch>
#include <vector>
using namespace dora3::native_time;
using namespace std::chrono_literals;
using Phase=FrameSession::Phase;
static std::mutex sent_mutex;
static std::condition_variable sent_changed;
static unsigned sent=0;
bool dora3_enqueue_native_frame_wake() {
    {std::lock_guard lock(sent_mutex);++sent;}
    sent_changed.notify_all();return true;
}
static void await_sent(unsigned count) {
    std::unique_lock lock(sent_mutex);
    assert(sent_changed.wait_for(lock,5s,[&] {return sent>=count;}));
    assert(sent==count); // A retained mailbox cannot produce a wake storm.
}
static void expect_quiet(unsigned count) {
    std::unique_lock lock(sent_mutex);
    assert(!sent_changed.wait_for(lock,25ms,[&] {return sent!=count;}));
}
static std::vector<FrameTicket> frames;
static FrameScheduler* frame_owner;
static void consumer(uint8_t*,recomp_context*,const FrameTicket& t,FrameScheduler& owner) {
    assert(owner.publish(t));frames.push_back(t);frame_owner=&owner;
}
static void finish(const FrameTicket& t) {
    assert(frame_owner->complete(t,Completion::GuestTask));
    assert(frame_owner->complete(t,Completion::HostRead));
}
static void handoff() {
    PresentationGate gate;FrameBridge bridge;FrameSession session(bridge,gate);
    assert(!session.begin(60,0,false,consumer) && !gate.reserved());
    std::latch reading(1),finish_read(1);
    std::thread renderer([&] {
        OriginalReadLease lease(gate);assert(lease);reading.count_down();finish_read.wait();
    });
    reading.wait();assert(session.begin(60,0,true,consumer));
    assert(session.poll(second)==Phase::Acquiring && !bridge.active());
    {OriginalReadLease late(gate);assert(!late);}
    session.change_rate(144,second);finish_read.count_down();renderer.join();
    assert(session.poll(2*second)==Phase::Native && bridge.active());
    assert(bridge.post_due(3*second,[] {return true;}));
    recomp_context ctx{};bridge.dispatch(WakeMailbox::message_value,3*second,nullptr,&ctx);
    session.leave(3*second);assert(session.phase()==Phase::Draining && gate.reserved());
    assert(!session.begin(60,0,true,consumer));
    assert(frame_owner->complete(frames.back(),Completion::GuestTask));
    assert(session.poll(4*second)==Phase::Draining);
    assert(frame_owner->complete(frames.back(),Completion::HostRead));
    assert(session.poll(4*second)==Phase::Original && !gate.reserved());
    {OriginalReadLease normal(gate);assert(normal);}
    assert(session.begin(60,0,true,consumer));session.leave(5*second);
    assert(session.phase()==Phase::Original && !bridge.active()); // Cancel before activation.
    auto old=gate.reserve();assert(old && gate.release(*old));auto fresh=gate.reserve();
    assert(fresh && *old!=*fresh && !gate.release(*old));assert(gate.release(*fresh));
    {OriginalReadLease reading_again(gate);
        try {gate.reset_after_runtime_barrier();assert(false);} catch(const std::logic_error&) {}
    }
    assert(session.begin(73,0,true,consumer));assert(session.poll(6*second)==Phase::Native);
    bridge.leave(7*second); // Existing pause handler may invalidate bridge first.
    assert(session.poll(7*second)==Phase::Original);
    bridge.reset_after_runtime_barrier(8*second);session.reset_after_runtime_barrier();
    frames.clear();
}
static void producer() {
    FrameBridge bridge;recomp_context ctx{};
    const auto start=monotonic_time();
    {
        FrameProducer producer(bridge,dora3_enqueue_native_frame_wake);
        expect_quiet(0);
        assert(bridge.enter(360,start,0,true,consumer));
        await_sent(1);expect_quiet(1); // Pending mailbox must sleep until receive.
        bridge.dispatch(WakeMailbox::message_value,monotonic_time(),nullptr,&ctx);
        await_sent(2);bridge.dispatch(WakeMailbox::message_value,monotonic_time(),nullptr,&ctx);
        assert(frames.size()==2);expect_quiet(2); // Both banks occupied.
        assert(frame_owner->complete(frames[0],Completion::GuestTask));expect_quiet(2);
        assert(frame_owner->complete(frames[0],Completion::HostRead));await_sent(3);
        bridge.change_rate(73,monotonic_time());
        bridge.dispatch(WakeMailbox::message_value,monotonic_time(),nullptr,&ctx); // Stale wake after rate change.
        assert(frames.size()==2);await_sent(4);
        bridge.dispatch(WakeMailbox::message_value,monotonic_time(),nullptr,&ctx);
        assert(frames.size()==3 && frames[2].step.elapsed>0);
        bridge.leave(monotonic_time());finish(frames[1]);finish(frames[2]);
        expect_quiet(4);assert(bridge.drained());
        bridge.reset_after_runtime_barrier(monotonic_time());
        assert(bridge.enter(144,monotonic_time(),0,true,consumer));await_sent(5);
    } // Stop wakes an indefinite wait and joins even with a retained game wake.
    expect_quiet(5);
    bridge.leave(monotonic_time());
    bridge.dispatch(WakeMailbox::message_value,monotonic_time(),nullptr,&ctx);
    assert(bridge.drained());frames.clear();
    // Runtime wrappers start idle, stop, and can restart without retaining a thread.
    dora3_start_native_frame_producer();expect_quiet(5);dora3_stop_native_frame_producer();
    dora3_start_native_frame_producer();dora3_stop_native_frame_producer();expect_quiet(5);
}
int main() {
    handoff();producer();
    std::cout<<"PASS: in-progress Original read delays native entry; late reads denied; exit waits both completions; cancellation/stale reservation/pause/reset; actual sleeping producer wakes on deadline, mailbox receive, bank release and rate change; stops/joins with pending wake and restarts idle\n";
}
