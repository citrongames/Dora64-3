// Component-only: synthetic RDRAM, original guest functions, no renderer/window.
#include "recomp.h"
#include "graphics_sync.hpp"
#include "blockingconcurrentqueue.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <mutex>
#include <vector>
#include "sync-functions.h"
std::mutex ram_mutex;
std::promise<void>* lock_attempt=nullptr;
namespace ultramodern {
std::mutex& get_graphics_rdram_mutex() {
    if(lock_attempt){auto* signal=lock_attempt;lock_attempt=nullptr;signal->set_value();}
    return ram_mutex;
}
}
extern "C" {
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
void dora3_scene_lists_release(uint8_t*,recomp_context*) {}
void dora3_process_slot_resume(uint8_t*,recomp_context*) {}
}
void set_word(std::vector<uint32_t>& ram,uint32_t address,uint32_t value){ram[(address&0x1FFFFFFF)/4]=value;}
int main(){
    using namespace std::chrono_literals;
    // Busy renderer: millions of periodic ticks must retain one queue entry.
    dora3::sync::PeriodicPresent gate;
    moodycamel::BlockingConcurrentQueue<unsigned> queue;
    unsigned token=0;
    for(unsigned cycle=0;cycle<2000;++cycle){
        for(unsigned tick=0;tick<1000;++tick)if(gate.acquire())assert(queue.enqueue(cycle));
        assert(queue.try_dequeue(token) && token==cycle);
        assert(!queue.try_dequeue(token));
        assert(!gate.acquire()); // Still in processing, no second queued request.
        gate.complete();
    }
    assert(gate.submissions()==2000 && gate.coalesced()==2000000);
    std::vector<uint32_t> initial(0x400000/4);
    for(unsigned i=0;i<0x25800/4;++i)initial[0x3DA800/4+i]=i^0x12345678;
    set_word(initial,0x8010E410,0x803B5000);set_word(initial,0x8013FB38,0x803DA800);
    // Compare ALL RDRAM and CPU context, including early exits and zero extent.
    for(unsigned bank=0;bank<2;++bank)for(unsigned mode=0;mode<4;++mode){
        auto reference=initial,adapted=initial;
        for(auto* memory:{&reference,&adapted}){
            set_word(*memory,0x801AA658,mode==1?0:320);
            set_word(*memory,0x801A48C4,240);
            set_word(*memory,0x801AA618,mode==2?1:0);
            set_word(*memory,0x801B1400,mode==3?2:0);
        }
        recomp_context a{},b{};a.r4=bank;a.r29=int32_t(0x803F0000);b=a;
        original_copy(reinterpret_cast<uint8_t*>(reference.data()),&a);
        dora3_scene_transition_framebuffer_copy(reinterpret_cast<uint8_t*>(adapted.data()),&b);
        assert(reference==adapted && std::memcmp(&a,&b,sizeof(a))==0);
        assert(ram_mutex.try_lock());ram_mutex.unlock();
    }
    // Model RT64 holding the shared lock while reading: original guest stores
    // must not begin until that read is complete. No scheduler sleeps needed.
    auto memory=initial;set_word(memory,0x801AA658,320);set_word(memory,0x801A48C4,240);
    std::promise<void> attempted;auto attempt=attempted.get_future();lock_attempt=&attempted;
    std::unique_lock reading(ram_mutex);
    auto writer=std::async(std::launch::async,[&]{
        recomp_context ctx{};ctx.r29=int32_t(0x803F0000);
        dora3_scene_transition_framebuffer_copy(reinterpret_cast<uint8_t*>(memory.data()),&ctx);
    });
    assert(attempt.wait_for(2s)==std::future_status::ready);
    assert(memory[0x3B5000/4]==0);
    reading.unlock();writer.get();lock_attempt=nullptr;
    assert(std::memcmp(memory.data()+0x3B5000/4,memory.data()+0x3DA800/4,0x25800)==0);
    // Original allocator: repeat allocation/free permutations, requiring the
    // full guest pool to coalesce again. This is NOT a full-game leak claim.
    auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    recomp_context heap{};const int32_t base=int32_t(0x80280000);constexpr unsigned size=0x20000;
    heap.r4=base;heap.r5=size;dora3_game_heap_region_init(rdram,&heap);assert(int32_t(heap.r2)==base);
    for(unsigned cycle=0;cycle<2000;++cycle){
        uint32_t pointers[64]{};
        for(unsigned i=0;i<64;++i){
            heap.r29=int32_t(0x803F0000);heap.r4=base;heap.r5=16+(i*37+cycle)%1000;
            dora3_heap_alloc_best_fit(rdram,&heap);pointers[i]=uint32_t(heap.r2);assert(pointers[i]!=0);
        }
        for(unsigned i=0;i<64;++i){heap.r4=base;heap.r5=int32_t(pointers[(i*17)%64]);dora3_s0_80033BA4(rdram,&heap);assert(heap.r2==0);}
        assert(MEM_W(0,base)==0 && MEM_W(4,base)==size-16 && MEM_W(8,base)==0);
    }
    std::cout<<"PASS: 2,000,000 periodic ticks bounded; framebuffer original equivalence/early exits; "
               "concurrent reader exclusion; 128,000 guest allocations freed and coalesced\n";
}
