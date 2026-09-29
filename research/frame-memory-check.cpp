// No game loop, window, renderer, ROM extraction, or user save access.
#include "recomp.h"
// Original-mode light harness; native scheduling has its own original-ROM check.
extern "C" void dora3_native_task_register(uint8_t*, recomp_context*) {}
extern "C" void dora3_native_task_submitted(uint8_t*, recomp_context*) {}
#include "frame_lights.hpp"
#include <cassert>
#include <cstring>
#include <future>
#include <iostream>
#include <vector>
using namespace dora3::frame;
extern "C" {
void dora3_graphics_task_submit(uint8_t*, recomp_context*);
void dora3_s0_80027B84(uint8_t*, recomp_context*);
void dora3_trace(uint8_t*, recomp_context*, uint32_t, uint32_t) {}
}
unsigned messages=0;
bool consume_in_send=false;
Lights immediate;
extern "C" void osSendMesg_recomp(uint8_t* rdram, recomp_context* ctx) {
    const int32_t bank=int32_t(ctx->r5);
    assert(uint32_t(bank)==bank0 || uint32_t(bank)==bank0+bank_stride);
    assert(ctx->r6==1);
    // The original JAL delay-slot store must still precede publication.
    assert(uint32_t(MEM_W(12,bank))==uint32_t(MEM_W(0x100,bank)));
    if(consume_in_send) assert(take(uint32_t(MEM_W(0x40,bank)),immediate));
    ++messages;ctx->r2=0;
}
int main() {
    std::vector<uint32_t> storage(0x400000/4);
    uint8_t* rdram=reinterpret_cast<uint8_t*>(storage.data());
    recomp_context ctx{};
    auto direction=[&](unsigned value) {
        ctx.r4=value;ctx.r5=value+1;ctx.r6=value+2;ctx.r7=1;
        dora3_s0_80027B84(rdram,&ctx);
    };
    auto submit=[&](int32_t bank) {
        MEM_W(0,int32_t(0x80171CC0))=bank;
        MEM_W(0,int32_t(0x801B1608))=uint32_t(bank)+0x108;
        MEM_W(0,int32_t(0x801B1424))=uint32_t(bank)+dl_offset+16;
        MEM_W(0x100,bank)=0x803B5000;
        ctx.r29=int32_t(0x803F0000);ctx.r4=0;ctx.r5=0x40;
        dora3_graphics_task_submit(rdram,&ctx);
    };
    // Populate the entire light range with a known pattern, preserving host order.
    for(unsigned i=0;i<light_size;++i) rdram[light_address+i]=uint8_t(i);
    enable();
    direction(10);
    Lights expected0;std::memcpy(expected0.bytes.data(),rdram+light_address,light_size);
    submit(bank0);
    direction(60);
    Lights expected1;std::memcpy(expected1.bytes.data(),rdram+light_address,light_size);
    submit(bank0+bank_stride);
    // Consumer starts only after the next frame changed the shared source.
    // Baseline live-RAM reads therefore produce the wrong first-frame direction.
    assert(std::memcmp(expected0.bytes.data(),rdram+light_address,light_size)!=0);
    auto consumer=std::async(std::launch::async,[&] {
        Lights first,second;
        assert(take(bank0+dl_offset,first));
        assert(take((bank0+bank_stride+dl_offset)&0x1FFFFFFF,second));
        assert(first.bytes==expected0.bytes && second.bytes==expected1.bytes);
        for(unsigned offset: {0u,8u,24u}) {
            assert(std::memcmp(first.lookup(light_address+offset),expected0.bytes.data()+offset,16)==0);
        }
        assert(first.lookup(light_address+1)==nullptr);
        assert(first.lookup(light_address+light_size)==nullptr);
        assert(!take(bank0+dl_offset,first));
        assert(!take(0x12340,first));
    });
    consumer.get();
    // Fast scheduler path: the snapshot must exist inside osSendMesg already.
    consume_in_send=true;direction(100);submit(bank0);
    assert(std::memcmp(immediate.bytes.data(),rdram+light_address,light_size)==0);
    consume_in_send=false;submit(bank0);reset();
    assert(!take(bank0+dl_offset,immediate));
    assert(messages==4);
    std::cout<<"PASS: original submit/delay slot and light writer; two queued banks; "
               "delayed consumer reproduces stale-light hazard and preserves snapshots; "
               "16-byte RT64 reads, reuse and reset\n";
}
