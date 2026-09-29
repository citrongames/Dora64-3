// Included after the actual pinned/adapted events.cpp. No runtime threads/GPU.
#include <cassert>
#include <vector>
#include <iostream>
int main(){
    std::vector<uint32_t> memory(0x400000/4);
    auto* rdram=reinterpret_cast<uint8_t*>(memory.data());
    *reinterpret_cast<OSViMode*>(rdram+0x1000)=dummy_mode;
    events_context.scheduler_screen_update_active=true;
    for(auto& state:events_context.vi.states){
        state.mode=&dummy_mode;state.framebuffer=int32_t(0x803B5000);state.control=dummy_mode.comRegs.ctrl;
    }
    constexpr unsigned rounds=30000;
    std::atomic<bool> go{false};
    std::thread setter([&]{while(!go.load()){};for(unsigned i=0;i<rounds;++i){
        osViSetMode(rdram,int32_t(0x80001000));
        osViSwapBuffer(rdram,int32_t(i&1?0x803B5000:0x803DA800));
        osViBlack(i&1);osViRepeatLine(i&1);osViSetSpecialFeatures(2);
    }});
    std::thread vi([&]{while(!go.load()){};for(unsigned i=0;i<rounds;++i)checked_vi_tick();});
    std::thread reader([&]{while(!go.load()){};for(unsigned i=0;i<rounds;++i){
        auto a=uint32_t(osViGetCurrentFramebuffer()),b=uint32_t(osViGetNextFramebuffer());
        assert((a==0x803B5000 || a==0x803DA800) && (b==0x803B5000 || b==0x803DA800));
    }});
    go=true;setter.join();vi.join();reader.join();
    osViSetMode(rdram,int32_t(0x80001000));osViSwapBuffer(rdram,int32_t(0x803DA800));
    osViRepeatLine(0);osViBlack(1);checked_vi_tick();
    assert(uint32_t(osViGetCurrentFramebuffer())==0x803DA800);
    assert(events_context.vi.regs.VI_H_START_REG==0);
    assert(events_context.vi.regs.VI_ORIGIN_REG==0x3DA800+dummy_mode.fldRegs[0].origin);
    osViBlack(0);checked_vi_tick();
    assert(events_context.vi.regs.VI_H_START_REG==dummy_mode.comRegs.hStart);
    assert(events_context.vi.regs.VI_WIDTH_REG==320);
    std::cout<<"PASS: real VI state/setters/getters; 3 concurrent threads x30000; swap/black/repeat/register semantics; no GPU\n";
}
