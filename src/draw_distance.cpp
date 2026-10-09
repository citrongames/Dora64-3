#include "performance_profile.hpp"
#include "draw_distance.hpp"
#include "guest_storage.hpp"
#include "recomp.h"
#include <algorithm>
#include <atomic>
#include <cmath>
namespace {
std::atomic<float> configured{1.f};
bool creating=false,failed=false;
constexpr uint32_t actors=0x804B1CD0u,stride=0xCC;
int32_t word(uint8_t* rdram,uint32_t at){return MEM_W(0,int32_t(at));}
unsigned free_actors(uint8_t* ram) {
    unsigned count=0;
    for(unsigned i=dora3::distance::common_begin;i<dora3::distance::common_end;++i)
        count+=word(ram,actors+i*stride+0x94)==0;
    return count;
}
unsigned free_records(uint8_t* ram,uint32_t base,unsigned count,unsigned bytes,unsigned used_offset=0) {
    unsigned free=0;for(unsigned i=0;i<count;++i)free+=word(ram,base+i*bytes+used_offset)==0;return free;
}
bool admit_distant(uint8_t* ram) {
    dora3::profile::Scope profile(dora3::profile::Tag::DistanceAdmission);
    using namespace dora3::distance;
    // Distant residents cannot consume the emergency room for Original-range
    // spawns, linked parts, rewards, shots and their delayed resource release.
    return free_actors(ram)>=80 && heap_available(ram)>=0x200000 &&
        free_records(ram,sprites,sprite_capacity,8)>=128 &&
        free_records(ram,resources,resource_capacity,20)>=256;
}
}
namespace dora3::distance {
float configure(float v){v=std::isfinite(v)?std::clamp(v,1.f,5.f):1.f;configured.store(v,std::memory_order_relaxed);return v;}
float multiplier(){return configured.load(std::memory_order_relaxed);}
}
extern "C" float dora3_distance_squared(float original) {
    const float value=dora3::distance::multiplier();
    return value==1.f?original:original*value*value;
}
extern "C" float dora3_distance_lifetime(float original) {
    //30000^2 is the large Original class, including story/transient owners.
    // Do not extend their destruction rules or write squared values to uint32.
    return original>=900000000.f?original:dora3_distance_squared(original);
}
extern "C" float dora3_distance_far(float original) {
    const float value=dora3::distance::multiplier();return value==1.f?original:original*value;
}
extern "C" float dora3_distance_spawn_limit(uint8_t* ram,float original,float distance) {
    if(distance<=original||original>=900000000.f||dora3::distance::multiplier()==1.f)return original;
    return admit_distant(ram)?dora3_distance_lifetime(original):original;
}
extern "C" void dora3_distance_spawn_begin(uint8_t*,recomp_context*){creating=true;failed=false;}
extern "C" void dora3_distance_spawn_result(uint8_t*,recomp_context* ctx){if(creating&&ctx->r2==0)failed=true;}
extern "C" void dora3_distance_spawn_end(uint8_t* rdram,recomp_context* ctx) {
    if(creating&&failed) {
        const unsigned home=unsigned(ctx->r23)-1;
        bool exists=false;
        for(unsigned i=dora3::distance::common_begin;i<dora3::distance::common_end;++i) {
            const auto actor=actors+i*stride;
            if(!word(rdram,actor+0x94))continue;
            const auto physical=uint32_t(word(rdram,actor+0x98));
            if(physical>=0x80000000u&&uint64_t(physical)+0xF4<=dora3::distance::game_ram_end&&MEM_H(0xBC,int32_t(physical))==int(home)) {exists=true;break;}
        }
        // The manager publishes flag3 before calling the constructor. Retry
        // only a genuine allocation failure, not quest/day/night exclusions.
        if(!exists&&home<256&&uint32_t(ctx->r21)==0x8021BCE8u+home)MEM_B(0,ctx->r21)=1;
    }
    creating=false;failed=false;
}
