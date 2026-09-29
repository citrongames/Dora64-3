#include "recomp.h"
#include "retirement-functions.h"
#include "resource_retirement.hpp"
#include "frame_lights.hpp"
#include <cassert>
#include <vector>
#include <set>
#include <iostream>
#include <thread>
static std::set<uint32_t> freed;
extern "C" void dora3_trace(uint8_t* ram,recomp_context* ctx,uint32_t pc,uint32_t){dora3::retirement::trace(ram,ctx,pc);}
extern "C" void dora3_s0_80033C50(uint8_t*,recomp_context* ctx){assert(freed.insert(uint32_t(ctx->r4)).second);ctx->r2=0;}
int main(){
 std::vector<uint32_t> memory(0x400000/4);auto* ram=reinterpret_cast<uint8_t*>(memory.data());auto* rdram=ram;
 recomp_context ctx{};ctx.r29=int32_t(0x803F0000);dora3::retirement::enable();
 auto ticks=[&](int count){while(count--)dora3_deferred_free_tick(ram,&ctx);};
 auto enqueue=[&](uint32_t p){ctx.r4=int32_t(p);dora3_s0_80038B10(ram,&ctx);};
 auto begin=[&](unsigned bank){ctx.r4=int32_t(dora3::frame::bank0+bank*dora3::frame::bank_stride);dora3::retirement::trace(ram,&ctx,0x8002FF64);};
 auto publish=[&](unsigned bank){MEM_W(0,int32_t(0x80171CC0))=dora3::frame::bank0+bank*dora3::frame::bank_stride;dora3::retirement::trace(ram,&ctx,0x8002FE20);};
 auto done=[&](unsigned bank){dora3::retirement::parsed(dora3::frame::bank0+bank*dora3::frame::bank_stride+dora3::frame::dl_offset);};
 auto clean=[&]{dora3::retirement::reset();dora3_lifetime_slots_init(ram,&ctx);freed.clear();};
 enqueue(0x80200000);ticks(15);assert(freed.empty());ticks(1);assert(freed.size()==1);
 clean();begin(0);enqueue(0x80200000);ticks(20);assert(freed.empty());
 publish(0);ticks(2);assert(freed.empty());done(0);ticks(1);assert(freed.size()==1);
 clean();begin(0);publish(0);begin(1);publish(1);enqueue(0x80200000);
 done(1);ticks(20);assert(freed.empty());done(0);
 begin(0);publish(0);ticks(1);assert(freed.size()==1);done(0); // New work does not extend old retirement.
 clean();begin(0);publish(0);
 for(unsigned i=0;i<102;++i)enqueue(0x80200000+i*32);
 ticks(40);assert(freed.empty() && dora3::retirement::stats().spill_pending==2);
 done(0);ticks(1);assert(freed.size()==102 && dora3::retirement::stats().spill_pending==0);
 // Repeated overflow batches must empty, not accumulate host/guest retirement records.
 for(unsigned batch=0;batch<100;++batch){
  freed.clear();for(unsigned i=0;i<102;++i)enqueue(0x80200000+i*32);
  ticks(16);assert(freed.size()==102 && dora3::retirement::stats().spill_pending==0);
 }
 clean();enqueue(0x80200000);ticks(7);dora3_lifetime_slots_init(ram,&ctx);
 assert(dora3::retirement::stats().spill_pending==1);ticks(8);assert(freed.empty());ticks(1);assert(freed.size()==1);
 for(unsigned batch=0;batch<100;++batch){
  clean();begin(0);publish(0);enqueue(0x80200000);
  std::thread consumer([&]{done(0);});ticks(16);consumer.join();ticks(1);assert(freed.size()==1);
 }
 // Scene heap reinitialization invalidates all old requests before address reuse.
 clean();for(unsigned i=0;i<102;++i)enqueue(0x80200000+i*32);
 begin(0); // New scene may initialize while this task is still being built.
 ctx.r4=int32_t(0x80200000);ctx.r5=0x10000;dora3_game_heap_region_init(ram,&ctx);
 assert(dora3::retirement::stats().spill_pending==0);
 ticks(20);assert(freed.empty());
 enqueue(0x80200000);publish(0);done(0);ticks(16);assert(freed.size()==1);
 clean();assert(dora3::retirement::stats().held_ticks==0);
 std::cout<<"PASS: original registry timing; build/publication/parse lifetime; out-of-order completions; newer tasks do not delay old retirement; 102-entry overflow recovery; 100 overflow cycles; registry-only reset; concurrent parse completion; heap-reset address reuse. No GPU.\n";
}
