#include "native_texture_animation.hpp"
#include "native_frame_bridge.hpp"
#include "frame_lights.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

namespace {
using namespace dora3::native_time;
uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));}
bool pointer(uint32_t p,uint64_t n){return p>=0x80000000u&&uint64_t(p)+n<=0x80400000ull;}
struct Bitmap {uint32_t header=0,pixels=0,palette=0;unsigned width=0,height=0,format=0,colors=0,palette_bytes=0;};
std::optional<Bitmap> bitmap(uint8_t* rdram,uint32_t p) {
    if(!pointer(p,16)||(p&7)||MEM_BU(0,int32_t(p))!='B'||MEM_BU(1,int32_t(p))!='M'||MEM_BU(2,int32_t(p))!='P')return {};
    Bitmap b{p,0,p+16,word(rdram,p+8),word(rdram,p+12),MEM_BU(3,int32_t(p)),MEM_HU(6,int32_t(p))};
    // SDK282EC places formats<5 directly after the16-byte header. The ROM
    // format2 family0653..656 is byte-per-pixel, confirmed by every resource
    // length (64*32+16). CI4 format5 can have fewer than16 palette entries:
    // 09CE..9D4 uses3 colors and bitmap0521..528 uses6, not16.
    if((b.format!=2&&b.format!=5)||!b.width||!b.height||b.width>64||b.height>64||(b.width&1))return {};
    if(b.format==5&&(!b.colors||b.colors>16))return {};
    b.palette_bytes=b.format==5?((2*b.colors+7)&~7u):0;
    if(b.format<5)b.palette=0; // SDK28320 returns no palette for byte format2.
    b.pixels=p+16+b.palette_bytes;
    if(!pointer(b.pixels,uint64_t(b.width)*b.height/(b.format==5?2:1)))return {};
    return b;
}
unsigned pixel(uint8_t* rdram,const Bitmap& b,unsigned x,unsigned y) {
    const auto at=y*b.width+x;
    const auto v=MEM_BU(int32_t(b.format==5?at/2:at),int32_t(b.pixels));
    return b.format==5?(x&1?v&15:v>>4):v;
}
struct Shift {int x=0,y=0;bool translated=false;};
struct CachedShift {uint64_t first=0,second=0;Shift value;};
uint64_t bitmap_signature(uint8_t* rdram,const Bitmap& b) {
    uint64_t hash=1469598103934665603ull;
    for(unsigned i=0;i<16;++i)hash=(hash^MEM_BU(int32_t(i),int32_t(b.header)))*1099511628211ull;
    for(unsigned i=0;i<b.palette_bytes;++i)hash=(hash^MEM_BU(int32_t(i),int32_t(b.palette)))*1099511628211ull;
    for(unsigned i=0;i<b.width*b.height/(b.format==5?2:1);++i)hash=(hash^MEM_BU(int32_t(i),int32_t(b.pixels)))*1099511628211ull;
    return hash;
}
Shift translated_bitmap(uint8_t* rdram,const Bitmap& a,const Bitmap& b) {
    if(a.width!=b.width||a.height!=b.height||a.format!=b.format||a.colors!=b.colors)return {};
    if(word(rdram,a.header+4)!=word(rdram,b.header+4))return {}; // Same authored header/format flags too.
    for(unsigned i=0;i<a.palette_bytes;++i)if(MEM_BU(int32_t(i),int32_t(a.palette))!=MEM_BU(int32_t(i),int32_t(b.palette)))return {};
    // This recognizes an authored toroidal scroll, never blends bitmap images.
    // ROM water113..122 is precisely the same32x32 CI4 bitmap shifted by2px.
    Shift result;unsigned best=~0u;
    for(unsigned y=0;y<a.height;++y)for(unsigned x=0;x<a.width;++x) {
        const int sx=x>a.width/2?int(x)-int(a.width):int(x),sy=y>a.height/2?int(y)-int(a.height):int(y);
        const unsigned length=unsigned(sx*sx+sy*sy);if(length>=best)continue;
        bool same=true;
        for(unsigned row=0;row<a.height&&same;++row)for(unsigned col=0;col<a.width;++col)
            if(pixel(rdram,b,col,row)!=pixel(rdram,a,(col+x)%a.width,(row+y)%a.height)){same=false;break;}
        if(same){result={sx,sy,true};best=length;}
    }
    return result;
}
struct Binding {uint32_t base=0,palette=0;unsigned width=0,height=0;double x=0,y=0;bool scroll=false;};
struct Parser {std::optional<Binding> image;std::array<uint32_t,8> tiles{};};
struct Copy {uint32_t address;Parser exit;};
struct FramePatch {uint32_t address,first,second,published_first,published_second;};
struct State {
    uint8_t* source_ram=nullptr,*frame_ram=nullptr;
    Nanoseconds source_time=0;
    double h=0;
    uint32_t bank=0,arena=0;
    bool active=false;
    std::array<Binding,16> segments{};
    std::map<std::pair<uint32_t,uint32_t>,CachedShift> shifts;
    std::vector<uint32_t> builders;
    std::vector<FramePatch> patches;
    std::map<std::pair<uint32_t,uint64_t>,Copy> copies;
} state;
uint32_t resolve(uint32_t p) {
    if(p>=0x80000000u&&p<0x80400000u)return p;
    if(p<0x400000u)return p|0x80000000u;
    const auto segment=p>>24;
    if(segment<16&&state.segments[segment].base) {
        const uint64_t result=uint64_t(state.segments[segment].base)+(p&0xffffffu);
        if(result<0x80400000ull)return uint32_t(result);
    }
    return 0;
}
// A cloned resource display list remains in the current ticket's bank until
// both guest and host completion. CPU Source RAM/asset data are never changed.
// Tail allocation keeps the original builder's local and global cursors intact.
uint32_t allocate(uint8_t* rdram,const std::vector<uint32_t>& commands) {
    const auto bytes=uint32_t(commands.size()*4);
    const auto cursor=word(rdram,0x801B1424);
    if(bytes>state.arena-state.bank||cursor<state.bank+dora3::frame::dl_offset||
       uint64_t(cursor)+0x10000u+bytes>state.arena)return 0;
    const auto at=(state.arena-bytes)&~7u;
    for(unsigned i=0;i<commands.size();++i)MEM_W(0,int32_t(at+4*i))=int32_t(commands[i]);
    state.arena=at;return at;
}
uint64_t binding_signature() {
    uint64_t result=1469598103934665603ull;
    for(const auto& b:state.segments)for(const auto v:{uint64_t(b.base),uint64_t(std::llround(b.x*4)),uint64_t(std::llround(b.y*4))})
        result=(result^v)*1099511628211ull;
    return result;
}
uint32_t clone_list(uint8_t*,uint32_t,unsigned,std::set<uint32_t>&,Parser&);
bool rewrite_command(uint8_t* rdram,uint32_t& first,uint32_t& second,unsigned depth,std::set<uint32_t>& path,Parser& parser) {
    auto& image=parser.image;bool changed=false;const auto op=first>>24;
        if(op==0xFD) {
            image.reset();const auto segment=second>>24;
            if(segment<16&&!(second&0xffffffu)&&state.segments[segment].scroll)image=state.segments[segment];
            else {const auto actual=resolve(second);for(const auto& b:state.segments)if(b.scroll&&b.base==actual){image=b;break;}}
        } else if(op==0xF5)parser.tiles[(second>>24)&7]=second;
        else if(op==0xF2&&image) {
            const auto& b=*image;
            // RDP tile origin subtracts from the vertex's S/T. A translated
            // bitmap reads (S+shift), hence the negative fractional tile origin.
            const int dx=int(std::llround(b.x*4)),dy=int(std::llround(b.y*4));
            const auto tile=parser.tiles[(second>>24)&7];
            const bool wrap_s=((tile>>8)&3)==0&&(tile&15)==0&&((1u<<((tile>>4)&15))==b.width);
            const bool wrap_t=((tile>>18)&3)==0&&((tile>>10)&15)==0&&((1u<<((tile>>14)&15))==b.height);
            const auto move=[&](unsigned shift,int delta,unsigned period,bool allowed) {
                const int low=(first>>shift)&4095,high=(second>>shift)&4095;
                if(!allowed||!delta||high<low)return;
                // Keep UL/LR ordered and the same width. Encoding a negative UL
                // as4095 would make a host decoder infer a negative tile width.
                int next=(low-delta)%int(period);if(next<0)next+=int(period);
                const int last=next+high-low;if(last>4095)return;
                const uint32_t mask=4095u<<shift;
                const uint32_t a=(first&~mask)|(uint32_t(next)<<shift),c=(second&~mask)|(uint32_t(last)<<shift);
                changed|=a!=first||c!=second;first=a;second=c;
            };
            move(12,dx,b.width*4,wrap_s);move(0,dy,b.height*4,wrap_t);
        } else if(op==0xDE) {
            const auto copied=clone_list(rdram,second,depth+1,path,parser);
            changed|=copied!=second;second=copied==second?second:(copied&0x1fffffff);

        }
    return changed;
}
uint32_t clone_list(uint8_t* rdram,uint32_t root,unsigned depth,std::set<uint32_t>& path,Parser& parser) {
    const auto address=resolve(root);if(depth>8||!pointer(address,8)||(address&7)||!path.insert(address).second)return root;
    const auto key=std::pair{address,binding_signature()};
    // A parent can inherit the current texture image; only cache independent
    // lists. This avoids conflating the same mesh under different material state.
    const bool independent=!parser.image&&std::all_of(parser.tiles.begin(),parser.tiles.end(),[](auto tile){return tile==0;});
    if(independent)if(auto at=state.copies.find(key);at!=state.copies.end()){parser=at->second.exit;path.erase(address);return at->second.address;}
    std::vector<uint32_t> commands;bool changed=false,ended=false;
    for(unsigned i=0;i<2048;++i) {
        const auto p=address+8*i;if(!pointer(p,8))break;
        auto first=word(rdram,p),second=word(rdram,p+4);const auto op=first>>24;
        changed|=rewrite_command(rdram,first,second,depth,path,parser);
        ended=op==0xDF||(op==0xDE&&(first&0x10000u));
        commands.push_back(first);commands.push_back(second);if(ended)break;
    }
    path.erase(address);
    if(!ended||!changed)return root;
    const auto copied=allocate(rdram,commands);if(!copied)return root;
    if(independent)state.copies.emplace(key,Copy{copied,parser});
    return copied;
}
}
namespace dora3::native_time {
void native_texture_source_completed(uint8_t* ram,Nanoseconds now){state.source_ram=ram;state.source_time=now;}
void native_texture_reset(){if(state.active)throw std::logic_error("Texture reset inside frame");state=State{};}
TextureFrameScope::TextureFrameScope(uint8_t* ram,Nanoseconds now,unsigned bank):ram_(ram) {
    const auto* binding=active_frame_binding();if(!binding||binding->ticket.bank!=bank||state.active)throw std::logic_error("Invalid texture frame ownership");
    state.active=true;state.frame_ram=ram;state.bank=dora3::frame::bank0+bank*dora3::frame::bank_stride;
    state.arena=state.bank+dora3::frame::bank_stride-0x100;
    state.h=state.source_ram==ram?std::clamp(double(now-state.source_time)*60./(word(ram,0x801A39DC)*1e9),0.,1.):0.;
    state.segments={};state.copies.clear();state.builders.clear();state.patches.clear();
}
TextureFrameScope::~TextureFrameScope(){if(ram_){state.active=false;state.frame_ram=nullptr;state.segments={};state.copies.clear();state.builders.clear();state.patches.clear();}}
}
extern "C" void dora3_native_texture_bind(uint8_t* rdram,recomp_context* c,unsigned world) {
    if(!state.active||state.frame_ram!=rdram)return;
    const auto actor=uint32_t(world?c->r20:c->r4),slot=unsigned(world?c->r18:c->r5);
    if(slot>=8||!pointer(actor,0xcc)||MEM_BU(int32_t(0x6b+slot),int32_t(actor))!=5)return;
    const auto handle=word(rdram,actor+0x74+4*slot);if(handle>=300)return;
    const auto entry=0x801A9CB0+8*handle,sequence=word(rdram,entry);
    if(!pointer(sequence,12))return;
    const unsigned segment=MEM_BU(8,int32_t(sequence)),palette=MEM_BU(9,int32_t(sequence));
    if(segment>=16||palette>=16)return;
    state.segments[segment]={};state.segments[palette]={};
    const auto indices=word(rdram,sequence),durations=word(rdram,sequence+4);
    const unsigned index=MEM_BU(5,int32_t(entry));
    if(!pointer(indices,uint64_t(index)+2)||!pointer(durations,uint64_t(index)+1))return;
    const unsigned current=MEM_BU(int32_t(index),int32_t(indices)),raw_next=MEM_BU(int32_t(index+1),int32_t(indices));
    const unsigned flags=MEM_BU(6,int32_t(entry));
    const unsigned next=raw_next==255?(flags&2?current:MEM_BU(0,int32_t(indices))):raw_next;
    if(current==255||next==255)return;
    const auto table=world?0x80217120u:word(rdram,actor+0xb8);
    if(!pointer(table,2*(std::max(current,next)+1)))return;
    const auto resource=[&](unsigned n)->uint32_t{
        const int id=MEM_H(int32_t(2*n),int32_t(table));return id>=0&&id<512?word(rdram,0x801AEC00+20*unsigned(id)):0;
    };
    const auto a=bitmap(rdram,resource(current)),b=bitmap(rdram,resource(next));if(!a||!b)return;
    state.segments[segment]={a->pixels,a->palette,a->width,a->height,0,0,false};
    state.segments[palette].base=a->palette;
    if(!(flags&1))return; // Source controls active/loop/clamp; last clamped frame holds
    const auto key=std::pair{a->header,b->header};const auto first=bitmap_signature(rdram,*a),second=bitmap_signature(rdram,*b);
    auto at=state.shifts.find(key);
    if(at==state.shifts.end()||at->second.first!=first||at->second.second!=second) {
        // Resource slots may be freed/reused inside the same loaded scene.
        if(state.shifts.size()>=512)state.shifts.clear();
        at=state.shifts.insert_or_assign(key,CachedShift{first,second,translated_bitmap(rdram,*a,*b)}).first;
    }
    if(!at->second.value.translated)return;
    const unsigned duration=MEM_BU(int32_t(index),int32_t(durations)),remaining=MEM_BU(4,int32_t(entry));
    if(!duration||remaining>=duration)return;
    const double h=(double(duration-1-remaining)+state.h)/duration;
    auto& binding=state.segments[segment];binding.x=at->second.value.x*h;binding.y=at->second.value.y*h;binding.scroll=true;
}
extern "C" void dora3_native_texture_builder(uint8_t* rdram,recomp_context* c,unsigned end) {
    if(!state.active||state.frame_ram!=rdram)return;
    if(!end) {
        const auto cursor_pointer=uint32_t(c->r4);
        state.builders.push_back(pointer(cursor_pointer,4)?word(rdram,cursor_pointer):0);return;
    }
    if(state.builders.empty())throw std::logic_error("Unmatched material builder boundary");
    const auto begin=state.builders.back(),last=uint32_t(c->r2);state.builders.pop_back();
    if(begin<state.bank+dora3::frame::dl_offset||last<begin||last>state.arena||((begin|last)&7))
        return; // Other Source-owned builders are outside this ticket's lease.
    if(std::none_of(state.segments.begin(),state.segments.end(),[](const auto& b){return b.scroll;}))return;
    // The actual SDK builds material setup before the mesh DE. Process that
    // prefix too, carrying its image/tile state into every nested mesh list.
    std::set<uint32_t> path;Parser parser;
    for(uint32_t at=begin;at<last;at+=8) {
        auto first=word(rdram,at),second=word(rdram,at+4);
        const auto original_first=first,original_second=second;
        if(rewrite_command(rdram,first,second,0,path,parser)) {
            state.patches.push_back({at,original_first,original_second,first,second});
            MEM_W(0,int32_t(at))=int32_t(first);MEM_W(0,int32_t(at+4))=int32_t(second);
        }
    }
}
extern "C" void dora3_native_texture_submit_guard(uint8_t* rdram,recomp_context*) {
    if(!state.active||state.frame_ram!=rdram)return;
    if(word(rdram,0x801B1424)<=state.arena)return;
    // A larger Source frame may use the reserved tail after material building.
    // Drop every outer reference to clones before submit and retain its exact
    // Source texture commands. An unusually large frame must not crash the game.
    for(auto i=state.patches.rbegin();i!=state.patches.rend();++i) {
        if(word(rdram,i->address)==i->published_first&&word(rdram,i->address+4)==i->published_second) {
            MEM_W(0,int32_t(i->address))=int32_t(i->first);
            MEM_W(0,int32_t(i->address+4))=int32_t(i->second);
        }
    }
    state.patches.clear();
}
