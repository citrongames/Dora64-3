#include "manual_fps_trace.hpp"
#include "diagnostics.hpp"
#include "guest_storage.hpp"
#include "localization.hpp"
#include "widescreen.hpp"
#include "frame_lights.hpp"
#include "recomp.h"
#include "librecomp/addresses.hpp"
#include <array>
#include <atomic>
#include <algorithm>
#include <map>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <cstring>
namespace {
std::atomic<uint8_t*> live_ram{nullptr};
std::array<std::atomic<uint32_t>,2> frames{};
std::map<uint32_t,uint32_t> free_blocks,allocated;
std::vector<uint32_t> requested;
int32_t& word(uint8_t* rdram,uint32_t at){return MEM_W(0,int32_t(at));}
unsigned current_bank(uint8_t* ram) {
    const auto task=uint32_t(word(ram,0x80171CC0));
    if(task==dora3::frame::bank0)return 0;
    if(task==dora3::frame::bank0+dora3::frame::bank_stride)return 1;
    throw std::logic_error("Distance storage: unknown frame bank");
}
bool live(uint8_t* ram){return live_ram.load(std::memory_order_acquire)==ram;}
void clear_heap() {
    allocated.clear();free_blocks.clear();free_blocks.emplace(dora3::distance::spill_begin,dora3::localization::arena_begin-dora3::distance::spill_begin);
}
uint32_t spill_allocate(uint8_t* ram,uint32_t bytes) {
    if(!bytes||bytes>0x1000000u)return 0;
    const auto payload=(bytes+15u)&~15u;bytes=payload+16;
    auto chosen=free_blocks.end();
    for(auto i=free_blocks.begin();i!=free_blocks.end();++i)
        if(i->second>=bytes&&(chosen==free_blocks.end()||i->second<chosen->second))chosen=i;
    if(chosen==free_blocks.end())return 0;
    const auto at=chosen->first,length=chosen->second;free_blocks.erase(chosen);
    if(length>bytes)free_blocks.emplace(at+bytes,length-bytes);
    word(ram,at)=0;word(ram,at+4)=int32_t(payload);word(ram,at+8)=1;word(ram,at+12)=int32_t(bytes);
    allocated.emplace(at+16,bytes);return at+16;
}
bool spill_free(uint32_t pointer) {
    using namespace dora3::distance;
    if(pointer<spill_begin||pointer>=game_ram_end)return false;
    auto found=allocated.find(pointer);
    if(found==allocated.end())throw std::logic_error("Distance storage: invalid spill free");
    uint32_t begin=pointer-16,length=found->second;allocated.erase(found);
    auto next=free_blocks.lower_bound(begin);
    if(next!=free_blocks.end()&&begin+length==next->first){length+=next->second;free_blocks.erase(next);}
    next=free_blocks.lower_bound(begin);
    if(next!=free_blocks.begin()) {
        auto previous=std::prev(next);
        if(previous->first+previous->second==begin){begin=previous->first;length+=previous->second;free_blocks.erase(previous);}
    }
    free_blocks.emplace(begin,length);return true;
}
}
namespace dora3::distance {
void reset_storage() {
    // Called only after the runtime drained graphics and discarded its heap.
    live_ram.store(nullptr,std::memory_order_release);
    for(auto& frame:frames)frame.store(0,std::memory_order_release);
    requested.clear();free_blocks.clear();allocated.clear();
}
void initialize_storage(uint8_t* ram) {
    if(live(ram))return;
    std::memset(ram+0x400000,0,0x400000);
    clear_heap();requested.clear();
    dora3::localization::initialize_memory(ram);
    for(auto& frame:frames) {
        auto* memory=static_cast<uint8_t*>(recomp::alloc(ram,matrix_bytes+display_list_bytes));
        if(!memory)throw std::bad_alloc{};
        const auto address=uint64_t(memory-ram)+0x80000000ull;
        if(address+matrix_bytes+display_list_bytes>0xA0000000ull)throw std::logic_error("Distance frame arena outside kseg0");
        std::memset(memory,0,matrix_bytes+display_list_bytes);
        frame.store(uint32_t(address),std::memory_order_release);
    }
    live_ram.store(ram,std::memory_order_release);
    DORA3_LOG(stderr,"DISTANCE STORAGE common=%u matrices=%u DL=%u sprites=%u graphics=%u resources=%u spill=%X\n",
        common_capacity,matrix_capacity,display_list_bytes/8,sprite_capacity,graphics_capacity,resource_capacity,game_ram_end-spill_begin);
}
uint32_t display_list_begin(unsigned bank) {
    const auto base=bank<2?frames[bank].load(std::memory_order_acquire):0;return base?base+matrix_bytes:0;
}
uint32_t display_list_end(unsigned bank){const auto begin=display_list_begin(bank);return begin?begin+display_list_bytes:0;}
uint32_t task_for_display_list(uint32_t at) {
    at=(at&0x1fffffffu)|0x80000000u;
    for(unsigned i=0;i<2;++i)if(at==display_list_begin(i)&&frames[i].load(std::memory_order_acquire))return dora3::frame::bank0+i*dora3::frame::bank_stride;
    // Original task identity also remains valid before arena initialization.
    for(unsigned i=0;i<2;++i)if(at==dora3::frame::bank0+i*dora3::frame::bank_stride+dora3::frame::dl_offset)return dora3::frame::bank0+i*dora3::frame::bank_stride;
    return 0;
}
bool frame_pointer(uint32_t at,uint64_t bytes) {
    at=(at&0x1fffffffu)|0x80000000u;
    for(const auto& frame:frames) {
        const auto base=frame.load(std::memory_order_acquire);
        if(base&&at>=base&&uint64_t(at)+bytes<=uint64_t(base)+matrix_bytes+display_list_bytes)return true;
    }
    return false;
}
uint32_t display_address(uint32_t at){return frame_pointer(at,8)?at:at&0x1fffffffu;}
uint32_t matrix_address(uint8_t* ram,unsigned index,uint32_t original) {
    if(!live(ram))return original;
    if(index>=matrix_capacity)throw std::logic_error("Distance matrix arena exhausted");
    return frames[current_bank(ram)].load(std::memory_order_acquire)+index*64;
}
uint64_t heap_available(uint8_t* ram) {
    uint64_t free=0;for(const auto& entry:free_blocks)free=std::max(free,uint64_t(entry.second));
    auto at=uint32_t(word(ram,0x80070F40));
    for(unsigned i=0;i<32768&&at&&at!=0xffffffffu;++i) {
        if(at<0x80000000u||uint64_t(at)+16>0x80400000ull)break;
        if(word(ram,at+8)==0)free=std::max(free,uint64_t(uint32_t(word(ram,at+4))));
        const auto next=uint32_t(word(ram,at));if(next&&next<=at)break;at=next;
    }
    return free;
}
bool draw_budget(uint8_t* ram) {
    if(!live(ram))return true;
    const auto at=uint32_t(word(ram,0x801B1424)),end=display_list_end(current_bank(ram));
    return uint32_t(word(ram,0x801A48D8))<matrix_capacity-1024&&at<=end-0x40000u;
}
}
extern "C" void dora3_distance_alloc_begin(uint8_t* ram,recomp_context* ctx) {
    if(live(ram))requested.push_back(uint32_t(ctx->r4));
}
extern "C" void dora3_distance_alloc_end(uint8_t* ram,recomp_context* ctx) {
    if(!live(ram))return;
    if(requested.empty())throw std::logic_error("Distance allocator boundaries mismatched");
    const auto bytes=requested.back();requested.pop_back();
    if(ctx->r2==0&&word(ram,0x80070F40)!=-1)ctx->r2=int32_t(spill_allocate(ram,bytes));
}
extern "C" int dora3_distance_free_address(uint8_t* ram,uint32_t pointer) {
    return live(ram)&&spill_free(pointer);
}
extern "C" int dora3_distance_free(uint8_t* ram,recomp_context* ctx) {
    if(!live(ram)||!spill_free(uint32_t(ctx->r4)))return 0;
    ctx->r2=0;return 1;
}
extern "C" void dora3_distance_heap_reset(uint8_t* ram,recomp_context* ctx) {
    if(live(ram)&&uint32_t(ctx->r5)>=32)clear_heap();
}
extern "C" void dora3_distance_frame_begin(uint8_t* ram,recomp_context*) {
    if(!live(ram))return;
    word(ram,0x801B1424)=int32_t(dora3::distance::display_list_begin(current_bank(ram)));
    dora3::manual_trace::frame(ram,"before-draw");
    dora3::wide::begin(ram);
}
extern "C" void dora3_distance_frame_submit(uint8_t* ram,recomp_context* ctx) {
    if(!live(ram))return;
    const auto begin=dora3::distance::display_list_begin(current_bank(ram)),end=dora3::distance::display_list_end(current_bank(ram));
    const auto cursor=uint32_t(word(ram,0x801B1424));
    if(cursor<begin||cursor>end||(cursor&7))throw std::logic_error("Distance DL arena exhausted");
    dora3::manual_trace::frame(ram,"post-draw");
    dora3::manual_trace::display_list(ram,begin,cursor);
    // Original submit computes data_ptr=data_base+a3 and size=cursor-data_ptr.
    ctx->r7=int32_t(begin-uint32_t(ctx->r6));
}
extern "C" void dora3_distance_frame_usage(uint8_t* ram,recomp_context* ctx) {
    if(live(ram))ctx->r5=int32_t(uint32_t(ctx->r4)-dora3::distance::display_list_begin(current_bank(ram)));
}
extern "C" uint32_t dora3_distance_matrix(uint8_t* ram,uint32_t index,uint32_t original) {
    return dora3::distance::matrix_address(ram,index,original);
}
extern "C" int dora3_distance_model_draw(uint8_t* ram,recomp_context*) {
    return !dora3::distance::draw_budget(ram);
}
extern "C" int dora3_distance_draw_budget(uint8_t* ram){return dora3::distance::draw_budget(ram);}
extern "C" uint32_t dora3_distance_display_address(uint32_t at){return dora3::distance::display_address(at);}
