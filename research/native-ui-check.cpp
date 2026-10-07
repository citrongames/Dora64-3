#include "native_ui.hpp"
#include "native_simulation_time.hpp"
#include "native_scene_policy.hpp"
#include "recomp_context_copy.hpp"
#include "native_gameplay_preview.hpp"
#include <algorithm>
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
using Image=std::vector<uint32_t>;
using Function=void(*)(uint8_t*,recomp_context*);
static Nanoseconds now=0;static uint32_t overlay=0;static bool owner=true;
static unsigned cases=0,endpoints=0,samples=0,draws=0,guards=0,sounds=0;
static std::vector<std::array<uint32_t,5>> commands;
static std::optional<double> charge_fraction;
std::optional<double> native_gameplay_preview_source_fraction(){return charge_fraction;}
namespace dora3::native_time {Nanoseconds monotonic_time(){return now;}}
// Previously checked world adapters are ineligible in this UI-only fixture.
bool native_gameplay_preview_eligible(uint8_t*){return false;}
bool native_cutscene_eligible(uint8_t*){return false;}
void native_gameplay_preview_update(uint8_t*,recomp_context*,Nanoseconds){std::abort();}
void native_cutscene_update(uint8_t*,recomp_context*,Nanoseconds){std::abort();}
extern "C" {
int dora3_overlay_loaded(uint32_t);
uint64_t dora3_overlay_generation(uint32_t rom){return dora3_overlay_loaded(rom)?1:0;}
int dora3_overlay_loaded(uint32_t rom){return rom==overlay||rom==0x5E400;}
int dora3_native_loop_owns_fade(){return owner;}
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t){}
void do_break(uint32_t){std::abort();}
void switch_error(const char*,uint32_t,uint32_t){std::abort();}
recomp_func_t* get_function(int32_t){std::abort();}
#define DECL(name) void name(uint8_t*,recomp_context*);void reference_##name(uint8_t*,recomp_context*);
DECL(dora3_s2_menu_801B9F10) DECL(dora3_s2_menu_801BA660) DECL(dora3_s2_menu_801B981C) DECL(dora3_s2_menu_801B96F8) DECL(dora3_s2_menu_801BA858)
DECL(dora3_s2_menu_801BBCEC) DECL(dora3_s2_menu_801BBEB0) DECL(dora3_s2_menu_801BC17C) DECL(dora3_s2_menu_801BC4C0)
DECL(dora3_s2_menu_801C0528) DECL(dora3_s2_menu_801C14B8) DECL(dora3_s2_menu_801C1A90) DECL(dora3_s2_menu_801C1CA0)
DECL(dora3_s2_menu_801BFC44) DECL(dora3_s2_menu_801C22C4) DECL(dora3_s2_menu_801BDEFC) DECL(dora3_lib_80057A80)
DECL(dora3_s1_801B5B04) DECL(dora3_s1_801B6E1C) DECL(dora3_s1_801B7DCC) DECL(dora3_s1_801B7BB4) DECL(dora3_s1_801B63B8)
DECL(dora3_s3_801BB848) DECL(dora3_fade_draw_dispatch) DECL(dora3_fade_tick)
DECL(dora3_s3_801C9E50) DECL(dora3_s3_801C9EBC)
DECL(dora3_scaled_texture_rect) DECL(dora3_s2_menu_801BA0CC) DECL(dora3_s3_801F0CB8)
DECL(dora3_s0_8002DA98) DECL(dora3_s3_801F1598) DECL(dora3_s3_801F4948) DECL(dora3_s3_801F6870)
DECL(dora3_s3_801BE5F0) DECL(dora3_s3_801BE72C) DECL(dora3_s3_801BE7EC)
void ui_fixture_boundary(uint8_t* rdram,recomp_context* c,const char* name){
 const std::string_view n(name);
 if(n=="dora3_color_buffer_setup"||n=="dora3_depth_buffer_clear"){commands.push_back({4,uint32_t(c->r4),0,0,0});return;}
 if(n=="dora3_s0_800444C0"){++sounds;return;}
 if(n=="dora3_s0_menu_8002841C"){c->r2=c->r4;return;}
 if(n=="dora3_texture_rect_state")return;
 if(n=="dora3_texture_info") {
  const std::array<uint32_t,6> ptr{uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7),uint32_t(MEM_W(16,c->r29)),uint32_t(MEM_W(20,c->r29)),uint32_t(MEM_W(24,c->r29))};
  const std::array<int32_t,6> values{int32_t(0x80330000),int32_t(0x80330100),16,16,0,4};
  for(unsigned i=0;i<6;++i){assert(ptr[i]>=0x80000000&&ptr[i]<0x80400000);MEM_W(0,int32_t(ptr[i]))=values[i];}
  return;
 }
 if(n=="dora3_s0_80028C74"||n=="dora3_s0_menu_8002A1B8"||n=="dora3_s0_8002A1B8")return;
 assert(n=="dora3_common_text_draw"||n=="dora3_common_glyph_draw"||n=="dora3_scaled_texture_rect");
 commands.push_back({uint32_t(n=="dora3_common_text_draw"?1:n=="dora3_common_glyph_draw"?2:3),uint32_t(c->r4),uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7)});
}
}
static uint8_t* bytes(Image& a){return reinterpret_cast<uint8_t*>(a.data());}
static int32_t& w(uint8_t* rdram,uint32_t p){return MEM_W(0,int32_t(p));}
static float f(uint8_t* r,uint32_t p){return std::bit_cast<float>(w(r,p));}
static void f(uint8_t* r,uint32_t p,float v){w(r,p)=std::bit_cast<int32_t>(v);}
static void clear(uint8_t* r,uint32_t p,unsigned size){std::memset(r+(p&0xFFFFFF),0,size);}
static recomp_context context(){recomp_context c{};c.r29=int32_t(0x803F0000);return c;}
static void call(Function fn,uint8_t* r,recomp_context& c){c.f_odd=&c.f0.u32h;fn(r,&c);}
static void context_equal(recomp_context a,recomp_context b){a.f_odd=b.f_odd=nullptr;assert(std::memcmp(&a,&b,sizeof(a))==0);}
static void pair(uint8_t* r,uint32_t update,uint32_t draw,unsigned divider){
 clear(r,0x801A44F0,160);for(unsigned i=0;i<2;++i){w(r,0x801A44F0+i*20)=2;w(r,0x801A44FC+i*20)=int(i);w(r,0x801A4500+i*20)=int32_t(i?draw:update);}w(r,0x801A39DC)=divider;w(r,0x801A9C88)=0;
}
static void observed(uint8_t* r,unsigned mask){auto c=context();c.r4=0;dora3_native_ui_source_begin(r,&c);dora3_native_ui_domain(r,mask);c.r21=0;dora3_native_ui_source(r,&c);}
static void begin(uint8_t* r,unsigned divider,unsigned mask){native_ui_reset();observed(r,mask);native_ui_begin(r,now,divider);}
static Nanoseconds interval(unsigned d){return (Nanoseconds(d)*second+59)/60;}
static void visible_equal(uint8_t* a,uint8_t* b,std::initializer_list<uint32_t> addresses){for(auto p:addresses){if(w(a,p)!=w(b,p)){std::cerr<<"visible mismatch "<<std::hex<<p<<std::dec<<" "<<f(a,p)<<" != "<<f(b,p)<<"\n";std::abort();}}}
static const std::array<Function,4> title{dora3_s2_menu_801BBCEC,dora3_s2_menu_801BBEB0,dora3_s2_menu_801BC17C,dora3_s2_menu_801BC4C0};
static const std::array<Function,4> reference_title{reference_dora3_s2_menu_801BBCEC,reference_dora3_s2_menu_801BBEB0,reference_dora3_s2_menu_801BC17C,reference_dora3_s2_menu_801BC4C0};
static void letters(uint8_t* r,unsigned mode,unsigned variant){
 clear(r,0x801C6488,8*0x34);w(r,0x801C647C)=mode;w(r,0x801C66F4)=100;w(r,0x801C6480)=1;
 for(unsigned i=0;i<8;++i){auto p=0x801C6488+i*0x34;for(auto off:{0u,4u,8u,12u})f(r,p+off,off==12?1.f:40.f);
 f(r,p+0x10,float(i<4?10:290));f(r,p+0x14,variant==0?20:variant==1?71:variant==2?-11:48);f(r,p+0x18,i<4?2:-2);f(r,p+0x1C,variant==1?29.5f:2);f(r,p+0x20,variant==3?0:1);f(r,p+0x24,float(i*32+32));f(r,p+0x28,72);w(r,p+0x2C)=variant==3?2:0;w(r,p+0x30)=variant%2;}
}
static void title_math(const Image& seed){
 overlay=0x635C0;
 for(unsigned d:{2u,3u})for(unsigned mode=0;mode<4;++mode)for(unsigned variant=0;variant<4;++variant){
  Image a=seed;auto* r=bytes(a);pair(r,0x801BCEF8,0x801BD078,d);letters(r,mode,variant);auto c=context();
  // The live image begins at a completed Original Source step.
  for(unsigned i=0;i<8;++i){c.r4=i;call(title[mode],r,c);}++w(r,0x801C66F4);
  Image source=a,b=a;auto cb=context();for(unsigned i=0;i<8;++i){cb.r4=i;call(reference_title[mode],bytes(b),cb);}
  begin(r,d,4);auto saved=context();auto render=saved;
  for(unsigned target:{60u,73u})for(unsigned j=1;Nanoseconds(j)*second/target<interval(d);++j){
   const auto stamp=now+Nanoseconds(j)*second/target;
   {UiFrameScope frame(r,&render,stamp);for(unsigned i=0;i<8;++i)assert(std::isfinite(f(r,0x801C6488+i*0x34+4)));}
   assert(a==source);context_equal(saved,render);++samples;
  }
  {UiFrameScope frame(r,&render,now+interval(d));for(unsigned i=0;i<8;++i)for(unsigned off:{0u,4u,8u,12u})visible_equal(r,bytes(b),{0x801C6488+i*0x34+off});}
  assert(a==source);context_equal(saved,render);native_ui_reset();++endpoints;++cases;
 }
 // Independent kick/drift check away from bounce/clamp branches.
 Image a=seed;auto* r=bytes(a);pair(r,0x801BCEF8,0x801BD078,3);letters(r,0,0);begin(r,3,4);auto c=context();
 {UiFrameScope frame(r,&c,now+second/40);const double h=.5;const float expected=float(20+2*h+.5*1*h*(h+1));assert(std::abs(f(r,0x801C648C)-expected)<1e-6);++samples;}
 native_ui_reset();
}
static void game_math(const Image& seed){
 overlay=0x70A50;
 for(unsigned d:{2u,3u})for(unsigned mode=0;mode<4;++mode){
  Image a=seed;auto* r=bytes(a);pair(r,0x801BB730,0x801BB848,d);clear(r,0x801B8AE0,8*0x44);auto c=context();
  for(unsigned i=0;i<8;++i){const auto p=0x801B8AE0+i*0x44;uint8_t* rdram=r;MEM_H(0,int32_t(p))=1;MEM_H(0x40,int32_t(p))=8;MEM_H(0x42,int32_t(p))=3;for(unsigned j=0;j<5;++j){w(r,p+4+j*4)=int(j*20);w(r,p+0x18+j*4)=int(j*20)+(i%2?-87:83);w(r,p+0x2C+j*4)=int(j*20)+(i%2?-10:10)*3;}}
  uint8_t* rdram=r;MEM_H(0,int32_t(0x8021811A))=1;MEM_H(0,int32_t(0x802171A4))=3;MEM_H(0,int32_t(0x80217186))=8;MEM_H(0,int32_t(0x8021719E))=int16_t(mode);
  Image source=a,b=a;auto cb=context();call(reference_dora3_s1_801B5B04,bytes(b),cb);call(reference_dora3_s3_801BE5F0,bytes(b),cb);
  begin(r,d,3);const auto saved=c;
  {UiFrameScope frame(r,&c,now+interval(d));for(unsigned i=0;i<8;++i)for(unsigned j=0;j<5;++j)visible_equal(r,bytes(b),{0x801B8AE0+i*0x44+0x2C+j*4});visible_equal(r,bytes(b),{0x8021811C,0x80217194,0x802171E4,0x80217198});}
  assert(a==source);context_equal(saved,c);++endpoints;
  const auto stamp=now+Nanoseconds(d)*second/120;const double h=double(stamp-now)*60/(d*double(second));
  {UiFrameScope frame(r,&c,stamp);assert(w(r,0x801B8B0C)==int(10*(3+h)));assert(std::abs(f(r,0x8021811C)-(160-(mode==0||mode==2?float((3+h)/8):1-float((3+h)/8))*300))<1e-4);++samples;}
  assert(a==source);observed(r,0);{UiFrameScope frame(r,&c,now+interval(d));assert(a==source);}++guards;
  native_ui_reset();++cases;
 }
}
static void selection_baseline(uint8_t* rdram,recomp_context& c,float phase){
 const double gain=std::bit_cast<double>(uint64_t(LD(0,int32_t(0x801C6350))));c.f12.fl=float(double(phase)*gain);call(reference_dora3_lib_80057A80,rdram,c);f(rdram,0x801C69F4,c.f0.fl*10);
}
static void menu_math(const Image& seed){
 overlay=0x635C0;
 for(unsigned d:{2u,3u})for(unsigned mode=0;mode<3;++mode){
  Image a=seed;auto* r=bytes(a);pair(r,0x801C24A4,0x801C267C,d);clear(r,0x801C69BC,0x48);w(r,0x8017255C)=1;w(r,0x801C69E4)=mode;f(r,0x801C69C8,7);f(r,0x801C69CC,40);f(r,0x801724D0,mode==0?-200:200);w(r,0x801C69C4)=110;w(r,0x801C69E0)=30;auto c=context();selection_baseline(r,c,40);call(dora3_s2_menu_801C14B8,r,c);
  Image source=a,b=a;auto cb=context();selection_baseline(bytes(b),cb,50);call(reference_dora3_s2_menu_801C14B8,bytes(b),cb);begin(r,d,16|32|64);
  {UiFrameScope frame(r,&c,now+interval(d));visible_equal(r,bytes(b),{0x801724CC,0x801724D0});assert(w(r,0x801C69C4)==109&&w(r,0x801C69E0)==29);}
  assert(a==source);native_ui_reset();++endpoints;++cases;
 }
 for(unsigned d:{2u,3u})for(unsigned mode:{0u,1u,2u})for(int alpha:{0,125,250}){
  Image a=seed;auto* r=bytes(a);pair(r,0x801C0BBC,0x801C0C88,d);w(r,0x801C68DC)=mode;w(r,0x801C69A8)=alpha;w(r,0x801C68D8)=2;Image source=a,b=a;auto c=context(),cb=context();call(reference_dora3_s2_menu_801C0528,bytes(b),cb);begin(r,d,8);
  {UiFrameScope frame(r,&c,now+interval(d));assert(w(r,0x801C69A8)==w(bytes(b),0x801C69A8));}assert(a==source);native_ui_reset();++endpoints;++cases;
 }
}
static void text_seed(uint8_t* rdram){
 clear(rdram,0x801B8AE0,0x400);clear(rdram,0x801B8EB8,0x50);
 w(rdram,0x801B8940)=int32_t(0x80320000);w(rdram,0x801B8D80)=int32_t(0x80321000);w(rdram,0x801B8D84)=0;w(rdram,0x801B8D90)=0;w(rdram,0x801B8D0C)=int32_t(0x80330000);w(rdram,0x801B8944)=int32_t(0x80330000);w(rdram,0x801AEC00)=int32_t(0x80330000);
 for(unsigned i=0;i<16;++i){w(rdram,0x80320000+i*4)=0;MEM_B(int32_t(i*2),int32_t(0x80321000))=0x82;MEM_B(int32_t(i*2+1),int32_t(0x80321000))=0xA0;}MEM_B(32,int32_t(0x80321000))=0;MEM_B(33,int32_t(0x80321000))=0;MEM_B(0,int32_t(0x801B374A))=7;
 const auto p=int32_t(0x801B8D98);MEM_H(0,p)=1;MEM_H(0x14,p)=0;MEM_H(0x16,p)=8;MEM_H(0x18,p)=1;MEM_H(0x1A,p)=2;MEM_H(0x1C,p)=1;MEM_H(0x20,p)=2;w(rdram,uint32_t(p)+4)=32;w(rdram,uint32_t(p)+8)=40;w(rdram,uint32_t(p)+0xC)=0;
 // Active dialogue with the Original continue-arrow path; token draw empty.
 MEM_B(0,int32_t(0x801B8EB8))=1;w(rdram,0x801B8D2C)=1;MEM_H(0x22,int32_t(0x801B8D50))=0;
}
static void map_compare_commands(uint8_t*,uint8_t*,uint32_t);
static unsigned map_sampling_commands(uint8_t*,uint32_t,uint32_t=0);
static void draw_pair(Image a,Function fn,Function ref,uint32_t address,unsigned size,unsigned divider,uint32_t update,uint32_t draw){
 auto* r=bytes(a);pair(r,update,draw,divider);w(r,0x801B1424)=int32_t(0x80360000);Image b=a;auto c=context(),cb=context();
 commands.clear();sounds=0;call(ref,bytes(b),cb);call(ref,bytes(b),cb);const auto expected=commands;const auto sound_count=sounds;const auto command_end=w(bytes(b),0x801B1424);assert(command_end>=int32_t(0x80360000)&&command_end<int32_t(0x80380000));
 begin(r,divider,0);commands.clear();sounds=0;
 {UiFrameScope frame(r,&c,now);call(fn,r,c);call(fn,r,c);}assert(commands==expected);assert(sounds==sound_count);assert(std::memcmp(r+(address&0xFFFFFF),bytes(b)+(address&0xFFFFFF),size)==0);assert(w(r,0x801B1424)==command_end);
 const Image source=a;
 for(unsigned target:{60u,73u})for(unsigned j=1;Nanoseconds(j)*second/target<interval(divider);++j){
  w(r,0x801B1424)=int32_t(0x80360000);c=context();commands.clear();const auto before_sound=sounds;
  {UiFrameScope frame(r,&c,now+Nanoseconds(j)*second/target);call(fn,r,c);call(fn,r,c);}
  assert(commands==expected);assert(sounds==before_sound);assert(std::memcmp(r+(address&0xFFFFFF),bytes(b)+(address&0xFFFFFF),size)==0);assert(w(r,0x801B1424)==command_end);
  if(fn==dora3_s2_menu_801BA858) {
    map_compare_commands(r,bytes(b),uint32_t(command_end));
    assert(map_sampling_commands(r,uint32_t(command_end))>0);
   } else assert(std::memcmp(r+0x360000,bytes(b)+0x360000,unsigned(command_end-int32_t(0x80360000)))==0);++draws;
 }
 // A new Source epoch advances exactly the next Original draw, not repeats.
 w(r,0x801B1424)=int32_t(0x80360000);b=source;w(bytes(b),0x801B1424)=int32_t(0x80360000);c=context();cb=context();call(ref,bytes(b),cb);observed(r,0);
 {UiFrameScope frame(r,&c,now);call(fn,r,c);}assert(std::memcmp(r+(address&0xFFFFFF),bytes(b)+(address&0xFFFFFF),size)==0);
 native_ui_reset();++cases;
}
static void drawing(const Image& game,const Image& menu){
 for(unsigned d:{2u,3u}){
  overlay=0x70A50;Image a=game;text_seed(bytes(a));draw_pair(a,dora3_s1_801B6E1C,reference_dora3_s1_801B6E1C,0x801B8D98,8*0x24,d,0x801BB730,0x801BB848);
  a=game;text_seed(bytes(a));draw_pair(a,dora3_s1_801B7DCC,reference_dora3_s1_801B7DCC,0x801B8D10,0x34,d,0x801BB730,0x801BB848);
  // Choice token-94 uses 8D08/8D88, unlike the continue-arrow fixture above.
   for(int selected:{0,1})for(int direction:{0,1})for(int offset:{-1,0,1}) {
    a=game;auto* r=bytes(a);auto* rdram=r;text_seed(r);w(r,0x801B8D2C)=0;w(r,0x801B8948)=0;
    w(r,0x801B8950)=-94;MEM_H(0x22,int32_t(0x801B8D50))=1;MEM_H(0x2C,int32_t(0x801B8D50))=int16_t(selected);
    w(r,0x801B8D08)=direction;w(r,0x801B8D88)=offset;
    draw_pair(a,dora3_s1_801B7DCC,reference_dora3_s1_801B7DCC,0x801B8D08,0x90,d,0x801BB730,0x801BB848);
   }
   a=game;text_seed(bytes(a));draw_pair(a,dora3_s1_801B63B8,reference_dora3_s1_801B63B8,0x801B8D10,0x30,d,0x801BB730,0x801BB848);
  overlay=0x635C0;a=menu;w(bytes(a),0x801C69D4)=115;w(bytes(a),0x801C69EC)=24;draw_pair(a,dora3_s2_menu_801C22C4,reference_dora3_s2_menu_801C22C4,0x801C69EC,4,d,0x801C24A4,0x801C267C);
  for(unsigned mode:{0u,2u,6u,8u}){a=menu;auto* r=bytes(a);clear(r,0x801C673C,0x18C);w(r,0x801C673C)=mode;w(r,0x801C6820)=3;w(r,0x801C682C)=4;w(r,0x801C683C)=5;draw_pair(a,dora3_s2_menu_801BDEFC,reference_dora3_s2_menu_801BDEFC,0x801C673C,0x18C,d,0x801BFAA4,0x801BFC44);}
 }
}
static void complete_files_draw(const Image& menu){
 overlay=0x635C0;
 for(unsigned d:{2u,3u})for(unsigned mode:{0u,1u,2u,5u,6u,8u})for(int blink:{1,2,3}){
  Image a=menu;auto* r=bytes(a);clear(r,0x801C6700,0x1C8);
  w(r,0x801C673C)=int(mode);w(r,0x801C6700)=32;w(r,0x801C6884)=2;
  w(r,0x801C68C0)=blink;w(r,0x801C6840)=2;w(r,0x801C681C)=3;
  w(r,0x801C6820)=3;w(r,0x801C682C)=4;w(r,0x801C683C)=5;
  w(r,0x801AEC00)=int32_t(0x80330000);
  draw_pair(a,dora3_s2_menu_801BFC44,reference_dora3_s2_menu_801BFC44,0x801C6700,0x1C8,d,0x801BFAA4,0x801BFC44);
 }
}
static void map_math(const Image& seed){
 overlay=0x635C0;
 for(unsigned d:{2u,3u})for(int old:{0,7,13,19,23})for(int next:{1,8,14,20,23})for(int step=0;step<13;++step){
  Image a=seed;auto* r=bytes(a);pair(r,0x801BB52C,0x801BB648,d);
  w(r,0x801B369C)=old;w(r,0x801C6390)=next;w(r,0x801C638C)=step;
  w(r,0x801C636C)=0;w(r,0x801C6398)=0;w(r,0x801C63A8)=2;auto c=context();
  call(dora3_s2_menu_801B9F10,r,c);call(dora3_s2_menu_801BA660,r,c);
   Image source=a,b=a;auto cb=context();w(bytes(b),0x801C638C)=std::max(0,step-1);
  call(reference_dora3_s2_menu_801B9F10,bytes(b),cb);call(reference_dora3_s2_menu_801BA660,bytes(b),cb);
  begin(r,d,128|256|512);assert(native_ui_menu_eligible(r));
  const int oldpos=w(r,0x801C29C4+152*old),newpos=w(r,0x801C29C4+152*next),phase=step-(oldpos==newpos?3:9);
  {UiFrameScope frame(r,&c,now+interval(d));
   if(step>0&&step<8)visible_equal(r,bytes(b),{0x801C6370,0x801C6380});
   if(phase>0&&phase<=3)visible_equal(r,bytes(b),{0x801C6364,0x801C6368});
   assert(w(r,0x801C63A8)==3);
  }assert(a==source);++endpoints;
   for(unsigned target:{60u,73u,120u,144u,165u,360u})for(unsigned j=1;Nanoseconds(j)*second/target<interval(d);++j){
    const auto stamp=now+Nanoseconds(j)*second/target;
    const double h=double(stamp-now)*60./(d*double(second));
    {UiFrameScope frame(r,&c,stamp);
     assert(w(r,0x801C638C)==step&&w(r,0x801B369C)==old&&w(r,0x801C6390)==next);
     if(step>0&&step<8)for(uint32_t address:{0x801C6370u,0x801C6380u}) {
      const auto expected=int(float(double(w(bytes(source),address))+(w(bytes(b),address)-w(bytes(source),address))*h));
      assert(w(r,address)==expected);
     }
    }
   assert(a==source);++samples;
  }native_ui_reset();++cases;
 }
 for(unsigned d:{2u,3u}){
  Image a=seed;auto* r=bytes(a);w(r,0x801C6384)=2;
  draw_pair(a,dora3_s2_menu_801B96F8,reference_dora3_s2_menu_801B96F8,0x801C6384,4,d,0x801BB52C,0x801BB648);
  a=seed;r=bytes(a);w(r,0x801B369C)=0;w(r,0x801C6364)=120;w(r,0x801C6368)=100;w(r,0x801C6370)=w(r,0x801C6380)=0;w(r,0x801C6388)=1;f(r,0x801C6378,0);w(r,0x801AEC00)=int32_t(0x80330000);
  draw_pair(a,dora3_s2_menu_801BA858,reference_dora3_s2_menu_801BA858,0x801C6378,0x14,d,0x801BB52C,0x801BB648);
 }
}
static void world_menu_models(const Image& seed){
 overlay=0x70A50;
 for(unsigned divider:{2u,3u})for(unsigned menu:{1u,2u})for(unsigned fps:{60u,73u,144u,360u}){
  Image source=seed;auto* r=bytes(source);pair(r,0x801BB730,0x801BB848,divider);
  w(r,0x8021D484)=int(menu);w(r,0x802170EC)=w(r,0x802170F0)=0;
  constexpr uint32_t selected=0x80192130;w(r,selected+0x94)=1;
  for(unsigned off:{0x18u,0x1Cu,0x20u})f(r,selected+off,1);
  f(r,selected+0x10,358);auto c=context();c.r4=int32_t(selected);call(dora3_s0_8002DA98,r,c);
  assert(native_ui_menu_eligible(r));const auto* policy=select_native_scene_policy(r);assert(policy&&policy->update==nullptr);
  begin(r,divider,1024);const auto saved=c;const auto baseline=source;
  for(unsigned n=1;Nanoseconds(n)*second/fps<=interval(divider);++n){
   const auto stamp=now+Nanoseconds(n)*second/fps;Image expected=source;auto* e=bytes(expected);auto math=context();
   const double phase=std::clamp(double(stamp-now)*60./(divider*double(second)),0.,1.);
   f(e,selected+0x10,float(std::fmod(358.+(menu==1?4:10)*phase,360.)));
   math.r4=int32_t(selected);call(reference_dora3_s0_8002DA98,e,math);
   {UiFrameScope frame(r,&c,stamp);
    for(unsigned k=0;k<16;++k)assert(w(r,selected+0x24+4*k)==w(e,selected+0x24+4*k));
    assert(f(r,selected+0x10)==f(e,selected+0x10));
   }
   assert(source==baseline);
   context_equal(c,saved);++samples;
  }
  // Full step uses the actual Source spin helper as the endpoint oracle.
  Image expected=source;auto original=context();call(menu==1?reference_dora3_s3_801F1598:reference_dora3_s3_801F4948,bytes(expected),original);
  original.r4=int32_t(selected);call(reference_dora3_s0_8002DA98,bytes(expected),original);
  {UiFrameScope frame(r,&c,now+interval(divider));
   assert(f(r,selected+0x10)==f(bytes(expected),selected+0x10));
   for(unsigned k=0;k<16;++k)assert(w(r,selected+0x24+4*k)==w(bytes(expected),selected+0x24+4*k));
  }
  native_ui_reset();++cases;++endpoints;
 }
}
// Actual Original input gate, unlock search and integer phase recurrence.
// Extra Native frames must neither accept another choice nor consume its wait.
static void character_carousel_cadence(const Image& seed) {
 overlay=0x70A50;
 for(unsigned divider:{2u,3u})for(unsigned fps:{60u,73u,120u,144u,165u,360u})
 for(unsigned initial:{0u,2u,5u})for(int direction:{-1,1})for(bool skip_locked:{false,true}) {
  Image a=seed,original;auto* rdram=bytes(a);pair(rdram,0x801BB730,0x801BB848,divider);
  w(rdram,0x8021D484)=3;w(rdram,0x802170EC)=w(rdram,0x802170F0)=0;
  MEM_B(0,int32_t(0x8021BFF4))=int8_t(initial);MEM_H(0,int32_t(0x8021BF70))=0;
  MEM_H(0,int32_t(0x801AC69C))=0;MEM_H(0,int32_t(0x801B3670))=direction<0?0x100:0x200;
  for(unsigned i=0;i<6;++i) {
   MEM_B(0,int32_t(0x801B361B+2*i))=1;MEM_B(0,int32_t(0x801B361C+2*i))=1;
   for(unsigned group=0;group<2;++group) {
    const uint32_t actor=(group?0x8018FFB8u:0x8018FAF0u)+i*0xCC,driver=0x80310000u+(group*6+i)*0x80;
    clear(rdram,actor,0xCC);clear(rdram,driver,0x80);
    w(rdram,actor+0x94)=1;w(rdram,actor+0x98)=int32_t(driver);MEM_H(0x34,int32_t(driver))=int16_t(60*i);
    for(unsigned offset:{0x18u,0x1Cu,0x20u})f(rdram,actor+offset,1);
   }
  }
  if(skip_locked)MEM_B(0,int32_t(0x801B361B+2*((int(initial)+direction+6)%6)))=0;
  original=a;auto c=context(),ref=context();const auto start=now;
  std::optional<Nanoseconds> first_selection,first_unlocked;
  for(unsigned tick=0;tick<16;++tick) {
   now=start+Nanoseconds((uint64_t(second)*divider*tick+59)/60);
   const int prior_phase=MEM_H(0,int32_t(0x8021BF70)),prior_selected=MEM_B(0,int32_t(0x8021BFF4));
   const auto sounds_before=sounds;call(dora3_s3_801F6870,rdram,c);const auto source_sounds=sounds-sounds_before;
   const auto ref_sounds_before=sounds;call(reference_dora3_s3_801F6870,bytes(original),ref);
   assert(source_sounds==sounds-ref_sounds_before);context_equal(c,ref);assert(a==original);
   const int phase=MEM_H(0,int32_t(0x8021BF70)),selected=MEM_B(0,int32_t(0x8021BFF4));
   if(prior_phase){assert(selected==prior_selected&&phase==prior_phase/2);assert(source_sounds==0);}
   if(!first_selection&&selected!=int(initial))first_selection=now;
   if(first_selection&&!phase&&!first_unlocked)first_unlocked=now;
   assert(native_ui_menu_eligible(rdram));const auto* policy=select_native_scene_policy(rdram);assert(policy&&policy->update==nullptr);
   begin(rdram,divider,4096);const Image saved=a;const auto saved_context=c;const auto saved_sounds=sounds;
   Image next=a;auto math=context();call(reference_dora3_s3_801F6870,bytes(next),math);sounds=saved_sounds;
   // Stop before the following Source update. A phase0 menu may accept new
   // input there, so its next-frame choice is not a visual endpoint oracle.
   for(unsigned n=0;Nanoseconds(n)*second/fps<interval(divider);++n) {
    const auto stamp=now+Nanoseconds(n)*second/fps;
    {UiFrameScope frame(rdram,&c,stamp);
     assert(MEM_H(0,int32_t(0x8021BF70))==phase&&MEM_B(0,int32_t(0x8021BFF4))==selected);
     assert(MEM_HU(0,int32_t(0x801B3670))==unsigned(direction<0?0x100:0x200));
     assert(sounds==saved_sounds);context_equal(c,saved_context);
    }
    assert(a==saved);++samples;
   }
   if(phase) {
    {UiFrameScope frame(rdram,&c,now+interval(divider));
     for(unsigned group=0;group<2;++group)for(unsigned i=0;i<6;++i) {
      const uint32_t actor=(group?0x8018FFB8u:0x8018FAF0u)+i*0xCC;
      for(unsigned offset:{0u,8u})assert(std::abs(f(rdram,actor+offset)-f(bytes(next),actor+offset))<.03f);
     }
    }
    assert(a==saved);++endpoints;
   }
   native_ui_reset();++cases;
  }
  assert(first_selection&&first_unlocked);
  // Adjacent selections take six authored updates including the input tick;
  // skipping one locked slot takes seven. Check duration in20/30Hz time.
  const unsigned waits=skip_locked?6:5;
  assert(std::abs(double(*first_unlocked-*first_selection)-double(second)*divider*waits/60.)<=1.);
  now=start;
 }
}
static void charge_rdp(const Image& seed){
 overlay=0x70A50;
 for(unsigned divider:{2u,3u})for(int count:{0,1,13,24,49,50})for(bool held:{false,true}){
  native_ui_reset();Image a=seed;auto* r=bytes(a);auto* rdram=r;pair(r,0x801BB730,0x801BB848,divider);
  w(r,0x8021D484)=0;w(r,0x801B36E0)=0;MEM_B(0,int32_t(0x801FA5D9))=2;
  w(r,0x8021D698)=count;MEM_H(0,int32_t(0x8021D644))=held?0x4000:0;
  MEM_H(0,int32_t(0x80218178))=0;w(r,0x801AEC00)=int32_t(0x80330000);
  auto c=context();call(dora3_s3_801C9E50,r,c);Image initial=a,original=a;
  w(bytes(original),0x801B1424)=int32_t(0x80360000);auto oracle=context();
  call(reference_dora3_s3_801C9EBC,bytes(original),oracle);
  const auto end=uint32_t(w(bytes(original),0x801B1424));assert(end>0x80360000);
  const auto command=end-8;const auto expected=uint32_t(w(bytes(original),command));assert(expected>>24==0xF6);
  begin(r,divider,0);
  for(double fraction:{0.,.125,.5,.875,1.}){
   charge_fraction=fraction;w(r,0x801B1424)=int32_t(0x80360000);c=context();
   {UiFrameScope frame(r,&c,now+interval(divider)/2);call(dora3_s3_801C9EBC,r,c);}
   const auto actual=uint32_t(w(r,command));assert(uint32_t(w(r,0x801B1424))==end);
   for(uint32_t at=0x80360000;at<end;at+=4)if(at!=command)assert(w(r,at)==w(bytes(original),at));
   assert((actual&~0x00FFF000u)==(expected&~0x00FFF000u));
   if(held&&count>0&&count<50){
    const double right=double((actual>>12)&0xFFF)/4.;
    assert(right>=187.+66.*count/50.-.25&&right<=187.+66.*(count+1)/50.);
    if(fraction==.5)assert(((actual>>12)&3)!=0||count==24);
   }else assert(actual==expected);
   assert(w(r,0x8021D698)==count&&MEM_HU(0,int32_t(0x8021D644))==(held?0x4000:0));++samples;
  }
  charge_fraction.reset();native_ui_reset();
  // The Source counter and width have not been borrowed by the RDP hook.
  for(auto at:{0x8021D698u,0x80218124u})assert(w(r,at)==w(bytes(initial),at));
  ++cases;
 }
}
// Minimal CPU decoder follows the pinned RT64 drawRect/FixedRect and sampler
// formulae. It checks actual emitted commands; this is not a renderer/GPU proof.
static unsigned map_sampling_commands(uint8_t* rdram,uint32_t end,uint32_t asset) {
 unsigned rectangles=0;uint32_t tile=0,bounds=0;
 for(uint32_t pos=0x80360000;pos<end;pos+=8) {
  const auto cmd=uint32_t(w(rdram,pos));
  if(cmd>>24==0xF5&&((uint32_t(w(rdram,pos+4))>>24)&7)==0)tile=uint32_t(w(rdram,pos+4));
  if(cmd==0xF2000000u)bounds=uint32_t(w(rdram,pos+4));
  if(cmd>>24!=0xE4)continue;
  assert((tile&0x00080200u)==0x00080200u); // S/T clamp from actual render tile.
  const auto upper=uint32_t(w(rdram,pos+4));
  const int left=(upper>>12)&0xFFF,top=upper&0xFFF,right=(cmd>>12)&0xFFF,bottom=cmd&0xFFF;
  const int max_s=((bounds>>12)&0xFFF)/4,max_t=(bounds&0xFFF)/4;
  assert(right-left==4*(max_s+1)&&bottom-top==4*(max_t+1));
  const int span_x=((right+3)>>2)-((left+3)>>2),span_y=((bottom+3)>>2)-((top+3)>>2);
  assert(span_x==max_s+1&&span_y==max_t+1); // Ceil geometry cannot change UV rate.
  assert(uint32_t(w(rdram,pos+8))==0xE1000000u&&uint32_t(w(rdram,pos+16))==0xF1000000u);
  const auto uv=uint32_t(w(rdram,pos+12)),delta=uint32_t(w(rdram,pos+20));
  const int ds=int16_t(delta>>16),dt=int16_t(delta);assert(ds==1024&&dt==1024);
  const double u=int16_t(uv>>16)/32.,v=int16_t(uv)/32.+(top&3?(dt>>5)/32.:0.);
  assert(u==0.&&v==0.); // Same first texel at every fractional screen position.
  for(double phase:{0.,.125,.5,.875,1.}) {
   const auto sx=std::clamp(int(std::floor(u+span_x*phase)),0,max_s);
   const auto sy=std::clamp(int(std::floor(v+span_y*phase)),0,max_t);
   assert(sx>=0&&sx<=max_s&&sy>=0&&sy<=max_t);
   if(asset&&phase==1.) {
    const unsigned colors=MEM_HU(6,int32_t(asset));
    const int width=MEM_W(8,int32_t(asset)),height=MEM_W(12,int32_t(asset));
    assert(width==32&&height==32&&colors==16&&sx==31&&sy==31);
    const uint32_t pixel=unsigned(sy*width+sx);const auto packed=MEM_BU(int32_t(pixel/2),int32_t(asset+16+2*colors));
    const unsigned index=pixel&1?packed&15:packed>>4;
    assert(!(MEM_HU(int32_t(2*index),int32_t(asset+16))&1)); // Real ROM bottom/right edge is transparent.
   }
   ++samples;
  }
  ++rectangles;
 }
 return rectangles;
}
static void map_compare_commands(uint8_t* r,uint8_t* original,uint32_t end) {
 for(uint32_t pos=0x80360000;pos<end;pos+=8) {
  const auto prior=uint32_t(w(original,pos)),actual=uint32_t(w(r,pos));
  const auto prior1=uint32_t(w(original,pos+4)),actual1=uint32_t(w(r,pos+4));
  if(prior>>24==0xE4) {
   assert((actual&0xFF000000u)==(prior&0xFF000000u)&&(actual1&0xFF000000u)==(prior1&0xFF000000u));
   const int dx=int((actual1>>12)&0xFFF)-int((prior1>>12)&0xFFF),dy=int(actual1&0xFFF)-int(prior1&0xFFF);
   assert(int((actual>>12)&0xFFF)-int((prior>>12)&0xFFF)==dx);
   const int bottom_shift=int(actual&0xFFF)-int(prior&0xFFF);assert(bottom_shift==dy||bottom_shift==dy+1);
  }else if(prior>>24==0xF5) {
   assert(actual==prior);assert(actual1==prior1||actual1==(prior1|0x00080200u));
  }else if(prior==0xF2000000u&&pos+8<end&&uint32_t(w(original,pos+8))>>24==0xE4) {
   assert(actual==prior&&((actual1>>12)&0xFFF)==((prior1>>12)&0xFFF)-4&&(actual1&0xFFF)==(prior1&0xFFF)-4);
  }else if(prior==0xE1000000u) {
   assert(actual==prior&&(actual1&0xFFFF0000u)==(prior1&0xFFFF0000u));
   const auto top=uint32_t(w(r,pos-4))&0xFFF;assert(int16_t(actual1)==int16_t(prior1)-(top&3?32:0));
  }else {assert(actual==prior&&actual1==prior1);}
 }
}
static void map_chunk_rectangles(const Image& seed) {
 overlay=0x635C0;
 for(unsigned divider:{2u,3u})for(float x:{20.f,20.125f,20.5f,20.875f})for(float y:{30.f,30.125f,30.25f,30.5f,30.875f})for(unsigned sprite:{0u,1u,2u}) {
  Image a=seed;auto* r=bytes(a);pair(r,0x801BB52C,0x801BB648,divider);
  w(r,0x801B369C)=w(r,0x801C6390)=w(r,0x801C638C)=0;
  begin(r,divider,128|256);auto c=context();auto* rdram=r;
  const uint32_t asset=sprite?0x80340000+0x400*(sprite-1):0;
  const unsigned width=asset?unsigned(w(r,asset+8)):16,height=asset?unsigned(w(r,asset+12)):128;
  auto setup=[&](recomp_context& target,float px,float py){
   target.f12.fl=px;target.f14.fl=py;target.r6=width;target.r7=height;
   MEM_W(0x10,target.r29)=MEM_W(0x14,target.r29)=std::bit_cast<int32_t>(1.f);
   MEM_W(0x18,target.r29)=int32_t(asset?asset+48:0x80330000);
   MEM_W(0x1C,target.r29)=int32_t(asset?asset+16:0x80330100);MEM_W(0x20,target.r29)=0;
  };
  w(r,0x801B1424)=int32_t(0x80360000);setup(c,std::floor(x),std::floor(y));
  call(reference_dora3_scaled_texture_rect,r,c);const auto end=uint32_t(w(r,0x801B1424));
  Image original=a;
  w(r,0x801B1424)=int32_t(0x80360000);c=context();setup(c,x,y);
  {UiFrameScope frame(r,&c,now+interval(divider)/2);
   assert(dora3_native_ui_map_bitmap_begin(r,&c,0));assert(c.f12.fl==std::floor(x)&&c.f14.fl==std::floor(y));
   call(dora3_scaled_texture_rect,r,c);dora3_native_ui_map_bitmap_end(r);
  }
  assert(uint32_t(w(r,0x801B1424))==end);map_compare_commands(r,bytes(original),end);
  const auto rectangles=map_sampling_commands(r,end,asset);assert(asset?rectangles==1:rectangles>=2);
  uint32_t previous_bottom=0;
  for(uint32_t pos=0x80360000;pos<end;pos+=8)if(uint32_t(w(r,pos))>>24==0xE4) {
   const auto top=uint32_t(w(r,pos+4))&0xFFF;if(previous_bottom)assert(top==previous_bottom);
   previous_bottom=uint32_t(w(r,pos))&0xFFF;
  }
  native_ui_reset();++cases;++endpoints;
 }
}
static void item_side_arrows(const Image& seed) {
 overlay=0x70A50;
 for(unsigned d:{2u,3u}) {
  Image a=seed;auto* r=bytes(a);auto* rdram=r;pair(r,0x801BB730,0x801BB848,d);
  MEM_B(0,int32_t(0x8021BE4D))=5;MEM_B(0,int32_t(0x8021BE49))=0;
  // All asset IDs referenced by this draw resolve to a small synthetic
  // byte bitmap. The actual payload/palette getters remain real Source code.
  for(unsigned id=0;id<2048;++id)w(r,0x801AEC00+20*id)=int32_t(0x80340000);
  MEM_B(3,int32_t(0x80340000))=2;
  MEM_H(8,int32_t(0x80340000))=MEM_H(10,int32_t(0x80340000))=16;
MEM_B(0,int32_t(0x8021BFCB))=2;MEM_B(0,int32_t(0x8021BE42))=0;
  draw_pair(a,dora3_s3_801F0CB8,reference_dora3_s3_801F0CB8,0x8021BE00,0x200,d,0x801BB730,0x801BB848);
 }
}
// Actual world hold draw + common fade RDP commands: original alpha endpoint,
// fractional alpha across schedules and unchanged Source hold/request counters.
static void held_world_fades(const Image& seed) {
 overlay=0x70A50;
 for(unsigned divider:{2u,3u})for(int direction:{-1,1})for(int hold:{1,10}) {
  Image a=seed;auto* rdram=bytes(a);pair(rdram,0x801BB730,0x801BB848,divider);
  w(rdram,0x801AA648)=w(rdram,0x801A9500)=1;w(rdram,0x801A9C88)=0;w(rdram,0x8021D484)=0;
  w(rdram,0x8021D494)=1;w(rdram,0x802170F0)=hold;w(rdram,0x802170EC)=2;
  MEM_B(0,int32_t(0x80217118))=2;
  MEM_B(0,int32_t(0x801A99A0))=int8_t(direction);MEM_B(0,int32_t(0x80171580))=16;
  MEM_H(0,int32_t(0x801AA660))=direction>0?0:255;
  for(uint32_t address:{0x801B1404u,0x80171CB8u,0x801A99B4u})MEM_B(0,int32_t(address))=-1;
  assert(select_native_scene_policy(rdram)&&select_native_scene_policy(rdram)->update==nullptr);
  Image reference=a;auto c=context();call(reference_dora3_fade_tick,bytes(reference),c);
  const auto reference_alpha=[&] {auto* rdram=bytes(reference);return int(MEM_H(0,int32_t(0x801AA660)));};
  const FadeState initial{MEM_H(0,int32_t(0x801AA660)),int8_t(direction),16};
  begin(rdram,divider,0);const auto start=now;
  for(unsigned fps:{60u,360u}) {
   FadeTimeline fade(initial,divider,start);
   MEM_H(0,int32_t(0x801AA660))=initial.alpha;MEM_B(0,int32_t(0x801A99A0))=initial.direction;
   for(unsigned frame=1;Nanoseconds(frame)*second/fps<interval(divider);++frame) {
    const auto stamp=start+Nanoseconds(frame)*second/fps;
    const FadeState observed{MEM_H(0,int32_t(0x801AA660)),MEM_B(0,int32_t(0x801A99A0)),16};
    const auto alpha=fade.sample(observed,stamp);MEM_H(0,int32_t(0x801AA660))=int16_t(alpha.alpha);MEM_B(0,int32_t(0x801A99A0))=int8_t(alpha.direction);
    w(rdram,0x801B1424)=int32_t(0x80360000);c=context();commands.clear();
    {UiFrameScope ui(rdram,&c,stamp);call(dora3_s3_801BB848,rdram,c);call(dora3_fade_draw_dispatch,rdram,c);}
    const auto end=uint32_t(w(rdram,0x801B1424));bool found=false;
    for(uint32_t at=0x80360000;at+8<=end;at+=8)if(uint32_t(w(rdram,at))>>24==0xFA){assert(uint32_t(w(rdram,at+4))==(0xFFFFFF00u|uint32_t(alpha.alpha)));found=true;}
    assert(found&&w(rdram,0x802170F0)==hold&&w(rdram,0x802170EC)==2&&w(rdram,0x8021D494)==1);
    assert(MEM_B(0,int32_t(0x80217118))==2);++samples;
   }
   const FadeState observed{MEM_H(0,int32_t(0x801AA660)),MEM_B(0,int32_t(0x801A99A0)),16};
   const auto endpoint=fade.sample(observed,start+interval(divider));
   assert(endpoint.alpha==reference_alpha());++endpoints;
  }
  native_ui_reset();++cases;
 }
}
static void ownership(const Image& seed){
 Image a=seed;auto* r=bytes(a);overlay=0x635C0;pair(r,0x801BCEF8,0x801BD078,3);assert(native_ui_menu_eligible(r));++guards;
 overlay=0;assert(!native_ui_menu_eligible(r));++guards;overlay=0x635C0;
 for(auto p:{0x801A4500u,0x801A4514u,0x801A39DCu,0x801A9C88u,0x801A4518u}){const auto value=w(r,p);w(r,p)=1;assert(!native_ui_menu_eligible(r));w(r,p)=value;++guards;}
 begin(r,3,4);Image source=a;auto c=context();w(r,0x801A4500)=0;source=a;{UiFrameScope frame(r,&c,now+interval(3));assert(a==source);}++guards;native_ui_reset();
 pair(r,0x801BB730,0x801BB848,2);overlay=0x70A50;assert(native_ui_game_eligible(r));++guards;
 for(unsigned d:{2u,3u})for(auto callbacks:std::array<std::array<uint32_t,2>,6>{{{0x801BB52C,0x801BB648},{0x801BCEF8,0x801BD078},{0x801BFAA4,0x801BFC44},{0x801C0BBC,0x801C0C88},{0x801C24A4,0x801C267C},{0x801BB730,0x801BB848}}}){
  overlay=callbacks[0]==0x801BB730?0x70A50:0x635C0;pair(r,callbacks[0],callbacks[1],d);
  const auto* policy=select_native_scene_policy(r);if(callbacks[0]==0x801BB730)assert(policy==nullptr);else assert(policy&&policy->divider==d&&policy->update==nullptr);++guards;
  w(r,0x801A9C88)=1;assert(select_native_scene_policy(r)==nullptr);w(r,0x801A9C88)=0;++guards;
 }
}
int main(int argc,char** argv){assert(argc==3);Image game(0x400000/4),menu(game.size());std::ifstream(argv[1],std::ios::binary).read(reinterpret_cast<char*>(game.data()),0x400000);std::ifstream(argv[2],std::ios::binary).read(reinterpret_cast<char*>(menu.data()),0x400000);
 title_math(menu);game_math(game);world_menu_models(game);character_carousel_cadence(game);charge_rdp(game);menu_math(menu);drawing(game,menu);complete_files_draw(menu);map_math(menu);map_chunk_rectangles(menu);item_side_arrows(game);held_world_fades(game);ownership(menu);native_ui_reset();
 std::cout<<"PASS: "<<cases<<" connected UI cases; "<<endpoints<<" Original motion endpoints; "<<samples<<" Native math samples; "<<draws<<" repeated Original UI draw/command comparisons; "<<guards<<" ownership/pause guards; full math RAM/context restoration; ASan/UBSan; no game/GPU.\n";
}
