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
extern "C" void dora3_native_task_retired(uint8_t*,recomp_context* ctx) {
    if (int32_t(ctx->r2)!=1) return; // Original task-complete returned false.
    auto frame=dora3::native_time::native_present_ledger().retire(uint32_t(ctx->r16));
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
