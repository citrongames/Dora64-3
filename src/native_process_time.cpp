#include "native_process_time.hpp"
#include "native_simulation_time.hpp"
#include <array>
#include <stdexcept>
extern "C" int dora3_native_frame_build_active();
namespace {
struct Stamp { uint64_t tick=0; bool seen=false,simulation=false,activated=false; };
// Scheduled game thread only; stamps hold no game-resource pointers.
std::array<Stamp,8> stamps{};
}
extern "C" int dora3_native_process_lifecycle_allowed(uint8_t* rdram,recomp_context* ctx) {
    using namespace dora3::native_time;
    const auto slot=uint32_t(ctx->r18),phase=uint32_t(ctx->r21);
    if(slot>=stamps.size())throw std::logic_error("Invalid process slot");
    const auto* step=active_simulation_slice();
    if(!step && phase!=1)return 1;
    const bool simulation=step!=nullptr;
    const uint64_t tick=step?step->tick:uint32_t(MEM_W(0,int32_t(0x801AC6B4)));
    const auto status=MEM_W(0,ctx->r17),wait=MEM_W(4,ctx->r17);
    auto& stamp=stamps[slot];
    const bool first=!stamp.seen || stamp.tick!=tick || stamp.simulation!=simulation;
    const bool original=!step && !dora3_native_frame_build_active();
    const bool advance=original || (first && (!step || step->phase==0));
    if(first || original)stamp={tick,true,simulation,false};
    if(advance) {
        // Original status4 becomes2 without invoking its callback in that tick.
        // Extra native dispatches must preserve this delayed first invocation.
        stamp.activated=status==4 || (status==1 && wait==0);
        return 1;
    }
    return status==2 && !stamp.activated;
}
void dora3_native_process_time_reset() { stamps={}; }
