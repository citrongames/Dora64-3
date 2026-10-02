#include "native_input_timeline.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>
static dora3::native_time::Nanoseconds input_now=0;
namespace dora3::native_time {Nanoseconds monotonic_time(){return input_now;}}
extern "C" {
void dora3_controller_accumulate(uint8_t*,recomp_context*);
void dora3_controller_latch(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
}
int main() {
    std::vector<uint32_t> memory(0x400000/4);
    auto* rdram=reinterpret_cast<uint8_t*>(memory.data());recomp_context ctx{};
    ctx.r29=int32_t(0x80300000);ctx.f_odd=&ctx.f0.u32h;
    auto raw=[&](uint16_t held,int8_t x,int8_t y,unsigned port=0) {
        MEM_H(0,int32_t(0x80171560+6*port))=int16_t(held);
        MEM_B(2,int32_t(0x80171560+6*port))=x;
        MEM_B(3,int32_t(0x80171560+6*port))=y;
        MEM_B(4,int32_t(0x80171560+6*port))=0;
        dora3_controller_accumulate(rdram,&ctx);
    };
    unsigned cases=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        memory.assign(memory.size(),0);MEM_B(0,int32_t(0x801A48B4))=15;input_now=0;
        raw(0,0,0);dora3_controller_latch(rdram,&ctx);native_input_begin(rdram);
        unsigned vi=0,presses=0,directions=0;
        for(unsigned frame=1;frame<=fps;++frame) {
            while(uint64_t(vi+1)*fps<=uint64_t(frame)*60) {
                ++vi;
                input_now=int64_t(vi)*dora3::native_time::second/60;
                // A full press/release can occur before the next native update.
                raw(vi==1 || (vi>=15&&vi<=25)?0x8000:0,vi==1 || (vi>=15&&vi<=25)?50:0,-50);
                raw(0x4000,-50,49,1);
                if(vi%divider==0)dora3_controller_latch(rdram,&ctx);
            }
            const auto before=memory;
            {
                NativeInputScope input(rdram);
                presses+=bool(MEM_HU(0,int32_t(0x801A48EC))&0x8000);
                directions+=bool(MEM_HU(0,int32_t(0x801A48CC))&0x100);
                assert(MEM_HU(2,int32_t(0x801A953C))==1);
                if(vi) {
                    assert(MEM_HU(2,int32_t(0x801A9504))==0x4000);
                    assert(MEM_HU(2,int32_t(0x801A39E2))==0x200);
                }
            }
            assert(memory==before);
        }
        assert(presses==2 && directions==2);native_input_end(rdram);
        dora3_controller_latch(rdram,&ctx);assert(MEM_HU(0,int32_t(0x801A48EC))==0);
        ++cases;
    }
    // Handoff preserves a pending short press, including release-before-latch.
    native_input_begin(rdram);raw(0x8000,50,0);raw(0,0,0);
    native_input_end(rdram);dora3_controller_latch(rdram,&ctx);
    assert(MEM_HU(0,int32_t(0x801A48EC))==0x8000);
    assert(MEM_HU(0,int32_t(0x801A48CC))==0x100);
    // A held consumed press is acknowledged by the original history.
    raw(0,0,0);native_input_begin(rdram);raw(0x8000,0,0);
    {NativeInputScope input(rdram);assert(MEM_HU(0,int32_t(0x801A48EC))==0x8000);}
    native_input_end(rdram);dora3_controller_latch(rdram,&ctx);
    assert(MEM_HU(0,int32_t(0x801A48EC))==0);
    // Disconnect clears edges. Reconnect produces one new edge.
    native_input_begin(rdram);MEM_B(4,int32_t(0x80171560))=1;dora3_controller_accumulate(rdram,&ctx);
    {NativeInputScope input(rdram);assert(!MEM_HU(0,int32_t(0x801A953C)) && !MEM_HU(0,int32_t(0x801A48EC)));}
    raw(0x8000,-50,50);
    {NativeInputScope input(rdram);assert(MEM_HU(0,int32_t(0x801A48EC))==0x8000);assert(MEM_HU(0,int32_t(0x801A39E2))==0xA00);}
    // Replacement state is not overwritten by an old scope destructor.
    {NativeInputScope input(rdram);native_input_reset();MEM_H(0,int32_t(0x801A9504))=123;}
    assert(MEM_HU(0,int32_t(0x801A9504))==123);
    std::cout<<"PASS: "<<cases<<" real VI accumulator/latch schedules at 60/73/120/144/165/360 FPS, four-port ownership, short presses, analog thresholds, Original handoff, disconnect/reconnect and scene reset; no game/GPU.\n";
}
