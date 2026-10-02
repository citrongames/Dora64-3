#include "native_simulation_time.hpp"
#include "recomp.h"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
using namespace dora3::native_time;
extern "C" {
void dora3_s3_801D55EC(uint8_t*,recomp_context*);
void reference_dora3_s3_801D55EC(uint8_t*,recomp_context*);
void dora3_s3_801DB7C8(uint8_t*,recomp_context*);
void dora3_s3_801D6EAC(uint8_t*,recomp_context*);
void reference_dora3_s3_801DB7C8(uint8_t*,recomp_context*);
void reference_dora3_s3_801D6EAC(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) {std::abort();}
}
static unsigned debug_family=0,debug_input=0,debug_fps=0,debug_tick=0,debug_offset=0;
constexpr uint32_t camera=0x801A99F8;
void wf(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
float rf(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(p))));}
void near(double a,double b,double tolerance=0.006) {
    if(!std::isfinite(a)||!std::isfinite(b)||std::abs(a-b)>tolerance){std::cerr<<debug_family<<" family "<<debug_input<<" input "<<debug_fps<<" fps "<<debug_tick<<" tick "<<debug_offset<<" offset: "<<a<<" != "<<b<<'\n';std::abort();}
}
void seed(uint8_t* rdram,unsigned family,unsigned input) {
    for(unsigned i=0;i<0x34;i+=4)wf(rdram,camera+i,0);
    wf(rdram,camera+0x18,350);wf(rdram,camera+0x1C,355);wf(rdram,camera+0x30,300);
    MEM_H(0,int32_t(0x801A9504))=input;
    MEM_H(0,int32_t(0x80218318))=family==2?1:0;MEM_H(0,int32_t(0x80218220))=0;
    for(auto p:{0x80218244u,0x80218258u,0x802182BCu,0x802182F0u,0x80218264u,0x802182ACu})wf(rdram,p,1);
    wf(rdram,0x801AC6AC,input&1?30.f:0.f);wf(rdram,0x801A39B0,input&2?20.f:0.f);
}
void run(uint8_t* rdram,unsigned family,bool reference) {
    recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=int32_t(camera);
    (family==0?(reference?reference_dora3_s3_801DB7C8:dora3_s3_801DB7C8):(reference?reference_dora3_s3_801D6EAC:dora3_s3_801D6EAC))(rdram,&c);
}
void compare(uint8_t* actual,uint8_t* expected) {
    for(unsigned offset=0;offset<0x34;offset+=4) {
        debug_offset=offset;auto a=rf(actual,camera+offset),b=rf(expected,camera+offset);
        if(offset==0x18||offset==0x1C)near(std::remainder(a-b,360),0,0.11);else near(a,b);
    }
    for(auto address:{0x80218318u,0x80218220u}) {
        auto* rdram=actual;auto a=MEM_H(0,int32_t(address));rdram=expected;assert(a==MEM_H(0,int32_t(address)));
    }
}
int main(int argc,char** argv) {
    assert(argc==2);std::vector<uint32_t> image(0x400000/4);
    std::ifstream in(argv[1],std::ios::binary);in.read(reinterpret_cast<char*>(image.data()),0x400000);assert(in.gcount()==0x400000);
    unsigned cases=0;
    for(unsigned family=0;family<3;++family)for(unsigned input:{0u,1u,2u,3u,4u,8u,12u,0x400u,0x800u,0xC00u}) {
        debug_family=family;debug_input=input;auto initial=image;seed(reinterpret_cast<uint8_t*>(initial.data()),family,input);
        auto original=initial,actual=initial;run(reinterpret_cast<uint8_t*>(original.data()),family,true);run(reinterpret_cast<uint8_t*>(actual.data()),family,false);assert(actual==original);++cases;
        actual=initial;{SimulationScope scope({3000000000ull,3000000000ull,0,0});run(reinterpret_cast<uint8_t*>(actual.data()),family,false);}assert(actual==original);++cases;
        for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
            debug_fps=fps;debug_tick=0;actual=initial;original=initial;SimulationTimeline time(divider);Nanoseconds previous=0;
            for(unsigned frame=1;frame<=fps;++frame) {
                const auto now=second*frame/fps;
                time.advance(now-previous,[&](const SimulationSlice& slice){SimulationScope scope(slice);run(reinterpret_cast<uint8_t*>(actual.data()),family,false);},[&](uint64_t tick){debug_tick=tick;run(reinterpret_cast<uint8_t*>(original.data()),family,true);compare(reinterpret_cast<uint8_t*>(actual.data()),reinterpret_cast<uint8_t*>(original.data()));});
                previous=now;
            }
            ++cases;
        }
    }
    for(double alpha:{0.,0.01,0.04,0.05,0.1,0.2,0.3,0.5,0.8,1.}) {
        double remaining=1,retention=1;
        for(unsigned i=0;i<7;++i) {
            SimulationScope scope({3000000000ull/7+(i==6?4:0),3000000000ull,0,i*(3000000000ull/7)});
            remaining*=1-dora3_native_approach_coefficient_f64(alpha);retention*=dora3_native_retention_f64(alpha);
        }
        near(remaining,1-alpha,1e-12);near(retention,alpha,1e-12);++cases;
    }


    debug_family=4;
    for(unsigned duration:{0u,1u,2u,7u,40u})for(unsigned constant:{0u,1u})for(unsigned divider:{2u,3u}) {
        auto initial=image;auto* rdram=reinterpret_cast<uint8_t*>(initial.data());
        wf(rdram,camera+4,100);MEM_H(0,int32_t(0x80218324))=1;MEM_H(0,int32_t(0x802182C4))=duration;
        MEM_H(0,int32_t(0x802181F0))=constant;wf(rdram,0x80218304,20);wf(rdram,0x80218254,0.25);
        auto call=[](uint8_t* ram,bool original){recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=int32_t(camera);(original?reference_dora3_s3_801D55EC:dora3_s3_801D55EC)(ram,&c);};
        auto actual=initial,original=initial;call(reinterpret_cast<uint8_t*>(original.data()),true);call(reinterpret_cast<uint8_t*>(actual.data()),false);assert(actual==original);++cases;
        for(unsigned fps:{60u,73u,144u,360u}) {
            actual=initial;original=initial;SimulationTimeline time(divider);Nanoseconds previous=0;
            for(unsigned frame=1;frame<=fps*3;++frame) {
                const auto now=second*frame/fps;
                time.advance(now-previous,[&](const SimulationSlice& slice){SimulationScope scope(slice);call(reinterpret_cast<uint8_t*>(actual.data()),false);},[&](uint64_t){
                    call(reinterpret_cast<uint8_t*>(original.data()),true);
                    near(rf(reinterpret_cast<uint8_t*>(actual.data()),camera+4),rf(reinterpret_cast<uint8_t*>(original.data()),camera+4),0.002);
                    for(auto address:{0x80218324u,0x802182C4u,0x80218304u}){
                        auto* rdram=reinterpret_cast<uint8_t*>(actual.data());auto a=MEM_W(0,int32_t(address));rdram=reinterpret_cast<uint8_t*>(original.data());assert(a==MEM_W(0,int32_t(address)));
                    }
                });previous=now;
            }++cases;
        }
    }
    debug_family=3;
    for(double alpha:{0.01,0.04,0.1,0.5,0.9})for(double limit:{3.,5.})
    for(double factor:{-100.,-1.01,-1.,-0.99,-0.1,0.,0.1,0.99,1.,1.01,100.}) {
        const double start=factor*limit/alpha;
        for(unsigned parts:{1u,3u,7u,18u}) {
            double actual=start,expected=start;
            for(unsigned tick=0;tick<20;++tick) {
                for(unsigned part=0;part<parts;++part) {
                    const uint64_t begin=3000000000ull*part/parts,end=3000000000ull*(part+1)/parts;
                    SimulationScope scope({end-begin,3000000000ull,tick,begin});
                    const double delta=dora3_native_capped_approach_f64(actual,alpha,limit);
                    // Original/full-step helper deliberately leaves the downstream original clamp intact.
                    actual-=std::clamp(delta,-limit,limit);
                }
                expected-=std::clamp(expected*alpha,-limit,limit);
                near(actual,expected,1e-8);
            }
            ++cases;
        }
    }
    std::cout<<"PASS: "<<cases<<" camera control/transition/coefficient cases; actual original angle helper, Original and full-step RAM equivalence; 60/73/120/144/165/360FPS, 20/30Hz; minimum distance20 unchanged; pitch dead-zone tolerance0.11 degrees; no game/GPU.\n";
}

extern "C" int dora3_native_preview_hook(uint8_t*,recomp_context*,uint32_t) {return 0;}
