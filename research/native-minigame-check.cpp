#include "native_minigames.hpp"
#include "native_minigame_math.hpp"
#include "native_minigame_registry.hpp"
#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
#include "recomp_context_copy.hpp"
#include "minigame-functions.h"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace dora3::native_time;
static Nanoseconds clock_now=1000000000;
static unsigned requested=0,commands=0;
static std::uint32_t active_rom=0;
static bool frame_active=false;
static std::filesystem::path seed_directory;
namespace dora3::native_time {
Nanoseconds monotonic_time(){return clock_now;}
const FrameBinding* active_frame_binding(){return nullptr;}
}
unsigned dora3_requested_native_fps(){return requested;}
extern "C" int dora3_overlay_loaded(std::uint32_t rom){return rom==0x18BDF0||rom==active_rom;}
extern "C" int dora3_native_loop_minigame_active(){return frame_active;}
extern "C" int dora3_native_loop_owns_frame_banks(){return frame_active;}
extern "C" void dora3_trace(std::uint8_t*,recomp_context*,std::uint32_t,std::uint32_t){}
extern "C" void switch_error(const char*,std::uint32_t,std::uint32_t){throw std::logic_error("Unseeded Source jump table");}
extern "C" void do_break(std::uint32_t){throw std::logic_error("Source break");}
extern "C" recomp_func_t* get_function(std::int32_t pc){return fixture_function(std::uint32_t(pc));}
// These are explicit command/resource boundaries. Spatial equations and pure
// SDK math execute their real generated bodies. No command is replayed by Native.
void fixture_command(){++commands;}
static void close(double actual,double expected,double tolerance=0.003) {
    if(!(std::abs(actual-expected)<=tolerance)) {
        std::cerr<<"mismatch actual="<<actual<<" expected="<<expected<<"\n";std::abort();
    }
}
struct Memory {
    std::vector<std::uint32_t> data;
    Memory():data(0x400000/4){}
    std::uint8_t* ram(){return reinterpret_cast<std::uint8_t*>(data.data());}
    std::int32_t& W(std::uint32_t p){auto* rdram=ram();return MEM_W(0,std::int32_t(p));}
    std::int16_t& H(std::uint32_t p){auto* rdram=ram();return MEM_H(0,std::int32_t(p));}
    std::int8_t& B(std::uint32_t p){auto* rdram=ram();return MEM_B(0,std::int32_t(p));}
    void F(std::uint32_t p,float v){W(p)=std::bit_cast<std::int32_t>(v);}
    float R(std::uint32_t p){return std::bit_cast<float>(W(p));}
};
static constexpr std::uint32_t actor=0x80171D9C,driver=0x80230000,camera=0x801A99F8;
static void arm(Memory& m,unsigned index,unsigned divider) {
    const auto& d=dora3::minigame::definitions[index];active_rom=d.rom;
    if(index<9) {
        std::ifstream input(seed_directory/(std::to_string(d.rom)+".bin"),std::ios::binary);
        std::vector<char> bytes((std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());assert(input.is_open()&&bytes.size()>0);
        std::memcpy(m.ram()+0x1C3680,bytes.data(),bytes.size());
    }
    for(unsigned i=0;i<8;++i){auto p=0x801A44F0+i*20;m.W(p)=i<2?2:0;m.W(p+12)=i;m.W(p+16)=i<2?std::int32_t(i?d.draw:d.update):0;}
    m.W(0x801A39DC)=divider;m.W(0x801AA648)=m.W(0x801A9500)=1;
    for(unsigned i=0;i<230;++i) {
        const auto p=0x80171CD0+i*0xCC;m.W(p+0x94)=1;m.W(p+0x98)=driver+i*0x100;
        m.F(p+0x18,1);m.F(p+0x1C,1);m.F(p+0x20,1);
    }
    m.F(camera+0x18,50);m.F(camera+0x1C,45);m.F(camera+0x30,2000);
}
static recomp_context context() {
    recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=std::int32_t(0x803F0000);return c;
}
using Function=void(*)(std::uint8_t*,recomp_context*);
static unsigned cases=0,slices=0;
template<class Prepare> static void replay(Memory seed,unsigned index,Function function,Prepare prepare,
                                         std::uint32_t moving_field,double rate,bool angle=false) {
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
        native_minigame_reset();requested=0;Memory original=seed,working=seed;
        arm(original,index,divider);arm(working,index,divider);
        auto a=context(),b=context();prepare(original,a);prepare(working,b);
        const auto& d=dora3::minigame::definitions[index];
        const auto prior=original.R(moving_field);
        function(original.ram(),&a);const auto command_count=commands;
        close(angle?std::remainder(double(original.R(moving_field))-prior,360.):double(original.R(moving_field))-prior,rate);
        requested=fps;dora3_native_minigame_source(working.ram(),&b,d.rom,d.update,0);
        function(working.ram(),&b);dora3_native_minigame_source(working.ram(),&b,d.rom,d.update,1);
        assert(original.data==working.data);assert(native_minigame_eligible(working.ram()));
        const auto source=working.data;const double initial=working.R(moving_field);
        const auto before_commands=commands;
        native_minigame_begin(working.ram(),clock_now,divider);
        const auto origin=clock_now;
        for(unsigned n=1;n<=fps*divider/60+1;++n) {
            clock_now=origin+Nanoseconds(std::min<double>(double(second)*divider/60.,double(second)*n/fps));
            native_minigame_update(working.ram(),&b,clock_now);
            const double h=double(clock_now-origin)*60./(double(second)*divider);
            double expected=initial+rate*h;if(angle)expected-=360*std::floor(expected/360);
            if(std::abs(working.R(moving_field)-expected)>.003)
                std::cerr<<"case="<<index<<" field="<<std::hex<<moving_field<<std::dec<<" initial="<<initial<<" rate="<<rate<<" h="<<h<<"\n";
            close(working.R(moving_field),expected);
            assert(commands==before_commands);++slices;
        }
        native_minigame_reset(working.ram());
        if(working.data!=source) {
            std::cerr<<"restore case="<<index<<" divider="<<divider<<" fps="<<fps<<"\n";
            unsigned shown=0;
            for(unsigned i=0;i<source.size()&&shown<25;++i)if(working.data[i]!=source[i]) {
                std::cerr<<std::hex<<(0x80000000+i*4)<<" source="<<source[i]<<" actual="<<working.data[i]<<std::dec<<"\n";++shown;
            }
            std::abort();
        }
        // Native draws can rewrite actor inputs and matrices. The next Source
        // update must see exact completed Source memory, including local XYZ.
        native_minigame_begin(working.ram(),clock_now,divider);frame_active=true;
        {
            MinigameFrameScope scope(working.ram());
            working.F(actor,500);working.F(actor+0x18,.5f);working.F(actor+0x9C,600);
            working.W(actor+0x24)=0x12345678;working.B(actor+0x6A)=1;
            for(auto p:{0x801AC6D0u,0x801A9530u,0x801A48DCu})working.F(p,3);
        }
        frame_active=false;assert(working.data==source);native_minigame_reset(working.ram());
        assert(command_count<=before_commands);++cases;
    }
}
static void equations() {
    Memory m;MinigameEquations graph;MinigameEquations::Lifetime owner;owner.scene=4;
    using Op=MinigameEquations::Operation;using Role=MinigameEquations::FieldRole;
    const auto source=[&]{graph.begin_source(m.ram(),1,4,clock_now,3);};
    // Linked acceleration precedes position. Check the discrete Euler sum,
    // including fractional partitions and exact Source restoration.
    m.F(driver,10);m.F(actor+0x9C,100);source();
    auto v=graph.load_float(driver,10),a=graph.constant(-2);
    auto next=graph.operation(Op::Add,v,a);graph.store_float(driver,next,8,Role::Motion,owner);m.F(driver,8);
    auto x=graph.load_float(actor+0x9C,100),step=graph.load_float(driver,8);
    graph.store_float(actor+0x9C,graph.operation(Op::Add,x,step),108,Role::Motion,owner);m.F(actor+0x9C,108);
    graph.end_source(m.ram(),clock_now);
    for(double h:{.1,.25,.5,.75,1.}) {graph.sample(m.ram(),clock_now+Nanoseconds(h*50000000));close(m.R(actor+0x9C),108+8*h-h*(h+1));}
    graph.restore_source(m.ram());close(m.R(driver),8);close(m.R(actor+0x9C),108);
    // A Source contact impulse becomes a fixed command, not repeated force.
    source();v=graph.load_float(driver,8);next=graph.operation(Op::Add,v,graph.constant(4));
    graph.store_float(driver,next,12,Role::Command,owner);m.F(driver,12);
    x=graph.load_float(actor+0x9C,108);step=graph.load_float(driver,12);
    graph.store_float(actor+0x9C,graph.operation(Op::Add,x,step),120,Role::Motion,owner);m.F(actor+0x9C,120);
    graph.end_source(m.ram(),clock_now);graph.sample(m.ram(),clock_now+25000000);close(m.R(actor+0x9C),126);close(m.R(driver),12);
    graph.restore_source(m.ram());
    // Multiple velocity terms copied through a scratch position still own
    // a real recurrence. Its moving input is integrated, not a pose blend.
    source();m.F(driver,10);m.F(actor+0x9C,100);
    auto target=graph.time_operand(10,[](double h){return 10+2*h;});
    x=graph.load_float(actor+0x9C,100);
    auto expression=graph.operation(Op::Add,graph.operation(Op::Add,x,target),graph.constant(3));
    graph.store_float(driver+8,expression,113,Role::Derived,owner);m.F(driver+8,113);
    graph.store_float(actor+0x9C,graph.load_float(driver+8,113),113,Role::Motion,owner);m.F(actor+0x9C,113);
    graph.end_source(m.ram(),clock_now);graph.sample(m.ram(),clock_now+25000000);
    close(m.R(actor+0x9C),113+13*.5+.5*1.5);graph.restore_source(m.ram());
    // Affine damping with a moving target must not freeze as an unsupported
    // forcing term. Its full step equals the actual next discrete equation.
    source();m.F(actor+0x9C,100);
    x=graph.load_float(actor+0x9C,100);target=graph.time_operand(10,[](double h){return 10+2*h;});
    expression=graph.operation(Op::Add,graph.operation(Op::Multiply,x,graph.constant(.5)),graph.operation(Op::Multiply,target,graph.constant(.5)));
    graph.store_float(actor+0x9C,expression,55,Role::Motion,owner);m.F(actor+0x9C,55);
    graph.end_source(m.ram(),clock_now);graph.sample(m.ram(),clock_now+50000000);
    close(m.R(actor+0x9C),33.5);graph.restore_source(m.ram());
    // Delayed session activation starts at its actual handoff, preserving the
    // completed Source equation without counting the old host-read wait.
    const auto activated=clock_now+5*second;
    graph.resume_after_handoff(m.ram(),activated);
    graph.sample(m.ram(),activated);close(m.R(actor+0x9C),55);
    graph.sample(m.ram(),activated+50000000);close(m.R(actor+0x9C),33.5);
    graph.restore_source(m.ram());close(m.R(actor+0x9C),55);
    // External command wins and lifetime replacement cancels old publication.
    m.F(actor+0x9C,900);graph.sample(m.ram(),activated+60000000);close(m.R(actor+0x9C),900);
    graph.reset();
}
int main(int argc,char** argv) {
    assert(argc==2);seed_directory=std::filesystem::path(argv[1]).parent_path();Memory seed;std::ifstream stream(argv[1],std::ios::binary);stream.read(reinterpret_cast<char*>(seed.data.data()),0x400000);assert(stream);
    equations();
    replay(seed,0,mini1_effect,[](Memory& m,recomp_context& c){c.r4=1;m.H(0x801CCE50+8+6)=10;},actor+0xA0,5);
    replay(seed,0,mini1_projected,[](Memory& m,recomp_context& c){c.r4=1;c.r5=std::int32_t(actor);const auto d=0x801CCAD0+0xA8;m.F(actor+0x9C,100);m.F(actor+0xA4,100);m.F(d+0x50,100);m.F(d+0x58,100);m.F(d+0x24,3);m.F(d+0x28,2);m.F(d+0x2C,0);m.F(d+0x30,0);},actor+0x9C,3);
    replay(seed,3,mini4_projected,[](Memory& m,recomp_context& c){c.r4=std::int32_t(actor);c.r5=std::int32_t(driver);m.F(actor+0x9C,100);m.F(actor+0xA4,100);m.F(driver,3);m.F(driver+4,2);},actor+0x9C,3);
    replay(seed,1,mini2_oscillation,[](Memory& m,recomp_context&){m.W(0x801C881C)=2;for(unsigned i=0;i<4;++i){m.W(0x801C8990+i*0x54+0x18)=1;m.F(0x801C8990+i*0x54+0x1C,0);}},0x801C8990+0x1C,4);
    replay(seed,2,mini3_camera,[](Memory& m,recomp_context& c){c.r4=0;m.W(0x801C8FE8)=3;},camera+0x30,-30);
    replay(seed,3,mini4_camera,[](Memory& m,recomp_context& c){c.r4=0;m.W(0x801CA4B0)=3;},camera+0x30,-20);
    replay(seed,4,mini5_control,[](Memory& m,recomp_context& c){c.r4=0;c.r5=std::int32_t(driver);m.W(0x801C721C)=2;m.H(driver+0xA)=10;m.H(driver+0xC)=4;m.H(driver+0xE)=0;},0x80173980,4);
    replay(seed,5,mini6_rotation,[](Memory& m,recomp_context& c){c.r4=0;c.r5=0;c.r6=1;c.r7=15;m.W(std::uint32_t(c.r29)+0x10)=1;},actor+0x10,15,true);
    replay(seed,6,mini7_result,[](Memory& m,recomp_context& c){c.r4=0;m.W(0x801C7008)=0;m.H(0x801C7030)=40;m.F(0x80174FD0+8,1000);},0x80174FD0+8,-5);
    replay(seed,7,mini8_physics,[](Memory& m,recomp_context& c){c.r4=std::int32_t(actor);c.r5=std::int32_t(driver);m.F(driver+0x10,3);m.F(driver+0x14,4);m.F(driver+0x18,5);},actor+0x9C,3);
    replay(seed,8,mini9_result,[](Memory& m,recomp_context& c){c.r4=0;c.r5=std::int32_t(actor);m.W(0x801C7DB8)=0;m.H(0x801C7DB6)=40;m.F(0x80175234+8,1000);},0x80175234+8,-5);
    // All common pairs have real Source update entry/exit, exact participant
    // input/wait state and fractional result draw operands.
    for(unsigned index=9;index<12;++index)for(unsigned divider:{2u,3u}) {
        native_minigame_reset();Memory m=seed;arm(m,index,divider);auto c=context();requested=144;
        m.W(0x801C34CC)=21;m.W(0x801C357C)=4;
        for(unsigned i=0;i<4;++i){m.W(0x801C3580+4*i)=21;m.W(0x801C3528+4*i)=1;}
        m.W(0x801C35AC)=2;m.W(0x801C3650)=2;m.W(0x801C3670)=150;m.W(0x801C3674)=150;
        fixture_common(index-9)(m.ram(),&c);auto completed=m.data;
        native_minigame_begin(m.ram(),clock_now,divider);clock_now+=Nanoseconds(double(second)*divider/120);
        native_minigame_update(m.ram(),&c,clock_now);frame_active=true;
        {
            MinigameFrameScope frame(m.ram());
            if(index==11)close(dora3_native_minigame_visual(m.ram(),0x801C3650),3.5);
        }
        frame_active=false;native_minigame_reset(m.ram());assert(m.data==completed);++cases;
    }
    std::cout<<"PASS: all9 minigame modules and3 common callback pairs; "<<cases<<" Source/Native cases, "<<slices<<" fractional slices;20/30Hz and Manual60/73/120/144/165/360; command-once, linked Euler motion, contact impulses, angle wrapping, model/draw input ownership and exact restoration; synthetic resources, no game/GPU\n";
}
