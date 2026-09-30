#include "native_simulation_time.hpp"
#include "recomp.h"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace dora3::native_time;
extern "C" {
void dora3_s3_core_801CB6D4(uint8_t*,recomp_context*);
void dora3_s3_core_801CB400(uint8_t*,recomp_context*);
void dora3_s3_core_801C1664(uint8_t*,recomp_context*);
void dora3_s3_801C0F1C(uint8_t*,recomp_context*);
void dora3_s28_mini_801C4300(uint8_t*,recomp_context*);
void dora3_s28_mini_801C42C8(uint8_t*,recomp_context*);
void dora3_s28_mini_801C7994(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801CB6D4(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801CB400(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801C1664(uint8_t*,recomp_context*);
void reference_dora3_s3_801C0F1C(uint8_t*,recomp_context*);
void reference_dora3_s28_mini_801C4300(uint8_t*,recomp_context*);
void reference_dora3_s28_mini_801C42C8(uint8_t*,recomp_context*);
void reference_dora3_s28_mini_801C7994(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) {std::abort();}
}
static float readf(uint8_t* rdram,uint32_t address) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(address))));}
static void writef(uint8_t* rdram,uint32_t address,float value) {MEM_W(0,int32_t(address))=int32_t(std::bit_cast<uint32_t>(value));}
static void near(double a,double b,double tolerance=1e-8) {if(std::abs(a-b)>tolerance) {std::cerr<<a<<" != "<<b<<"\n";std::abort();}}
struct Pair {recomp_func_t* patched;recomp_func_t* reference;};
int main() {
    constexpr uint32_t object=0x80280000,physics=0x80281000,control=0x80282000,params=0x80283000;
    const std::array<Pair,7> pairs{{
        {dora3_s3_core_801CB6D4,reference_dora3_s3_core_801CB6D4},
        {dora3_s3_core_801CB400,reference_dora3_s3_core_801CB400},
        {dora3_s3_core_801C1664,reference_dora3_s3_core_801C1664},
        {dora3_s3_801C0F1C,reference_dora3_s3_801C0F1C},
        {dora3_s28_mini_801C4300,reference_dora3_s28_mini_801C4300},
        {dora3_s28_mini_801C42C8,reference_dora3_s28_mini_801C42C8},
        {dora3_s28_mini_801C7994,reference_dora3_s28_mini_801C7994}}};
    std::vector<uint32_t> memory(0x400000/4),reference(memory.size());
    auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    recomp_context c{}; c.r29=int32_t(0x80300000);
    MEM_W(0,int32_t(0x8021D49C))=int32_t(object);MEM_W(0,int32_t(0x8021D4B0))=int32_t(physics);
    MEM_W(0,int32_t(0x8021D6C0))=int32_t(control);MEM_W(0,int32_t(0x8021D648))=int32_t(params);
    for(int i=0;i<6;++i)MEM_H(i*2,int32_t(params))=int16_t(40+i*10);
    writef(rdram,params+0x14,4);writef(rdram,control+0x24,1.25f);
    // Execute actual unpatched and patched MIPS in Original and whole-tick scope.
    unsigned differential=0;
    for(const auto& pair:pairs)for(int state:{0,1,0x3C,0x3D})for(int mode:{0,1,2,3,4})for(bool whole:{false,true}) {
        MEM_W(0x94,int32_t(object))=state;MEM_B(3,int32_t(control))=int8_t(state&1);
        writef(rdram,physics,35.0f);writef(rdram,physics+0x14,-54.5f);writef(rdram,physics+0x1C,3.0f);
        c.r4=int32_t(object);c.r5=int32_t(physics);
        if(pair.patched==dora3_s3_core_801CB400 || pair.patched==dora3_s28_mini_801C42C8)c.r4=int32_t(physics);
        if(pair.patched==dora3_s3_core_801C1664)c.r4=mode;
        reference=memory;auto expected=c;
        pair.reference(reinterpret_cast<uint8_t*>(reference.data()),&expected);
        if(whole) {SimulationScope scope({3*std::uint64_t(second),3*std::uint64_t(second),0,0});pair.patched(rdram,&c);}
        else pair.patched(rdram,&c);
        assert(memory==reference);++differential;
    }
    unsigned slices=0, schedules=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        SimulationTimeline clock(divider);
        double x=0,v=31,filtered=10,ramped=-5,seconds_seen=0;
        unsigned events=0;Nanoseconds previous=0;
        writef(rdram,object+0x9C,0);writef(rdram,object+0xA0,0);writef(rdram,object+0xA4,0);
        writef(rdram,physics+0x10,2.5f);writef(rdram,physics+0x14,-1.25f);writef(rdram,physics+0x18,0.125f);
        for(unsigned frame=1;frame<=fps;++frame) {
            const auto now=second*frame/fps;
            clock.advance(now-previous,[&](const SimulationSlice& step) {
                SimulationScope scope(step);assert(active_simulation_slice());
                const double h=step.ticks();
                const auto k=legacy_kinematic_step(v,-0.25,h);v=k.velocity;x+=k.displacement;
                filtered=decay_value(filtered,0,0.8,h);ramped=approach_value(ramped,9,0.5,h);
                seconds_seen+=step.seconds();
                c.r4=int32_t(object);c.r5=int32_t(physics);
                (divider==3?dora3_s3_core_801CB6D4:dora3_s28_mini_801C4300)(rdram,&c);
                ++slices;
            },[&](uint64_t tick) {assert(!active_simulation_slice());assert(tick==++events);});
            assert(!active_simulation_slice());previous=now;
        }
        const double n=60/divider;
        near(v,31-0.25*n);near(x,31*n-0.25*n*(n+1)/2);near(filtered,10*std::pow(0.8,n));
        near(ramped,std::min(9.0,-5+0.5*n));near(seconds_seen,1);
        near(readf(rdram,object+0x9C),2.5*n,0.001);near(readf(rdram,object+0xA0),-1.25*n,0.001);
        near(readf(rdram,object+0xA4),0.125*n,0.0001);
        assert(clock.ticks()==unsigned(n) && clock.phase()==0 && events==unsigned(n));++schedules;
    }
    // Rate changes and long gaps preserve every event; no elapsed cap or frame-count clock.
    SimulationTimeline changed(3);unsigned events=0;double elapsed=0;
    for(Nanoseconds gap:std::array<Nanoseconds,5>{1,16'000'000,33'999'999,2*second,7*second})
        changed.advance(gap,[&](const SimulationSlice& step) {elapsed+=step.seconds();},[&](uint64_t n){assert(n==++events);});
    assert(events==181 && changed.phase()==0);near(elapsed,9.05);
    changed.advance(0,[](const SimulationSlice&){std::abort();},[](uint64_t){std::abort();});
    bool rejected=false;try {changed.advance(-1,[](const SimulationSlice&){},[](uint64_t){});}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    try {SimulationScope scope({1,3*std::uint64_t(second),0,0});throw 1;}catch(int) {}assert(!active_simulation_slice());
    near(dora3_native_rate_f32(3.25f),3.25);
    std::cout<<"PASS: "<<differential<<" differential executions of seven original MIPS motion/camera helpers; Original and whole ticks unchanged; "
      <<schedules<<" 20/30 Hz schedules at 60/73/120/144/165/360 FPS, "<<slices
      <<" real motion slices, exact discrete boundaries, constant-force kick/drift trajectory, approach/decay, long gaps and scope cleanup; no game/GPU.\n";
}
