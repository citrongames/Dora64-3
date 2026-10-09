#pragma once
#include <array>
#include <cstddef>

namespace dora3::native_time {
// Clear in place: assigning {} to a whole actor pool can materialize a
// pool-sized temporary on the host thread stack. Storage must not scale
// with common_capacity during a scene handoff or a simulation slice.
template<class T, std::size_t N>
void reset_slots(std::array<T, N>& slots) {
    for (auto& slot : slots) {
        if constexpr (requires { slot.reset(); }) slot.reset();
        else slot = T{};
    }
}
}
