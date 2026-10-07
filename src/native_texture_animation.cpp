#include "native_texture_animation.hpp"
#include "native_frame_bridge.hpp"
#include "native_simulation_time.hpp"
#include "frame_lights.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <tuple>

namespace {
using namespace dora3::native_time;
uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));}
bool pointer(uint32_t p,uint64_t n){return p>=0x80000000u&&uint64_t(p)+n<=0x80400000ull;}
struct Bitmap {uint32_t header=0,pixels=0,palette=0;unsigned width=0,height=0,format=0,colors=0,palette_bytes=0;};
std::optional<Bitmap> bitmap_raw(uint8_t* rdram,uint32_t p) {
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
    if(b.format==5)for(unsigned i=0;i<b.width*b.height/2;++i) {
        const auto packed=MEM_BU(int32_t(i),int32_t(b.pixels));
        if((packed>>4)>=b.colors||(packed&15)>=b.colors)return {};
    }
    return b;
}
unsigned pixel(uint8_t* rdram,const Bitmap& b,unsigned x,unsigned y) {
    const auto at=y*b.width+x;
    const auto v=MEM_BU(int32_t(b.format==5?at/2:at),int32_t(b.pixels));
    const auto index=b.format==5?(x&1?v&15:v>>4):v;
    // CI4 indices are local to each bitmap's palette. ROM water00DF and
    // 00E0..00EE reorder equal colors, including duplicate entries. Compare
    // the actual RGBA5551 texel so a palette permutation cannot break the
    // scroll at the loop seam. Format2 has no palette (SDK28320).
    return b.format==5?MEM_HU(int32_t(2*index),int32_t(b.palette)):index;
}
struct Shift {int x=0,y=0;bool translated=false;};
struct CachedShift {uint64_t first=0,second=0;Shift value;};
uint64_t bitmap_signature_raw(uint8_t* rdram,const Bitmap& b) {
    uint64_t hash=1469598103934665603ull;
    for(unsigned i=0;i<16;++i)hash=(hash^MEM_BU(int32_t(i),int32_t(b.header)))*1099511628211ull;
    for(unsigned i=0;i<b.palette_bytes;++i)hash=(hash^MEM_BU(int32_t(i),int32_t(b.palette)))*1099511628211ull;
    for(unsigned i=0;i<b.width*b.height/(b.format==5?2:1);++i)hash=(hash^MEM_BU(int32_t(i),int32_t(b.pixels)))*1099511628211ull;
    return hash;
}
Shift translated_bitmap(uint8_t* rdram,const Bitmap& a,const Bitmap& b) {
    if(a.width!=b.width||a.height!=b.height||a.format!=b.format||a.palette_bytes!=b.palette_bytes)return {};
    if((word(rdram,a.header+4)&0xffff0000u)!=(word(rdram,b.header+4)&0xffff0000u))return {};
    // The low halfword is the palette count, not a material flag. ROM water
    // 0207/0209 declares14 entries between16-entry frames (same32-byte TLUT).
    // This recognizes an authored toroidal scroll, never blends bitmap images.
    // ROM water113..122 is precisely the same32x32 CI4 bitmap shifted by2px.
    // Palette storage/order may differ, but every displayed texel must match.
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
struct Binding {
    uint32_t base=0,palette=0;
    unsigned width=0,height=0,pixel_bytes=0;
    double x=0,y=0;
    bool scroll=false;
    uint32_t sampled_pixels=0,sampled_palette=0;
};
struct SampledBitmap {uint32_t pixels=0,palette=0;};
struct ScrollCycle {double x=0,y=0;unsigned ticks=0;std::vector<Shift> offsets;std::vector<unsigned> starts;};
struct ChannelClock {uint32_t sequence=0;uint64_t age=0,updated_epoch=0;};
struct ChannelUpdate {uint32_t entry=0,sequence=0;unsigned index=0,remaining=0;bool active=false;};
struct Parser {std::optional<Binding> image;std::array<uint32_t,8> tiles{};};
struct Copy {uint32_t address;Parser exit;std::array<Binding,16> segments;};
struct FramePatch {uint32_t address,first,second,published_first,published_second;};
struct State {
    uint8_t* source_ram=nullptr,*frame_ram=nullptr;
    Nanoseconds source_time=0;
    uint64_t source_epoch=0;
    std::array<ChannelClock,300> channels{};
    std::vector<ChannelUpdate> updates;
    double h=0;
    uint32_t bank=0,arena=0;
    bool active=false;
    std::array<Binding,16> segments{};
    std::map<std::pair<uint32_t,uint32_t>,CachedShift> shifts;
    std::map<uint32_t,std::optional<Bitmap>> bitmaps;
    std::map<uint32_t,uint64_t> signatures;
    std::map<std::tuple<uint64_t,uint64_t,unsigned>,SampledBitmap> samples;
    std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::optional<ScrollCycle>> cycles;
    std::vector<uint32_t> builders;
    std::vector<FramePatch> patches;
    std::map<std::pair<uint32_t,uint64_t>,Copy> copies;
} state;
// Assets are immutable while this ticket's phase1 draw borrows them. Cache
// validation/hashes once per frame; clear at each Source/ticket boundary so
// reused resource slots are still content-qualified on the following frame.
std::optional<Bitmap> bitmap(uint8_t* rdram,uint32_t p) {
    if(!state.active||state.frame_ram!=rdram)return bitmap_raw(rdram,p);
    auto at=state.bitmaps.find(p);
    if(at==state.bitmaps.end())at=state.bitmaps.emplace(p,bitmap_raw(rdram,p)).first;
    return at->second;
}
uint64_t bitmap_signature(uint8_t* rdram,const Bitmap& b) {
    if(!state.active||state.frame_ram!=rdram)return bitmap_signature_raw(rdram,b);
    auto at=state.signatures.find(b.header);
    if(at==state.signatures.end())at=state.signatures.emplace(b.header,bitmap_signature_raw(rdram,b)).first;
    return at->second;
}
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
Shift cached_translation(uint8_t* rdram,const Bitmap& a,const Bitmap& b) {
    const auto key=std::pair{a.header,b.header};
    const auto first=bitmap_signature(rdram,a),second=bitmap_signature(rdram,b);
    auto at=state.shifts.find(key);
    if(at==state.shifts.end()||at->second.first!=first||at->second.second!=second) {
        if(state.shifts.size()>=512)state.shifts.clear();
        at=state.shifts.insert_or_assign(key,CachedShift{first,second,translated_bitmap(rdram,a,b)}).first;
    }
    return at->second.value;
}
// Stage material frames include deformation, not just translations: sand
// 0239..247, pool025D..260, vortex0364..373 and waves043F..446. Sample
// their displayed colors in the free ticket bank. Source indices, palettes,
// timers and asset bytes remain intact. CI4 stays CI4, including its TMEM size.
SampledBitmap sample_stage_bitmap(uint8_t* rdram,const Bitmap& a,const Bitmap& b,double phase) {
    if(a.format!=b.format||a.width!=b.width||a.height!=b.height||a.palette_bytes!=b.palette_bytes||
       (word(rdram,a.header+4)&0xffff0000u)!=(word(rdram,b.header+4)&0xffff0000u)||phase<=0)return {};
    if(phase>=1)return {b.pixels,b.palette};
    const auto key=std::tuple{bitmap_signature(rdram,a),bitmap_signature(rdram,b),unsigned(std::llround(phase*65535))};
    if(auto i=state.samples.find(key);i!=state.samples.end())return i->second;
    if(a.format==2) {
        // ROM stage IA8 materials use FD700000 (load as16-bit) and render
        // F568/F560 IA8 tiles. I/A are separate nibbles, not one byte scalar.
        std::vector<uint32_t> storage((a.width*a.height+7)/4,0);
        const auto pixels=allocate(rdram,storage);if(!pixels)return {};
        for(unsigned i=0;i<a.width*a.height;++i) {
            const unsigned first=MEM_BU(int32_t(i),int32_t(a.pixels)),last=MEM_BU(int32_t(i),int32_t(b.pixels));
            const unsigned intensity=unsigned(std::llround((first>>4)*(1-phase)+(last>>4)*phase));
            const unsigned alpha=unsigned(std::llround((first&15)*(1-phase)+(last&15)*phase));
            MEM_B(int32_t(i),int32_t(pixels))=int8_t((intensity<<4)|alpha);
        }
        return state.samples.emplace(key,SampledBitmap{pixels,0}).first->second;
    }
    using Color=std::array<unsigned,3>;
    std::vector<Color> colors;colors.reserve(a.width*a.height);
    std::map<Color,unsigned> counts;
    for(unsigned y=0;y<a.height;++y)for(unsigned x=0;x<a.width;++x) {
        const auto first=pixel(rdram,a,x,y),last=pixel(rdram,b,x,y);
        // An authored visibility/cutout change is a discrete sprite event.
        // This material sampler only continues opaque fluid/sand/color motion.
        if(!(first&last&1u))return {};
        Color color{};
        for(unsigned c=0;c<3;++c) {
            const auto shift=11-5*c;
            color[c]=unsigned(std::llround(double((first>>shift)&31)*(1-phase)+double((last>>shift)&31)*phase));
        }
        colors.push_back(color);++counts[color];
    }
    struct WeightedColor {Color value;unsigned count;};
    std::vector<std::vector<WeightedColor>> boxes(1);
    for(const auto& [color,count]:counts)boxes[0].push_back({color,count});
    // A dynamic CI4 palette avoids converting64x64 to an8KiB RGBA16 image,
    // which would exceed the original4KiB TMEM and corrupt adjacent materials.
    while(boxes.size()<a.colors) {
        size_t chosen=boxes.size();unsigned chosen_axis=0;uint64_t best=0;
        for(size_t i=0;i<boxes.size();++i) {
            if(boxes[i].size()<2)continue;
            Color lo{31,31,31},hi{};uint64_t weight=0;
            for(const auto& c:boxes[i]){weight+=c.count;for(unsigned axis=0;axis<3;++axis){lo[axis]=std::min(lo[axis],c.value[axis]);hi[axis]=std::max(hi[axis],c.value[axis]);}}
            for(unsigned axis=0;axis<3;++axis) {
                const uint64_t score=uint64_t(hi[axis]-lo[axis])*(hi[axis]-lo[axis])*weight;
                if(score>best){best=score;chosen=i;chosen_axis=axis;}
            }
        }
        if(chosen==boxes.size())break;
        auto& box=boxes[chosen];
        std::sort(box.begin(),box.end(),[&](const auto& left,const auto& right){return std::pair{left.value[chosen_axis],left.value}<std::pair{right.value[chosen_axis],right.value};});
        uint64_t total=0,sum=0;for(const auto& c:box)total+=c.count;
        size_t split=0;while(split+1<box.size()&&sum<total/2){sum+=box[split].count;++split;}
        split=std::clamp(split,size_t(1),box.size()-1);
        std::vector<WeightedColor> right(box.begin()+split,box.end());box.resize(split);boxes.push_back(std::move(right));
    }
    std::vector<Color> palette;
    for(const auto& box:boxes) {
        uint64_t weight=0;std::array<uint64_t,3> sums{};
        for(const auto& c:box){weight+=c.count;for(unsigned axis=0;axis<3;++axis)sums[axis]+=uint64_t(c.value[axis])*c.count;}
        Color value{};for(unsigned axis=0;axis<3;++axis)value[axis]=unsigned((sums[axis]+weight/2)/weight);palette.push_back(value);
    }
    std::map<Color,unsigned> encoded;
    for(const auto& [color,count]:counts) {
        (void)count;unsigned best_index=0,distance=~0u;
        for(unsigned i=0;i<palette.size();++i) {
            unsigned candidate=0;for(unsigned axis=0;axis<3;++axis){const int delta=int(color[axis])-int(palette[i][axis]);candidate+=unsigned(delta*delta);}
            if(candidate<distance){distance=candidate;best_index=i;}
        }
        encoded.emplace(color,best_index);
    }
    std::vector<uint32_t> bytes((a.palette_bytes+a.width*a.height/2+7)/4,0);
    const auto address=allocate(rdram,bytes);if(!address)return {};
    for(unsigned i=0;i<a.colors;++i) {
        const auto c=palette[std::min(i,unsigned(palette.size()-1))];
        MEM_H(int32_t(2*i),int32_t(address))=int16_t((c[0]<<11)|(c[1]<<6)|(c[2]<<1)|1);
    }
    const auto pixels=address+a.palette_bytes;
    for(unsigned i=0;i<colors.size();i+=2)MEM_B(int32_t(i/2),int32_t(pixels))=int8_t((encoded.at(colors[i])<<4)|encoded.at(colors[i+1]));
    return state.samples.emplace(key,SampledBitmap{pixels,address}).first->second;
}
// Read the entire authored cycle, not only current->next. Four water tables
// omit frame2: local pair interpolation makes that one interval twice as fast.
// A unidirectional, equal-duration toroidal cycle has one continuous velocity;
// compensate the current Source image by its offset within the whole period.
std::optional<ScrollCycle> scroll_cycle(uint8_t* rdram,uint32_t indices,uint32_t durations,uint32_t table) {
    ScrollCycle cycle;std::vector<Bitmap> frames;
    for(unsigned i=0;i<255;++i) {
        if(!pointer(indices+i,1)||!pointer(durations+i,1))return {};
        const auto index=MEM_BU(int32_t(i),int32_t(indices));if(index==255)break;
        if(!pointer(table+2*index,2))return {};
        const int handle=MEM_H(int32_t(2*index),int32_t(table));if(handle<0||handle>=512)return {};
        const auto frame=bitmap(rdram,word(rdram,0x801AEC00+20*unsigned(handle)));if(!frame)return {};
        const auto duration=MEM_BU(int32_t(i),int32_t(durations));
        if(!duration||duration!=MEM_BU(0,int32_t(durations)))return {};
        cycle.starts.push_back(cycle.ticks);cycle.ticks+=duration;frames.push_back(*frame);
    }
    if(frames.size()<3)return {};
    int direction_x=0,direction_y=0;Shift sum{0,0,true};std::vector<Shift> steps;
    bool unidirectional=true;
    for(unsigned i=0;i<frames.size();++i) {
        cycle.offsets.push_back(sum);
        const auto shift=cached_translation(rdram,frames[i],frames[(i+1)%frames.size()]);
        if(!shift.translated||(!shift.x&&!shift.y))return {};
        const auto agrees=[](int delta,int& direction){if(!delta)return true;const int next=delta>0?1:-1;if(direction&&direction!=next)return false;direction=next;return true;};
        const bool x=agrees(shift.x,direction_x),y=agrees(shift.y,direction_y);
        unidirectional&=x&&y;steps.push_back(shift);sum.x+=shift.x;sum.y+=shift.y;
    }
    if(unidirectional&&sum.x%int(frames[0].width)==0&&sum.y%int(frames[0].height)==0) {
        cycle.x=sum.x;cycle.y=sum.y;return cycle;
    }
    // ROM waterfalls0877..87E and0462..469 only store HALF a32px tile.
    // The seven regular -2 steps are followed by a +14 numeric reset. Use
    // Source's actual update age to continue -2 through the next image cycle;
    // no backwards sweep and no double-stepping of the Source counter.
    const auto regular=steps.front();
    if(steps.size()<3||!std::all_of(steps.begin(),steps.end()-1,[&](const auto& value){return value.x==regular.x&&value.y==regular.y;}))return {};
    const auto reset=steps.back();
    if(reset.x!=-regular.x*int(steps.size()-1)||reset.y!=-regular.y*int(steps.size()-1))return {};
    cycle.x=regular.x*int(steps.size());cycle.y=regular.y*int(steps.size());return cycle;
}
uint64_t binding_signature() {
    uint64_t result=1469598103934665603ull;
    for(const auto& b:state.segments)for(const auto v:{uint64_t(b.base),uint64_t(std::llround(b.x*4)),uint64_t(std::llround(b.y*4)),uint64_t(b.sampled_pixels),uint64_t(b.sampled_palette)})
        result=(result^v)*1099511628211ull;
    return result;
}
uint32_t clone_list(uint8_t*,uint32_t,unsigned,std::set<uint32_t>&,Parser&);
bool rewrite_command(uint8_t* rdram,uint32_t& first,uint32_t& second,unsigned depth,std::set<uint32_t>& path,Parser& parser) {
    auto& image=parser.image;bool changed=false;const auto op=first>>24;
        if(op==0xFD) {
            image.reset();const auto actual=resolve(second);const auto segment=second>>24;
            const auto apply=[&](const Binding& b) {
                if(b.sampled_palette&&actual==b.palette){second=b.sampled_palette&0x1fffffffu;changed=true;return true;}
                if(b.sampled_pixels&&actual>=b.base&&uint64_t(actual)<uint64_t(b.base)+b.pixel_bytes) {
                    second=(b.sampled_pixels+actual-b.base)&0x1fffffffu;changed=true;return true;
                }
                if(b.scroll&&b.base==actual){image=b;return true;}
                return false;
            };
            // Identical Source bitmaps can have different channel phases.
            // A segmented image identifies its channel; don't pick the first
            // matching physical pointer belonging to another component.
            if(segment<16&&segment>0&&state.segments[segment].base)apply(state.segments[segment]);
            else for(const auto& b:state.segments)if(apply(b))break;
        } else if(op==0xDB&&(first&0xffff0000u)==0xDB060000u) {
            const auto offset=first&0xffffu;
            if(!(offset&3)&&offset/4<16) {
                const auto base=second<0x400000u?second|0x80000000u:second;
                auto& segment=state.segments[offset/4];if(segment.base!=base)segment=Binding{base};
            }
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
    if(independent)if(auto at=state.copies.find(key);at!=state.copies.end()){parser=at->second.exit;state.segments=at->second.segments;path.erase(address);return at->second.address;}
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
    if(independent)state.copies.emplace(key,Copy{copied,parser,state.segments});
    return copied;
}
}
namespace dora3::native_time {
void native_texture_source_completed(uint8_t* ram,Nanoseconds now){state.source_ram=ram;state.source_time=now;++state.source_epoch;}
void native_texture_reset(){if(state.active)throw std::logic_error("Texture reset inside frame");state=State{};}
TextureFrameScope::TextureFrameScope(uint8_t* ram,Nanoseconds now,unsigned bank):ram_(ram) {
    const auto* binding=active_frame_binding();if(!binding||binding->ticket.bank!=bank||state.active)throw std::logic_error("Invalid texture frame ownership");
    state.active=true;state.frame_ram=ram;state.bank=dora3::frame::bank0+bank*dora3::frame::bank_stride;
    state.arena=state.bank+dora3::frame::bank_stride-0x100;
    state.h=state.source_ram==ram?std::clamp(double(now-state.source_time)*60./(word(ram,0x801A39DC)*1e9),0.,1.):0.;
    state.segments={};state.copies.clear();state.samples.clear();state.cycles.clear();state.bitmaps.clear();state.signatures.clear();state.builders.clear();state.patches.clear();
}
TextureFrameScope::~TextureFrameScope(){if(ram_){state.active=false;state.frame_ram=nullptr;state.segments={};state.copies.clear();state.samples.clear();state.cycles.clear();state.bitmaps.clear();state.signatures.clear();state.builders.clear();state.patches.clear();}}
}
extern "C" void dora3_native_texture_channel_clock(uint8_t* rdram,recomp_context* c,unsigned boundary) {
    if(state.active||dora3::native_time::active_simulation_slice())return;
    if(boundary==2) {
        // Constructor28BC8 retains actor/slot in s2/s1 through28C58.
        const auto actor=uint32_t(c->r18),slot=unsigned(c->r17);
        if(slot>=8||!pointer(actor,0xcc)||MEM_BU(int32_t(0x6b+slot),int32_t(actor))!=5)return;
        const auto handle=word(rdram,actor+0x74+4*slot);if(handle>=300)return;
        state.channels[handle]={word(rdram,0x801A9CB0+8*handle),0,0};return;
    }
    if(!boundary) {
        const auto actor=uint32_t(c->r4),slot=unsigned(c->r5);ChannelUpdate update;
        if(slot<8&&pointer(actor,0xcc)&&MEM_BU(int32_t(0x6b+slot),int32_t(actor))==5) {
            const auto handle=word(rdram,actor+0x74+4*slot);
            if(handle<300) {
                update.entry=0x801A9CB0+8*handle;update.sequence=word(rdram,update.entry);
                update.index=MEM_BU(5,int32_t(update.entry));update.remaining=MEM_BU(4,int32_t(update.entry));
                update.active=MEM_BU(6,int32_t(update.entry))&1;
            }
        }
        state.updates.push_back(update);return;
    }
    if(state.updates.empty())throw std::logic_error("Unmatched bitmap Source update boundary");
    const auto update=state.updates.back();state.updates.pop_back();
    if(!update.entry||!update.active||!pointer(update.sequence,12)||word(rdram,update.entry)!=update.sequence)return;
    auto& clock=state.channels[(update.entry-0x801A9CB0)/8];
    if(clock.sequence!=update.sequence||!clock.age) {
        const auto durations=word(rdram,update.sequence+4);
        if(!pointer(durations,uint64_t(update.index)+1))return;
        uint64_t age=0;for(unsigned i=0;i<update.index;++i)age+=MEM_BU(int32_t(i),int32_t(durations));
        const auto duration=MEM_BU(int32_t(update.index),int32_t(durations));
        if(!duration||update.remaining>duration)return;
        clock={update.sequence,age+duration-update.remaining,0};
    }
    ++clock.age;clock.updated_epoch=state.source_epoch+1;
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
    state.segments[segment]={a->pixels,a->palette,a->width,a->height,a->width*a->height/(a->format==5?2:1),0,0,false};
    state.segments[palette].base=a->palette;state.segments[palette].palette=a->palette;
    if(!(flags&1))return; // Source controls active/loop/clamp; last clamped frame holds
    // Content-qualified pairs survive Source resource reuse; whole-cycle and
    // sampled-image caches last only this immutable native ticket's draw.
    const auto shift=cached_translation(rdram,*a,*b);
    const unsigned duration=MEM_BU(int32_t(index),int32_t(durations)),remaining=MEM_BU(4,int32_t(entry));
    if(!duration||remaining>=duration)return;
    const auto& clock=state.channels[handle];
    const bool observed=clock.sequence==sequence&&clock.age;
    const double source_phase=observed&&clock.updated_epoch!=state.source_epoch?0.:state.h;
    const double elapsed=double(duration-1-remaining)+source_phase,h=elapsed/duration;
    auto& binding=state.segments[segment];
    if(shift.translated) {
        binding.x=shift.x*h;binding.y=shift.y*h;binding.scroll=true;
        if(!(flags&2)) {
            const auto key=std::tuple{indices,durations,table};auto cycle=state.cycles.find(key);
            if(cycle==state.cycles.end())cycle=state.cycles.emplace(key,scroll_cycle(rdram,indices,durations,table)).first;
            if(cycle->second&&index<cycle->second->offsets.size()) {
                const auto& value=*cycle->second;
                const double age=observed?double(clock.age-1)+source_phase:value.starts[index]+elapsed;
                const double phase=age/value.ticks;
                binding.x=std::fmod(value.x*phase,double(a->width))-value.offsets[index].x;
                binding.y=std::fmod(value.y*phase,double(a->height))-value.offsets[index].y;
            }
        }
    } else if(world) {
        const auto sample=sample_stage_bitmap(rdram,*a,*b,h);
        binding.sampled_pixels=sample.pixels;binding.sampled_palette=sample.palette;state.segments[palette].sampled_palette=sample.palette;
    }
}
extern "C" void dora3_native_texture_builder(uint8_t* rdram,recomp_context* c,unsigned end) {
    if(!state.active||state.frame_ram!=rdram)return;
    if(!end) {
        const auto cursor_pointer=uint32_t(c->r4),resource=uint32_t(c->r6);
        // SDK2A684 installs the GIMO/resource and vertex segments immediately
        // BEFORE this builder. Seed their exact header bindings, so nested
        // segmented DE lists are reachable too, then honor in-list DB06 writes.
        if(pointer(resource,0x50)&&word(rdram,resource)==0x47494d4fu) {
            const auto object_segment=word(rdram,resource+0x44),vertex_segment=word(rdram,resource+0x4c);
            const uint64_t vertices=uint64_t(resource)+word(rdram,resource+0x2c);
            if(object_segment<16&&state.segments[object_segment].base!=resource)state.segments[object_segment]=Binding{resource};
            if(vertex_segment<16&&vertices<0x80400000ull&&state.segments[vertex_segment].base!=vertices)state.segments[vertex_segment]=Binding{uint32_t(vertices)};
        }
        state.builders.push_back(pointer(cursor_pointer,4)?word(rdram,cursor_pointer):0);return;
    }
    if(state.builders.empty())throw std::logic_error("Unmatched material builder boundary");
    const auto begin=state.builders.back(),last=uint32_t(c->r2);state.builders.pop_back();
    if(begin<state.bank+dora3::frame::dl_offset||last<begin||last>state.arena||((begin|last)&7))
        return; // Other Source-owned builders are outside this ticket's lease.
    if(std::none_of(state.segments.begin(),state.segments.end(),[](const auto& b){return b.scroll||b.sampled_pixels;}))return;
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
