#include "native_process_time.hpp"
#include "native_simulation_time.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <vector>
using namespace dora3::native_time;
static bool native=false;
static unsigned calls=0;
extern "C" {
void dora3_process_dispatch(uint8_t*,recomp_context*);
void reference_dora3_process_dispatch(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) { std::abort(); }
int dora3_native_frame_build_active() {return native;}
void callback(uint8_t*,recomp_context*) {++calls;}
recomp_func_t* get_function(int32_t pc) {assert(uint32_t(pc)==0x802F0000);return callback;}
}
void setup(uint8_t* rdram,unsigned status,unsigned delay,unsigned phase) {
    for(unsigned i=0;i<8;++i) {
        const auto p=int32_t(0x801A44F0+20*i);
        MEM_W(0,p)=status;MEM_W(4,p)=delay;MEM_W(12,p)=phase;MEM_W(16,p)=int32_t(0x802F0000);
    }
}
void dispatch(uint8_t* rdram,unsigned phase,bool reference) {
    recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=phase;
    (reference?reference_dora3_process_dispatch:dora3_process_dispatch)(rdram,&c);
}
int main() {
    unsigned cases=0;
    for(unsigned status=0;status<6;++status)for(unsigned delay:{0u,1u,3u})for(unsigned phase:{0u,1u}) {
        std::vector<uint32_t> original(0x400000/4);setup(reinterpret_cast<uint8_t*>(original.data()),status,delay,phase);
        auto actual=original;native=false;dora3_native_process_time_reset();
        calls=0;dispatch(reinterpret_cast<uint8_t*>(original.data()),phase,true);const auto reference_calls=calls;
        calls=0;dispatch(reinterpret_cast<uint8_t*>(actual.data()),phase,false);assert(actual==original && calls==reference_calls);++cases;
    }
    for(unsigned initial=0;initial<6;++initial)for(unsigned delay:{0u,1u,3u})for(bool simulation:{false,true}) {
        std::vector<uint32_t> original(0x400000/4);const unsigned phase=simulation?0:1;
        setup(reinterpret_cast<uint8_t*>(original.data()),initial,delay,phase);auto actual=original;
        native=!simulation;dora3_native_process_time_reset();
        auto* rdram=reinterpret_cast<uint8_t*>(actual.data());auto* baseline=reinterpret_cast<uint8_t*>(original.data());
        for(unsigned tick=0;tick<8;++tick) {
            MEM_W(0,int32_t(0x801AC6B4))=tick;
            calls=0;dispatch(baseline,phase,true);const auto expected_calls=calls;
            for(unsigned part=0;part<7;++part) {
                calls=0;
                if(simulation) {SimulationScope scope({3000000000ull/7+(part==6?4:0),3000000000ull,tick,part*(3000000000ull/7)});dispatch(rdram,phase,false);}
                else dispatch(rdram,phase,false);
                assert(calls==expected_calls);
                for(unsigned i=0;i<8;++i)for(unsigned off:{0u,4u}) {
                    const auto p=(0x1A44F0+20*i+off)/4;assert(actual[p]==original[p]);
                }
                assert(MEM_W(0,int32_t(0x80171590))==7);++cases;
            }
        }
    }
    // A normal draw followed by native extra draws must not shorten activation.
    std::vector<uint32_t> ram(0x400000/4);auto* rdram=reinterpret_cast<uint8_t*>(ram.data());setup(rdram,4,0,1);
    dora3_native_process_time_reset();native=false;calls=0;dispatch(rdram,1,false);assert(calls==0);
    native=true;dispatch(rdram,1,false);assert(calls==0);MEM_W(0,int32_t(0x801AC6B4))=1;
    dispatch(rdram,1,false);assert(calls==8);++cases;
    std::cout<<"PASS: "<<cases<<" process dispatcher checks; all eight slots, states0..5, waits0/1/3, Original memory equivalence, repeated draw/simulation dispatch, delayed first activation; no game/GPU.\n";
}
