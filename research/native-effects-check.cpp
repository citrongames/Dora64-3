#include "native_effects.hpp"
#include "native_simulation_time.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
using namespace dora3::native_time;
extern "C" {
void dora3_s3_801BE5F0(uint8_t*,recomp_context*);
void reference_dora3_s3_801BE5F0(uint8_t*,recomp_context*);
void dora3_s3_core_801D56F4(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801D56F4(uint8_t*,recomp_context*);
void dora3_s0_80028C74(uint8_t*,recomp_context*);
void reference_dora3_s0_80028C74(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) {std::abort();}
}
static void wf(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
static float rf(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(p))));}
static void near(float a,float b,uint32_t p) {
    if(!std::isfinite(a)||!std::isfinite(b)||std::abs(a-b)>0.003f) {
        std::cerr<<std::hex<<p<<std::dec<<": "<<a<<" != "<<b<<"\n";std::abort();
    }
}
constexpr uint32_t camera=0x801A99F8,object=0x80250000,sequence=0x801A9CB0;
static void seed(uint8_t* rdram,unsigned mode,int duration) {
    MEM_H(0,int32_t(0x8021811A))=1;MEM_H(0,int32_t(0x802171A4))=0;
    MEM_H(0,int32_t(0x80217186))=int16_t(duration);MEM_H(0,int32_t(0x8021719E))=int16_t(mode);
    for(auto p:{0x8021811Cu,0x80217194u,0x802171E4u,0x80217198u})wf(rdram,p,0);
    MEM_H(0,int32_t(0x80218278))=1;MEM_H(0,int32_t(0x80218308))=int16_t(duration);
    wf(rdram,0x80218238,500);wf(rdram,camera+0x30,100);
    MEM_B(0x6B,int32_t(object))=5;MEM_W(0x74,int32_t(object))=0;
    MEM_W(0,int32_t(sequence))=int32_t(0x80251000);
    MEM_B(4,int32_t(sequence))=2;MEM_B(5,int32_t(sequence))=0;
    MEM_B(6,int32_t(sequence))=mode&1?3:1;
    MEM_W(0,int32_t(0x80251000))=int32_t(0x80251100);
    MEM_W(4,int32_t(0x80251000))=int32_t(0x80251200);
    for(int j=0;j<4;++j) {
        MEM_B(j,int32_t(0x80251100))=j==3?255:j;
        MEM_B(j,int32_t(0x80251200))=j+1;
    }
}
static void run(uint8_t* rdram,recomp_context* c,bool reference) {
    c->f_odd=&c->f0.u32h;
    (reference?reference_dora3_s3_801BE5F0:dora3_s3_801BE5F0)(rdram,c);
    c->r4=int32_t(camera);
    (reference?reference_dora3_s3_core_801D56F4:dora3_s3_core_801D56F4)(rdram,c);
    c->r4=int32_t(object);c->r5=0;
    (reference?reference_dora3_s0_80028C74:dora3_s0_80028C74)(rdram,c);
}
static void compare(uint8_t* rdram,uint8_t* baseline) {
    for(auto p:{0x8021811Au,0x802171A4u,0x80218278u,0x80218308u}) {
        auto x=MEM_H(0,int32_t(p));auto* old=rdram;rdram=baseline;auto y=MEM_H(0,int32_t(p));rdram=old;assert(x==y);
    }
    for(auto p:{0x8021811Cu,0x80217194u,0x802171E4u,0x80217198u,0x80218238u,camera+0x30})near(rf(rdram,p),rf(baseline,p),p);
    for(unsigned j=0;j<8;++j)assert(rdram[(sequence&0xFFFFFF)+j]==baseline[(sequence&0xFFFFFF)+j]);
}
int main(int argc,char** argv) {
    assert(argc==2);std::vector<uint32_t> original(0x400000/4),reference,actual;
    std::ifstream in(argv[1],std::ios::binary);in.read(reinterpret_cast<char*>(original.data()),0x400000);assert(in.gcount()==0x400000);
    unsigned cases=0;
    for(unsigned mode=0;mode<4;++mode)for(int duration:{0,1,2,7,40}) {
        seed(reinterpret_cast<uint8_t*>(original.data()),mode,duration);reference=original;actual=original;
        recomp_context c{};c.r29=int32_t(0x80300000);auto expected=c;
        run(reinterpret_cast<uint8_t*>(reference.data()),&expected,true);
        run(reinterpret_cast<uint8_t*>(actual.data()),&c,false);assert(actual==reference);++cases;
        actual=original;c={};c.r29=int32_t(0x80300000);
        {SimulationScope scope({3*std::uint64_t(second),3*std::uint64_t(second),0,0});run(reinterpret_cast<uint8_t*>(actual.data()),&c,false);}
        compare(reinterpret_cast<uint8_t*>(actual.data()),reinterpret_cast<uint8_t*>(reference.data()));++cases;
        for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,144u,360u}) {
            reference=original;expected={};expected.r29=int32_t(0x80300000);
            actual=original;c={};c.r29=int32_t(0x80300000);SimulationTimeline timeline(divider);Nanoseconds previous=0;
            unsigned intermediate=0;
            for(unsigned frame=1;frame<=fps;++frame) {
                const auto now=second*frame/fps;
                timeline.advance(now-previous,[&](const SimulationSlice& step) {
                    auto* ram=reinterpret_cast<uint8_t*>(actual.data());
                    const float before=rf(ram,camera+0x30);
                    {SimulationScope scope(step);run(ram,&c,false);}
                    if(!step.ends_tick() && before!=rf(ram,camera+0x30))++intermediate;
                },[&](uint64_t) {
                    run(reinterpret_cast<uint8_t*>(reference.data()),&expected,true);
                    compare(reinterpret_cast<uint8_t*>(actual.data()),reinterpret_cast<uint8_t*>(reference.data()));
                });previous=now;
            }
            assert(intermediate>0);++cases;
        }
    }
    std::cout<<"PASS: "<<cases<<" rectangle/zoom/sprite cases; all four rectangle modes, loop/clamp sprite events, durations0/1/2/7/40; original whole-step equivalence and per-tick state/event comparison at60/73/144/360FPS and20/30Hz; real fractional zoom changes; no game/GPU.\n";
}
