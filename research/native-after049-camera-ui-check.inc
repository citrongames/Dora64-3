// Connected independent camera and auxiliary UI equations, using Original ROM
// handlers/geometry and actual Source domain pins. No player/world replay or GPU.
extern "C" {
DECL(dora3_s3_801D5864)
DECL(dora3_s3_801D6EAC)
DECL(dora3_s3_801D7294)
DECL(dora3_s3_801D8578)
DECL(dora3_s3_801D8A24)
DECL(dora3_s3_801D8FF0)
DECL(dora3_s3_801D9B00)
DECL(dora3_s3_801D9C94)
DECL(dora3_s3_801DA06C)
DECL(dora3_s3_801DA588)
DECL(dora3_s3_801DAB58)
DECL(dora3_s3_801DAF50)
DECL(dora3_s3_801DB3D0)
DECL(dora3_s3_801DB720)
DECL(dora3_s3_801DB7C8)
DECL(dora3_s3_801DBD58)
DECL(dora3_s0_8002F064) DECL(dora3_s49_remaining_801B5788) DECL(dora3_s3_core_801F96E0)
}
namespace camera_ui {
constexpr uint32_t camera=0x801A99F8,player=0x80250000,physics=0x80251000,control=0x80252000,params=0x80253000;
static void wf(uint8_t* r,uint32_t p,float v){f(r,p,v);}
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

static void eye(uint8_t* ram,unsigned count,bool original) {
 auto c=context();for(unsigned i=0;i<count;++i){c.r4=int32_t(camera+i*0x34);call(original?reference_dora3_s0_8002F064:dora3_s0_8002F064,ram,c);}
}
static void camera_equal(uint8_t* a,uint8_t* b,unsigned count) {
 for(unsigned off=0;off<count*0x34;off+=4) {
  if(w(a,camera+off)!=w(b,camera+off)) {
   const float x=f(a,camera+off),y=f(b,camera+off);
   if(!(std::isfinite(x)&&std::isfinite(y)&&std::abs(x-y)<.001f)) {
    std::cerr<<"camera UI mismatch field "<<std::hex<<off<<std::dec<<" "<<x<<" != "<<y<<"\n";std::abort();
   }
  }
 }
}
static void camera_source_begin(uint8_t* r) {auto c=context();c.r4=0;dora3_native_ui_source_begin(r,&c);}
static void camera_source_end(uint8_t* r,unsigned divider) {auto c=context();c.r21=0;dora3_native_ui_source(r,&c);native_ui_begin(r,now,divider);}
static void ordinary(const Image& base) {
 constexpr std::array<Function,16> original{reference_dora3_s3_801DB7C8,reference_dora3_s3_801DB7C8,reference_dora3_s3_801D6EAC,reference_dora3_s3_801D8FF0,reference_dora3_s3_801D7294,reference_dora3_s3_801D5864,reference_dora3_s3_801D8578,reference_dora3_s3_801DB720,reference_dora3_s3_801D9B00,reference_dora3_s3_801D8A24,reference_dora3_s3_801D9C94,reference_dora3_s3_801DA06C,reference_dora3_s3_801DAB58,reference_dora3_s3_801DA588,reference_dora3_s3_801DAF50,reference_dora3_s3_801DB3D0};
 overlay=0x70A50;
 unsigned covered=0;
 for(unsigned divider:{2u,3u})for(unsigned mode=0;mode<16;++mode)for(unsigned variant=0;variant<6;++variant) {
  native_ui_reset();Image a=base;auto* r=bytes(a);auto* rdram=r;seed(r,variant);
  pair(r,0x801BB730,0x801BB848,divider);
  w(r,0x801AA648)=w(r,0x801A9500)=1;
  w(r,0x80218AD8)=w(r,0x8021D484)=w(r,0x802170EC)=w(r,0x802170F0)=0;
  MEM_B(0,int32_t(0x80217118))=0;MEM_B(0,int32_t(control))=1;
  // Block player ownership while its actual Source camera still runs.
  MEM_B(0,int32_t(0x801B361A))=int8_t(mode);MEM_H(0,int32_t(0x801AC69C))=0;
  MEM_H(0,int32_t(0x801A9504))=0x401;
  eye(r,2,false);Image before=a,reference=a;auto c=context(),rc=context();
  call(reference_dora3_s3_801DBD58,bytes(reference),rc);eye(bytes(reference),2,true);
  camera_source_begin(r);call(dora3_s3_801DBD58,r,c);eye(r,2,false);camera_source_end(r,divider);
  // Observers are exact-ROM Source calls, not a manufactured domain bit.
  assert(a==reference);assert(native_ui_menu_eligible(r));
  const auto* policy=select_native_scene_policy(r);assert(policy&&policy->update==nullptr);
  assert(native_scene_identity(r,*policy).module==0x70A50);
  const unsigned reached=MEM_BU(0,int32_t(0x801B361A));assert(reached<16);
  Image source=a,next=a;rc=context();rc.r4=int32_t(camera);
  call(original[reached],bytes(next),rc);eye(bytes(next),2,true);
  const auto saved=context();auto shown=saved;
  for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
   for(unsigned frame=1;Nanoseconds(frame)*second/fps<interval(divider);++frame) {
    {UiFrameScope lease(r,&shown,now+Nanoseconds(frame)*second/fps);
     for(unsigned i=0;i<13;++i)assert(std::isfinite(f(r,camera+4*i)));
     if(mode==0||mode==1)assert(f(r,camera+0x30)>f(bytes(source),camera+0x30));
    }
    assert(a==source);context_equal(saved,shown);++samples;
   }
  }
  {UiFrameScope lease(r,&shown,now+interval(divider));camera_equal(r,bytes(next),2);}
  assert(a==source);context_equal(saved,shown);++endpoints;
  gameplay_owner=true;{UiFrameScope lease(r,&shown,now+interval(divider));assert(a==source);}gameplay_owner=false;++guards;
  // Room/resource or scripted owners invalidate the independent camera path.
  for(auto p:{0x802170ECu,0x802170F0u,0x80218AD8u,0x8021D484u}) {
   w(r,p)=1;Image held=a;{UiFrameScope lease(r,&shown,now+interval(divider));assert(a==held);}w(r,p)=0;++guards;
  }
  // A later Source pass that did not reach a camera holds its last pose.
  observed(r,0);{UiFrameScope lease(r,&shown,now+interval(divider));assert(a==source);}++guards;
  native_ui_reset();++covered;++cases;(void)before;
 }
 std::cout<<"PASS: "<<covered<<" reached Original ordinary camera owners with blocked player,16 mode entries, both Source rates/60..360 FPS, real collision geometry, independent camera policy, Source endpoint/whole-image/context restore, no gameplay double-step and held/resource/script guards.\n";
}
static void auxiliary(const Image& base) {
 unsigned covered=0;
 for(bool four:{false,true})for(unsigned divider:{2u,3u})
 for(unsigned held:{0u,1u,2u,3u,4u,8u,12u,0x400u,0x800u,0xC00u,0xC0Fu})
 for(float distance:{20.f,50.f,1990.f,2000.f,2010.f}) {
  native_ui_reset();Image a=base;auto* r=bytes(a);auto* rdram=r;overlay=four?0x59E80:0x70A50;
  pair(r,four?0x801B59D8u:0x801F9B58u,four?0x801B5A70u:0x801F9C88u,divider);
  w(r,0x801AA648)=w(r,0x801A9500)=1;
  clear(r,camera,4*0x34);
  for(unsigned i=0;i<4;++i){f(r,camera+i*0x34+0x18,10);f(r,camera+i*0x34+0x1C,359);f(r,camera+i*0x34+0x28,1);f(r,camera+i*0x34+0x30,distance);}
  MEM_H(0,int32_t(0x801A9504))=int16_t(held);MEM_H(0,int32_t(0x801A9AD0))=int16_t(held);
  eye(r,four?4:1,false);
  const auto actual=four?dora3_s49_remaining_801B5788:dora3_s3_core_801F96E0;
  const auto original=four?reference_dora3_s49_remaining_801B5788:reference_dora3_s3_core_801F96E0;
  Image reference=a;auto c=context(),rc=context();c.r4=rc.r4=int32_t(camera);
  call(original,bytes(reference),rc);if(!four)eye(bytes(reference),1,true);
  camera_source_begin(r);call(actual,r,c);if(!four)eye(r,1,false);camera_source_end(r,divider);
  assert(a==reference);assert(native_ui_menu_eligible(r));
  const auto* policy=select_native_scene_policy(r);assert(policy&&policy->update==nullptr);
  assert(native_scene_identity(r,*policy).module==(four?0x59E80u:0x70A50u));
  Image source=a,next=a;rc=context();rc.r4=int32_t(camera);call(original,bytes(next),rc);if(!four)eye(bytes(next),1,true);
  const bool cut=four&&f(bytes(source),camera+0x9C+0x30)>2000.f;
  if(cut)std::memcpy(bytes(next)+((camera+0x9C)&0xFFFFFF),bytes(source)+((camera+0x9C)&0xFFFFFF),0x34);
  const auto saved=context();auto shown=saved;
  for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned frame=1;Nanoseconds(frame)*second/fps<interval(divider);++frame) {
   {UiFrameScope lease(r,&shown,now+Nanoseconds(frame)*second/fps);
    if(four&&cut)assert(w(r,camera+0x9C+0x30)==w(bytes(source),camera+0x9C+0x30));
    assert(std::isfinite(f(r,camera+0xC)));
   }assert(a==source);context_equal(saved,shown);++samples;
  }
  {UiFrameScope lease(r,&shown,now+interval(divider));camera_equal(r,bytes(next),four?4:1);}
  assert(a==source);context_equal(saved,shown);++endpoints;
  overlay=0;{UiFrameScope lease(r,&shown,now+interval(divider));assert(a==source);}overlay=four?0x59E80:0x70A50;++guards;
  if(cut){c=context();c.r4=int32_t(camera);call(actual,r,c);assert(f(r,camera+0x9C+0x30)==50.f);++guards;}
  native_ui_reset();++covered;++cases;
 }
 std::cout<<"PASS: "<<covered<<" auxiliary Source camera cases, exact held combinations/floors, both rates/60..360 FPS, all4 eye/up views, Source-only >2000 cut, module identity and full frame restore.\n";
}
}
static void camera_ui_regressions(const Image& game){camera_ui::ordinary(game);camera_ui::auxiliary(game);}
