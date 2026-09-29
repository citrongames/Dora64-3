#include "native_frame_producer.hpp"
#include "native_frame_bridge.hpp"
#include <stdexcept>
namespace dora3::native_time {
FrameProducer::FrameProducer(FrameBridge& bridge,WakeSender sender) {
    if (!sender) throw std::invalid_argument("Missing native wake sender");
    thread_=std::jthread([&bridge,sender](std::stop_token stop) {
        while (!stop.stop_requested()) {
            // Read revision BEFORE readiness. Mutations in either gap force
            // wait() to return; acknowledgements cannot be lost before sleep.
            const auto seen=bridge.wake_revision();
            const auto deadline=bridge.deadline();
            const auto now=monotonic_time();
            if (deadline && now>=*deadline) {
                bridge.post_due(now,sender);
                continue;
            }
            bridge.wait_for_change(seen,deadline,stop);
        }
    });
}
FrameProducer::~FrameProducer() { thread_.request_stop(); } // jthread joins before its referents may be destroyed.
}
namespace {
struct RuntimeProducer {
    dora3::native_time::FrameBridge& bridge=dora3::native_time::game_frame_bridge();
    std::optional<dora3::native_time::FrameProducer> producer;
};
RuntimeProducer& runtime_producer() { static RuntimeProducer state;return state; }
bool send_wake() {
    if (!dora3_enqueue_native_frame_wake()) throw std::bad_alloc{};
    return true;
}
}
void dora3_start_native_frame_producer() {
    auto& state=runtime_producer();
    if (state.producer) throw std::logic_error("Native producer started twice");
    state.producer.emplace(state.bridge,send_wake);
}
void dora3_stop_native_frame_producer() { runtime_producer().producer.reset(); }
