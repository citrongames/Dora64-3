#pragma once
#include <thread>
#include "native_stop_token.hpp"
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
#ifdef __ANDROID__
    FrameBridge* bridge_=nullptr;
    std::atomic_bool stop_{false};
    std::thread thread_;
#else
    std::jthread thread_;
#endif
};
}
void dora3_start_native_frame_producer();
void dora3_stop_native_frame_producer();
