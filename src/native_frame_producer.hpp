#pragma once
#include <thread>
namespace dora3::native_time {
class FrameBridge;
// Lifecycle belongs to the runtime coordinator. The producer only sends a wake;
// it never runs guest code, reads guest RAM, or manufactures VI/AI events.
class FrameProducer {
public:
    FrameProducer(FrameBridge& bridge,bool (*sender)());
    ~FrameProducer();
    FrameProducer(const FrameProducer&)=delete;
    FrameProducer& operator=(const FrameProducer&)=delete;
private:
    std::jthread thread_;
};
}
void dora3_start_native_frame_producer();
void dora3_stop_native_frame_producer();
