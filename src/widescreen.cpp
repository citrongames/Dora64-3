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
struct Scope { Layout saved; uint32_t draw_begin; };
std::vector<Scope> scopes;
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
std::vector<uint8_t> decode(size_t record,const char* signature="BMP"){
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
    if(bytes.size()!=size||size<std::strlen(signature)||std::memcmp(bytes.data(),signature,std::strlen(signature)))throw std::logic_error("Widescreen resource header");
    return bytes;
}
// B84/B96 combine buildings, ground and a distant sky quad. Extend only
// sky and ground in an immutable rendering copy; gameplay bounds stay original.
struct SkyVertices { std::vector<uint8_t> original; uint32_t address=0; };
std::array<SkyVertices,2> skies;
void extend_sky(uint8_t* ram,uint32_t begin,unsigned index){
    const auto end=uint32_t(word(ram,0x801B1424));
    if(end<=begin||!distance::frame_pointer(begin,uint64_t(end)-begin))return;
    auto& sky=skies[index];
    if(sky.original.empty()){
        const auto data=decode(index?0x4D7EC0:0x4CEBC0,"GIMO");
        const size_t offset=index?0x4598:0x55D8,count=index?380:440,first=index?372:100;
        if(data.size()!=(index?27276u:34496u)||offset+count*16>data.size())
            throw std::logic_error("Widescreen sky resource changed");
        sky.original.assign(data.begin()+offset,data.begin()+offset+count*16);
        auto wide=sky.original;
        auto read16=[&](size_t p){return int16_t(uint16_t(wide[p])<<8|wide[p+1]);};
        auto write16=[&](size_t p,int value){wide[p]=uint16_t(value)>>8;wide[p+1]=uint8_t(value);};
        for(size_t i=first;i<first+4;++i){
            const int x=read16(i*16),s=read16(i*16+8);
            if(x!=(i<first+2?1000:-1000)||s!=(i<first+2?7168:-3072))
                throw std::logic_error("Widescreen sky vertices changed");
            write16(i*16,x*4);
            write16(i*16+8,2048+(s-2048)*4); // Same cloud texel density/phase.
        }
        // ROM-qualified ground only: B84 has16 flat quads, B96 one quad.
        // Stretch X/texture together as requested; retain Y/Z, UVs and colors.
        // Building bases also reach Y=0, so never select vertices by height alone.
        const size_t ground_first=index?100:372,ground_count=index?4:64;
        for(size_t i=ground_first;i<ground_first+ground_count;++i){
            const int x=read16(i*16);
            if(read16(i*16+2)!=0||x<-700||x>700||x%350!=0)
                throw std::logic_error("Widescreen ground vertices changed");
            write16(i*16,x*4);
        }
        auto* memory=static_cast<uint8_t*>(recomp::alloc(ram,wide.size()));
        if(!memory)throw std::bad_alloc();
        const uint64_t address=uint64_t(memory-ram)+0x80000000ull;
        if(address+wide.size()>0xA0000000ull)throw std::bad_alloc();
        sky.address=uint32_t(address);
        for(size_t i=0;i<wide.size();++i)memory[i^3]=wide[i];
        for(size_t i=0;i<sky.original.size();i+=4){
            std::swap(sky.original[i],sky.original[i+3]);
            std::swap(sky.original[i+1],sky.original[i+2]);
        }
    }
    // Original8003AC14 binds GIMO vertex segment2. Retarget only commands
    // emitted by this ROM-qualified background call, after full vertex identity
    // comparison. Pending frames retain immutable buffers across scene changes.
    for(uint32_t p=begin;p<end;p+=8){
        if(uint32_t(word(ram,p))!=0xDB060008u)continue;
        const auto address=uint32_t(word(ram,p+4))&0x1FFFFFFFu;
        if(uint64_t(address)+sky.original.size()>distance::game_ram_size)continue;
        if(!std::memcmp(ram+address,sky.original.data(),sky.original.size()))
            // Keep the extended-RDRAM tag in the segment base. RSP adds the
            // vertex offset, then maskPhysicalAddress preserves tagged addresses;
            // stripping it truncates this allocation to24 bits and reads garbage.
            word(ram,p+4)=int32_t(sky.address);
    }
}
// B0C separates grass, red line/checker atlas and the distant wall material.
// Replay their original lists unchanged, then append right-hand decorative quads.
struct FruitExtension {
    std::array<std::vector<uint8_t>,3> original;
    std::array<uint32_t,3> lists{};
} fruit;
uint32_t immutable_bytes(uint8_t* ram,const std::vector<uint8_t>& bytes){
    auto* memory=static_cast<uint8_t*>(recomp::alloc(ram,bytes.size()));
    if(!memory)throw std::bad_alloc();
    const uint64_t address=uint64_t(memory-ram)+0x80000000ull;
    if(address+bytes.size()>0xA0000000ull)throw std::bad_alloc();
    for(size_t i=0;i<bytes.size();++i)memory[i^3]=bytes[i];
    return uint32_t(address); // Preserve extended-RDRAM tag in DL/VTX operands.
}
void extend_fruit(uint8_t* ram,uint32_t begin){
    const auto end=uint32_t(word(ram,0x801B1424));
    if(end<=begin||!distance::frame_pointer(begin,uint64_t(end)-begin))return;
    if(!fruit.lists[0]){
        const auto data=decode(0x4B2CA0,"GIMO");
        if(data.size()!=13048)throw std::logic_error("Widescreen fruit resource changed");
        auto vertex=[&](unsigned i){return std::vector<uint8_t>(data.begin()+0x1900+i*16,data.begin()+0x1910+i*16);};
        auto read16=[](const std::vector<uint8_t>& v,size_t p){return int16_t(uint16_t(v[p])<<8|v[p+1]);};
        auto write16=[](std::vector<uint8_t>& v,size_t p,int n){v[p]=uint16_t(n)>>8;v[p+1]=uint8_t(n);};
        auto append32=[](std::vector<uint8_t>& v,uint32_t n){for(int s=24;s>=0;s-=8)v.push_back(uint8_t(n>>s));};
        std::array<std::vector<uint8_t>,3> vertices;
        auto grass_vertex=[&](unsigned i,bool outer){
            auto v=vertex(i);
            if(read16(v,0)!=800||read16(v,2)!=0)throw std::logic_error("Widescreen fruit ground changed");
            if(outer){write16(v,0,2800);write16(v,8,read16(v,8)+13280);}
            vertices[0].insert(vertices[0].end(),v.begin(),v.end());
        };
        // Preserve winding, edge colors, and grass UV phase at original X=800.
        grass_vertex(65,false);grass_vertex(65,true);grass_vertex(66,true);grass_vertex(66,false);
        grass_vertex(68,true);grass_vertex(68,false);grass_vertex(71,false);grass_vertex(71,true);
        for(unsigned col=0;col<20;++col){
            const int origin=col%2?540:640;
            unsigned quads=0;
            for(unsigned first=100;first<292;first+=4){
                int low=32767,high=-32768;
                for(unsigned j=0;j<4;++j){const int x=read16(vertex(first+j),0);if(x<low)low=x;if(x>high)high=x;}
                if(low!=origin||high!=origin+100)continue;
                for(unsigned j=0;j<4;++j){
                    auto v=vertex(first+j);
                    if(read16(v,2)!=0)throw std::logic_error("Widescreen fruit cells changed");
                    write16(v,0,read16(v,0)+800+int(col)*100-origin);
                    vertices[1].insert(vertices[1].end(),v.begin(),v.end());
                }
                ++quads;
            }
            if(quads!=6)throw std::logic_error("Widescreen fruit row count changed");
        }
        // Original wall quad20..23: continue its right edge at the same height
        // and depth. Preserve the repeating128px wall texture's density/phase.
        auto wall_vertex=[&](unsigned i,bool outer){
            auto v=vertex(i);
            if(read16(v,0)!=800||read16(v,4)!=-500||read16(v,2)!=(i==21?0:400))
                throw std::logic_error("Widescreen fruit wall changed");
            if(outer){write16(v,0,2800);write16(v,8,read16(v,8)+(2000*12288+775)/1550);}
            vertices[2].insert(vertices[2].end(),v.begin(),v.end());
        };
        wall_vertex(21,false);wall_vertex(21,true);wall_vertex(22,true);wall_vertex(22,false);
        const std::array<size_t,3> starts{0x2E70,0x2F10,0x2C80},ends{0x2F08,0x3170,0x2D08};
        for(unsigned i=0;i<3;++i){
            fruit.original[i].assign(data.begin()+starts[i],data.begin()+ends[i]+8);
            auto list=std::vector<uint8_t>(data.begin()+starts[i],data.begin()+ends[i]);
            const auto vbase=immutable_bytes(ram,vertices[i]);
            for(size_t offset=0;offset<vertices[i].size();offset+=64){
                append32(list,0x01004008);append32(list,vbase+uint32_t(offset));
                append32(list,0x06000206);append32(list,0x00020406);
            }
            append32(list,0xDF000000);append32(list,0);
            fruit.lists[i]=immutable_bytes(ram,list);
            for(size_t j=0;j<fruit.original[i].size();j+=4){
                std::swap(fruit.original[i][j],fruit.original[i][j+3]);
                std::swap(fruit.original[i][j+1],fruit.original[i][j+2]);
            }
        }
    }
    // Only this exact background's three material calls qualify. Keep their
    // original segment bases, textures and model matrix, then add our vertices.
    for(uint32_t p=begin;p<end;p+=8){
        if(uint32_t(word(ram,p))!=0xDE000000u)continue;
        const uint32_t address=uint32_t(word(ram,p+4))&0x1FFFFFFFu;
        for(unsigned i=0;i<3;++i){
            if(uint64_t(address)+fruit.original[i].size()<=distance::game_ram_size&&
               !std::memcmp(ram+address,fruit.original[i].data(),fruit.original[i].size())){
                word(ram,p+4)=int32_t(fruit.lists[i]);break;
            }
        }
    }
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
    // The Original scaled leaf clips X below 2 and Y below 1, even
    // with a full-screen scissor. Draw within that safe region, then move
    // its emitted rectangles back; texel zero must cover screen zero.
    const auto rect_begin=uint32_t(word(ram,0x801B1424));
    c.f12.fl=2;c.f14.fl=1;c.r6=320;c.r7=240;c.r31=0;
    word(ram,scratch+0x10)=0x3F800000;word(ram,scratch+0x14)=0x3F800000;
    word(ram,scratch+0x18)=int32_t(blue_asset+48);word(ram,scratch+0x1C)=int32_t(blue_asset+16);word(ram,scratch+0x20)=0;
    dora3_scaled_texture_rect(ram,&c);
    const auto rect_end=uint32_t(word(ram,0x801B1424));
    for(auto p=rect_begin;p+8<=rect_end;p+=8) {
        if((uint32_t(word(ram,p))>>24)!=0xE4)continue;
        for(unsigned j=0;j<2;++j) {
            const auto w=uint32_t(word(ram,p+4*j));
            const auto x=(w>>12)&0xFFF,y=w&0xFFF;
            if(x<8||y<4)throw std::logic_error("Widescreen blue rectangle bounds");
            word(ram,p+4*j)=int32_t((w&0xFF000000)|((x-8)<<12)|(y-4));
        }
    }
    drawing_underlay=false;layout=saved;apply(ram);
    // Restore the bitmap pipeline before the caller's own texture/TLUT load.
    dora3_texture_rect_state(ram,&c);
}
}
void reset(){scopes.clear();layout={};blue_asset=scratch=0;drawing_underlay=false;enabled=hud_enabled=0;art={};skies={};fruit={};emitted_valid=false;}
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
    scopes.push_back({layout,uint32_t(word(ram,0x801B1424))});
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
    case 24:break; // Right-hand grass/checker extension on leave.
    case 22:case 23:break; // Normal world projection; sky-only vertex copy on leave.
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
extern "C" void dora3_wide_leave(uint8_t* ram,recomp_context*,unsigned kind){
    using namespace dora3::wide;
    if(!enabled)return;
    if(scopes.empty())throw std::logic_error("Widescreen layer boundary mismatch");
    if(kind==24)extend_fruit(ram,scopes.back().draw_begin);
    if(kind==22||kind==23)extend_sky(ram,scopes.back().draw_begin,kind-22);
    layout=scopes.back().saved;scopes.pop_back();apply(ram);
}
