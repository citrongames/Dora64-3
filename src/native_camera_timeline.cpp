#include <numeric>
#include "native_camera_timeline.hpp"
#include <algorithm>
#include <span>
#include <stdexcept>
namespace dora3::native_time {
namespace {
struct Region {uint32_t address,size;};
constexpr std::array<Region,5> shake{{{0x802181F0,2},{0x80218254,4},{0x802182C4,2},{0x80218304,4},{0x80218324,2}}};
constexpr std::array<Region,13> fields_801D5864{{{0x80218210u,4u},{0x80218218u,2u},{0x80218228u,2u},{0x8021824Cu,2u},{0x80218284u,4u},{0x8021828Cu,4u},{0x80218294u,2u},{0x802182B4u,4u},{0x802182C0u,4u},{0x802182E4u,4u},{0x802182E8u,4u},{0x802182F4u,4u},{0x80218314u,4u}}};
constexpr std::array<Region,13> fields_801D7294{{{0x802181F8u,4u},{0x80218206u,2u},{0x80218208u,4u},{0x8021820Cu,2u},{0x80218214u,4u},{0x80218224u,4u},{0x80218230u,4u},{0x80218260u,2u},{0x80218280u,4u},{0x80218288u,2u},{0x802182D0u,4u},{0x802182D8u,4u},{0x80218320u,4u}}};
constexpr std::array<Region,3> fields_801D8578{{{0x802182D4u,4u},{0x802182E0u,4u},{0x802182F8u,4u}}};
constexpr std::array<Region,3> fields_801D8A24{{{0x8021827Au,2u},{0x80218298u,4u},{0x802182FCu,4u}}};
constexpr std::array<Region,7> fields_801D8FF0{{{0x802181F2u,2u},{0x8021825Cu,4u},{0x80218268u,4u},{0x8021829Cu,4u},{0x802182A0u,4u},{0x802182C6u,2u},{0x80218310u,4u}}};
constexpr std::array<Region,3> fields_801D9C94{{{0x8021821Cu,4u},{0x8021822Cu,4u},{0x8021830Cu,4u}}};
constexpr std::array<Region,3> fields_801DA06C{{{0x80218234u,4u},{0x802182B8u,4u},{0x802182ECu,4u}}};
constexpr std::array<Region,3> fields_801DAB58{{{0x80218250u,4u},{0x8021827Cu,4u},{0x802182A8u,4u}}};
constexpr std::array<Region,2> fields_801DA588{{{0x8021826Cu,2u},{0x80218270u,4u}}};
constexpr std::array<Region,2> fields_801DAF50{{{0x80218290u,4u},{0x802182CCu,4u}}};
constexpr std::array<Region,1> fields_801DB3D0{{{0x802181ECu,4u}}};
constexpr std::array<Region,0> fields_801DB720{{}};
constexpr std::array<Region,0> fields_801D9B00{{}};
constexpr std::array<Region,2> fields_801D6EAC{{{0x80218220u,2u},{0x80218318u,2u}}};
constexpr std::array<Region,0> fields_801DB7C8{{}};
std::span<const Region> fields(uint32_t root) {
    switch(root){
case 0x801D5864:return fields_801D5864;
case 0x801D7294:return fields_801D7294;
case 0x801D8578:return fields_801D8578;
case 0x801D8A24:return fields_801D8A24;
case 0x801D8FF0:return fields_801D8FF0;
case 0x801D9C94:return fields_801D9C94;
case 0x801DA06C:return fields_801DA06C;
case 0x801DAB58:return fields_801DAB58;
case 0x801DA588:return fields_801DA588;
case 0x801DAF50:return fields_801DAF50;
case 0x801DB3D0:return fields_801DB3D0;
case 0x801DB720:return fields_801DB720;
case 0x801D9B00:return fields_801D9B00;
case 0x801D6EAC:return fields_801D6EAC;
case 0x801DB7C8:return fields_801DB7C8;
    default:throw std::invalid_argument("Unreviewed ordinary camera root");
    }
}
std::array<uint8_t,256> capture(uint8_t* rdram,uint32_t camera,std::span<const Region> own) {
    std::array<uint8_t,256> result{};size_t i=0;
    auto copy=[&](Region region){for(uint32_t b=0;b<region.size;++b)result.at(i++)=MEM_BU(0,int32_t(region.address+b));};
    copy({camera,0x0C});copy({camera+0x18,0x0C});copy({camera+0x30,4});for(auto region:own)copy(region);for(auto region:shake)copy(region);
    return result;
}
void restore(uint8_t* rdram,uint32_t camera,std::span<const Region> own,const std::array<uint8_t,256>& state) {
    size_t i=0;
    auto copy=[&](Region region){for(uint32_t b=0;b<region.size;++b)MEM_B(0,int32_t(region.address+b))=state.at(i++);};
    copy({camera,0x0C});copy({camera+0x18,0x0C});copy({camera+0x30,4});for(auto region:own)copy(region);for(auto region:shake)copy(region);
}
}
void OrdinaryCameraTimeline::reset(){initialized_=anchored_=false;tick_=phase_=denominator_=0;root_=camera_=0;}
void OrdinaryCameraTimeline::advance(uint8_t* rdram,recomp_context* ctx,const SimulationSlice& slice,uint32_t root,Update update) {
    if(!rdram||!ctx||!update||active_simulation_slice())throw std::invalid_argument("Invalid ordinary camera evaluation");
    {SimulationScope validate(slice);}
    const auto own=fields(root);const auto camera=uint32_t(ctx->r4);
    if(camera<0x801A99F8||camera>0x801A9A94||(camera-0x801A99F8)%0x34)throw std::invalid_argument("Invalid ordinary camera pointer");
    if(initialized_&&(slice.tick!=tick_||slice.phase!=phase_||slice.denominator!=denominator_))throw std::logic_error("Non-contiguous camera time");
    const auto current=capture(rdram,camera,own);
    if(slice.phase==0){initial_=current;anchored_=true;}
    else if(anchored_&&initialized_&&root==root_&&camera==camera_&&slice.ends_tick()&&current!=published_&&
            std::equal(current.begin(),current.begin()+28+std::accumulate(own.begin(),own.end(),size_t{0},[](size_t n,Region v){return n+v.size;}),published_.begin())) {
        // A Source-end shake command changes only the five shared shake
        // fields. Keep the original camera equation anchor and apply the new
        // command in this ending Source camera evaluation, regardless of how
        // many variable frames preceded it. External pose/root writes still
        // use the ordinary invalidation path below.
        const auto shake_start=28+std::accumulate(own.begin(),own.end(),size_t{0},[](size_t n,Region v){return n+v.size;});
        std::copy(current.begin()+shake_start,current.end(),initial_.begin()+shake_start);
        restore(rdram,camera,own,initial_);
    }
    else if(!initialized_||root!=root_||camera!=camera_||current!=published_) {
        // A cut, new shake command, restore, or another owner changed the state.
        // Never roll that write back. Run incremental equations until the next
        // tick, then establish a fresh anchor. No stale guest pointers are kept.
        anchored_=false;
    } else if(anchored_)restore(rdram,camera,own,initial_);
    const SimulationSlice evaluation=anchored_?SimulationSlice{slice.phase+slice.numerator,slice.denominator,slice.tick,0}:slice;
    {SimulationScope scope(evaluation);update(rdram,ctx);}
    published_=capture(rdram,camera,own);root_=root;camera_=camera;initialized_=true;
    denominator_=slice.denominator;tick_=slice.tick+(slice.ends_tick()?1:0);phase_=slice.ends_tick()?0:slice.phase+slice.numerator;
}
}

