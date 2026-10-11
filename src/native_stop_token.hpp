#pragma once
#ifdef __ANDROID__
#include <atomic>
namespace dora3::native_time {
// NDK27 libc++ does not expose C++20 jthread/stop_token in its stable ABI.
// Producer owns the flag through join; shutdown also notifies its WakeSignal.
struct StopToken {
    const std::atomic_bool* flag;
    bool stop_requested() const { return flag->load(); }
};
}
#else
#include <stop_token>
namespace dora3::native_time { using StopToken=std::stop_token; }
#endif
