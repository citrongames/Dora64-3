#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
#include "model-functions.h"
#include <cassert>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
using namespace dora3::native_time;
static Nanoseconds now=0;
static bool owns_banks=false;
static const FrameBinding* binding=nullptr;
extern "C" int dora3_native_loop_owns_frame_banks(){return owns_banks;}
static uint32_t next_address=0x80200000;
static std::set<uint32_t> allocations;
namespace dora3::native_time {
Nanoseconds monotonic_time(){return now;}
const FrameBinding* active_frame_binding(){return binding;}
}
extern "C" void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
extern "C" void dora3_game_heap_alloc(uint8_t*,recomp_context* c) {
    const uint32_t size=uint32_t(c->r4);assert(size && size<4096);
    c->r2=int32_t(next_address);assert(allocations.insert(next_address).second);
    next_address+=(size+15)&~15u;
}
extern "C" void dora3_s0_80033C50(uint8_t*,recomp_context* c) {assert(allocations.erase(uint32_t(c->r4))==1);}
extern "C" void dora3_s0_80038B10(uint8_t* ram,recomp_context* c) {dora3_s0_80033C50(ram,c);}
extern "C" void switch_error(const char*,uint32_t,uint32_t) {throw std::logic_error("Unexpected source jump table index");}
int main() {
    std::vector<uint32_t> memory(0x400000/4);auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    auto W=[&](uint32_t a)->int32_t& {return MEM_W(0,int32_t(a));};
    auto F=[&](uint32_t a,float f) {W(a)=std::bit_cast<int32_t>(f);};
    auto R=[&](uint32_t a) {return std::bit_cast<float>(W(a));};
    auto B=[&](uint32_t a)->int8_t& {return MEM_B(0,int32_t(a));};
    auto H=[&](uint32_t a)->int16_t& {return MEM_H(0,int32_t(a));};
    recomp_context ctx{};ctx.r29=int32_t(0x803F0000);
    auto& adapter=game_model_animation();
    constexpr uint32_t asset=0x80100000,clip=0x80101000;
    B(asset+0x57)=0;B(asset+0x56)=3;B(asset+0x55)=3;
    W(asset+0x14)=0x100;W(asset+0x2C)=0x200;H(asset+0x2A)=1;W(asset+0x34)=0x280;W(asset+0x3C)=1;
    F(asset+0x100+0x1C,1);F(asset+0x100+0x20,1);F(asset+0x100+0x24,1);
    W(asset+0x280)=0x300;H(asset+0x300)=0;H(asset+0x302)=100;H(asset+0x304)=50;H(asset+0x306)=30;H(asset+0x30C)=-1;
    W(clip+0x14)=0x100;H(clip+0x1A)=2;W(clip+0x1C)=0x200;W(clip+0x24)=0x300;
    F(clip+0x100+0x1C,1);F(clip+0x100+0x20,1);F(clip+0x100+0x24,1);
    const uint32_t ch=clip+0x200,key=clip+0x300;
    W(ch)=4;F(ch+4,10);W(ch+0x10)=0;W(ch+0x14)=0;W(ch+0x18)=0;
    W(ch+28)=10;F(ch+32,10);W(ch+28+0x10)=0;W(ch+28+0x14)=1;
    W(key)=0;W(key+8)=2;W(key+12)=0x400;
    W(key+16)=0;W(key+24)=2;W(key+28)=0x440;
    F(clip+0x400,0);F(clip+0x404,0);F(clip+0x408,10);F(clip+0x40C,100);
    F(clip+0x440,0);F(clip+0x444,0);F(clip+0x448,10);F(clip+0x44C,1);
    // Resident Hermite constants, numerical values rather than game assets.
    W(0x8007CFA0)=0x3FF00000;W(0x8007CFA4)=0;W(0x8007CFA8)=0x40080000;W(0x8007CFAC)=0;
    auto create=[&]() {ctx.r4=int32_t(asset);dora3_s0_8003B560(rdram,&ctx);return uint32_t(ctx.r2);};
    auto model=create();assert(adapter.model_count()==1);
    const auto bone=uint32_t(W(model+0x5C));
    auto bind=[&]() {ctx.r4=int32_t(model);ctx.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&ctx);};
    bind();ctx.r4=int32_t(model);dora3_s0_core_80038BC8(rdram,&ctx);
    const uint32_t banks[2]={uint32_t(W(model+0x6C)),uint32_t(W(model+0x70))};
    auto step=[&](float rate,unsigned divider) {
        W(0x801A39DC)=int(divider);adapter.hook(rdram,&ctx,0x80030850,now);
        ctx.r4=int32_t(model);dora3_s0_800393C4(rdram,&ctx);
        ctx.r4=int32_t(model);ctx.r5=std::bit_cast<uint32_t>(rate);dora3_s0_8003948C(rdram,&ctx);
    };
    auto seek=[&](int time) {ctx.r4=int32_t(model);ctx.r5=time;dora3_s0_80039564(rdram,&ctx);};
    unsigned frames=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        now+=second;seek(0);step(3,divider);
        const auto original_time=W(model+0x64);const auto flags=B(model+0x7C);const float original_pose=R(bone+0x14);
        for(unsigned f=1;uint64_t(f)*60<=uint64_t(fps)*divider;++f) {
            unsigned bank=f&1;W(0x801A9544)=int(bank);
            auto other=std::vector<uint8_t>(rdram+(banks[bank^1]&0x3fffff),rdram+(banks[bank^1]&0x3fffff)+16);
            const auto context=ctx;const auto global=W(0x80082724);
            adapter.begin_frame(rdram,&ctx,now+Nanoseconds(f)*second/fps,bank);
            float t=float(double(Nanoseconds(f)*second/fps)*60/(double(second)*divider)*3);
            assert(std::abs(R(bone+0x14)-10*t)<0.0001f);
            assert(std::abs(int(H(banks[bank]))-int(t*10))<=1);
            assert(std::memcmp(other.data(),rdram+(banks[bank^1]&0x3fffff),16)==0);
            assert(W(model+0x64)==original_time && B(model+0x7C)==flags);
            assert(W(0x80082724)==global && std::memcmp(&ctx,&context,sizeof(ctx))==0);
            adapter.end_frame(rdram);assert(R(bone+0x14)==original_pose);++frames;
        }
    }
    // Non-linear original Hermite channels must be evaluated, not blended poses.
    W(key)=1;W(key+12)=0x480;
    F(clip+0x480,0);F(clip+0x484,0);F(clip+0x488,0);F(clip+0x48C,0);
    F(clip+0x490,10);F(clip+0x494,100);F(clip+0x498,0);F(clip+0x49C,0);
    now+=second;seek(0);step(5,3);W(0x801A9544)=0;
    adapter.begin_frame(rdram,&ctx,now+25'000'000,0);
    assert(std::abs(R(bone+0x14)-15.625f)<0.001f);
    W(0x801B1608)=int32_t(0x80300000);
    for(unsigned i=0;i<16;++i) {H(0x8030FB40+2*i)=(i%5==0)?1:0;H(0x8030FB60+2*i)=0;}
    ctx.r4=int32_t(model);dora3_s0_8003997C(rdram,&ctx);
    assert(std::abs(R(bone+0x68)-15.625f)<0.001f); // Source matrix builder used the new pose.
    adapter.end_frame(rdram);
    // End pulse is nondestructive and survives every extra pose evaluation.
    seek(9);step(3,3);assert(B(model+0x7C)&2);
    for(int i=0;i<10;++i) {
        adapter.begin_frame(rdram,&ctx,now+i*4'000'000,0);adapter.end_frame(rdram);
        ctx.r4=int32_t(model);dora3_s0_menu_80039544(rdram,&ctx);assert(ctx.r2==2);
        ctx.r4=int32_t(model);dora3_s0_menu_80039544(rdram,&ctx);assert(ctx.r2==2);
    }
    ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_80039578(rdram,&ctx); // pause
    step(3,3);const auto paused=W(model+0x64);
    adapter.begin_frame(rdram,&ctx,now+second,0);adapter.end_frame(rdram);assert(W(model+0x64)==paused && (B(model+0x7C)&2));
    // Same-value seek invalidates observation; no accidental old-clock continuation.
    seek(2);F(bone+0x14,777);adapter.begin_frame(rdram,&ctx,now+second,0);assert(R(bone+0x14)==777);adapter.end_frame(rdram);
    // Stale models do not keep moving if the latest logic step did not update them.
    ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_80039578(rdram,&ctx);step(3,3);
    const auto stale_pose=R(bone+0x14);
    now+=second;adapter.hook(rdram,&ctx,0x80030850,now);
    adapter.begin_frame(rdram,&ctx,now+second,0);assert(R(bone+0x14)==stale_pose);adapter.end_frame(rdram);
    // Unknown time writes fail with all modified transforms/time rolled back.
    step(3,3);const auto last_pose=R(bone+0x14);F(model+0x64,7);bool rejected=false;
    try {adapter.begin_frame(rdram,&ctx,now,0);}catch(const std::logic_error&){rejected=true;}
    assert(rejected && R(bone+0x14)==last_pose);
    // Rebinding the exact same clip pointer still resets the timeline and pose.
    ctx.r4=int32_t(model);dora3_s0_8003B744(rdram,&ctx);bind();
    assert(W(model+0x64)==0 && R(bone+0x14)==0);
    adapter.begin_frame(rdram,&ctx,now+second,0);assert(R(bone+0x14)==0);adapter.end_frame(rdram);
    seek(9);ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_800395BC(rdram,&ctx);step(3,3);
    adapter.begin_frame(rdram,&ctx,now+25'000'000,0);
    assert(R(model+0x64)==10 && R(bone+0x14)==100 && (B(model+0x7C)&3)==3);
    adapter.end_frame(rdram);
    ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_800395BC(rdram,&ctx);
    // Logic updates while native owns the banks must not touch either vertex output.
    seek(1);owns_banks=true;
    const auto bank0_before=H(banks[0]),bank1_before=H(banks[1]);step(3,3);
    assert(H(banks[0])==bank0_before && H(banks[1])==bank1_before);
    FrameBinding frame{{{},0,{16'666'667,60}},nullptr};binding=&frame;
    adapter.begin_frame(rdram,&ctx,now+25'000'000,0);adapter.end_frame(rdram);
    assert(H(banks[0])!=bank0_before && H(banks[1])==bank1_before);
    // Wrong-bank writes fail before the original routine touches vertices.
    W(0x801A9544)=1;bool wrong_bank=false;const auto before_wrong_bank=ctx;
    try {ctx.r4=int32_t(model);dora3_s0_800393C4(rdram,&ctx);}catch(const std::logic_error&){wrong_bank=true;}
    assert(wrong_bank && H(banks[1])==bank1_before);ctx=before_wrong_bank;W(0x801A9544)=0;
    binding=nullptr;owns_banks=false;
    // Original model deletion is legal during draw: restore CPU bone data first.
    seek(0);step(3,3);adapter.begin_frame(rdram,&ctx,now+25'000'000,0);
    // System reset itself does not destroy the model; the allocator still owns it.
    adapter.end_frame(rdram);dora3_object_subsystem_reset(rdram,&ctx);assert(adapter.model_count()==1);
    adapter.begin_frame(rdram,&ctx,now,0);adapter.end_frame(rdram);
    step(3,3);adapter.begin_frame(rdram,&ctx,now+25'000'000,0);
    // Same-pointer recreation must not retain the old clip or observations.
    ctx.r4=int32_t(model);dora3_s0_8003B77C(rdram,&ctx);assert(adapter.model_count()==0 && allocations.empty());adapter.end_frame(rdram);
    next_address=model;const auto recreated=create();assert(recreated==model && adapter.model_count()==1);
    assert(W(model+0x64)==0 && W(model+0x60)==0);adapter.begin_frame(rdram,&ctx,now,0);adapter.end_frame(rdram);
    ctx.r4=int32_t(model);dora3_s0_8003B77C(rdram,&ctx);assert(allocations.empty());adapter.reset();
    std::cout<<"PASS: "<<frames<<" original linear/morph native pose samples at 60/73/120/144/165/360 Hz and both dividers; Hermite curve through original matrix builder, bank isolation, guest context/time/event preservation, pause/seek/clamp/rebind/stale updates, unknown-write rollback, deferred logic morph, draw-time deletion and same-address recreation; no game/GPU.\n";
}
