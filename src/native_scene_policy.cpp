#include "native_scene_policy.hpp"
#include "recomp.h"
namespace {
int32_t word(uint8_t* rdram,uint32_t address) {return MEM_W(0,int32_t(address));}
bool scene34_armed=false;
bool image_eligible(uint8_t* rdram) {
    if(!scene34_armed || word(rdram,0x801C68C4)!=0 || word(rdram,0x801C68CC)!=0 || word(rdram,0x801C68D0)<=1)return false;
    if(word(rdram,0x801A39DC)!=3 || word(rdram,0x801AA648)==0 || word(rdram,0x801A9500)==0)return false;
    for(unsigned i=0;i<8;++i) {
        const uint32_t slot=0x801A44F0+i*20;
        if(i<2) {
            if(word(rdram,slot)!=2 || word(rdram,slot+12)!=int(i) || uint32_t(word(rdram,slot+16))!=(i?0x801C012Cu:0x801C0088u))return false;
        } else if(word(rdram,slot)!=0 && word(rdram,slot)!=5)return false;
    }
    // Cross-frame framebuffer copy reads the other bank. It needs an explicit
    // dependency model and is not authorized by this initial scene policy.
    return word(rdram,0x801A9C88)==0;
}
const ScenePolicy image_policy{"scene34-image",image_eligible,3};
}
const ScenePolicy* select_native_scene_policy(uint8_t* rdram) {
    // Additional reviewed scene families join this registry, without changing
    // the event loop, ownership protocol or settings implementation.
    if(image_policy.eligible(rdram))return &image_policy;
    return nullptr;
}
void native_scene34_ready() {scene34_armed=true;}
void invalidate_native_scene_policy() {scene34_armed=false;}
