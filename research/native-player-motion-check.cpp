#include "native_simulation_time.hpp"
#include "native_camera_timeline.hpp"
#include "native_gameplay_preview.hpp"
#include <cstring>
#include "recomp.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
using namespace dora3::native_time;
extern "C" {
void dora3_s3_801DBD58(uint8_t*,recomp_context*);
void reference_dora3_s3_801DBD58(uint8_t*,recomp_context*);
void dora3_s3_801DBAA0(uint8_t*,recomp_context*) {std::cerr<<"Uncovered camera button toggle\n";std::abort();}
void dora3_s3_core_801C1664(uint8_t*,recomp_context*);
void dora3_s3_801C0D88(uint8_t*,recomp_context*);
void dora3_s3_801C0E3C(uint8_t*,recomp_context*);
void dora3_s3_801C0EC0(uint8_t*,recomp_context*);
void dora3_s3_801C0F1C(uint8_t*,recomp_context*);
void dora3_s3_801BF828(uint8_t*,recomp_context*);
void dora3_s3_801BFE60(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801C1664(uint8_t*,recomp_context*);
void reference_dora3_s3_801C0D88(uint8_t*,recomp_context*);
void reference_dora3_s3_801C0E3C(uint8_t*,recomp_context*);
void reference_dora3_s3_801C0EC0(uint8_t*,recomp_context*);
void reference_dora3_s3_801C0F1C(uint8_t*,recomp_context*);
void reference_dora3_s3_801BF828(uint8_t*,recomp_context*);
void reference_dora3_s3_801BFE60(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) {std::abort();}
void switch_error(const char*,uint32_t,uint32_t) {std::abort();}
void dora3_s3_core_801C07DC(uint8_t*,recomp_context*) {std::cerr<<"Uncovered special-mode transition\n";std::abort();}
}
extern "C" int dora3_native_preview_hook(uint8_t*,recomp_context*,uint32_t){return 0;}
extern "C" recomp_func_t* get_function(int32_t) {std::abort();}
constexpr uint32_t player=0x80250000,physics=0x80251000,control=0x80252000,params=0x80253000;
void wf(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
float rf(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(p))));}
void room(uint8_t* rdram) {
    constexpr uint32_t map=0x80280000;
    MEM_W(0,int32_t(0x800DAE64))=int32_t(map);
    MEM_W(0,int32_t(map))=-2000;MEM_W(4,int32_t(map))=-2000;MEM_W(8,int32_t(map))=2000;MEM_W(12,int32_t(map))=2000;
    MEM_W(16,int32_t(map))=0x20;MEM_W(20,int32_t(map))=0x200;MEM_W(24,int32_t(map))=0x400;
    const std::array<std::array<float,6>,2> triangles{{{-2000,-2000,-2000,2000,2000,2000},{-2000,-2000,2000,2000,2000,-2000}}};
    for(unsigned n=0;n<4;++n) {
        const auto base=map+0x20+n*44;auto vertices=triangles[n%2];
        if(n>=2){std::swap(vertices[2],vertices[4]);std::swap(vertices[3],vertices[5]);}
        for(unsigned k=0;k<6;++k)wf(rdram,base+4*k,vertices[k]);
        wf(rdram,base+24,0);wf(rdram,base+28,n<2?1:-1);wf(rdram,base+32,0);wf(rdram,base+36,n<2?0:-250);MEM_W(40,int32_t(base))=0;
        MEM_H(int32_t(2*n),int32_t(map+0x200))=n;
    }
    MEM_H(8,int32_t(map+0x200))=-1;
    for(unsigned n=0;n<80*80;++n)MEM_H(int32_t(2*n),int32_t(map+0x400))=0;
}

