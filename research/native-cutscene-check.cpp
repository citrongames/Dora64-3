#include "native_cutscene.hpp"
#include "native_simulation_time.hpp"
#include "recomp_context_copy.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
using namespace dora3::native_time;
namespace {Nanoseconds fixture_time=0;bool loop_active=true;unsigned freed=0,text_restores=0,widget_restores=0;}
namespace dora3::native_time {Nanoseconds monotonic_time(){return fixture_time;}}
extern "C" {
int dora3_native_loop_cutscene_active(){return loop_active;}
int dora3_overlay_loaded(uint32_t rom){return rom==0x70A50;}
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t){}
void do_break(uint32_t){std::abort();}
void switch_error(const char*,uint32_t,uint32_t){std::abort();}
void dora3_s3_801E4484(uint8_t*,recomp_context*);
void reference_dora3_s3_801E4484(uint8_t*,recomp_context*);
void reference_dora3_s3_801E3768(uint8_t*,recomp_context*);
void reference_dora3_s3_801E0760(uint8_t*,recomp_context*);
void reference_dora3_s3_801BBDDC(uint8_t*,recomp_context*);
void reference_dora3_s0_8002F064(uint8_t*,recomp_context*);
void dora3_s3_801BBDDC(uint8_t*,recomp_context*);
void dora3_s0_8002F064(uint8_t*,recomp_context*);
void cutscene_fixture_boundary(uint8_t* rdram,recomp_context* ctx,const char* name){
    assert(!active_simulation_slice());const std::string_view n(name);
    if(n=="dora3_memset") {
        for(unsigned i=0;i<unsigned(ctx->r6);++i)MEM_B(int32_t(i),ctx->r4)=uint8_t(ctx->r5);
        ctx->r2=ctx->r4;
    } else if(n=="dora3_s0_80033C50"){assert(uint32_t(ctx->r4)==0x80328000);++freed;}
    else if(n=="dora3_s1_801B5740")++text_restores;
    else if(n=="dora3_common_widgets_init")++widget_restores;
    else std::abort();
}
}
using Image=std::vector<uint32_t>;
constexpr uint32_t cc=0x802183D0,pc=0x80218B10,cam=0x801A99F8,thread=0x80218350;
static uint8_t* bytes(Image& x){return reinterpret_cast<uint8_t*>(x.data());}
static void wf(uint8_t* rdram,uint32_t p,float v){MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
static float rf(uint8_t* rdram,uint32_t p){return std::bit_cast<float>(MEM_W(0,int32_t(p)));}
static void clear(uint8_t* rdram,uint32_t p,unsigned size){for(unsigned i=0;i<size;i+=4)MEM_W(int32_t(i),int32_t(p))=0;}
static recomp_context context(){recomp_context c{};c.r29=int32_t(0x80300000);return c;}
static void finish_geometry(uint8_t* rdram,recomp_context* c,bool ref){
    c->f12.fl=rf(rdram,cc+0x38);c->f14.fl=rf(rdram,cc+0x3C);c->r6=MEM_W(0x40,int32_t(cc));
    (ref?reference_dora3_s3_801BBDDC:dora3_s3_801BBDDC)(rdram,c);
    for(unsigned i=0;i<4;++i)if(MEM_H(0x34,int32_t(cc+i*0x1B8))) {
        const auto mode=MEM_H(0x36,int32_t(cc+i*0x1B8));
        if(mode==0||mode==2){c->r4=int32_t(cam+i*0x34);(ref?reference_dora3_s0_8002F064:dora3_s0_8002F064)(rdram,c);}
    }
}
static void source(uint8_t* rdram,recomp_context* c,bool ref,bool script=false){
    c->f_odd=&c->f0.u32h;
    if(script)(ref?reference_dora3_s3_801E4484:dora3_s3_801E4484)(rdram,c);
    else if(ref){reference_dora3_s3_801E3768(rdram,c);reference_dora3_s3_801E0760(rdram,c);}
    else {SimulationScope whole({3*uint64_t(second),3*uint64_t(second),0,0});native_cutscene_evaluate(rdram,c);return;}
    finish_geometry(rdram,c,ref);
}
static void equal_context(recomp_context a,recomp_context b){a.f_odd=b.f_odd=nullptr;assert(std::memcmp(&a,&b,sizeof(a))==0);}
static void seed(uint8_t* rdram,unsigned duration,unsigned profile){
    clear(rdram,cc,4*0x1B8);clear(rdram,pc,60*0xD4);clear(rdram,cam,4*0x34);
    clear(rdram,thread,8*0x10);clear(rdram,0x801A44F0,8*20);
    MEM_W(0,int32_t(0x80218AD8))=1;MEM_W(0,int32_t(0x80218B08))=int32_t(0x80328000);
    for(auto a:{0x802170ECu,0x802170F0u,0x8021D484u,0x8021D494u,0x801A9C88u})MEM_W(0,int32_t(a))=0;
    MEM_B(0,int32_t(0x80217118))=0;MEM_W(0,int32_t(0x801AA648))=1;MEM_W(0,int32_t(0x801A9500))=1;
    for(unsigned i=0;i<2;++i){auto slot=0x801A44F0+i*20;MEM_W(0,int32_t(slot))=2;MEM_W(12,int32_t(slot))=int(i);MEM_W(16,int32_t(slot))=int32_t(i?0x801BB848:0x801BB730);}
    for(unsigned i=0;i<4;++i){
        auto ctl=cc+i*0x1B8,part=pc+i*0xD4,actor=0x80320000+i*0x200,physics=0x80324000+i*0x200,model=0x80326000+i*0x200;
        clear(rdram,actor,0xCC);clear(rdram,physics,0xF4);clear(rdram,model,0x100);
        MEM_H(0x34,int32_t(ctl))=1;MEM_H(0x36,int32_t(ctl))=int16_t(i);MEM_H(0x10A,int32_t(ctl))=int16_t(i);
        MEM_H(4,int32_t(part))=1;MEM_W(0,int32_t(part))=int32_t(actor);
        MEM_W(0x98,int32_t(actor))=int32_t(physics);MEM_W(0x74,int32_t(actor))=int32_t(model);MEM_W(0x94,int32_t(actor))=1;
        for(unsigned axis=0;axis<3;++axis){
            wf(rdram,part+0x18+axis*4,float(100+axis*30+i*10));wf(rdram,actor+0x9C+axis*4,float(100+axis*30+i*10));
            wf(rdram,part+0x24+axis*4,float(350+axis*2));wf(rdram,part+0x30+axis*4,1);
            wf(rdram,part+0x3C+axis*4,0.5f+axis*0.25f);wf(rdram,part+0x60+axis*4,0.125f);
            wf(rdram,part+0x48+axis*4,3);wf(rdram,part+0x6C+axis*4,0.0625f);
            wf(rdram,part+0x54+axis*4,0.03125f);wf(rdram,part+0x78+axis*4,0.0078125f);
            wf(rdram,ctl+0x38+axis*4,float(140+axis*40+i*10));wf(rdram,ctl+0x44+axis*4,float(25+axis*10));
            wf(rdram,ctl+0x50+axis*4,float(20+axis*5+i));wf(rdram,ctl+0x68+axis*4,float(5+axis));
            for(auto rate:{0x74u,0x80u,0x8Cu,0xA4u})wf(rdram,ctl+rate+axis*4,0.25f);
            for(auto accel:{0xB0u,0xBCu,0xC8u,0xE0u})wf(rdram,ctl+accel+axis*4,0.0625f);
        }
        for(unsigned j=0;j<9;++j)MEM_H(int32_t(6+j*2),int32_t(part))=int16_t(duration);
        for(unsigned j=0;j<15;++j)MEM_H(int32_t(0xEC+j*2),int32_t(ctl))=int16_t(duration);
        for(unsigned j=0;j<6;++j){
            MEM_H(int32_t(0x110+j*2),int32_t(ctl))=1;MEM_H(int32_t(0x11C+j*2),int32_t(ctl))=int16_t(duration);
            wf(rdram,ctl+0x128+j*4,0.25f);wf(rdram,ctl+0x140+j*4,0.0625f);
            MEM_H(int32_t(0x158+j*2),int32_t(ctl))=1;MEM_H(int32_t(0x164+j*2),int32_t(ctl))=int16_t(duration);
            wf(rdram,ctl+0x170+j*4,0.25f);wf(rdram,ctl+0x188+j*4,0.0625f);wf(rdram,ctl+0x1A0+j*4,30);
        }
        wf(rdram,ctl+0x5C,300);wf(rdram,ctl+0x98,-0.5f);wf(rdram,ctl+0xD4,0.0625f);
        wf(rdram,part+0xC8,0.2f);wf(rdram,part+0xCC,0.15f);MEM_H(0xD0,int32_t(part))=7;
        wf(rdram,model+0x78,0.2f);MEM_W(0x74,int32_t(model))=1;
        MEM_H(0xA0,int32_t(part))=1;MEM_H(0xA4,int32_t(part))=2;MEM_H(0xA6,int32_t(part))=2;
        if(profile==1){MEM_H(0xC4,int32_t(part))=int16_t(duration);MEM_H(0xC6,int32_t(part))=int16_t(duration);wf(rdram,part+0xAC,1);wf(rdram,part+0xB0,0.5f);wf(rdram,part+0xB4,0.125f);wf(rdram,part+0xB8,0.0625f);wf(rdram,part+0xBC,-1);wf(rdram,part+0xC0,25);}
        if(profile>=2){MEM_H(0x9C,int32_t(part))=1;wf(rdram,part+0x98,0.5f);wf(rdram,part+0x40,profile==2?5:-47.875f);wf(rdram,part+0x90,0);wf(rdram,part+0x94,0);}
    }
}
static std::vector<uint32_t> publications(uint8_t* rdram){
    std::vector<uint32_t> out{0x800DAE54,0x800FE878,0x800DD3F0};
    auto add=[&](uint32_t a,unsigned size){for(unsigned j=0;j<size;j+=4)out.push_back(a+j);};
    for(unsigned i=0;i<4;++i){auto control=cc+i*0x1B8;auto part=pc+i*0xD4;
        if(MEM_H(0x34,int32_t(control))){add(cam+i*0x34,0x34);add(control+0x38,0x3C);}
        if(MEM_H(4,int32_t(part))){auto actor=uint32_t(MEM_W(0,int32_t(part)));add(part+0x18,0x24);add(actor+0xC,0x18);add(actor+0x9C,0xC);
            if(rf(rdram,part+0xC8)!=-1)add(uint32_t(MEM_W(0x74,int32_t(actor)))+0x74,8);}
    }return out;
}
static void check_publications(Image& actual,Image& expected,const std::vector<uint32_t>& owned,unsigned duration,unsigned profile,unsigned divider,unsigned fps,unsigned tick){
    for(auto address:owned){const auto a=actual[(address&0xFFFFFF)/4],b=expected[(address&0xFFFFFF)/4];
        if(a==b)continue;
        const auto x=std::bit_cast<float>(a),y=std::bit_cast<float>(b);
        if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x-y)>0.003f){std::cerr<<std::hex<<address<<std::dec<<": "<<x<<" != "<<y<<" duration="<<duration<<" profile="<<profile<<" divider="<<divider<<" fps="<<fps<<" tick="<<tick<<"\n";std::abort();}
    }
}
static void program(uint8_t* rdram){
    for(unsigned i=0;i<8;++i){MEM_H(4,int32_t(thread+i*0x10))=i?1:0;MEM_H(8,int32_t(thread+i*0x10))=int16_t(i);}
    MEM_W(0,int32_t(thread))=int32_t(0x80328000);
    // Actual opcodes: activate24, mode25, wait2, separators1 and end0.
    const std::array<int16_t,40> script{24,0,25,0,2,0,5,6,7,2,2,1,25,0,3,1,9,10,11,2,1,1,25,0,1,2,0,1,25,0,0,1,0};
    for(unsigned i=0;i<script.size();++i)MEM_H(int32_t(i*2),int32_t(0x80328000))=script[i];
}
int main(int argc,char** argv){
    assert(argc==2);Image original(0x400000/4);std::ifstream in(argv[1],std::ios::binary);in.read(reinterpret_cast<char*>(original.data()),0x400000);assert(in.gcount()==0x400000);
    unsigned cases=0,endpoints=0,intervals=0,guards=0;
    for(unsigned duration:{0u,1u,7u})for(unsigned profile=0;profile<4;++profile){
        Image initial=original;seed(bytes(initial),duration,profile);auto owned=publications(bytes(initial));
        Image expected=initial,actual=initial;auto c=context(),r=context();source(bytes(expected),&r,true);source(bytes(actual),&c,false);
        if(actual!=expected){for(size_t i=0;i<actual.size();++i)if(actual[i]!=expected[i]){std::cerr<<"whole RAM "<<std::hex<<(0x80000000+i*4)<<"\n";break;}std::abort();}equal_context(c,r);++cases;
        for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u}){
            actual=initial;expected=initial;c=context();r=context();CutsceneTimeline timeline;timeline.begin(bytes(actual),0,divider);
            Nanoseconds previous=0;unsigned next_frame=1;
            for(unsigned tick=1;tick<=8;++tick){
                const Nanoseconds endpoint=(second*divider*tick+59)/60;
                while(second*next_frame/fps<endpoint){
                    auto now=second*next_frame/fps;++next_frame;timeline.advance(bytes(actual),&c,now,native_cutscene_evaluate);
                    auto sample=actual;auto ccopy=c;timeline.advance(bytes(actual),&c,now,native_cutscene_evaluate);assert(sample==actual);equal_context(ccopy,c);
                    Image one=expected;CutsceneTimeline direct;direct.begin(bytes(one),previous,divider);auto dc=context();direct.advance(bytes(one),&dc,now,native_cutscene_evaluate);assert(one==actual);++intervals;
                }
                timeline.advance(bytes(actual),&c,endpoint,native_cutscene_evaluate);
                Image future=expected;auto fc=r;source(bytes(future),&fc,true);check_publications(actual,future,owned,duration,profile,divider,fps,tick);
                timeline.before_source(bytes(actual));assert(actual==expected);equal_context(c,r);
                source(bytes(actual),&c,true);source(bytes(expected),&r,true);assert(actual==expected);equal_context(c,r);++endpoints;
                timeline.after_source(bytes(actual),endpoint);previous=endpoint;owned=publications(bytes(actual));
            }
            timeline.reset(bytes(actual));assert(actual==expected);++cases;
        }
        if(duration==7&&profile==0){
            auto baseline=initial;auto rc=context();source(bytes(baseline),&rc,true);
            auto sample=baseline;auto oc=publications(bytes(sample));auto tc=context();CutsceneTimeline continuous;continuous.begin(bytes(sample),0,3);continuous.advance(bytes(sample),&tc,1,native_cutscene_evaluate);
            check_publications(sample,baseline,oc,duration,profile,3,0,0);
        }
    }
    // Real Source interpreter through a multi-command, multi-wait camera script.
    for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u}){
        Image actual=original,expected;seed(bytes(actual),7,1);program(bytes(actual));expected=actual;
        auto c=context(),r=context();native_cutscene_begin(bytes(actual),0,divider);fixture_time=0;unsigned frame=1;
        unsigned source_calls=0;freed=text_restores=widget_restores=0;
        for(unsigned tick=1;tick<=16;++tick){
            auto end=(second*divider*tick+59)/60;
            while(second*frame/fps<end){fixture_time=second*frame/fps;++frame;native_cutscene_update(bytes(actual),&c,fixture_time);++intervals;}
            fixture_time=end;dora3_native_cutscene_source(bytes(actual),&c,0);assert(actual==expected);
            source(bytes(actual),&c,false,true);source(bytes(expected),&r,true,true);++source_calls;
            assert(actual==expected);equal_context(c,r);++endpoints;
            dora3_native_cutscene_source(bytes(actual),&c,1);
            if(!native_cutscene_eligible(bytes(actual)))break;
        }
        assert(source_calls==8);assert(freed==2&&text_restores==2&&widget_restores==2);assert(!native_cutscene_render_time());
        auto finished=actual;fixture_time+=second;native_cutscene_update(bytes(actual),&c,fixture_time);assert(actual==finished);native_cutscene_reset();++cases;
    }
    Image actual=original;seed(bytes(actual),7,0);auto initial=actual;auto c=context();auto* rdram=bytes(actual);
    assert(native_cutscene_eligible(bytes(actual)));
    // Every new policy's resource/mode/process gate must reject its changed input.
    for(auto a:{0x80218AD8u,0x802170ECu,0x802170F0u,0x8021D484u,0x8021D494u,0x801A9C88u,0x801AA648u,0x801A9500u,0x801A4500u}){
        actual=initial;MEM_W(0,int32_t(a))=MEM_W(0,int32_t(a))?0:1;assert(!native_cutscene_eligible(bytes(actual)));++guards;
    }
    actual=initial;MEM_B(0,int32_t(0x80217118))=1;assert(!native_cutscene_eligible(bytes(actual)));++guards;
    actual=initial;MEM_H(0x36,int32_t(cc))=4;assert(!native_cutscene_eligible(bytes(actual)));++guards;
    actual=initial;MEM_H(0x10A,int32_t(cc+2*0x1B8))=60;assert(!native_cutscene_eligible(bytes(actual)));++guards;
    actual=initial;MEM_W(0,int32_t(pc))=0x100;assert(!native_cutscene_eligible(bytes(actual)));++guards;
    // Pause-style restore, unchanged-value command rebase, external rate change,
    // actor/model replacement and duplicate-time guarantees share the same owner.
    for(unsigned change=0;change<5;++change){
        actual=initial;CutsceneTimeline timeline;timeline.begin(bytes(actual),0,3);timeline.advance(bytes(actual),&c,second/60,native_cutscene_evaluate);
        if(change==0){timeline.reset(bytes(actual));assert(actual==initial);}
        if(change==1){timeline.before_source(bytes(actual));assert(actual==initial);wf(bytes(actual),cc+0x38,rf(bytes(actual),cc+0x38));timeline.after_source(bytes(actual),second/60);auto snap=actual;timeline.advance(bytes(actual),&c,second/60,native_cutscene_evaluate);assert(actual==snap);}
        if(change==2){wf(bytes(actual),cc+0x74,3);timeline.advance(bytes(actual),&c,second/30,native_cutscene_evaluate);assert(rf(bytes(actual),cc+0x74)==3);timeline.reset(bytes(actual));}
        if(change==3){auto* rdram=bytes(actual);for(unsigned off=0;off<0xCC;off+=4)MEM_W(int32_t(off),int32_t(0x80320800))=MEM_W(int32_t(off),int32_t(0x80320000));MEM_W(0,int32_t(pc))=int32_t(0x80320800);auto replacement=actual;timeline.advance(rdram,&c,second/30,native_cutscene_evaluate);for(unsigned off=0;off<0xCC;off+=4)assert(actual[(0x320800+off)/4]==replacement[(0x320800+off)/4]);timeline.reset();}
        if(change==4){auto* rdram=bytes(actual);for(unsigned off=0;off<0x100;off+=4)MEM_W(int32_t(off),int32_t(0x80326800))=MEM_W(int32_t(off),int32_t(0x80326000));MEM_W(0x74,int32_t(0x80320000))=int32_t(0x80326800);auto replacement=actual;timeline.reset(rdram);for(unsigned off=0;off<0x100;off+=4)assert(actual[(0x326800+off)/4]==replacement[(0x326800+off)/4]);}
        ++guards;
    }
    std::cout<<"PASS: "<<cases<<" connected cutscene cases, "<<endpoints<<" exact Source endpoints, "<<intervals<<" Native intervals, "<<guards<<" ownership/policy guards; 52 actual motion functions, all four linked cameras, XYZ/rotation/scale/polar/jump/alpha, Original20/30 at60/73FPS, actual interpreter wait/cut/finish, Source restoration and full-RAM/context duplicate-time checks; ASan/UBSan; no game/GPU.\n";
}
