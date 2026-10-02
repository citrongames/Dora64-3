#include "native_simulation_time.hpp"
#include "native_camera_timeline.hpp"
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
void dora3_s0_80040104(uint8_t*,recomp_context*);
void dora3_s3_core_801D5594(uint8_t*,recomp_context*);
void reference_dora3_s3_core_801D5594(uint8_t*,recomp_context*);
void dora3_s3_801D5864(uint8_t*,recomp_context*);
void reference_dora3_s3_801D5864(uint8_t*,recomp_context*);
void dora3_s3_801D7294(uint8_t*,recomp_context*);
void reference_dora3_s3_801D7294(uint8_t*,recomp_context*);
void dora3_s3_801D8578(uint8_t*,recomp_context*);
void reference_dora3_s3_801D8578(uint8_t*,recomp_context*);
void dora3_s3_801D8A24(uint8_t*,recomp_context*);
void reference_dora3_s3_801D8A24(uint8_t*,recomp_context*);
void dora3_s3_801D8FF0(uint8_t*,recomp_context*);
void reference_dora3_s3_801D8FF0(uint8_t*,recomp_context*);
void dora3_s3_801DB720(uint8_t*,recomp_context*);
void reference_dora3_s3_801DB720(uint8_t*,recomp_context*);
void dora3_s3_801D9B00(uint8_t*,recomp_context*);
void reference_dora3_s3_801D9B00(uint8_t*,recomp_context*);
void dora3_s3_801D9C94(uint8_t*,recomp_context*);
void reference_dora3_s3_801D9C94(uint8_t*,recomp_context*);
void dora3_s3_801DA06C(uint8_t*,recomp_context*);
void reference_dora3_s3_801DA06C(uint8_t*,recomp_context*);
void dora3_s3_801DAB58(uint8_t*,recomp_context*);
void reference_dora3_s3_801DAB58(uint8_t*,recomp_context*);
void dora3_s3_801DA588(uint8_t*,recomp_context*);
void reference_dora3_s3_801DA588(uint8_t*,recomp_context*);
void dora3_s3_801DAF50(uint8_t*,recomp_context*);
void reference_dora3_s3_801DAF50(uint8_t*,recomp_context*);
void dora3_s3_801DB3D0(uint8_t*,recomp_context*);
void reference_dora3_s3_801DB3D0(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void do_break(uint32_t) {std::abort();}
}
constexpr uint32_t camera=0x801A99F8,player=0x80250000,physics=0x80251000,control=0x80252000,params=0x80253000;
void wf(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
float rf(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(p))));}
struct CameraCase {const char* name;recomp_func_t* actual;recomp_func_t* original;};
const std::array<CameraCase,13> modes{{{"801D5864",dora3_s3_801D5864,reference_dora3_s3_801D5864},
{"801D7294",dora3_s3_801D7294,reference_dora3_s3_801D7294},
{"801D8578",dora3_s3_801D8578,reference_dora3_s3_801D8578},
{"801D8A24",dora3_s3_801D8A24,reference_dora3_s3_801D8A24},
{"801D8FF0",dora3_s3_801D8FF0,reference_dora3_s3_801D8FF0},
{"801DB720",dora3_s3_801DB720,reference_dora3_s3_801DB720},
{"801D9B00",dora3_s3_801D9B00,reference_dora3_s3_801D9B00},
{"801D9C94",dora3_s3_801D9C94,reference_dora3_s3_801D9C94},
{"801DA06C",dora3_s3_801DA06C,reference_dora3_s3_801DA06C},
{"801DAB58",dora3_s3_801DAB58,reference_dora3_s3_801DAB58},
{"801DA588",dora3_s3_801DA588,reference_dora3_s3_801DA588},
{"801DAF50",dora3_s3_801DAF50,reference_dora3_s3_801DAF50},
{"801DB3D0",dora3_s3_801DB3D0,reference_dora3_s3_801DB3D0}}};

