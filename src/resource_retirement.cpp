#include "resource_retirement.hpp"
#include "frame_lights.hpp"
#include "recomp.h"
#include <array>
#include <vector>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

extern "C" void dora3_s0_80033C50(uint8_t*, recomp_context*);
namespace {
struct Task { uint64_t ticket=0; bool active=false, published=false; };
struct Entry { uint32_t pointer=0; uint64_t ticket=0; };
struct Spill { Entry entry; uint8_t remaining=16; };
constexpr int32_t table=int32_t(0x800DD420);
constexpr uint32_t maximum_records=0x400000/16; // Physical RAM / allocator alignment.
std::mutex task_mutex;
std::array<Task,2> tasks{};
uint64_t sequence=0;
bool enabled=false;
// Only the scheduled guest CPU accesses retirement state. Renderer touches tasks only.
std::array<Entry,100> entries{};
std::vector<Spill> overflow;
dora3::retirement::Stats counters{};
[[noreturn]] void invalid(const char* why,uint32_t value=0){
 std::fprintf(stderr,"RESOURCE RETIRE invariant: %s value=%08X\n",why,value);std::abort();
}
unsigned index_for_bank(uint32_t bank){
 if(bank==dora3::frame::bank0)return 0;
 if(bank==dora3::frame::bank0+dora3::frame::bank_stride)return 1;
 invalid("unknown bank",bank);
}
uint64_t current_ticket(){std::scoped_lock lock(task_mutex);return sequence;}
bool ready(uint64_t ticket){
 std::scoped_lock lock(task_mutex);
 for(auto& task:tasks)if(task.active && task.ticket<=ticket)return false;
 return true;
}
void clear_entries(){entries={};std::vector<Spill>().swap(overflow);}
void spill(Entry entry,uint8_t remaining){
 if(overflow.size()>=maximum_records)invalid("more retirements than RAM allocations");
 overflow.push_back({entry,remaining});++counters.spilled;
 if((counters.spilled&(counters.spilled-1))==0)
  std::fprintf(stderr,"RESOURCE RETIRE overflow retained count=%llu pending=%zu\n",
   static_cast<unsigned long long>(counters.spilled),overflow.size());
}
void registry_reset(uint8_t* rdram){
 // A registry-only reset is not permission to reuse the heap. Keep its live
 // requests in host storage; a real heap reinitialization invalidates them earlier.
 for(int i=0;i<100;++i){
  auto pointer=uint32_t(MEM_W(i*8,table));
  if(pointer){
   if(entries[i].pointer!=pointer)invalid("untracked entry at registry reset",pointer);
   spill(entries[i],MEM_BU(i*8+4,table));
  }
 }
 entries={};
}
void begin(uint32_t bank){
 std::scoped_lock lock(task_mutex);auto& t=tasks[index_for_bank(bank)];
 if(t.active)invalid("bank reused before parsing",bank);
 if(++sequence==0)invalid("ticket overflow");
 t={sequence,true,false};
}
void publish(uint32_t bank){
 std::scoped_lock lock(task_mutex);auto& t=tasks[index_for_bank(bank)];
 if(!t.active || t.published)invalid("unmatched publication",bank);
 t.published=true;
}
void clear_for_heap(uint8_t* rdram){
 // A newly building task may initialize its scene, but no published old task
 // may still read the heap being reinitialized. The guest transition waits for DP.
 {
  std::scoped_lock lock(task_mutex);
  for(auto& t:tasks)if(t.active && t.published)invalid("heap reset before published task parsed");
 }
 clear_entries();
 for(int i=0;i<100;++i){MEM_W(i*8,table)=0;MEM_B(i*8+4,table)=0;}
}
void enqueue(uint8_t* rdram,uint32_t pointer){
 if(pointer==0)return;
 if(pointer<0x80000010 || pointer>=0x80400000 || (pointer&15))invalid("invalid retired allocation",pointer);
 Entry entry{pointer,current_ticket()};
 for(int i=0;i<100;++i)if(MEM_W(i*8,table)==0){entries[i]=entry;return;}
 // The original enqueue will return without storing; preserve this request here.
 spill(entry,16);
}
void tick(uint8_t* rdram,recomp_context* ctx){
 for(int i=0;i<100;++i){
  auto pointer=uint32_t(MEM_W(i*8,table));
  if(!pointer){entries[i]={};continue;}
  if(pointer!=entries[i].pointer)invalid("untracked guest retirement",pointer);
  if(MEM_BU(i*8+4,table)==1){
   if(!ready(entries[i].ticket)){
    // The original tick immediately decrements this byte. Hold at 1, not 0
    // (which would wrap to255), until this retirement's older tasks finish.
    MEM_B(i*8+4,table)=2;++counters.held_ticks;
   }else entries[i]={};
  }
 }
 size_t keep=0;
 for(size_t i=0;i<overflow.size();++i){
  auto item=overflow[i];
  if(item.remaining>0)--item.remaining;
  if(item.remaining==0 && ready(item.entry.ticket)){
   // Reviewed free wrapper/allocator are call-free with respect to guest SDK:
   // no yield, DMA or context switch. Preserve the trace caller's registers.
   auto saved=*ctx;ctx->r4=int32_t(item.entry.pointer);
   dora3_s0_80033C50(rdram,ctx);*ctx=saved;++counters.released;
  }else{overflow[keep++]=item;if(item.remaining==0)++counters.held_ticks;}
 }
 overflow.resize(keep);
}
}
namespace dora3::retirement {
void enable(){reset();enabled=true;}
void reset(){std::scoped_lock lock(task_mutex);tasks={};sequence=0;clear_entries();counters={};}
Stats stats(){auto result=counters;result.spill_pending=uint32_t(overflow.size());return result;}
void parsed(uint32_t dl){
 if(!enabled)return;
 const uint32_t bank=(dl&0x1fffffff)+0x80000000-dora3::frame::dl_offset;
 std::scoped_lock lock(task_mutex);auto& t=tasks[index_for_bank(bank)];
 if(!t.active || !t.published)invalid("unmatched parse completion",dl);
 t.active=false;
}
void trace(uint8_t* rdram,recomp_context* ctx,uint32_t address){
 if(!enabled)return;
 switch(address){
 case 0x8002FF64:begin(uint32_t(ctx->r4));break;
 case 0x8002FE20:publish(uint32_t(MEM_W(0,int32_t(0x80171CC0))));break;
 case 0x80038B10:enqueue(rdram,uint32_t(ctx->r4));break;
 case 0x80038B4C:tick(rdram,ctx);break;
 case 0x80038AE4:registry_reset(rdram);break;
 case 0x80033A14:if(uint32_t(ctx->r5)>=32)clear_for_heap(rdram);break;
 default:break;
 }
}
}
