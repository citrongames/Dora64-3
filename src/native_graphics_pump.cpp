#include "recomp_context_copy.hpp"
#include "native_graphics_pump.hpp"
#include "native_frame_bridge.hpp"
#include <stdexcept>

extern "C" void dora3_scheduler_retrace(uint8_t*, recomp_context*);

namespace {
// Every scheduled guest thread has its own host thread in the pinned runtime.
// An SDK yield must not put another guest thread's VI handler in pump mode.
thread_local bool pumping = false;
struct PumpScope {
    bool previous = pumping;
    PumpScope() { pumping = true; }
    ~PumpScope() { pumping = previous; }
};
}
extern "C" int dora3_native_scheduler_pumping() { return pumping; }

extern "C" int dora3_native_scheduler_message(uint8_t* rdram, recomp_context* ctx) {
    if (uint32_t(MEM_W(0x10, ctx->r29)) != dora3::native_time::graphics_pump_message) return 0;
    // A retained wake can arrive after leaving Manual. Do not let it start
    // Original tasks ahead of the next real VI; normal completions still run.
    if (!dora3::native_time::game_frame_bridge().active() ||
        MEM_W(0, int32_t(0x801AA680)) == 1) return 1;
    recomp_context pump_context;dora3::copy_recomp_context(pump_context,*ctx);
    pump_context.r4 = ctx->r17; // Original scheduler thread holds OSSc* in s1.
    PumpScope scope;
    // Reuse original drain/select/yield/launch, with only the retrace counter
    // write and client broadcast bypassed by verified generated hooks.
    dora3_scheduler_retrace(rdram, &pump_context);
    return 1;
}

extern "C" void dora3_native_task_submitted(uint8_t* rdram, recomp_context* ctx) {
    if (!dora3_native_frame_build_active()) return;
    // This hook runs AFTER the original blocking command enqueue returns.
    // Publishing the wake before that enqueue could let the scheduler drain an
    // empty command queue and leave the new frame waiting for the next VI.
    if (int32_t(ctx->r2) != 0) throw std::runtime_error("Native graphics command enqueue failed");
    const uint32_t command_queue = uint32_t(MEM_W(0, int32_t(0x80171584)));
    // N3DJ rev0: scheduler_init stores scheduler+0x78; receive queue is +0x40.
    if (command_queue != 0x801A6A48u) throw std::runtime_error("Unreviewed native scheduler queue");
    // Do not block the game thread on another guest queue: the scheduler can
    // itself be waiting to send a completion to that game's full receive queue.
    // The retained external message is delivered by the normal runtime drain.
    if (!dora3_enqueue_native_graphics_wake()) throw std::bad_alloc{};
}