// Original spatial grid format from 80040104: signed X/Z bounds, relative
// polygon/list/grid offsets; 44-byte triangles with projected vertices, plane
// Ax+By+Cz=D and flags. All cells share four synthetic room triangles.
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
void seed(uint8_t* rdram,unsigned variant) {
    const bool with_geometry=variant>=3;variant%=3;
    for(unsigned p=0x802181F0;p<0x80218330;p+=4)MEM_W(0,int32_t(p))=0;
    for(auto pair:std::array<std::array<uint32_t,2>,5>{{{0x8021D49C,player},{0x8021D4B0,physics},{0x8021D6C0,control},{0x8021D648,params},{0x8021D654,params}}})MEM_W(0,int32_t(pair[0]))=int32_t(pair[1]);
    MEM_W(0x98,int32_t(player))=int32_t(physics);MEM_W(0x94,int32_t(player))=1;
    MEM_B(3,int32_t(control))=variant&1;MEM_B(2,int32_t(control))=2;
    wf(rdram,player+0x9C,100);wf(rdram,player+0xA0,50);wf(rdram,player+0xA4,200);
    wf(rdram,player+0x10,variant==2?180:20);wf(rdram,physics,10);wf(rdram,physics+0xC,15);
    for(unsigned off=0;off<0x34;off+=4)wf(rdram,camera+off,0);
    wf(rdram,camera+4,100);wf(rdram,camera+0x18,15);wf(rdram,camera+0x1C,variant==2?350:150);wf(rdram,camera+0x30,500);
    for(unsigned off=0x24;off<=0x2E;off+=2)MEM_H(int32_t(off),int32_t(params))=off==0x2E?500:10;
    // Original collision query's documented no-map path; no query stubs.
    MEM_W(0,int32_t(0x800DAE64))=0;if(with_geometry)room(rdram);
    for(unsigned i=0;i<64;++i) {
        const auto obj=0x801734B8+i*0xCC,phy=0x80260000+i*0x100;
        MEM_W(0x94,int32_t(obj))=1;MEM_W(0x98,int32_t(obj))=int32_t(phy);MEM_H(0x20,int32_t(phy))=int16_t(0x180+i);
        wf(rdram,obj+0x9C,300+i*10.f);wf(rdram,obj+0xA0,80);wf(rdram,obj+0xA4,500);
    }
}
void run(uint8_t* rdram,recomp_func_t* function) {
    recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=int32_t(camera);function(rdram,&c);
}
int main(int argc,char** argv) {
    assert(argc==2);std::vector<uint32_t> image(0x400000/4);std::ifstream in(argv[1],std::ios::binary);
    in.read(reinterpret_cast<char*>(image.data()),0x400000);assert(in.gcount()==0x400000);unsigned checks=0;

    {auto state=image;auto* rdram=reinterpret_cast<uint8_t*>(state.data());room(rdram);
        for(double height:{-100.,100.,300.}) {
            recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.f12.d=20;c.f14.d=height;
            SD(std::bit_cast<uint64_t>(10.0),16,c.r29);dora3_s0_80040104(rdram,&c);
            const auto flags=MEM_W(0,int32_t(0x800DACF0));
            if(flags!=(height==100?0:1)){std::cerr<<"Synthetic room query height="<<height<<" flags="<<flags<<" floor="<<rf(rdram,0x800DACB8)<<" ceiling="<<rf(rdram,0x800DACD4)<<'\n';return 1;}
            if(height==100){assert(rf(rdram,0x800DACB8)==0);assert(rf(rdram,0x800DACD4)==250);}++checks;
        }
    }
    for(const auto& mode:modes) {
        double spread=0;unsigned worst_field=0,worst_variant=0;
        for(unsigned variant=0;variant<6;++variant) {
            auto initial=image;seed(reinterpret_cast<uint8_t*>(initial.data()),variant);
            auto original=initial,actual=initial;run(reinterpret_cast<uint8_t*>(original.data()),mode.original);run(reinterpret_cast<uint8_t*>(actual.data()),mode.actual);
            if(actual!=original){std::cerr<<mode.name<<" Original mismatch\n";return 1;}++checks;
            actual=initial;{SimulationScope scope({3000000000ull,3000000000ull,0,0});run(reinterpret_cast<uint8_t*>(actual.data()),mode.actual);}
            if(actual!=original){std::cerr<<mode.name<<" full-step mismatch\n";return 1;}++checks;
            std::array<float,13> baseline{};std::array<std::array<float,13>,60> native_samples{};unsigned changed_samples=0;
            for(unsigned fps:{60u,144u,360u}) {
                actual=initial;auto* rdram=reinterpret_cast<uint8_t*>(actual.data());SimulationTimeline timeline(3);OrdinaryCameraTimeline camera_time;Nanoseconds previous=0;auto reference=initial;
                for(unsigned frame=1;frame<=fps;++frame) {
                    const auto now=second*frame/fps;timeline.advance(now-previous,[&](const SimulationSlice& slice){recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=int32_t(camera);camera_time.advance(rdram,&c,slice,uint32_t(std::stoul(mode.name,nullptr,16)),mode.actual);},[&](uint64_t tick){
                        auto* expected=reinterpret_cast<uint8_t*>(reference.data());run(expected,mode.original);
                        for(unsigned off=0;off<0x34;off+=4)if(rf(rdram,camera+off)!=rf(expected,camera+off)){
                            std::cerr<<mode.name<<" endpoint mismatch field="<<off<<" tick="<<tick<<" fps="<<fps<<" actual="<<rf(rdram,camera+off)<<" expected="<<rf(expected,camera+off)<<'\n';std::abort();
                        }
                        ++checks;
                    });previous=now;
                    if(fps==60){
                        for(unsigned i=0;i<13;++i)native_samples[frame-1][i]=rf(rdram,camera+4*i);
                        if(frame>1&&native_samples[frame-1]!=native_samples[frame-2])++changed_samples;
                    } else if(fps==360&&frame%6==0) {
                        for(unsigned i=0;i<13;++i)if(rf(rdram,camera+4*i)!=native_samples[frame/6-1][i]){
                            std::cerr<<mode.name<<" fractional sample mismatch at "<<frame<<" field "<<4*i<<'\n';std::abort();
                        }++checks;
                    }
                }
                for(unsigned i=0;i<13;++i) {
                    const auto value=rf(rdram,camera+4*i);
                    if(!std::isfinite(value)){std::cerr<<mode.name<<" nonfinite at "<<i<<'\n';return 1;}
                    if(fps==60)baseline[i]=value;
                    else {double delta=value-baseline[i];if(i==6||i==7)delta=std::remainder(delta,360.);if(std::abs(delta)>spread){spread=std::abs(delta);worst_field=4*i;worst_variant=variant;}}
                }
            }
        }

        // The real charge ctor starts shake after movement at Source end,
        // before the camera callback. That command must not discard the camera
        // equation anchor merely because the final Native fraction is shorter.
        // Compare its endpoint to the actual Original command + camera once,
        // including all shared camera/shake fields, for different final cuts.
        for(unsigned variant=0;variant<6;++variant)for(unsigned divider:{2u,3u})
        for(unsigned cut:{2u,3u,10u})for(unsigned command:{0u,1u,2u}) {
            auto initial=image;auto* initial_ram=reinterpret_cast<uint8_t*>(initial.data());seed(initial_ram,variant);
            {auto* rdram=initial_ram;MEM_W(0,int32_t(0x801A39DC))=int(divider);}
            auto actual=initial,expected=initial;
            auto* ram=reinterpret_cast<uint8_t*>(actual.data());auto* ref=reinterpret_cast<uint8_t*>(expected.data());
            OrdinaryCameraTimeline time;
            const auto den=uint64_t(divider)*second,first=den-den/cut;
            recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=int32_t(camera);
            time.advance(ram,&c,{first,den,0,0},uint32_t(std::stoul(mode.name,nullptr,16)),mode.actual);
            auto cue=[&](uint8_t* memory,recomp_func_t* function) {
                recomp_context ctx{};ctx.f_odd=&ctx.f0.u32h;ctx.r29=int32_t(0x80300000);
                ctx.r4=command==1?3:1;ctx.r5=command==1?9:command==2?1:6;
                ctx.r6=std::bit_cast<int32_t>(command==1?-4.f:command==2?0.f:10.f);
                function(memory,&ctx);
            };
            cue(ram,dora3_s3_core_801D5594);cue(ref,reference_dora3_s3_core_801D5594);
            run(ref,mode.original);
            c.r4=int32_t(camera);time.advance(ram,&c,{den-first,den,0,first},uint32_t(std::stoul(mode.name,nullptr,16)),mode.actual);
            auto equal_region=[&](uint32_t start,unsigned size) {
                auto* rdram=ram;
                for(unsigned off=0;off<size;++off)if(MEM_BU(int32_t(off),int32_t(start))!=ref[((start&0xFFFFFF)+off)^3]) {
                    std::cerr<<mode.name<<" Source-end shake mismatch variant="<<variant<<" divider="<<divider<<" cut="<<cut<<" command="<<command<<" address="<<std::hex<<start+off<<std::dec<<'\n';std::abort();
                }
            };
            equal_region(camera,0x34);equal_region(0x802181EC,0x144);++checks;
        }

        // A write from another subsystem must survive the next sample, including
        // halfwords adjacent to camera fields. Owned external writes invalidate
        // the anchor; an explicit reset also permits a new scene clock.
        auto actual=image;auto* rdram=reinterpret_cast<uint8_t*>(actual.data());seed(rdram,0);
        OrdinaryCameraTimeline camera_time;
        auto advance=[&](SimulationSlice slice){recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);c.r4=int32_t(camera);camera_time.advance(rdram,&c,slice,uint32_t(std::stoul(mode.name,nullptr,16)),mode.actual);};
        advance({1000000000ull,3000000000ull,0,0});
        wf(rdram,camera+0x0C,321);wf(rdram,camera+0x10,654);wf(rdram,camera+0x14,987);wf(rdram,camera+0x28,-1);MEM_W(0,int32_t(0x80218328))=0x12345678;MEM_H(0,int32_t(0x8021823E))=1234;MEM_H(0,int32_t(0x80218278))=5678;
        advance({1000000000ull,3000000000ull,0,1000000000ull});
        assert(rf(rdram,camera+0x0C)==321&&rf(rdram,camera+0x10)==654&&rf(rdram,camera+0x14)==987&&rf(rdram,camera+0x28)==-1);++checks;
        assert(MEM_W(0,int32_t(0x80218328))==0x12345678&&MEM_H(0,int32_t(0x8021823E))==1234&&MEM_H(0,int32_t(0x80218278))==5678);++checks;
        wf(rdram,camera+4,777);auto expected=actual;
        {SimulationScope scope({1000000000ull,3000000000ull,0,2000000000ull});run(reinterpret_cast<uint8_t*>(expected.data()),mode.actual);}
        advance({1000000000ull,3000000000ull,0,2000000000ull});
        assert(actual==expected);++checks;
        camera_time.reset();advance({3000000000ull,3000000000ull,0,0});++checks;
        std::cout<<mode.name<<" diagnostic_max_rate_spread="<<spread<<" field="<<worst_field<<" variant="<<worst_variant<<'\n';
    }
    std::cout<<"PASS: "<<checks<<" Original/full-tick memory comparisons across thirteen ordinary camera roots. Native equations also match at shared fractional timestamps; Source-end real shake commands at three final cuts and two dividers, external writes/reset preserved; synthetic room triangles through original collision query, no real level/GPU.\n";
}

extern "C" int dora3_native_preview_hook(uint8_t*,recomp_context*,uint32_t) {return 0;}
