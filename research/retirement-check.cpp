#include "recomp.h"
#include "retirement-functions.h"
#include <cassert>
#include <vector>
#include <iostream>
unsigned frees=0;
uint32_t last=0;
extern "C" void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
extern "C" void dora3_s0_80033C50(uint8_t*,recomp_context* ctx){++frees;last=uint32_t(ctx->r4);ctx->r2=0;}
int main(){
 std::vector<uint32_t> memory(0x400000/4);auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
 recomp_context ctx{};ctx.r29=int32_t(0x803F0000);
 const int32_t table=int32_t(0x801AEC00),deferred=int32_t(0x800DD420),pending=int32_t(0x801AA618);
 MEM_W(0,pending)=2; // Deliberately keep two graphics tasks outstanding throughout.
 MEM_W(0,table)=int32_t(0x80350000);MEM_H(12,table)=1;MEM_H(14,table)=1;
 for(unsigned i=0;i<15;++i)dora3_object_lifetime_tick(rdram,&ctx);
 assert(frees==0 && MEM_BU(16,table)==1);
 dora3_object_lifetime_tick(rdram,&ctx);
 assert(frees==1 && last==0x80350000 && MEM_W(0,table)==0 && MEM_W(0,pending)==2);
 ctx.r4=int32_t(0x80350100);dora3_s0_80038B10(rdram,&ctx);
 for(unsigned i=0;i<15;++i)dora3_deferred_free_tick(rdram,&ctx);
 assert(frees==1 && MEM_BU(4,deferred)==1);
 dora3_deferred_free_tick(rdram,&ctx);
 assert(frees==2 && last==0x80350100 && MEM_W(0,deferred)==0 && MEM_W(0,pending)==2);
 // The original registry has 100 slots; record its overflow behavior explicitly.
 for(unsigned i=0;i<101;++i){ctx.r4=int32_t(0x80350000+i*32);dora3_s0_80038B10(rdram,&ctx);}
 unsigned occupied=0;for(int i=0;i<100;++i)occupied+=MEM_W(i*8,deferred)!=0;
 assert(occupied==100);
 for(unsigned i=0;i<16;++i)dora3_deferred_free_tick(rdram,&ctx);
 assert(frees==102); // 101st request was not registered or freed by the original code.
 std::cout<<"OBSERVED: both original 16-tick retire paths free with 2 outstanding tasks; "
            "original deferred registry stores 100 of 101 requests. Synthetic boundary cases, not gameplay reachability.\n";
}
