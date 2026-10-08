#include "game_reset.hpp"
#include "native_frame_bridge.hpp"
#include <atomic>
#include <cstdio>

extern "C" void recomp_request_game_reset(uint8_t*);
namespace { std::atomic_bool requested{false}; }
void dora3::request_game_reset() { requested.store(true, std::memory_order_release); }

extern "C" void dora3_game_reset_poll(uint8_t* rdram, recomp_context* ctx) {
    if (!requested.exchange(false, std::memory_order_acq_rel)) return;
    // N3DJ resident800278E0: between complete message dispatches, before the
    // next blocking receive and outside every native simulation/build scope.
    // Cancel future native wakes without a scene-transition drain: at reset,
    // legacy entry-pending banks and paused guest completion messages may still
    // be outstanding. The runtime barrier retires host reads before discarding
    // those guest owners; no old-session completion must run on the new RAM.
    (void)ctx;
    dora3::native_time::game_frame_bridge().leave(dora3::native_time::monotonic_time());
    std::fputs("Game reset: guest message boundary; restarting internal session\n", stderr);
    // Does not return: the supervisor stops guest threads, drains SP/graphics,
    // clears session adapters and RAM, then runs this ROM's original entrypoint.
    recomp_request_game_reset(rdram);
}
