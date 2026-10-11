#pragma once
#include "native_frame_time.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include "native_stop_token.hpp"
namespace dora3::native_time {
class WakeSignal {
public:
    uint64_t revision() const { std::lock_guard lock(mutex_);return revision_; }
    void notify() {
        {std::lock_guard lock(mutex_);++revision_;}
        changed_.notify_all();
    }
    void wait(uint64_t seen,std::optional<Nanoseconds> deadline,StopToken stop) {
        std::unique_lock lock(mutex_);
#ifdef __ANDROID__
        const auto changed=[&] {return revision_!=seen||stop.stop_requested();};
        if(deadline)changed_.wait_until(lock,std::chrono::steady_clock::time_point(std::chrono::nanoseconds(*deadline)),changed);
        else changed_.wait(lock,changed);
#else
        const auto changed=[&] {return revision_!=seen;};
        if (deadline) changed_.wait_until(lock,stop,std::chrono::steady_clock::time_point(std::chrono::nanoseconds(*deadline)),changed);
        else changed_.wait(lock,stop,changed);
#endif
    }
    struct Notice {
        WakeSignal& signal;
        ~Notice() { signal.notify(); }
    };
private:
    mutable std::mutex mutex_;
    std::condition_variable_any changed_;
    uint64_t revision_=0;
};
}
