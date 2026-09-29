#include "native_frame_time.hpp"
#include "recomp.h"
#include "scene34-functions.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
using namespace dora3::native_time;
static std::set<uint32_t> called;
extern "C" void do_break(uint32_t) { std::abort(); }
extern "C" void dora3_trace(uint8_t*,recomp_context*,uint32_t pc,uint32_t) { called.insert(pc); }
extern "C" void osVirtualToPhysical_recomp(uint8_t*,recomp_context* c) { c->r2=uint32_t(c->r4)&0x1fffffff; }
// Explicitly outside this audit: controller warning's text renderer.
extern "C" void dora3_scene34_controller_message(uint8_t*,recomp_context*) { std::abort(); }
static constexpr uint32_t dl_cursor=0x801B1424,stack=0x803F0000;
static size_t index(uint32_t address) { return (address&0x3fffff)/4; }
static void frame(std::vector<uint32_t>& memory,unsigned bank) {
    auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    const uint32_t task=0x8010E310+bank*31728,dl=task+12328;
    MEM_W(0,int32_t(dl_cursor))=int32_t(dl);
    MEM_W(0,int32_t(0x80171CC0))=int32_t(task);
    MEM_W(0x100,int32_t(task))=int32_t(bank?0x803DA800:0x803B5000);
    const auto before=memory;
    recomp_context c{};c.r29=int32_t(stack);
    dora3_scene34_draw(rdram,&c);
    dora3_fade_draw_dispatch(rdram,&c);
    assert(uint32_t(c.r29)==stack);
    const uint32_t end=uint32_t(MEM_W(0,int32_t(dl_cursor)));
    assert(end>dl && end<=task+31728 && (end-dl)%8==0);
    // Only generated DL, its cursor and bounded call-stack scratch may change.
    // Includes image descriptors/resources, every game counter, both FBs and
    // the other frame bank in the invariant part of all 4 MiB guest memory.
    size_t start=0;
    for (auto [lo,hi]:std::vector<std::pair<size_t,size_t>>{
            {index(dl),index(end)}, {index(dl_cursor),index(dl_cursor)+1},
            {index(stack-0x100),index(stack)}}) {
        assert(start<=lo);
        assert(std::equal(memory.begin()+start,memory.begin()+lo,before.begin()+start));
        start=hi;
    }
    assert(std::equal(memory.begin()+start,memory.end(),before.begin()+start));
    bool image=false,color=false,depth=false;
    for (uint32_t p=dl;p<end;p+=8) {
        const uint32_t command=uint32_t(MEM_W(0,int32_t(p))),arg=uint32_t(MEM_W(4,int32_t(p)));
        image|=(command>>24)==0xE4;
        color|=(command>>24)==0xFF && arg==(bank?0x3DA800u:0x3B5000u);
        depth|=(command>>24)==0xFE && arg==0x38F800;
    }
    assert(image && color && depth);
}
int main(int argc,char** argv) {
    assert(argc==2);
    std::vector<uint32_t> memory(0x400000/4);
    auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    std::ifstream input(argv[1],std::ios::binary);
    input.read(reinterpret_cast<char*>(memory.data()+index(0x8007CD80)),128);assert(input.gcount()==128);
    MEM_W(0,int32_t(0x801AA650))=int32_t(0x8038F800);
    MEM_W(0,int32_t(0x801AA658))=320;MEM_W(0,int32_t(0x801A48C4))=240;
    MEM_W(0,int32_t(0x801C68C4))=0;MEM_W(0,int32_t(0x801C68C8))=2;
    MEM_W(0,int32_t(0x801AEC00+2*20))=int32_t(0x80280000);
    MEM_W(0,int32_t(0x801C68CC))=0;MEM_W(0,int32_t(0x801C68D0))=60;
    MEM_B(0,int32_t(0x801B1404))=255;MEM_B(0,int32_t(0x80171CB8))=255;MEM_B(0,int32_t(0x801A99B4))=255;
    for (int format:{5,6}) for (auto [w,h]:std::vector<std::pair<int,int>>{{2,2},{64,32},{320,240}}) {
        MEM_B(3,int32_t(0x80280000))=format;MEM_H(6,int32_t(0x80280000))=format==5?16:256;
        MEM_W(8,int32_t(0x80280000))=w;MEM_W(12,int32_t(0x80280000))=h;
        for (int alpha:{0,128,255}) for (unsigned bank:{0u,1u}) {
            MEM_H(0,int32_t(0x801AA660))=alpha;frame(memory,bank);
        }
    }
    unsigned frames=0;
    for (unsigned fps:{60u,73u,144u,165u,360u}) {
        ManualClock clock;clock.configure(Mode::Manual,fps,0);Fade fade({255,-1,16},3);
        std::set<int> alphas;
        for (unsigned i=1;i<=fps;++i) {
            const auto step=clock.take(*clock.deadline());assert(step);
            const auto value=fade.advance(step->elapsed);
            MEM_H(0,int32_t(0x801AA660))=value.alpha;
            frame(memory,i%2);alphas.insert(value.alpha);++frames;
        }
        assert(alphas.size()>16); // Actual new fade values, not repeating 20 Hz DL.
        assert(MEM_H(0,int32_t(0x801AA660))==0);
        assert(MEM_W(0,int32_t(0x801C68D0))==60 && MEM_W(0,int32_t(0x801C68CC))==0);
    }
    assert(called.contains(0x801C012C) && called.contains(0x801BFF24));
    assert(!called.contains(0x801C0088) && !called.contains(0x801BFE80));
    std::cout<<"PASS: scene34 image branch original draw closure; CI4/CI8 headers, 3 sizes, 2 banks, alpha 0/128/255; "
        <<frames<<" actual 2D command lists at 60/73/144/165/360 with native fade; all guest RAM outside DL/cursor/stack unchanged; no renderer or live scene activation\n";
}
