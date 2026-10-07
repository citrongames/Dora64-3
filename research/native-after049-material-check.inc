// Actual room sequence/bitmap inputs, including every last->first transition.
// Generated decoded bytes remain in build-local; no ROM assets are tracked.
#include <fstream>
#include <iterator>
static uint32_t material_channel(Memory& m){return channel+8*uint32_t(m.W(actor+0x78));}
static unsigned material_u32(std::ifstream& file){std::array<unsigned char,4> b{};file.read(reinterpret_cast<char*>(b.data()),4);assert(file);return unsigned(b[0])<<24|unsigned(b[1])<<16|unsigned(b[2])<<8|b[3];}
static void material_bytes(std::ifstream& file,Memory& m,uint32_t address,unsigned count){for(unsigned i=0;i<count;++i){char b{};file.get(b);assert(file);m.B(address+i)=int8_t(b);}}
static void material_mesh(Memory& m,unsigned format,unsigned width,unsigned height,unsigned segment,unsigned palette){
 const unsigned s=unsigned(std::log2(width)),t=unsigned(std::log2(height));
 unsigned at=0;
 if(format==5){m.W(mesh)=int32_t(0xFD100000u);m.W(mesh+4)=int32_t(palette<<24);m.W(mesh+8)=int32_t(0xF0000000u);m.W(mesh+12)=int32_t(15u<<14);at=16;}
 m.W(mesh+at)=int32_t(format==5?0xFD500000u:0xFD700000u);m.W(mesh+at+4)=int32_t(segment<<24);
 m.W(mesh+at+8)=int32_t(format==5?0xF5400000u:0xF5680000u);m.W(mesh+at+12)=int((s<<4)|(t<<14));
 m.W(mesh+at+16)=int32_t(0xF2000000u);m.W(mesh+at+20)=int(((width-1)*4<<12)|((height-1)*4));
 m.W(mesh+at+24)=int32_t(0xDF000000u);m.W(mesh+at+28)=0;
}
struct MaterialImage {uint32_t pixels=0,palette=0,origin=0;};
static MaterialImage material_image(Memory& m,uint32_t root,unsigned format){
 const auto descriptor=uint32_t(m.W(material_channel(m))),indices=uint32_t(m.W(descriptor));
 const auto index=uint8_t(m.B(indices+uint8_t(m.B(material_channel(m)+5))));const auto handle=uint16_t(m.H(0x80217120+2*index));
 const auto header=uint32_t(m.W(0x801AEC00+20*handle));
 const auto pixels=header+16+(format==5?((2*uint16_t(m.H(header+6))+7)&~7u):0);
 const auto address=[&](uint32_t p){if(p>=0x04000000u&&p>>24<16){if(p>>24==uint8_t(m.B(descriptor+8)))return pixels+(p&0xFFFFFF);if(p>>24==uint8_t(m.B(descriptor+9)))return header+16+(p&0xFFFFFF);}return resolved(p);};
 MaterialImage image;for(unsigned i=0;i<32;++i){const auto a=resolved(root)+8*i,w=uint32_t(m.W(a));
  if(w>>24==0xFD){const auto p=address(uint32_t(m.W(a+4)));if((w&0x00E00000u)==0&&format==5)image.palette=p;else image.pixels=p;}
  if(w>>24==0xF2)image.origin=w;
  if(w>>24==0xDF)break;
 }return image;
}
static uint32_t material_payload(Memory& m,uint32_t header){const auto format=uint8_t(m.B(header+3));return header+16+(format==5?((2*uint16_t(m.H(header+6))+7)&~7u):0);}
static void material_ctor(Memory& m,uint32_t descriptor){auto c=context();c.f_odd=&c.f0.u32h;c.r4=int32_t(actor);c.r5=1;c.r6=int32_t(descriptor);dora3_s0_80028BC8(m.ram(),&c);assert(uint32_t(m.W(actor+0x78))<300&&uint8_t(m.B(actor+0x6C))==5);}
static void material_tick(Memory& m,Nanoseconds time){auto c=context();c.f_odd=&c.f0.u32h;c.r4=int32_t(actor);c.r5=1;dora3_s0_80028C74(m.ram(),&c);native_texture_source_completed(m.ram(),time);}
static void material_rom_regressions(const char* input){
 std::ifstream file(input,std::ios::binary);assert(file);const unsigned count=material_u32(file);assert(count==35);
 unsigned transitions=0,sampled=0,scrolled=0,pauses=0,seams=0;
 FrameBinding frame{{{},0,{}},nullptr};binding=&frame;
 for(unsigned row=0;row<count;++row){
  native_texture_reset();Memory m;setup(m,32,32,2,0);
  const auto descriptor=material_u32(file),index_address=material_u32(file),duration_address=material_u32(file),n=material_u32(file),resources=material_u32(file),segment=material_u32(file),palette=material_u32(file);
  const bool full=material_u32(file),half=material_u32(file);const int cycle_x=int32_t(material_u32(file)),cycle_y=int32_t(material_u32(file));
  assert(n&&n<255&&resources<50);m.W(descriptor)=int32_t(index_address);m.W(descriptor+4)=int32_t(duration_address);m.B(descriptor+8)=int8_t(segment);m.B(descriptor+9)=int8_t(palette);
  material_bytes(file,m,index_address,n+1);material_bytes(file,m,duration_address,n);
  std::vector<uint32_t> headers,sizes;unsigned width=0,height=0,format=0;
  for(unsigned i=0;i<resources;++i){(void)material_u32(file);const auto size=material_u32(file),p=0x80230000u+i*0x2000;
   material_bytes(file,m,p,size);headers.push_back(p);sizes.push_back(size);m.H(0x80217120+2*i)=int16_t(i+1);m.W(0x801AEC00+20*(i+1))=int32_t(p);
  }
  std::vector<std::array<int,2>> offsets(n);int sx=0,sy=0;
  for(unsigned i=0;i<n;++i){offsets[i]={sx,sy};sx+=int32_t(material_u32(file));sy+=int32_t(material_u32(file));}
  const auto first=headers[uint8_t(m.B(index_address))];width=unsigned(m.W(first+8));height=unsigned(m.W(first+12));format=uint8_t(m.B(first+3));
  unsigned cycle_ticks=0;for(unsigned i=0;i<n;++i)cycle_ticks+=uint8_t(m.B(duration_address+i));assert(cycle_ticks);
  material_mesh(m,format,width,height,segment,palette);
  for(unsigned i=0;i<300;++i){m.W(channel+8*i)=0;m.W(channel+8*i+4)=0;}
  m.B(actor+0x6C)=0;material_ctor(m,descriptor);
  const auto created=m.values;const unsigned divider=row&1?2:3;m.W(0x801A39DC)=int(divider);
  const Nanoseconds period=(Nanoseconds(divider)*second+59)/60;
  std::vector<bool> seen(n);unsigned last_index=0;
  for(unsigned tick=1;tick<=2*cycle_ticks+1;++tick){
   material_tick(m,Nanoseconds(tick)*period);const unsigned index=uint8_t(m.B(material_channel(m)+5));assert(index<n);seen[index]=true;
   if(index!=last_index){++transitions;if(index==0)++seams;}last_index=index;
   const unsigned current=uint8_t(m.B(index_address+index)),next=uint8_t(m.B(index_address+(index+1)%n));
   const auto a=headers[current],b=headers[next],pa=material_payload(m,a),pb=material_payload(m,b);
   const unsigned duration=uint8_t(m.B(duration_address+index)),remaining=uint8_t(m.B(material_channel(m)+4));
   assert(remaining<duration);const auto assets=m.values;
   for(unsigned quarter:{1u,3u}){
    const double fraction=std::min(1.,double(Nanoseconds(quarter)*period/4)*60/(double(divider)*second));frame.ticket.bank=(quarter-1)/2;const auto stamp=Nanoseconds(tick)*period+Nanoseconds(quarter)*period/4;
    // Long identical holds need only the reached interval boundaries.
    if(n==1&&tick!=1&&tick!=cycle_ticks&&tick!=cycle_ticks+1&&tick!=2*cycle_ticks)continue;
    TextureFrameScope lease(m.ram(),stamp,frame.ticket.bank);const auto mesh_copy=resolved(draw(m,frame.ticket.bank,true));const auto image=material_image(m,mesh_copy,format);
    if(full||half){
     const double age=tick-1+fraction;const auto wrap=[](int value,int p){return (value%p+p)%p;};
     const int dx=int(std::llround((std::fmod(double(cycle_x)*age/cycle_ticks,width)-offsets[index][0])*4));
     const int dy=int(std::llround((std::fmod(double(cycle_y)*age/cycle_ticks,height)-offsets[index][1])*4));
     assert(int((image.origin>>12)&4095)==wrap(-dx,int(width*4)));assert(int(image.origin&4095)==wrap(-dy,int(height*4)));++scrolled;
    }else if(image.pixels!=pa){
     assert(image.pixels>=dora3::frame::bank0&&image.pixels<dora3::frame::bank0+2*dora3::frame::bank_stride);
     const double h=(duration-1-remaining+fraction)/duration;
     if(format==2){for(unsigned i=0;i<width*height;++i){const unsigned u=uint8_t(m.B(pa+i)),v=uint8_t(m.B(pb+i)),shown=uint8_t(m.B(image.pixels+i));
       const unsigned intensity=unsigned(std::llround((u>>4)*(1-h)+(v>>4)*h)),alpha=unsigned(std::llround((u&15)*(1-h)+(v&15)*h));assert(shown==((intensity<<4)|alpha));}}
     else {assert(image.palette>=dora3::frame::bank0&&image.palette<dora3::frame::bank0+2*dora3::frame::bank_stride);
      const unsigned colors=uint16_t(m.H(a+6));for(unsigned i=0;i<width*height/2;++i){const unsigned pixel=uint8_t(m.B(image.pixels+i));assert((pixel>>4)<colors&&(pixel&15)<colors);}
      for(unsigned i=0;i<colors;++i)assert(uint16_t(m.H(image.palette+2*i))&1);}
     ++sampled;
    }
   }
   for(unsigned i=0;i<resources;++i){const auto p=headers[i];for(unsigned off=0;off<sizes[i]/4;++off)assert(m.W(p+4*off)==int32_t(assets[(p&0xFFFFFF)/4+off]));}
   for(uint32_t p:{material_channel(m),descriptor})for(unsigned off=0;off<(p==material_channel(m)?8u:12u);off+=4){if(m.W(p+off)!=int32_t(assets[(p&0xFFFFFF)/4+off/4])){std::cerr<<"material owner changed row="<<row<<" tick="<<tick<<" pc="<<std::hex<<p+off<<" was="<<assets[(p&0xFFFFFF)/4+off/4]<<" now="<<m.W(p+off)<<std::dec<<"\n";std::abort();}}
   if(remaining==0&&!full&&!half){frame.ticket.bank=0;TextureFrameScope lease(m.ram(),Nanoseconds(tick+1)*period,0);const auto image=material_image(m,resolved(draw(m,0,true)),format);
    const int ox=int((image.origin>>12)&4095)/4,oy=int(image.origin&4095)/4;
    for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){const unsigned shown=((y+height-oy%height)%height)*width+(x+width-ox%width)%width,expected=y*width+x;
     if(format==2)assert(uint8_t(m.B(image.pixels+shown))==uint8_t(m.B(pb+expected)));
     else {const unsigned v=uint8_t(m.B(image.pixels+shown/2)),q=uint8_t(m.B(pb+expected/2));const unsigned si=shown&1?v&15:v>>4,ei=expected&1?q&15:q>>4;assert(m.H(image.palette+2*si)==m.H(b+16+2*ei));}
    }
   }
  }
  assert(std::all_of(seen.begin(),seen.end(),[](bool value){return value;}));
  // Pause/no Source update must not gain another fractional interval. Rebinding
  // the same descriptor resets the Source age, without changing its bytes.
  const auto stopped=m.values;native_texture_source_completed(m.ram(),3*cycle_ticks*period);
  std::array<uint32_t,2> origins{};
  for(unsigned i=0;i<2;++i){frame.ticket.bank=i;TextureFrameScope lease(m.ram(),3*cycle_ticks*period+Nanoseconds(i+1)*period/4,i);origins[i]=material_image(m,resolved(draw(m,i,true)),format).origin;}
  assert(origins[0]==origins[1]);++pauses;
  for(uint32_t p:{material_channel(m),descriptor})for(unsigned off=0;off<(p==material_channel(m)?8u:12u);off+=4)assert(m.W(p+off)==int32_t(stopped[(p&0xFFFFFF)/4+off/4]));
  material_ctor(m,descriptor);assert(uint8_t(m.B(material_channel(m)+5))==0&&uint8_t(m.B(material_channel(m)+4))==uint8_t(m.B(duration_address)));material_tick(m,0);
  frame.ticket.bank=0;{TextureFrameScope lease(m.ram(),0,0);const auto image=material_image(m,resolved(draw(m,0,true)),format);assert(image.pixels==material_payload(m,headers[uint8_t(m.B(index_address))]));assert((image.origin&0xFFFFFFu)==0);}
  for(unsigned off=0;off<12;off+=4)assert(m.W(descriptor+off)==int32_t(created[(descriptor&0xFFFFFF)/4+off/4]));
 }
 binding=nullptr;native_texture_reset();
 assert(sampled&&scrolled&&seams&&pauses==35);
 std::cout<<"PASS:35 actual ROM room-material cycles/339 frame pairs, "<<transitions<<" real Source transitions, "<<scrolled<<" complete-cycle scroll samples, "<<sampled<<" CI4/IA8 deformation samples, skipped/half-tile seams, native palettes/TMEM shape, all immutable assets, two frame banks and35 paused/rebound owners; no game/GPU.\n";
}
