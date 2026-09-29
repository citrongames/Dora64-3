#include "native_present.hpp"
#include "native_frame_bridge.hpp"
#include "recomp.h"
#include <stdexcept>

extern "C" void dora3_native_task_register(uint8_t* rdram,recomp_context* ctx) {
    const auto* active=dora3::native_time::active_frame_binding();
    if (!active) return;
    const auto task=uint32_t(ctx->r8);
    // Called before the original SendMesg delay slot writes task+0C: v0 is FB.
    if (MEM_W(0x10,int32_t(task))!=1 || MEM_W(8,int32_t(task))!=0x63 ||
        MEM_W(0x50,int32_t(task))!=int32_t(dora3::native_time::game_queue) ||
        MEM_H(0xC0,int32_t(task))!=2)
        throw std::logic_error("Unreviewed native graphics task");
    dora3::native_time::native_present_ledger().bind(active->ticket,*active->owner,task,
        uint32_t(ctx->r2),uint32_t(MEM_W(0x54,int32_t(task))));
}
extern "C" void dora3_native_task_retired(uint8_t* rdram,recomp_context* ctx) {
    // SP/DP callers still touch task timing fields after task_complete returns.
    // Called at 80026BEC/80026C94, after THEIR final task access, while s0 is task.
    const auto task=uint32_t(ctx->r16);
    if ((uint32_t(MEM_W(4,int32_t(task)))&3u)!=0) return;
    auto frame=dora3::native_time::native_present_ledger().retire(task);
    if (frame && !dora3_enqueue_native_present(*frame)) throw std::bad_alloc{};
}
extern "C" void dora3_native_guest_completion(uint8_t* rdram,recomp_context* ctx) {
    // Type2 has been consumed. The branch delay-slot stores outstanding next;
    // this hook never yields, so no new guest build can precede that store.
    dora3::native_time::native_present_ledger().consumed(uint32_t(MEM_W(0x10,ctx->r29)));
}
extern "C" int dora3_native_framebuffer_ready(uint32_t task) {
    // Only registered native tasks use explicit present. Original keeps VI gate.
    return dora3::native_time::native_present_ledger().owns_task(task) ? 1 : 0;
}

extern "C" int dora3_native_completion_during_pause(uint8_t* rdram,recomp_context* ctx) {
    // Original pause handling discards messages before type2 dispatch. A native
    // completion still owns memory; consume just that registered message via
    // the original type2 branch, with gameplay/update remaining paused.
    return MEM_W(0,int32_t(0x801AA680))==1 &&
        dora3::native_time::native_present_ledger().awaiting_message(uint32_t(MEM_W(0x10,ctx->r29)));
}
