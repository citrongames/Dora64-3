#pragma once
#include <cstdint>
#include <map>
#include <utility>
namespace RT64 {struct State;struct Workload;}
namespace dora3::graphics {
// Owned by the graphics RDRAM exclusion, including the guest CPU-copy hook.
class FramebufferReadback {
    struct Copy {uint32_t address,width,height;uint8_t siz,fmt;bool depth;uint32_t seed=0;};
    std::map<std::pair<uint32_t,bool>,Copy> pending_;
    bool requested_=false;
    uint32_t depth_address_=0;
public:
    void request(bool native_gameplay,uint32_t depth_address){requested_=native_gameplay;depth_address_=depth_address;}
    bool pending()const{return !pending_.empty();}
    void reset(){pending_.clear();requested_=false;depth_address_=0;}
    bool defer(RT64::State&,const RT64::Workload&);
    void materialize(RT64::State&);
};
}
