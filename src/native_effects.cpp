#include "native_effects.hpp"
#include "native_simulation_time.hpp"
#include <bit>
#include <cmath>
namespace {
int32_t& word(uint8_t* rdram,uint32_t address) {return MEM_W(0,int32_t(address));}
void real(uint8_t* rdram,uint32_t address,float value) {word(rdram,address)=std::bit_cast<int32_t>(value);}
}
extern "C" int dora3_native_simulation_tick_end() {
    const auto* step=dora3::native_time::active_simulation_slice();
    return !step || step->ends_tick();
}
extern "C" int dora3_native_rectangle_update(uint8_t* rdram,recomp_context*) {
    const auto* step=dora3::native_time::active_simulation_slice();
    if(!step)return 0;
    if(MEM_H(0,int32_t(0x8021811A))!=1)return 1;
    const int previous=MEM_H(0,int32_t(0x802171A4));
    const int duration=MEM_H(0,int32_t(0x80217186));
    const int mode=MEM_H(0,int32_t(0x8021719E));
    const bool expanding=mode==0 || mode==2;
    float elapsed=float(previous)+float(double(step->phase+step->numerator)/double(step->denominator));
    if(step->ends_tick()) {
        const auto next=uint16_t(unsigned(uint16_t(previous))+1u);
        MEM_H(0,int32_t(0x802171A4))=std::bit_cast<int16_t>(next);
        elapsed=float(std::bit_cast<int16_t>(next));
    }
    if(elapsed>float(duration)) {
        // The original deactivates on the following whole update, not early
        // in its last display interval. Closing modes intentionally keep active.
        if(step->ends_tick() && expanding)MEM_H(0,int32_t(0x8021811A))=0;
        return 1;
    }
    const float ratio=elapsed/float(duration);
    const float radius=(expanding?ratio:1.0f-ratio)*300.0f;
    real(rdram,0x8021811C,160.0f-radius);real(rdram,0x80217194,120.0f-radius);
    real(rdram,0x802171E4,radius+160.0f);real(rdram,0x80217198,radius+120.0f);
    return 1;
}
extern "C" int dora3_native_zoom_update(uint8_t* rdram,recomp_context* ctx) {
    const auto* step=dora3::native_time::active_simulation_slice();
    if(!step)return 0;
    if(!MEM_H(0,int32_t(0x80218278)))return 1;
    const float current=std::bit_cast<float>(uint32_t(MEM_W(0x30,ctx->r4)));
    const float target=std::bit_cast<float>(uint32_t(word(rdram,0x80218238)));
    const double alpha=std::bit_cast<double>(uint64_t(LD(int32_t(0x80210C88),0)));
    const double h=step->ticks();
    const double fraction=h==1.0?alpha:-std::expm1(h*std::log1p(-alpha));
    // Preserve the original single-precision subtraction before widening.
    const float next=float(double(current)+double(target-current)*fraction);
    MEM_W(0x30,ctx->r4)=std::bit_cast<int32_t>(next);
    if(step->ends_tick()) {
        const auto count=uint16_t(MEM_H(0,int32_t(0x80218308)));
        const auto remaining=std::bit_cast<int16_t>(uint16_t(unsigned(count)-1u));
        MEM_H(0,int32_t(0x80218308))=remaining;
        if(remaining<=0) {
            MEM_H(0,int32_t(0x80218278))=0;MEM_H(0,int32_t(0x80218308))=0;
            word(rdram,0x80218238)=0;
        }
    }
    return 1;
}
