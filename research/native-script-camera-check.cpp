#include "native_simulation_time.hpp"
#include "native_script_camera.hpp"
#include "recomp.h"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
using namespace dora3::native_time;
extern "C" {
void dora3_s3_801E0760(uint8_t*,recomp_context*);
void reference_dora3_s3_801E0760(uint8_t*,recomp_context*);
void dora3_s0_8002F064(uint8_t*,recomp_context*);
void reference_dora3_s0_8002F064(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) {std::abort();}
}
constexpr uint32_t control=0x802183D0,camera=0x801A99F8,actor=0x80218B10;
static void wf(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=int32_t(std::bit_cast<uint32_t>(v));}
static float rf(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(p))));}
static void near(float a,float b,float tolerance,uint32_t p) {
    if(!std::isfinite(a)||!std::isfinite(b)||std::abs(a-b)>tolerance) {
        std::cerr<<std::hex<<p<<std::dec<<": "<<a<<" != "<<b<<"\n";std::abort();
    }
}
static void seed(uint8_t* rdram,unsigned mode,unsigned duration,unsigned channel) {
    for(unsigned i=0;i<4;++i) {
        auto s=control+i*0x1B8;
        for(unsigned j=0;j<0x1B8;j+=4)MEM_W(int32_t(j),int32_t(s))=0;
        for(unsigned j=0;j<0x34;j+=4)MEM_W(int32_t(j),int32_t(camera+i*0x34))=0;
        MEM_H(0x34,int32_t(s))=1;MEM_H(0x36,int32_t(s))=int16_t(mode);
        MEM_H(0x10A,int32_t(s))=int16_t(i);MEM_H(4,int32_t(actor+i*0xD4))=1;
        for(unsigned axis=0;axis<3;++axis) {
            wf(rdram,actor+i*0xD4+0x18+axis*4,float(50+axis*20+i*10));
            wf(rdram,s+0x38+axis*4,float(100+axis*40+i*10));
            wf(rdram,s+0x50+axis*4,float(20+axis*5+i));
            wf(rdram,s+0x44+axis*4,float(20+axis*10));
            wf(rdram,s+0x74+axis*4,float(axis+1));wf(rdram,s+0xB0+axis*4,0.125f);
            wf(rdram,s+0x80+axis*4,0.25f);wf(rdram,s+0xBC+axis*4,0.0625f);
            wf(rdram,s+0x8C+axis*4,float(axis+1));wf(rdram,s+0xC8+axis*4,0.125f);
            wf(rdram,s+0x68+axis*4,float(5+axis));wf(rdram,s+0xA4+axis*4,0.125f);
        }
        if(channel) {
            // Isolate each orbit plane and each fixed-direction translation.
            for(unsigned off=0x74;off<0xEC;off+=4)wf(rdram,s+off,0);
            if(channel==13) {
                for(unsigned j=0;j<6;++j) {
                    MEM_H(int32_t(0x11C+2*j),int32_t(s))=int16_t(duration);
                    MEM_H(int32_t(0x110+2*j),int32_t(s))=1;
                    wf(rdram,s+0x128+4*j,0.25f);wf(rdram,s+0x140+4*j,0.0625f);
                }
            } else if(channel<=6) {
                const auto j=channel-1;
                MEM_H(int32_t(0x11C+2*j),int32_t(s))=int16_t(duration);
                MEM_H(int32_t(0x110+2*j),int32_t(s))=1;
                wf(rdram,s+0x128+4*j,0.25f);wf(rdram,s+0x140+4*j,0.0625f);
            } else {
                const auto j=channel-7;
                MEM_H(int32_t(0x164+2*j),int32_t(s))=int16_t(duration);
                MEM_H(int32_t(0x158+2*j),int32_t(s))=1;
                wf(rdram,s+0x170+4*j,0.25f);wf(rdram,s+0x188+4*j,0.0625f);
                wf(rdram,s+0x1A0+4*j,30.0f);
            }
        }
        wf(rdram,s+0x5C,300);wf(rdram,s+0x98,channel?0:-0.5f);wf(rdram,s+0xD4,channel?0:0.0625f);
        for(unsigned j=0;j<15;++j)MEM_H(int32_t(0xEC+2*j),int32_t(s))=int16_t(duration);
    }
}
static void run(uint8_t* rdram,recomp_context* c,bool reference) {
    // Runtime uses FR=0: odd single registers alias high halves of even pairs.
    // Rebind after every context copy/reset; never retain a pointer into another context.
    c->f_odd=&c->f0.u32h;
    (reference?reference_dora3_s3_801E0760:dora3_s3_801E0760)(rdram,c);
    for(unsigned i=0;i<4;++i) {
        c->r4=int32_t(camera+i*0x34);
        (reference?reference_dora3_s0_8002F064:dora3_s0_8002F064)(rdram,c);
    }
}
static void run_native(uint8_t* rdram,recomp_context* c) { run(rdram,c,false); }
int main(int argc,char** argv) {
    assert(argc==2);std::vector<uint32_t> original(0x400000/4),reference,actual;
    std::ifstream in(argv[1],std::ios::binary);in.read(reinterpret_cast<char*>(original.data()),0x400000);assert(in.gcount()==0x400000);
    unsigned cases=0;
    for(unsigned channel=0;channel<=13;++channel)for(unsigned mode=0;mode<4;++mode)for(unsigned duration:{0u,1u,2u,7u,40u}) {
        seed(reinterpret_cast<uint8_t*>(original.data()),mode,duration,channel);
reference=original;actual=original;
        recomp_context c{};c.r29=int32_t(0x80300000);auto expected=c;
        run(reinterpret_cast<uint8_t*>(reference.data()),&expected,true);
        run(reinterpret_cast<uint8_t*>(actual.data()),&c,false);
        assert(actual==reference);++cases;
        // Equal one-tick scope must preserve all guest RAM, including saved stack words.
        actual=original;c={};c.r29=int32_t(0x80300000);
        {SimulationScope scope({3*std::uint64_t(second),3*std::uint64_t(second),0,0});run(reinterpret_cast<uint8_t*>(actual.data()),&c,false);}
        assert(actual==reference);++cases;
        for(unsigned divider:{2u,3u}) {
            reference=original;expected={};expected.r29=int32_t(0x80300000);
            for(unsigned tick=0;tick<60/divider;++tick)run(reinterpret_cast<uint8_t*>(reference.data()),&expected,true);
            for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
                actual=original;c={};c.r29=int32_t(0x80300000);SimulationTimeline timeline(divider);ScriptCameraTimeline cameras;Nanoseconds previous=0;
                for(unsigned frame=1;frame<=fps;++frame) {
                    const auto now=second*frame/fps;
                    timeline.advance(now-previous,[&](const SimulationSlice& step) {
                        cameras.advance(reinterpret_cast<uint8_t*>(actual.data()),&c,step,run_native);
                    },[](uint64_t){});previous=now;
                }
                auto* ram=reinterpret_cast<uint8_t*>(actual.data());auto* baseline=reinterpret_cast<uint8_t*>(reference.data());
                for(unsigned i=0;i<4;++i) {
                    // All actual model-space camera outputs: target, eye, angles, distance.
                    for(unsigned off=0;off<0x34;off+=4)near(rf(ram,camera+i*0x34+off),rf(baseline,camera+i*0x34+off),0.025f,camera+i*0x34+off);
                    for(unsigned off=0x38;off<0xEC;off+=4)near(rf(ram,control+i*0x1B8+off),rf(baseline,control+i*0x1B8+off),off<0x74?0.025f:0.002f,control+i*0x1B8+off);
                    for(unsigned off=0xEC;off<0x10A;off+=2) {
                        const auto address=control+i*0x1B8+off;
                        const auto word=address&~3u;assert(actual[(word&0xFFFFFF)/4]==reference[(word&0xFFFFFF)/4]);
                    }
                }
                ++cases;
            }
        }
    }
    std::cout<<"PASS: "<<cases<<" script-camera cases; four cameras, all four modes, six orbit channels, local channels and combined rotations; Original/whole-step RAM differential, durations 0/1/2/7/40, 20/30 Hz timing at 60/73/120/144/165/360 FPS, original trigonometry and eye geometry; no game/GPU.\n";
}
