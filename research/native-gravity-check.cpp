#include "native_simulation_time.hpp"
#include "recomp.h"
#include <bit>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
using namespace dora3::native_time;
extern "C" {
void dora3_s3_801C0F1C(uint8_t*,recomp_context*);
void dora3_s3_core_801CB400(uint8_t*,recomp_context*);
void dora3_s3_core_801CB6D4(uint8_t*,recomp_context*);
void dora3_s28_mini_801C42C8(uint8_t*,recomp_context*);
void dora3_s28_mini_801C4300(uint8_t*,recomp_context*);
void reference_dora3_s3_801C0F1C(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801CB400(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801CB6D4(uint8_t*,recomp_context*);
void reference_dora3_s28_mini_801C42C8(uint8_t*,recomp_context*);
void reference_dora3_s28_mini_801C4300(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) { std::abort(); }
}
constexpr uint32_t object=0x80250000,physics=0x80251000,control=0x80252000,params=0x80253000;
void wf(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
float rf(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(p))));}
void near(float a,float b) { if(!std::isfinite(a)||!std::isfinite(b)||std::abs(a-b)>0.035f){std::cerr<<a<<" != "<<b<<'\n';std::abort();} }
void setup(uint8_t* rdram,float v,float g,int state=1,int airborne=1) {
    MEM_W(0,int32_t(0x8021D49C))=int32_t(object);MEM_W(0,int32_t(0x8021D4B0))=int32_t(physics);
    MEM_W(0,int32_t(0x8021D6C0))=int32_t(control);MEM_W(0,int32_t(0x8021D648))=int32_t(params);
    MEM_W(0x94,int32_t(object))=state;MEM_B(3,int32_t(control))=airborne;
    wf(rdram,physics+0x14,v);wf(rdram,physics+0x1C,g);wf(rdram,params+0x14,g);
    wf(rdram,physics+0x10,3);wf(rdram,physics+0x18,-4);
}
void run(uint8_t* rdram,int family,bool reference) {
    recomp_context c{};c.f_odd=&c.f0.u32h;c.r4=int32_t(physics);
    if(family==0)(reference?reference_dora3_s3_801C0F1C:dora3_s3_801C0F1C)(rdram,&c);
    if(family==1)(reference?reference_dora3_s3_core_801CB400:dora3_s3_core_801CB400)(rdram,&c);
    if(family==2)(reference?reference_dora3_s28_mini_801C42C8:dora3_s28_mini_801C42C8)(rdram,&c);
    c.r4=int32_t(object);c.r5=int32_t(physics);
    (family==2?(reference?reference_dora3_s28_mini_801C4300:dora3_s28_mini_801C4300):(reference?reference_dora3_s3_core_801CB6D4:dora3_s3_core_801CB6D4))(rdram,&c);
}
int main() {
    unsigned cases=0;
    for(int family=0;family<3;++family)for(float v:{-60.f,-55.f,-49.f,0.f,50.f})for(float g:{0.f,1.5f,3.f,-2.f}) {
        std::vector<uint32_t> initial(0x400000/4);setup(reinterpret_cast<uint8_t*>(initial.data()),v,g);
        auto original=initial,actual=initial;
        run(reinterpret_cast<uint8_t*>(original.data()),family,true);run(reinterpret_cast<uint8_t*>(actual.data()),family,false);
        assert(original==actual);++cases;
        for(unsigned divider:{2u,3u}) {
            auto full=initial;
            for(unsigned tick=0;tick<20;++tick) {
                SimulationScope scope({divider*uint64_t(second),divider*uint64_t(second),tick,0});run(reinterpret_cast<uint8_t*>(full.data()),family,false);
            }
            for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
                actual=initial;SimulationTimeline time(divider);Nanoseconds previous=0;
                const auto duration=Nanoseconds(divider)*second/3; // exactly 20 ticks
                for(unsigned frame=1;previous<duration;++frame) {
                    auto now=std::min(second*frame/fps,duration);
                    time.advance(now-previous,[&](const SimulationSlice& step){SimulationScope scope(step);run(reinterpret_cast<uint8_t*>(actual.data()),family,false);},[](uint64_t){});
                    previous=now;
                }
                auto* ram=reinterpret_cast<uint8_t*>(actual.data());auto* expected=reinterpret_cast<uint8_t*>(full.data());
                near(rf(ram,physics+0x14),rf(expected,physics+0x14));
                for(auto offset:{0x9Cu,0xA0u,0xA4u})near(rf(ram,object+offset),rf(expected,object+offset));
                ++cases;
            }
        }
    }
    // The player's forced ascent and grounded branches must not inherit gravity.
    for(int state:{1,0x3C,0x3D})for(int airborne:{0,1}) {
        std::vector<uint32_t> ram(0x400000/4);auto* rdram=reinterpret_cast<uint8_t*>(ram.data());setup(rdram,10,9,state,airborne);
        {SimulationScope scope({uint64_t(second),3*uint64_t(second),0,0});run(rdram,0,false);}
        near(rf(rdram,physics+0x14),state==0x3C?50.f:!airborne?0.f:state==0x3D?9.5f:7.f);++cases;
    }
    // Multiple actors, repeated displacement reads, contact override and scope expiry.
    {
        SimulationScope scope({uint64_t(second),3*uint64_t(second),0,0});
        auto a=dora3_native_gravity_f32(20,3,-55,100),b=dora3_native_gravity_f32(-49,3,-50,200);
        near(dora3_native_gravity_displacement_f32(a,100),6);
        near(dora3_native_gravity_displacement_f32(a,100),6);
        near(dora3_native_gravity_displacement_f32(b,200),-50.f/3);
        near(dora3_native_gravity_displacement_f32(0,100),0);
        near(dora3_native_gravity_displacement_f32(a,999),a/3);
    }
    {SimulationScope scope({uint64_t(second),3*uint64_t(second),0,uint64_t(second)});near(dora3_native_gravity_displacement_f32(19,100),19.f/3);}
    near(dora3_native_gravity_displacement_f32(19,100),19);
    std::cout<<"PASS: "<<cases<<" original/gravity trajectory cases; 60/73/120/144/165/360FPS, 20/30Hz, both terminal caps, player special branches, contacts and scoped actor isolation; no game/GPU.\n";
}
