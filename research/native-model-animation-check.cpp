#include "native_model_animation.hpp"
#include "native_simulation_time.hpp"
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
extern "C" int dora3_native_defer_player_shot(){return 0;}
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
static unsigned player_impulses=0;
extern "C" void dora3_s3_core_801C120C(uint8_t*,recomp_context*) {++player_impulses;}
extern "C" void dora3_s3_core_801C1664(uint8_t*,recomp_context*) {}
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
    const auto bound_run=adapter.run_token(model);assert(bound_run);
    seek(0);const auto sought_run=adapter.run_token(model);assert(sought_run!=bound_run);
    seek(0);assert(adapter.run_token(model)!=sought_run); // Same frame is still a real seek.
    const auto before_rebind=adapter.run_token(model);
    ctx.r4=int32_t(model);dora3_s0_8003B744(rdram,&ctx);bind();assert(adapter.run_token(model)!=before_rebind);
    ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_80039578(rdram,&ctx);
    const auto paused_run=adapter.run_token(model);
    ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_80039578(rdram,&ctx);assert(adapter.run_token(model)==paused_run);
    ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_80039578(rdram,&ctx);assert(adapter.run_token(model)!=paused_run);

    // Original's counter is one source step ahead of its last evaluated pose.
    // A reviewed owner must acquire that pose before starting native elapsed
    // time; foreign owners and stale observations remain untouched.
    seek(0);B(model+0x7C)=0;step(3,3);assert(R(model+0x64)==3);
    adapter.acquire_simulation(rdram,[](uint32_t)->std::optional<float> {return std::nullopt;});
    assert(R(model+0x64)==3);
    adapter.acquire_simulation(rdram,[&](uint32_t owner)->std::optional<float> {return owner==model?std::optional<float>{3}:std::nullopt;});
    assert(R(model+0x64)==0);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,0});
     assert(!adapter.frame_event(rdram,model,0,0,1234,true));
     assert(!adapter.frame_event(rdram,model,0,0,1234,true));step(3,3);}
    assert(R(model+0x64)==1);
    seek(0);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,0});
     assert(adapter.frame_event(rdram,model,0,0,1234,true));
     assert(adapter.frame_event(rdram,model,0,0,1234,true));}
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
    // Actual Source pause/zero-rate calls still record an observation. Native
    // must preserve a later game-authored bone override while regenerating the
    // actual type10 morph into each free bank. This is the stop-after-sound path.
    for(bool pause:{false,true})for(unsigned divider:{2u,3u}) {
        seek(4);B(model+0x7C)=pause?8:0;step(pause?3.f:0.f,divider);
        F(bone+0x14,123.f);const auto clip_time=W(model+0x64);
        for(unsigned bank:{0u,1u}) {
            W(0x801A9544)=int(bank);const auto other=H(banks[bank^1]);
            adapter.begin_frame(rdram,&ctx,now+second*divider/120,bank);
            assert(R(bone+0x14)==123.f&&W(model+0x64)==clip_time&&H(banks[bank^1])==other);
            assert(std::abs(int(H(banks[bank]))-40)<=1);
            adapter.end_frame(rdram);assert(R(bone+0x14)==123.f);++frames;
        }
    }
    B(model+0x7C)=0;
    unsigned simulation_cases=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,144u,360u})for(float rate:{3.f,-2.f,25.f}) {
        seek(0);B(model+0x7C)=0;SimulationTimeline timeline(divider);Nanoseconds previous=0;
        owns_banks=true;binding=nullptr;unsigned events=0;
        for(unsigned frame_index=1;frame_index<=fps;++frame_index) {
            const auto timestamp=second*frame_index/fps;
            timeline.advance_while(timestamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);step(rate,divider);
                ctx.r4=int32_t(model);dora3_s0_menu_80039544(rdram,&ctx);const auto ended=ctx.r2;
                ctx.r4=int32_t(model);dora3_s0_menu_80039544(rdram,&ctx);assert(ctx.r2==ended);
                if(ended)++events;
            },[](uint64_t){},[]{return true;},[&](const SimulationSlice& slice) {
                return adapter.clip_boundary_limit(rdram,slice,[&](uint32_t owner)->std::optional<float> {
                    return owner==model?std::optional<float>{rate}:std::nullopt;
                });
            });
            const auto saved_time=W(model+0x64);const auto saved_flags=B(model+0x7C);const auto pose=R(bone+0x14);
            const unsigned bank=frame_index&1;W(0x801A9544)=int(bank);
            FrameBinding native_frame{{{},bank,{timestamp-previous,fps}},nullptr};binding=&native_frame;
            const auto other=H(banks[bank^1]);
            adapter.begin_frame(rdram,&ctx,now+second,bank);
            assert(std::abs(R(bone+0x14)-10*R(model+0x64))<0.0002f && W(model+0x64)==saved_time && B(model+0x7C)==saved_flags);
            assert(H(banks[bank^1])==other);adapter.end_frame(rdram);assert(R(bone+0x14)==pose);binding=nullptr;
            previous=timestamp;
        }
        float expected=std::fmod(rate*float(60/divider),10.f);if(expected<0)expected+=10;
        if(std::abs(R(model+0x64)-expected)>=0.0001f){std::cerr<<"Native model mismatch divider="<<divider<<" fps="<<fps<<" rate="<<rate<<" actual="<<R(model+0x64)<<" expected="<<expected<<" events="<<events<<"\n";std::abort();}
        assert(events==unsigned(std::ceil(std::abs(rate)*float(60/divider)/10)));
        owns_banks=false;++simulation_cases;
    }
    unsigned crossing_cases=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,144u,360u})for(float rate:{3.f,-2.f}) {
        seek(0);B(model+0x7C)=0;SimulationTimeline timeline(divider);Nanoseconds previous=0;
        unsigned hits[4]={};
        for(unsigned frame_index=1;frame_index<=fps;++frame_index) {
            const auto timestamp=second*frame_index/fps;
            timeline.advance(timestamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);
                ctx.r4=int32_t(model);ctx.r5=std::bit_cast<uint32_t>(rate);dora3_s0_8003948C(rdram,&ctx);
                for(unsigned k=0;k<4;++k) {
                    const float target=float(k*3);
                    const auto hit=adapter.frame_event(rdram,model,R(model+0x64),target,100+k);
                    assert(adapter.frame_event(rdram,model,R(model+0x64),target,100+k)==hit);
                    if(hit)++hits[k];
                }
            },[](uint64_t){});previous=timestamp;
        }
        for(auto count:hits)assert(count==unsigned(std::abs(rate)*float(60/divider)/10));
        ++crossing_cases;
    }
    // Execute the real player's frame-18 branch. Only its impulse consumer and
    // separately-verified movement closure are observation boundaries here.
    const uint32_t object=0x80370000;W(object+0x74)=int32_t(model);W(0x8021D49C)=int32_t(object);
    seek(12);B(model+0x7C)=0;
    step(3,3);assert(R(model+0x64)==5 && (B(model+0x7C)&2));
    adapter.acquire_simulation(rdram,[](uint32_t)->std::optional<float>{return std::nullopt;});
    assert(R(model+0x64)==5);
    adapter.acquire_simulation(rdram,[&](uint32_t owner)->std::optional<float>{return owner==model?std::optional<float>{3}:std::nullopt;});
    assert(R(model+0x64)==12 && !(B(model+0x7C)&2));
    auto seek_reference=memory;auto* seek_ram=reinterpret_cast<uint8_t*>(seek_reference.data());
    ctx.r4=int32_t(model);ctx.r5=std::bit_cast<int32_t>(3.f);dora3_s0_8003948C(seek_ram,&ctx);
    assert(MEM_BU(0x7C,int32_t(model))==0);
    for(unsigned part=0;part<4;++part) {
        SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull*part});
        ctx.r4=int32_t(model);ctx.r5=std::bit_cast<int32_t>(3.f);adapter.advance_simulation(rdram,&ctx);
        assert(MEM_BU(0x7C,int32_t(model))&2);
    }
    {auto* rdram=seek_ram;assert(MEM_W(0x64,int32_t(model))==W(model+0x64));assert(MEM_BU(0x7C,int32_t(model))==uint8_t(B(model+0x7C)));}
    {SimulationScope simulation({750000000ull,3000000000ull,1,0});ctx.r4=int32_t(model);ctx.r5=std::bit_cast<int32_t>(3.f);adapter.advance_simulation(rdram,&ctx);assert(!(B(model+0x7C)&2));}
    // A seven-frame clip ends between quarter updates in source interval2.
    // Its raw pulse clears on the next advance, while the reviewed discrete
    // owner can still consume that crossing at the end of the same interval.
    seek(0);B(model+0x7C)=0;F(uint32_t(W(model+0x18))+4,7);
    for(unsigned part=0;part<12;++part) {
        SimulationScope simulation({750000000ull,3000000000ull,part/4,750000000ull*(part%4)});
        ctx.r4=int32_t(model);ctx.r5=std::bit_cast<int32_t>(3.f);adapter.advance_simulation(rdram,&ctx);
        assert(adapter.ended_this_tick(model)==(part>=9));
        assert(bool(B(model+0x7C)&2)==(part==9));
    }
    {SimulationScope simulation({750000000ull,3000000000ull,3,0});
     assert(!adapter.ended_this_tick(model));}
    seek(0);F(uint32_t(W(model+0x18))+4,10);
    // A float opacity crosses zero within a partial source tick. Its original
    // start-of-tick test must still permit the rest of that tick, then retire
    // on the following endpoint. Check exact guest float endpoints, not merely
    // a tolerance that could hide a changed deletion tick.
    unsigned scalar_cases=0;
    const uint32_t alpha=object+0x30;
    auto scalar_plan=[&](uint32_t owner)->std::optional<float> {return owner==model?std::optional<float>{0}:std::nullopt;};
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        adapter.acquire_simulation(rdram,scalar_plan);F(alpha,1);float original=1;
        SimulationTimeline timeline(divider);Nanoseconds previous=0;bool retired=false,original_retired=false;
        for(unsigned f=1;f<=fps;++f) {
            const auto stamp=second*f/fps;
            timeline.advance(stamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);
                if(retired)return;
                if(adapter.scalar_positive(rdram,model,alpha,R(alpha)))
                    F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.1));
                else if(slice.ends_tick())retired=true;
            },[&](uint64_t) {
                if(!original_retired) {
                    if(original>0)original=float(double(original)-0.1);
                    else original_retired=true;
                }
                assert(retired==original_retired && W(alpha)==std::bit_cast<int32_t>(original));
            });previous=stamp;
        }
        assert(retired);++scalar_cases;
    }
    unsigned angle_cases=0;const uint32_t oscillator=object+0x36;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        adapter.acquire_simulation(rdram,scalar_plan);MEM_H(0,int32_t(oscillator))=350;int original=350;
        SimulationTimeline timeline(divider);Nanoseconds previous=0;
        for(unsigned frame_index=1;frame_index<=fps;++frame_index) {
            const auto stamp=second*frame_index/fps;
            timeline.advance(stamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);
                double expected=std::fmod(original+40.*double(slice.phase+slice.numerator)/double(slice.denominator),360.);
                const auto angle=adapter.angle_short(rdram,model,oscillator,40);
                assert(std::abs(angle-expected)<0.0001 && MEM_H(0,int32_t(oscillator))==int16_t(expected));
            },[&](uint64_t){original=(original+40)%360;assert(MEM_H(0,int32_t(oscillator))==original);});
            previous=stamp;
        }
        ++angle_cases;
    }
    unsigned preceding_cases=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        adapter.acquire_simulation(rdram,scalar_plan);F(alpha,0.9f);float original=0.9f;
        SimulationTimeline timeline(divider);Nanoseconds previous=0;float displayed=1;
        for(unsigned frame_index=1;frame_index<=fps/2;++frame_index) {
            const auto stamp=second*frame_index/fps;
            timeline.advance(stamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);
                displayed=adapter.scalar_previous(rdram,model,alpha,R(alpha),-0.1);
                F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.1));
            },[&](uint64_t){assert(displayed==original);original=float(double(original)-0.1);assert(R(alpha)==original);});
            previous=stamp;
        }
        ++preceding_cases;
    }
    // A model constructed inside a live interval has no Original lookahead;
    // its first opacity command starts at1 and must never overshoot above it.
    adapter.acquire_simulation(rdram,scalar_plan);F(alpha,0.1f);
    const uint32_t birth_alpha=object+0x44;F(birth_alpha,1);
    uint32_t native_born=0;
    {SimulationScope simulation({750000000ull,3000000000ull,0,0});
     F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.8));assert(R(alpha)<0);
     native_born=create();assert(native_born!=model);
     assert(adapter.scalar_previous(rdram,native_born,birth_alpha,R(birth_alpha),-0.1)==0.975f);}
    adapter.acquire_simulation(rdram,[&](uint32_t owner)->std::optional<float> {return owner==native_born?std::optional<float>{3}:std::nullopt;},false);
    {SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull});
     assert(adapter.scalar_positive(rdram,model,alpha,R(alpha)));
     ctx.r4=int32_t(native_born);dora3_s0_8003B77C(rdram,&ctx);assert(adapter.model_count()==1);}
    const uint32_t steering=object+0x40;unsigned steering_cases=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        adapter.acquire_simulation(rdram,scalar_plan);F(steering,359);float original=359;
        SimulationTimeline timeline(divider);Nanoseconds previous=0;
        for(unsigned frame_index=1;frame_index<=fps;++frame_index) {
            const auto stamp=second*frame_index/fps;
            timeline.advance(stamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);
                const float chosen=(slice.tick&1)?-2:2;
                // A repeated branch reverses direction after the first part;
                // that does not replace this tick's already chosen command.
                const auto proposed=slice.phase==0?chosen:-chosen;
                F(steering,adapter.scalar_latched_turn(rdram,model,steering,R(steering),proposed));
            },[&](uint64_t tick){original=std::fmod(original+((tick&1)?2:-2)+360.f,360.f);assert(R(steering)==original);});
            previous=stamp;
        }
        ++steering_cases;
    }
    // Partial heap reset invalidates fields even when the model remains alive.
    const uint32_t motion_physical=object+0x200;unsigned motion_initializations=0;
    auto motion_seed=[&] {SourceMotionSample s;s.position[0]=float(++motion_initializations);return s;};
    auto sample_motion=[&](uint64_t tick,uint64_t phase) {
        SimulationScope simulation({750000000ull,3000000000ull,tick,phase});
        const auto& s=adapter.motion_begin(rdram,model,object,motion_physical,motion_seed);
        assert(adapter.motion_current(model,object)==&s);
        const auto value=s.position[0];adapter.motion_end(rdram,model,object);
        assert(!adapter.motion_current(model,object));return value;
    };
    adapter.acquire_simulation(rdram,scalar_plan);
    assert(sample_motion(0,0)==1 && sample_motion(0,750000000ull)==1);
    assert(sample_motion(0,2250000000ull)==2); // Missing update: pause gap.
    assert(sample_motion(1,0)==3);F(object+0x9C,123);
    assert(sample_motion(1,750000000ull)==4); // External position command.
    ctx.r4=int32_t(motion_physical+0x30);ctx.r5=0x20;adapter.hook(rdram,&ctx,0x80033A14,now);
    assert(adapter.model_count()==1 && sample_motion(1,1500000000ull)==5);
    adapter.acquire_simulation(rdram,[](uint32_t)->std::optional<float> {return std::nullopt;});
    assert(sample_motion(1,2250000000ull)==6);
    assert(sample_motion(2,0)==7);
    adapter.hook(rdram,&ctx,0x8003B7D0,now);assert(sample_motion(2,750000000ull)==8);
    const uint32_t captured_actor=object+0x400,captured_physical=object+0x600,capture_slot=object+0x800;
    const auto captured_model=create();const auto captured_lifetime=adapter.lifetime_token(captured_model);assert(captured_lifetime);
    W(captured_actor+0x94)=1;W(captured_actor+0x98)=int32_t(captured_physical);W(captured_actor+0x74)=int32_t(captured_model);
    adapter.bind_actor(rdram,model,capture_slot,captured_actor);
    assert(adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor));
    ctx.r4=int32_t(captured_model);dora3_s0_8003B77C(rdram,&ctx);
    assert(!adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor));
    next_address=captured_model;assert(create()==captured_model);
    assert(adapter.lifetime_token(captured_model)!=captured_lifetime);
    assert(!adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor)); // Same addresses, new allocation.
    adapter.bind_actor(rdram,model,capture_slot,captured_actor);
    assert(adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor));
    ctx.r4=int32_t(captured_physical+0x30);ctx.r5=0x20;adapter.hook(rdram,&ctx,0x80033A14,now);
    assert(!adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor));
    adapter.bind_actor(rdram,model,capture_slot,captured_actor);
    adapter.acquire_simulation(rdram,[&](uint32_t owner)->std::optional<float>{return owner==captured_model?std::optional<float>{3}:std::nullopt;},false);
    assert(adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor)); // Foreign birth cannot revoke capture.
    adapter.acquire_simulation(rdram,scalar_plan);
    assert(!adapter.bound_actor_valid(rdram,model,capture_slot,captured_actor));
    ctx.r4=int32_t(captured_model);dora3_s0_8003B77C(rdram,&ctx);assert(adapter.model_count()==1);
    adapter.acquire_simulation(rdram,scalar_plan);MEM_H(0,int32_t(oscillator))=0;
    {SimulationScope simulation({750000000ull,3000000000ull,0,0});assert(adapter.angle_short(rdram,model,oscillator,41)==10.25f);}
    ctx.r4=int32_t(object+0x30);ctx.r5=0x20;adapter.hook(rdram,&ctx,0x80033A14,now);
    assert(adapter.model_count()==1);
    {SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull});assert(adapter.angle_short(rdram,model,oscillator,41)==20.25f);}
    adapter.acquire_simulation(rdram,scalar_plan);F(alpha,0.1f);
    {SimulationScope simulation({750000000ull,3000000000ull,0,0});assert(adapter.scalar_positive(rdram,model,alpha,R(alpha)));
     F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.8));}
    assert(R(alpha)<0);
    {SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull});assert(adapter.scalar_positive(rdram,model,alpha,R(alpha)));
     F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.8));}
    // Missing updates break the interval (pause); external writes and owner
    // acquisition establish a new baseline even when tick/phase are reused.
    {SimulationScope simulation({750000000ull,3000000000ull,0,2250000000ull});assert(!adapter.scalar_positive(rdram,model,alpha,R(alpha)));}
    F(alpha,0.5f);
    {SimulationScope simulation({750000000ull,3000000000ull,0,2250000000ull});assert(adapter.scalar_positive(rdram,model,alpha,R(alpha)));
     F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.8));}
    adapter.acquire_simulation(rdram,scalar_plan);
    {SimulationScope simulation({750000000ull,3000000000ull,0,0});assert(adapter.scalar_positive(rdram,model,alpha,R(alpha)));
     F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.8));}
    // A paused/excluded model also belongs to the replaced timeline. Reusing
    // its exact old phase and float bits after resume must not reuse baseline.
    F(alpha,0.1f);adapter.acquire_simulation(rdram,scalar_plan);
    {SimulationScope simulation({750000000ull,3000000000ull,0,0});
     F(alpha,adapter.scalar_linear(rdram,model,alpha,R(alpha),-0.8));}
    adapter.acquire_simulation(rdram,[](uint32_t)->std::optional<float> {return std::nullopt;});
    {SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull});assert(!adapter.scalar_positive(rdram,model,alpha,R(alpha)));}
    for(unsigned fps:{60u,73u,144u,360u}) {
        seek(0);B(model+0x7C)=0;F(uint32_t(W(model+0x18))+4,100);SimulationTimeline timeline(3);Nanoseconds previous=0;player_impulses=0;
        for(unsigned frame_index=1;frame_index<=fps;++frame_index) {
            const auto timestamp=second*frame_index/fps;
            timeline.advance(timestamp-previous,[&](const SimulationSlice& slice) {
                SimulationScope simulation(slice);
                dora3_s3_core_801C4650(rdram,&ctx);
                ctx.r4=int32_t(model);ctx.r5=std::bit_cast<uint32_t>(3.f);dora3_s0_8003948C(rdram,&ctx);
            },[](uint64_t){});previous=timestamp;
        }
        assert(player_impulses==1);++crossing_cases;
    }
    F(uint32_t(W(model+0x18))+4,10);
    // Only advancing owners may cap a shared timeline. A newly bound clip or
    // a changed rate must use the pending command before its first advance.
    seek(9);B(model+0x7C)=0;
    const SimulationSlice offered{3000000000ull,3000000000ull,0,0};
    auto plan=[&](float rate) {return [&,rate](uint32_t owner)->std::optional<float> {
        return owner==model?std::optional<float>{rate}:std::nullopt;
    };};
    assert(adapter.clip_boundary_limit(rdram,offered,plan(25))==120000000ull);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,0});step(1,3);}
    assert(adapter.clip_boundary_limit(rdram,offered,plan(25))<120000000ull);
    assert(adapter.clip_boundary_limit(rdram,offered,[](uint32_t)->std::optional<float> {return std::nullopt;})==offered.numerator);
    B(model+0x7C)=8;
    assert(adapter.clip_boundary_limit(rdram,offered,plan(25))==offered.numerator);
    // Stationary/paused clips must not fire the same one-shot on every native
    // frame. Reads inside the same simulation slice remain nondestructive.
    seek(3);B(model+0x7C)=8;
    {SimulationScope simulation({1000000000ull,3000000000ull,0,0});
     assert(adapter.frame_event(rdram,model,3,3,10));assert(adapter.frame_event(rdram,model,3,3,10));}
    {SimulationScope simulation({1000000000ull,3000000000ull,0,1000000000ull});
     ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_80039578(rdram,&ctx);
     ctx.r5=0;dora3_s0_800395BC(rdram,&ctx);
     assert(!adapter.frame_event(rdram,model,3,3,10));}
    seek(3);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,2000000000ull});assert(adapter.frame_event(rdram,model,3,3,10));}
    // A same-value seek and pause/resume must break the high precision clock too.
    seek(2);B(model+0x7C)=8|2;
    {SimulationScope simulation({1000000000ull,3000000000ull,0,0});step(3,3);}
    assert(R(model+0x64)==2 && (B(model+0x7C)&2));
    ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_80039578(rdram,&ctx);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,1000000000ull});step(3,3);}
    assert(R(model+0x64)==3 && !(B(model+0x7C)&2));
    seek(3);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,2000000000ull});step(-3,3);}
    assert(R(model+0x64)==2);
    seek(9);ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_800395BC(rdram,&ctx);
    {SimulationScope simulation({1000000000ull,3000000000ull,0,0});step(25,3);}
    assert(R(model+0x64)==10 && (B(model+0x7C)&2));
    {SimulationScope simulation({1000000000ull,3000000000ull,0,1000000000ull});step(-3,3);}
    assert(R(model+0x64)==9 && !(B(model+0x7C)&2));
    ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_800395BC(rdram,&ctx);seek(0);
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
    ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_80038ABC(rdram,&ctx);
    assert(uint32_t(ctx.r2)==bone);
    assert(std::abs(R(uint32_t(ctx.r2)+0x68)-15.625f)<0.001f);
    // 38ABC is the collider's actual last-render joint cache. Returning the
    // pre-session matrix would freeze it while Native owns every draw bank.
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
    // A host-read handoff can be delayed without advancing the authored pose.
    seek(0);B(model+0x7C)=0;F(uint32_t(W(model+0x18))+4,10);step(3,3);
    adapter.begin_frame(rdram,&ctx,now+25'000'000,0);const auto phase_pose=R(bone+0x14);adapter.end_frame(rdram);
    const auto handoff_begin=now+5*second;adapter.resume_frame_clock(rdram,handoff_begin);
    adapter.begin_frame(rdram,&ctx,handoff_begin,0);assert(R(bone+0x14)==0&&R(model+0x64)==3);adapter.end_frame(rdram);
    adapter.begin_frame(rdram,&ctx,handoff_begin+25'000'000,0);assert(R(bone+0x14)==phase_pose&&R(model+0x64)==3);adapter.end_frame(rdram);
    unsigned original_handoffs=0;
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        seek(0);B(model+0x7C)=0;F(uint32_t(W(model+0x18))+4,100);
        SimulationTimeline clock(divider);Nanoseconds previous=0;
        // First continuous cue was delivered while Native, second lies after exit.
        for(unsigned frame_index=1;frame_index<=3;++frame_index) {
            const auto stamp=second*frame_index/fps;
            clock.advance(stamp-previous,[&](const SimulationSlice& slice){
                SimulationScope explicit_time(slice);step(3,divider);
                (void)adapter.frame_event(rdram,model,R(model+0x64),1,0xABC);
            },[](uint64_t){});previous=stamp;
        }
        const auto fractional=R(model+0x64),rate=3.f;
        adapter.handoff_to_original(rdram);assert(adapter.handoff_active(rdram,model));
        assert(R(model+0x64)==fractional); // No rounding or reset of the actual clip.
        step(rate,divider);assert(R(model+0x64)==fractional+rate);
        const float pending=fractional+1;
        assert(adapter.frame_event(rdram,model,R(model+0x64),pending,0xDEF));
        assert(!adapter.frame_event(rdram,model,R(model+0x64),pending,0xDEF));
        step(rate,divider);assert(!adapter.frame_event(rdram,model,R(model+0x64),pending,0xDEF));
        // A real pause/run change preserves continuation until a seek/rebind.
        ctx.r4=int32_t(model);ctx.r5=1;dora3_s0_80039578(rdram,&ctx);assert(adapter.handoff_active(rdram,model));
        const auto paused_time=R(model+0x64);step(rate,divider);assert(R(model+0x64)==paused_time);
        ctx.r4=int32_t(model);ctx.r5=0;dora3_s0_80039578(rdram,&ctx);assert(adapter.handoff_active(rdram,model));
        step(rate,divider);assert(R(model+0x64)==paused_time+rate);
        seek(0);assert(!adapter.handoff_active(rdram,model));
        // Fresh Original retains equality, including a deliberately skipped cue.
        seek(18);step(3,divider);assert(R(model+0x64)==21);
        assert(!adapter.frame_event(rdram,model,21,20,0x123));
        assert(adapter.frame_event(rdram,model,21,21,0x123));++original_handoffs;
    }
    // Enemy decisions already consumed the Source21 sample while physical time
    // was fractional. Returning to Original advances that view21→24; crossing20
    // must never invent an attack that original18→21 equality skipped.
    seek(18);B(model+0x7C)=0;F(uint32_t(W(model+0x18))+4,100);
    {SimulationScope explicit_time({750000000ull,3000000000ull,0,0});step(3,3);}
    const auto discrete=adapter.source_command_sample(rdram,model,{750000000ull,3000000000ull,0,0});
    assert(discrete.checkpoint.time==21);adapter.remember_source_clock(model,discrete);
    adapter.handoff_to_original(rdram);assert(adapter.handoff_active(rdram,model));
    assert(!adapter.handoff_source_event(rdram,model,R(model+0x64),20));
    assert(adapter.handoff_source_event(rdram,model,R(model+0x64),21));
    step(3,3);assert(R(model+0x64)==21.75f);
    assert(adapter.handoff_source_event(rdram,model,R(model+0x64),24));
    assert(!adapter.handoff_source_event(rdram,model,R(model+0x64),21));
    assert(!adapter.handoff_source_event(rdram,model,18,24)); // Saved pre-seek value remains its own comparison.
    // Renderer bookkeeping does not delete a live continuation.
    adapter.hook(rdram,&ctx,0x8003B7D0,now);assert(adapter.handoff_active(rdram,model));
    seek(2);assert(!adapter.handoff_active(rdram,model));
    F(uint32_t(W(model+0x18))+4,10);seek(0);
    std::cout<<"PASS: "<<original_handoffs<<" fractional Native→Original model/cue continuations, activation rebase without pose/time mutation, authored Source equality18→21 skip and21→24 continuation, pending/delivered cue once, pause/resume/run/seek/renderer reset; actual original SDK counter; no game/GPU.\n";
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
    F(alpha,-0.5f);
    {SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull});assert(!adapter.scalar_positive(rdram,model,alpha,R(alpha)));}
    adapter.begin_frame(rdram,&ctx,now,0);adapter.end_frame(rdram);
    step(3,3);adapter.begin_frame(rdram,&ctx,now+25'000'000,0);
    // Same-pointer recreation must not retain the old clip or observations.
    ctx.r4=int32_t(model);dora3_s0_8003B77C(rdram,&ctx);assert(adapter.model_count()==0 && allocations.empty());adapter.end_frame(rdram);
    next_address=model;const auto recreated=create();assert(recreated==model && adapter.model_count()==1);
    F(alpha,-0.5f);
    {SimulationScope simulation({750000000ull,3000000000ull,0,750000000ull});assert(!adapter.scalar_positive(rdram,model,alpha,R(alpha)));}
    assert(W(model+0x64)==0 && W(model+0x60)==0);adapter.begin_frame(rdram,&ctx,now,0);adapter.end_frame(rdram);
    ctx.r4=int32_t(model);dora3_s0_8003B77C(rdram,&ctx);assert(allocations.empty());adapter.reset();
    std::cout<<"PASS: "<<crossing_cases<<" crossing timelines / actual player frame-18 event; "<<simulation_cases<<" native simulation model timelines with nondestructive events and no render double-advance; "<<angle_cases<<" fractional short-angle wrap schedules; "<<steering_cases<<" latched angular-command schedules; "<<preceding_cases<<" preceding opacity-command schedules; "<<scalar_cases<<" exact-float scalar/retirement timelines with pause/write/acquisition/lifetime invalidation; "<<frames<<" original linear/morph native pose samples at 60/73/120/144/165/360 Hz and both dividers; Hermite curve through original matrix builder, bank isolation, guest context/time/event preservation, pause/seek/clamp/rebind/stale updates, unknown-write rollback, deferred logic morph, draw-time deletion and same-address recreation; no game/GPU.\n";
}

extern "C" int dora3_native_preview_hook(uint8_t*,recomp_context*,uint32_t) {return 0;}
