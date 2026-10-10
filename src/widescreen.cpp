#include "widescreen.hpp"
#include "guest_storage.hpp"
#include "recomp_context_copy.hpp"
#include "librecomp/addresses.hpp"
#include "librecomp/game.hpp"
#ifdef DORA3_LIVE_RT64
#include "port_settings.hpp"
#endif
#include <array>
#include <vector>
#include <stdexcept>
#include <cstring>

extern "C" {
int dora3_overlay_loaded(uint32_t);
void dora3_scaled_texture_rect(uint8_t*,recomp_context*);
void dora3_texture_rect_state(uint8_t*,recomp_context*);
}
namespace dora3::wide {
namespace {
constexpr unsigned None=0x800,Center=0x200,Blue=0x801,Title=0x802;
struct Layout {unsigned origin=None,aspect=2,viewport=None;bool menu=false;};
Layout layout,emitted;
bool emitted_valid=false;
std::vector<Layout> scopes;
int enabled=0,hud_enabled=0;
bool drawing_underlay=false;
uint32_t blue_asset=0,scratch=0;
int32_t& word(uint8_t* rdram,uint32_t a){return MEM_W(0,int32_t(a));}
void command(uint8_t* ram,uint32_t a,uint32_t b){
    const auto p=uint32_t(word(ram,0x801B1424));
    if(!distance::frame_pointer(p,8))throw std::logic_error("Widescreen command outside frame arena");
    word(ram,p)=int32_t(a);word(ram,p+4)=int32_t(b);word(ram,0x801B1424)=int32_t(p+8);
}
void apply(uint8_t* ram){
    if(!emitted_valid||layout.aspect!=emitted.aspect)command(ram,0x64000033,enabled?layout.aspect:0);
    if(!emitted_valid||layout.origin!=emitted.origin){
        // Extended origins interpret coordinates relative to the anchor. Our
        // guest still emits absolute320px coordinates, so cancel that base.
        const uint32_t offset=layout.origin<None?uint32_t(uint16_t(-int(layout.origin*320*4/1024)))<<16:0;
        command(ram,0x64000006,layout.origin|(layout.origin<<12));command(ram,offset,offset);
    }
    if(!emitted_valid||layout.viewport!=emitted.viewport){
        const uint32_t offset=layout.viewport<None?uint32_t(uint16_t(-int(layout.viewport*320*4/1024)))<<16:0;
        command(ram,0x64000007,layout.viewport);command(ram,offset,0);
    }
    emitted=layout;emitted_valid=true;
}
std::vector<uint8_t> decode(size_t record){
    const auto rom=recomp::get_rom();
    auto be=[&](size_t p){if(p+4>rom.size())throw std::logic_error("Widescreen ROM bounds");return uint32_t(rom[p])<<24|uint32_t(rom[p+1])<<16|uint32_t(rom[p+2])<<8|uint32_t(rom[p+3]);};
    const auto flag=be(record),packed=be(record+8),size=be(record+12);
    if(flag>1||be(record+4)||!size||size>0x100000||record+16+packed>rom.size())throw std::logic_error("Widescreen resource bounds");
    std::vector<uint8_t> bytes;bytes.reserve(size);std::array<uint8_t,1024> ring{};unsigned write=0x3BE;
    size_t pos=record+16,end=pos+packed;
    auto emit=[&](uint8_t v){if(bytes.size()>=size)throw std::logic_error("Widescreen LZSS length");bytes.push_back(v);ring[write]=v;write=(write+1)&1023;};
    if(flag==0){while(pos<end)emit(rom[pos++]);}
    else while(pos<end&&bytes.size()<size){const auto flags=rom[pos++];for(unsigned bit=0;bit<8&&pos<end&&bytes.size()<size;++bit){
        if(flags&(1u<<bit))emit(rom[pos++]);else{if(pos+2>end)throw std::logic_error("Widescreen LZSS bounds");const auto lo=rom[pos++],hi=rom[pos++];const unsigned index=lo|((hi&0xC0)<<2),count=(hi&63)+3;for(unsigned j=0;j<count;++j)emit(ring[(index+j)&1023]);}
    }}
    if(bytes.size()!=size||std::memcmp(bytes.data(),"BMP",3))throw std::logic_error("Widescreen BMP header");
    return bytes;
}
std::array<std::vector<uint8_t>,2> art;
bool finite_art(uint8_t* ram,recomp_context* c){
    if(c->r6!=320||c->r7!=240||c->f12.fl!=0.f||c->f14.fl!=0.f||
        uint32_t(word(ram,uint32_t(c->r29)+0x10))!=0x3F800000||
        uint32_t(word(ram,uint32_t(c->r29)+0x14))!=0x3F800000)return false;
    if(art[0].empty()){
        art[0]=decode(0x4DEC40);art[1]=decode(0x568630);
        // Compare complete immutable palette and texels, not a texture hash or
        // an apparent color/rectangle shape. Only the two ROM assets qualify.
        for(auto& a:art)for(size_t j=0;j+3<a.size();j+=4){std::swap(a[j],a[j+3]);std::swap(a[j+1],a[j+2]);}
    }
    const uint32_t pixels=uint32_t(word(ram,uint32_t(c->r29)+0x18))&0x1FFFFFFF;
    const uint32_t palette=uint32_t(word(ram,uint32_t(c->r29)+0x1C))&0x1FFFFFFF;
    for(unsigned i=0;i<2;++i){const size_t pal=i?32:512,bytes=i?38400:76800;
        if(pixels+bytes<=distance::game_ram_end-0x80000000u&&palette+pal<=distance::game_ram_end-0x80000000u&&
            !std::memcmp(ram+palette,art[i].data()+16,pal)&&!std::memcmp(ram+pixels,art[i].data()+16+pal,bytes))return true;
    }
    return false;
}
void blue(uint8_t* ram,recomp_context* source){
    if(!enabled||drawing_underlay)return;
    if(!blue_asset){
        const auto bytes=decode(0x543FC0);const auto size=bytes.size();
        if(size!=38448)throw std::logic_error("Widescreen blue resource changed");
        auto* memory=static_cast<uint8_t*>(recomp::alloc(ram,size+1024));
        if(!memory)throw std::bad_alloc();
        const uint64_t a=uint64_t(memory-ram)+0x80000000ull;
        if(a+size+1024>0xA0000000ull)throw std::bad_alloc();
        blue_asset=uint32_t(a);scratch=blue_asset+size+768;
        for(unsigned j=0;j<size;++j)memory[j^3]=bytes[j];
    }
    const auto saved=layout;layout.origin=Blue;layout.aspect=2;apply(ram);
    recomp_context c;copy_recomp_context(c,*source);c.r29=int32_t(scratch);c.r31=0;
    drawing_underlay=true;
    dora3_texture_rect_state(ram,&c);
    c.f12.fl=0;c.f14.fl=0;c.r6=320;c.r7=240;c.r31=0;
    word(ram,scratch+0x10)=0x3F800000;word(ram,scratch+0x14)=0x3F800000;
    word(ram,scratch+0x18)=int32_t(blue_asset+48);word(ram,scratch+0x1C)=int32_t(blue_asset+16);word(ram,scratch+0x20)=0;
    dora3_scaled_texture_rect(ram,&c);
    drawing_underlay=false;layout=saved;apply(ram);
    // Restore the bitmap pipeline before the caller's own texture/TLUT load.
    dora3_texture_rect_state(ram,&c);
}
}
void reset(){scopes.clear();layout={};blue_asset=scratch=0;drawing_underlay=false;enabled=hud_enabled=0;art={};emitted_valid=false;}
int aspect(){return enabled;}int hud(){return hud_enabled;}
void begin(uint8_t* ram){
#ifdef DORA3_LIVE_RT64
    const auto s=port::settings();enabled=s.aspect_ratio;hud_enabled=s.hud_layout;
#endif
    scopes.clear();scopes.reserve(32);layout={};emitted_valid=false;command(ram,0xE0525464,0x10000064);apply(ram);
}
}
extern "C" void dora3_wide_enter(uint8_t* ram,recomp_context* ctx,unsigned kind){
    using namespace dora3::wide;
    if(!enabled)return;
    if(kind==10){
        // Original projection index1 is the animated menu backdrop; index0
        // holds the independently composed item/weapon/character models.
        const unsigned mode=layout.menu&&ctx->r4==1?1:0;
        command(ram,0x6400000C,0xFFFFFFFF);command(ram,2u|(2u<<3)|(2u<<5)|(2u<<7)|(2u<<9)|(2u<<11)|(2u<<13)|(2u<<15)|(1u<<17)|(mode<<20)|(2u<<22)|(2u<<24),0);
        return;
    }
    scopes.push_back(layout);
    switch(kind){
    case 1: // Bitmap nested inside an explicit ROM-qualified draw scope.
        if(!drawing_underlay&&layout.origin!=Blue&&layout.origin!=Title&&finite_art(ram,ctx))blue(ram,ctx);
        break;
    case 2:layout.origin=0;break;
    case 3:layout.origin=0x400;break;
    case 4:layout.aspect=1;layout.origin=None;break;
    case 11:
        if(word(ram,uint32_t(ctx->r29)+0x10)<=0&&word(ram,uint32_t(ctx->r29)+0x14)<=0&&
           word(ram,uint32_t(ctx->r29)+0x18)>=319&&word(ram,uint32_t(ctx->r29)+0x1C)>=239){layout.aspect=1;layout.origin=None;}
        break;
    case 5:layout.viewport=Center;break; // Entire map composition, including nodes.
    case 6:blue(ram,ctx);break;
    case 9:layout.menu=true;break;
    case 12:layout.origin=Blue;break;
    case 13:layout.origin=Title;break;
    case 14:layout.origin=0x803;layout.aspect=2;break; // Circular mask, unscaled texel slope.
    case 15:layout.viewport=0x803;break; // Finite background model only: stretch its projection.
    case 17:layout.origin=ctx->f12.fl<160.f?0x100:0x300;break;
    case 18:layout.origin=ctx->f12.fl<160.f?0:0x400;break;
    case 19:layout.origin=Center;break;
    // Per-view content follows the image aspect even with Original HUD. These
    // markers encode fixed quarter/three-quarter centers in the renderer.
    case 20:layout.origin=ctx->f12.fl<160.f?0x804:0x805;break;
    case 21:layout.origin=(uint32_t(ctx->r4)&1)?0x400:0;break; // Complete HP group.
    }
    apply(ram);
}
extern "C" void dora3_wide_leave(uint8_t* ram,recomp_context*,unsigned){
    using namespace dora3::wide;
    if(!enabled)return;
    if(scopes.empty())throw std::logic_error("Widescreen layer boundary mismatch");
    layout=scopes.back();scopes.pop_back();apply(ram);
}
