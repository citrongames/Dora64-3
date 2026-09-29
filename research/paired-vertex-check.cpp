#include "recomp.h"
#include "retirement-functions.h"
#include <cassert>
#include <vector>
#include <set>
#include <cstring>
#include <iostream>
static uint32_t next_address=0x80200000;
static std::set<uint32_t> allocated,freed;
extern "C" void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
extern "C" void dora3_game_heap_alloc(uint8_t*,recomp_context* c){
 assert(uint32_t(c->r4)==32);c->r2=int32_t(next_address);allocated.insert(next_address);next_address+=32;
}
extern "C" void dora3_s0_80033C50(uint8_t*,recomp_context* c){
 assert(allocated.contains(uint32_t(c->r4)));assert(freed.insert(uint32_t(c->r4)).second);c->r2=0;
}
int main(){
 std::vector<uint32_t> memory(0x400000/4);auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
 recomp_context c{};c.r29=int32_t(0x803F0000);
 const int32_t object=int32_t(0x80100000),source=int32_t(0x80101000),target=int32_t(0x80101100);
 MEM_W(0x20,object)=source;MEM_W(0x30,object)=2;
 for(int i=0;i<32;++i)MEM_B(i,source)=i;
 MEM_H(0,source)=10;MEM_H(2,source)=20;MEM_H(4,source)=30;
 c.r4=object;dora3_s0_core_80038BC8(rdram,&c);
 int32_t bank0=MEM_W(0x6c,object),bank1=MEM_W(0x70,object);
 assert(bank0!=bank1 && allocated.size()==2);
 assert(std::memcmp(rdram+(uint32_t(bank0)&0x3fffff),rdram+(uint32_t(source)&0x3fffff),32)==0);
 assert(std::memcmp(rdram+(uint32_t(bank1)&0x3fffff),rdram+(uint32_t(source)&0x3fffff),32)==0);
 MEM_W(0,int32_t(0x80082724))=object;
 MEM_H(0,target)=0;MEM_H(2,target)=30;MEM_H(4,target)=40;MEM_H(6,target)=50;
 MEM_B(8,target)=20;MEM_B(9,target)=22;MEM_B(10,target)=24;MEM_H(12,target)=-1;
 auto unchanged=[&](int32_t bank){return std::vector<uint8_t>(rdram+(uint32_t(bank)&0x3fffff),rdram+(uint32_t(bank)&0x3fffff)+32);};
 for(int index=0;index<2;++index){
  int32_t selected=index?bank1:bank0,other=index?bank0:bank1;auto saved=unchanged(other);
  MEM_W(0,int32_t(0x801A9544))=index;c.r5=target;c.f12.fl=0.5f;
  dora3_s0_80038D58(rdram,&c);
  assert(MEM_H(0,selected)==20 && MEM_H(2,selected)==30 && MEM_H(4,selected)==40);
  assert(unchanged(other)==saved);
  // The second vertex and texture coordinates/alpha are not overwritten by this morph.
  for(int i=16;i<32;++i)assert(MEM_BU(i,selected)==MEM_BU(i,source));
  assert(MEM_H(8,selected)==MEM_H(8,source) && MEM_H(10,selected)==MEM_H(10,source));
  assert(MEM_BU(15,selected)==MEM_BU(15,source));
 }
 c.r4=object;dora3_s0_80038C58(rdram,&c);
 assert(MEM_W(0x6c,object)==0 && MEM_W(0x70,object)==0);
 MEM_W(0,int32_t(0x801AA618))=2;
 for(int i=0;i<15;++i)dora3_deferred_free_tick(rdram,&c);
 assert(freed.empty());dora3_deferred_free_tick(rdram,&c);assert(freed==allocated);
 // 51 successive pair destructions before any ticks exceed the original 100 entries.
 allocated.clear();freed.clear();
 for(int i=0;i<51;++i){
  c.r4=object;dora3_s0_core_80038BC8(rdram,&c);
  c.r4=object;dora3_s0_80038C58(rdram,&c);
  assert(MEM_W(0x6c,object)==0 && MEM_W(0x70,object)==0);
 }
 for(int i=0;i<16;++i)dora3_deferred_free_tick(rdram,&c);
 assert(allocated.size()==102 && freed.size()==100);
 std::cout<<"PASS: original paired vertex allocation/copy, bank-selected morph isolation and retirement. OBSERVED: 51 pair releases before ticks lose 2 registrations while clearing owner pointers. Synthetic schedule; gameplay frequency unknown.\n";
}

extern "C" int dora3_native_model_morph_allowed(uint8_t*,recomp_context*) {return 1;}