namespace {
struct CameraLane {
    dora3::native_time::OrdinaryCameraTimeline time;
    uint64_t tick=0,phase=0,denominator=0;
    bool used=false;
};
thread_local std::array<CameraLane,4> camera_lanes;
}
void dora3_native_camera_dispatch_reset(){camera_lanes={};}
extern "C" void dora3_native_camera_dispatch(uint8_t* rdram,recomp_context* ctx,uint32_t root,recomp_func_t* update) {
    using namespace dora3::native_time;
    const auto* current=active_simulation_slice();
    if(!current){update(rdram,ctx);return;}
    const auto slice=*current;const auto camera=uint32_t(ctx->r4);
    if(camera<0x801A99F8||camera>0x801A9A94||(camera-0x801A99F8)%0x34)throw std::logic_error("Invalid camera dispatch pointer");
    auto& lane=camera_lanes[(camera-0x801A99F8)/0x34];
    if(lane.used && (slice.tick!=lane.tick||slice.phase!=lane.phase||slice.denominator!=lane.denominator))lane.time.reset();
    {
        SimulationSuspension subsystem_boundary;
        lane.time.advance(rdram,ctx,slice,root,update);
    }
    lane.used=true;lane.denominator=slice.denominator;
    lane.tick=slice.tick+(slice.ends_tick()?1:0);lane.phase=slice.ends_tick()?0:slice.phase+slice.numerator;
}
