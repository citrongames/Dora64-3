#include "native_texture_animation.hpp"
#include "native_frame_bridge.hpp"
#include "frame_lights.hpp"
#include "texture-functions.h"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace dora3::native_time;
static const FrameBinding* binding=nullptr;
namespace dora3::native_time {const FrameBinding* active_frame_binding(){return binding;}}
extern "C" void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t){}
extern "C" void do_break(uint32_t){throw std::logic_error("Source break");}
extern "C" void switch_error(const char*,uint32_t,uint32_t){throw std::logic_error("Unseeded material switch");}
struct Memory {
 std::vector<uint32_t> values=std::vector<uint32_t>(0x400000/4);
 uint8_t* ram(){return reinterpret_cast<uint8_t*>(values.data());}
 int32_t& W(uint32_t p){auto* rdram=ram();return MEM_W(0,int32_t(p));}
 int16_t& H(uint32_t p){auto* rdram=ram();return MEM_H(0,int32_t(p));}
 int8_t& B(uint32_t p){auto* rdram=ram();return MEM_B(0,int32_t(p));}
};
constexpr uint32_t actor=0x80171CD0+300*0xCC,sequence=0x80300000,indices=sequence+16,durations=sequence+32,table=sequence+48;
constexpr uint32_t channel=0x801A9CB0,model=0x80320000,mesh=model+0x200,cursor_pointer=0x80321000;
static uint8_t texel(unsigned x,unsigned y,unsigned width){
 uint32_t n=y*width+x+1;n=(n^(n>>16))*0x45d9f3b;n=(n^(n>>16))*0x45d9f3b;return uint8_t((n^(n>>16))&15);
}
static void bitmap(Memory& m,uint32_t p,unsigned width,unsigned height,int dx,int dy,bool deform=false,unsigned format=5,unsigned colors=16){
 m.B(p)='B';m.B(p+1)='M';m.B(p+2)='P';m.B(p+3)=int8_t(format);m.H(p+6)=int16_t(colors);m.W(p+8)=int(width);m.W(p+12)=int(height);
 if(format==5)for(unsigned i=0;i<colors;++i)m.H(p+16+2*i)=int16_t(i*0x421|1);
 const auto payload=p+16+(format==5?((2*colors+7)&~7u):0);
 for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;x+=format==5?2:1){
  const unsigned a=(int(x)+dx+int(width))%width,b=(int(x+1)+dx+int(width))%width,c=(int(y)+dy+int(height))%height;
  if(format==5)m.B(payload+(y*width+x)/2)=int8_t(((texel(a,c,width)%colors)<<4)|(texel(b,c,width)%colors));
  else m.B(payload+y*width+x)=int8_t((texel(a,c,width)<<4)|texel((a+5)%width,c,width));
 }
 if(deform)m.B(payload)^=1;
}
static recomp_context context(){recomp_context c{};c.r29=int32_t(0x803F0000);return c;}
static void setup(Memory& m,unsigned width,unsigned height,int dx,int dy,bool deform=false){
 const uint32_t first=0x80310000,next=0x80312000;
 bitmap(m,first,width,height,0,0);bitmap(m,next,width,height,dx,dy,deform);
 m.W(actor+0xB8)=int32_t(table);m.B(actor+0x6C)=5;m.W(actor+0x78)=0;
 m.W(channel)=int32_t(sequence);m.B(channel+4)=1;m.B(channel+5)=0;m.B(channel+6)=1;
 m.W(sequence)=int32_t(indices);m.W(sequence+4)=int32_t(durations);m.B(sequence+8)=4;m.B(sequence+9)=5;
 m.B(indices)=0;m.B(indices+1)=1;m.B(indices+2)=int8_t(255);m.B(durations)=2;m.B(durations+1)=2;
 m.H(table)=1;m.H(table+2)=2;m.H(0x80217120)=1;m.H(0x80217122)=2;
 m.W(0x801AEC00+20)=int32_t(first);m.W(0x801AEC00+40)=int32_t(next);m.W(0x801A39DC)=3;
 m.W(model)=0x100;m.W(model+0x18)=0x27;m.W(model+0x100)=0x200;
 m.W(model+0x100+0x2C)=0;m.W(0x80091640)=1;m.W(0x8010E8C0)=1;
 const unsigned s=unsigned(std::log2(width)),t=unsigned(std::log2(height));
 m.W(mesh)=int32_t(0xFD500000u);m.W(mesh+4)=0x04000000;
 m.W(mesh+8)=int32_t(0xF5400000u);m.W(mesh+12)=int((s<<4)|(t<<14));
 m.W(mesh+16)=int32_t(0xF2000000u);m.W(mesh+20)=int(((width-1)*4<<12)|((height-1)*4));
 m.W(mesh+24)=int32_t(0xDF000000u);m.W(mesh+28)=0;
}
static uint32_t draw(Memory& m,unsigned bank,bool world){
 auto c=context();c.f_odd=&c.f0.u32h;
 const auto begin=dora3::frame::bank0+bank*dora3::frame::bank_stride+dora3::frame::dl_offset;
 m.W(0x801B1424)=int32_t(begin);c.r4=int32_t(actor);c.r5=1;
 if(world)dora3_s3_801BBFF4(m.ram(),&c);else dora3_s0_80028D50(m.ram(),&c);
 m.W(cursor_pointer)=m.W(0x801B1424);c.r4=int32_t(cursor_pointer);c.r5=int32_t(model);c.r6=int32_t(model);c.r7=255;
 dora3_s0_8003A3B8(m.ram(),&c);m.W(0x801B1424)=m.W(cursor_pointer);
 const auto end=uint32_t(m.W(cursor_pointer));
 for(uint32_t at=begin;at<end;at+=8)if(uint32_t(m.W(at))>>24==0xDE)return uint32_t(m.W(at+4));
 throw std::logic_error("Actual SDK material builder emitted no mesh DE");
}
static uint32_t resolved(uint32_t p){return p<0x400000?p|0x80000000u:p;}
int main(){
 unsigned cases=0;
 for(auto size:std::array<std::array<unsigned,2>,4>{{{16,16},{16,32},{32,32},{64,64}}})
 for(auto shift:std::array<std::array<int,2>,3>{{{2,0},{0,-2},{0,4}}})for(bool world:{false,true}){
  native_texture_reset();Memory m;setup(m,size[0],size[1],shift[0],shift[1]);const auto source=m.values;
  native_texture_source_completed(m.ram(),0);FrameBinding frame{{{},0,{}},nullptr};binding=&frame;
  uint32_t copied=0;
  {TextureFrameScope lease(m.ram(),25'000'000,0);copied=resolved(draw(m,0,world));
   assert(copied!=mesh);assert(copied>=dora3::frame::bank0&&copied<dora3::frame::bank0+dora3::frame::bank_stride);
   const auto ul=uint32_t(m.W(copied+16)),lr=uint32_t(m.W(copied+20));
   const int x=int(std::llround(shift[0])),y=int(std::llround(shift[1])); // .5 Source phase /duration2 in quarter pixels
   const auto wrap=[](int v,int period){return (v%period+period)%period;};
   assert(int((ul>>12)&4095)==wrap(-x,int(size[0]*4)));assert(int(ul&4095)==wrap(-y,int(size[1]*4)));
   assert(((lr>>12)&4095)-((ul>>12)&4095)==(size[0]-1)*4);assert((lr&4095)-(ul&4095)==(size[1]-1)*4);
   dora3_native_texture_submit_guard(m.ram(),nullptr);
  }
  for(uint32_t at:{actor,channel,sequence,mesh,0x80310000u,0x80312000u}){
   const unsigned length=at==0x80310000||at==0x80312000?48+size[0]*size[1]/2:at==actor?0xCC:at==mesh?32:at==channel?8:12;
   for(unsigned i=0;i<length/4;++i)assert(m.values[((at&0xffffff)/4)+i]==source[((at&0xffffff)/4)+i]);
  }
  const std::vector<uint32_t> retained(m.values.begin()+((copied&0xffffff)/4),m.values.begin()+((copied&0xffffff)/4)+8);
  frame.ticket.bank=1;{TextureFrameScope lease(m.ram(),33'333'333,1);draw(m,1,world);}
  for(unsigned i=0;i<8;++i)assert(m.W(copied+4*i)==int32_t(retained[i]));
  ++cases;
 }
 // Three additional ROM families: byte format2 and CI4 with3/6 colors.
 for(unsigned variant=0;variant<3;++variant)for(bool world:{false,true}){
  const unsigned width=variant==1?16:variant==0?64:32,height=variant==1?4:32;
  const unsigned format=variant==0?2:5,colors=variant==1?3:variant==2?6:8;
  const int dx=variant==0?-16:variant==1?1:0,dy=variant==0?8:variant==2?4:0;
  native_texture_reset();Memory m;setup(m,width,height,dx,dy);
  bitmap(m,0x80310000,width,height,0,0,false,format,colors);
  bitmap(m,0x80312000,width,height,dx,dy,false,format,colors);
  const auto assets=m.values;native_texture_source_completed(m.ram(),0);
  FrameBinding frame{{{},0,{}},nullptr};binding=&frame;
  {TextureFrameScope lease(m.ram(),25'000'000,0);const auto copied=resolved(draw(m,0,world));assert(copied!=mesh);
   const auto ul=uint32_t(m.W(copied+16)),lr=uint32_t(m.W(copied+20));
   const auto wrap=[](int v,int period){return (v%period+period)%period;};
   assert(int((ul>>12)&4095)==wrap(-dx,int(width*4)));assert(int(ul&4095)==wrap(-dy,int(height*4)));
   assert(((lr>>12)&4095)-((ul>>12)&4095)==(width-1)*4);assert((lr&4095)-(ul&4095)==(height-1)*4);
  }
  for(uint32_t at:{0x80310000u,0x80312000u})for(unsigned i=0;i<0x1800/4;++i)assert(m.values[(at&0xFFFFFF)/4+i]==assets[(at&0xFFFFFF)/4+i]);
  ++cases;
 }
   // Reordered CI4 palettes at a cycle seam (ROM00DF..00EE): both directions
  // must still scroll. A changed displayed color must invalidate that match.
  for(bool world:{false,true})for(int direction:{-2,2}) {
   native_texture_reset();Memory m;setup(m,32,32,direction,0);
   constexpr uint32_t next=0x80312000;
   std::array<int16_t,16> palette{};for(unsigned i=0;i<16;++i)palette[i]=m.H(next+16+2*i);
   for(unsigned i=0;i<16;++i)m.H(next+16+2*((i+1)%16))=palette[i];
   for(unsigned i=0;i<512;++i){const auto packed=uint8_t(m.B(next+48+i));m.B(next+48+i)=int8_t((((packed>>4)+1)%16<<4)|((packed+1)%16));}
   const auto assets=m.values;FrameBinding frame{{{},0,{}},nullptr};binding=&frame;
   for(unsigned index:{0u,1u}) {
    m.B(channel+5)=int8_t(index);native_texture_source_completed(m.ram(),0);
    {TextureFrameScope lease(m.ram(),25'000'000,0);const auto copied=resolved(draw(m,0,world));assert(copied!=mesh);
     const int delta=index?-direction:direction;const auto ul=uint32_t(m.W(copied+16));
     assert(int((ul>>12)&4095)==(-delta%128+128)%128);
    }++cases;
   }
   for(unsigned i=0;i<560/4;++i)assert(m.W(next+4*i)==int32_t(assets[(next&0xFFFFFF)/4+i]));
   m.B(channel+5)=0;m.H(next+16)^=0x40;
   {TextureFrameScope lease(m.ram(),25'000'000,0);assert(resolved(draw(m,0,world))==mesh);}++cases;
  }
  // Same header addresses reused for a different image must invalidate the
 // translation cache. A stopped or final clamped channel must hold its frame.
 for(unsigned variant=0;variant<4;++variant){
  native_texture_reset();Memory m;setup(m,32,32,2,0);native_texture_source_completed(m.ram(),0);
  FrameBinding frame{{{},0,{}},nullptr};binding=&frame;
  {TextureFrameScope lease(m.ram(),25'000'000,0);assert(resolved(draw(m,0,false))!=mesh);}
  if(variant==0)bitmap(m,0x80312000,32,32,2,0,true);
  if(variant==1)m.B(channel+6)=0;
  if(variant==2){m.B(channel+6)=3;m.B(channel+5)=1;}
  if(variant==3)m.W(0x80312004)^=0x10000; // Foreign header/format flags do not become a scroll.
  {TextureFrameScope lease(m.ram(),25'000'000,0);assert(resolved(draw(m,0,false))==mesh);}++cases;
 }
 // Source may later occupy the tail. Remove outer clone references before
 // submission, without changing immutable assets or throwing a game exception.
 native_texture_reset();Memory m;setup(m,32,32,2,0);native_texture_source_completed(m.ram(),0);
 FrameBinding frame{{{},0,{}},nullptr};binding=&frame;
 {TextureFrameScope lease(m.ram(),25'000'000,0);assert(resolved(draw(m,0,false))!=mesh);
  const auto end=uint32_t(m.W(cursor_pointer));m.W(0x801B1424)=int32_t(dora3::frame::bank0+dora3::frame::bank_stride-16);
  dora3_native_texture_submit_guard(m.ram(),nullptr);bool original=false;
  for(auto at=dora3::frame::bank0+dora3::frame::dl_offset;at<end;at+=8)if(uint32_t(m.W(at))>>24==0xDE)original=uint32_t(m.W(at+4))==mesh;
  assert(original);++cases;
 }
 binding=nullptr;native_texture_reset();
 std::cout<<"PASS: "<<cases<<" actual Source bitmap/material and Native RDP scroll cases; generic/world binding, horizontal/vertical shifts, ordered tile bounds, immutable resources, both frame banks, reuse/stopped/clamped guards and pre-submit fallback; synthetic assets, no game/GPU\n";
}