void seed(uint8_t* rdram,unsigned mode,bool drive) {
    for(auto pair:std::array<std::array<uint32_t,2>,4>{{{0x8021D49C,player},{0x8021D4B0,physics},{0x8021D6C0,control},{0x8021D648,params}}})MEM_W(0,int32_t(pair[0]))=int32_t(pair[1]);
    MEM_W(0x98,int32_t(player))=int32_t(physics);MEM_W(0x94,int32_t(player))=9;
    MEM_B(3,int32_t(control))=mode==0;MEM_B(0x28,int32_t(control))=mode==1;MEM_B(0x2C,int32_t(control))=mode==2;
    MEM_B(0,int32_t(0x8021D4CB))=100;wf(rdram,0x8021D6A0,-10000);wf(rdram,0x8021D4AC,10000);
    wf(rdram,player+0x9C,20);wf(rdram,player+0xA0,150);wf(rdram,player+0xA4,10);
    wf(rdram,physics,drive?0:12);wf(rdram,physics+4,30);wf(rdram,physics+8,6);wf(rdram,physics+0xC,10);
    wf(rdram,physics+0x14,10);wf(rdram,params+0x14,1.5f);
    for(unsigned n=0;n<5;++n)MEM_H(int32_t(n*2),int32_t(params))=int16_t(n*6);
    MEM_B(0,int32_t(0x801B361A))=12;MEM_H(0,int32_t(0x801AC69C))=0;
    constexpr uint32_t camera=0x801A99F8;
    for(unsigned off=0;off<0x34;off+=4)wf(rdram,camera+off,0);
    wf(rdram,camera+4,100);wf(rdram,camera+0x18,15);wf(rdram,camera+0x1C,150);wf(rdram,camera+0x30,500);
    for(unsigned off=0x24;off<=0x2E;off+=2)MEM_H(int32_t(off),int32_t(params))=off==0x2E?500:10;
    room(rdram);
}
void run(uint8_t* rdram,unsigned mode,bool drive,bool reference) {
    recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=2;
    if(drive)(reference?reference_dora3_s3_core_801C1664:dora3_s3_core_801C1664)(rdram,&c);
    if(mode==1)(reference?reference_dora3_s3_801C0D88:dora3_s3_801C0D88)(rdram,&c);
    else {
        (reference?reference_dora3_s3_801C0E3C:dora3_s3_801C0E3C)(rdram,&c);
        if(mode==2)(reference?reference_dora3_s3_801C0EC0:dora3_s3_801C0EC0)(rdram,&c);
        else (reference?reference_dora3_s3_801C0F1C:dora3_s3_801C0F1C)(rdram,&c);
    }
    c.r4=int32_t(mode==1?0x801FA3AC:0x801FA28C);c.r5=0;
    c.r6=std::bit_cast<int32_t>(mode==1?100.f:40.f);c.r7=0;
    (reference?reference_dora3_s3_801BF828:dora3_s3_801BF828)(rdram,&c);
    if(mode==0)(reference?reference_dora3_s3_801BFE60:dora3_s3_801BFE60)(rdram,&c);
    const double h=simulation_ticks();
    const float paired=dora3_native_gravity_displacement_f32(rf(rdram,physics+0x10),physics+0x10);
    (reference?reference_dora3_s3_801DBD58:dora3_s3_801DBD58)(rdram,&c);
    assert(simulation_ticks()==h);
    assert(dora3_native_gravity_displacement_f32(rf(rdram,physics+0x10),physics+0x10)==paired);

}
int main(int argc,char**argv) {
    assert(argc==2);std::vector<uint32_t> image(0x400000/4);std::ifstream in(argv[1],std::ios::binary);
    in.read(reinterpret_cast<char*>(image.data()),0x400000);assert(in.gcount()==0x400000);
    // Direct motion pairing covers acceleration, braking and both slope signs,
    // including speed bounds and reversals. No frame/render state is involved.
    unsigned drive_cases=0;
    for(float start:{-30.f,-10.f,0.f,11.f,12.f,13.f,24.f})for(float slope:{-10.f,-8.f,-1.25f,0.f,3.f,10.f}) {
        std::array<double,2> baseline{};
        for(unsigned parts:{1u,3u,7u,18u}) {
            float velocity=start;double distance=0;
            for(unsigned tick=0;tick<10;++tick)for(unsigned part=0;part<parts;++part) {
                const uint64_t begin=3000000000ull*part/parts,end=3000000000ull*(part+1)/parts;
                SimulationScope scope({end-begin,3000000000ull,tick,begin});
                velocity=dora3_native_drive_f32(velocity,12,6,slope,24,100);
                dora3_native_project_drive_f32(velocity,1,std::bit_cast<uint32_t>(velocity),100,116);
                distance+=dora3_native_gravity_displacement_f32(velocity,116);
            }
            if(parts==1)baseline={velocity,distance};
            else if(std::abs(velocity-baseline[0])>0.003 || std::abs(distance-baseline[1])>0.02) {
                std::cerr<<"Drive mismatch start="<<start<<" slope="<<slope<<" parts="<<parts<<" velocity="<<velocity<<" distance="<<distance<<" expected="<<baseline[0]<<','<<baseline[1]<<'\n';return 1;
            }
            ++drive_cases;
        }
    }
    for(unsigned mode=0;mode<3;++mode)for(bool drive:{false,true}) {
        auto initial=image;seed(reinterpret_cast<uint8_t*>(initial.data()),mode,drive);
        auto original=initial,actual=initial;
        run(reinterpret_cast<uint8_t*>(original.data()),mode,drive,true);
        run(reinterpret_cast<uint8_t*>(actual.data()),mode,drive,false);
        assert(actual==original);
        actual=initial;{SimulationScope scope({3000000000ull,3000000000ull,0,0});run(reinterpret_cast<uint8_t*>(actual.data()),mode,drive,false);}
        for(auto address:{player+0x9C,player+0xA0,player+0xA4,physics,physics+0x10,physics+0x14,physics+0x18}) {
            auto* rdram=reinterpret_cast<uint8_t*>(actual.data());auto* expected=reinterpret_cast<uint8_t*>(original.data());
            assert(rf(rdram,address)==rf(expected,address));
        }
        std::array<float,7> baseline{};
        for(unsigned fps:{60u,73u,144u,360u}) {
            dora3_native_camera_dispatch_reset();
            actual=initial;auto* rdram=reinterpret_cast<uint8_t*>(actual.data());SimulationTimeline clock(3);Nanoseconds previous=0;
            for(unsigned frame=1;frame<=fps;++frame) {
                const auto now=second*frame/fps;
                clock.advance(now-previous,[&](const SimulationSlice& s){SimulationScope scope(s);run(rdram,mode,drive,false);},[](uint64_t){});
                previous=now;
            }
            unsigned field=0;
            for(auto address:{player+0x9C,player+0xA0,player+0xA4,physics,0x801A99F8u,0x801A99FCu,0x801A9A00u}) {
                const float value=rf(rdram,address);
                if(fps==60)baseline[field]=value;
                else if(!std::isfinite(value)||std::abs(value-baseline[field])>0.02f) {
                    std::cerr<<"Partition mismatch mode="<<mode<<" drive="<<drive<<" fps="<<fps<<" field="<<field<<" actual="<<value<<" expected="<<baseline[field]<<'\n';return 1;
                }
                ++field;
            }
        }
    }
    std::cout<<"PASS: "<<drive_cases<<" drive partitions; 24 joint player/vector/gravity/contact/camera trajectories at 60/73/144/360FPS; Original equivalence; movement cache preserved across camera boundary; no game/GPU.\n";
}
