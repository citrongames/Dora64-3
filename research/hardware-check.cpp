// Component harness: real save storage, input conversion, message queue and
// scheduling sources. Platform threads/renderer are replaced by test endpoints.
#include "../build-deps/lib/N64ModernRuntime/librecomp/src/pi.cpp"
#include "../src/n64_hardware.hpp"
#include <iostream>
#include <cassert>
#include <thread>

extern "C" {
void dora3_si_raw_dma(uint8_t*,recomp_context*);
void dora3_dp_set_buffer(uint8_t*,recomp_context*);
void dora3_disable_int(uint8_t*,recomp_context*);
void dora3_restore_int(uint8_t*,recomp_context*);
void dora3_sp_set_pc(uint8_t*,recomp_context*);
void dora3_pak_font_decode(uint8_t*,recomp_context*);
void dora3_trace(uint8_t*,recomp_context*,uint32_t,uint32_t) {}
}
void dora3_configure_hardware(const recomp::Configuration&);
void dequeue_external_messages(uint8_t*);
unsigned si_events=0, submissions=0, scheduling_checks=0;
uint32_t submitted_address=0,submitted_size=0;
uint16_t buttons=0xA010;
uint8_t* test_ram=nullptr;
bool test_busy=false;
constexpr int32_t queue_address=int32_t(0x80000100);
namespace recomp {
SaveType get_save_type() { return SaveType::Eep16k; }
}
namespace ultramodern {
bool is_game_thread() { return true; }
bool thread_queue_empty(uint8_t* rdram,int32_t queue) {
    if(queue==running_queue) { ++scheduling_checks; return true; }
    return *TO_PTR(int32_t,queue)==0;
}
void thread_queue_insert(uint8_t*,int32_t,int32_t) { std::abort(); }
int32_t thread_queue_pop(uint8_t*,int32_t) { std::abort(); }
int32_t thread_queue_peek(uint8_t*,int32_t) { std::abort(); }
void resume_thread_and_wait(uint8_t*,OSThread*) { std::abort(); }
void run_next_thread_and_wait(uint8_t*) { std::abort(); }
int32_t this_thread() { std::abort(); }
void send_si_message() {
    ++si_events;
    enqueue_external_message_src(queue_address,0x5349,false,EventMessageSource::Si);
}
}
void dora3_enqueue_raw_rdp(uint32_t address,uint32_t size) {
    ++submissions; submitted_address=address; submitted_size=size;
}
void check(bool value) { if(!value) std::abort(); }

int main(int argc,char** argv) {
    assert(argc==4);
    std::ifstream input(argv[1],std::ios::binary);
    rom.assign(std::istreambuf_iterator<char>(input),{});
    assert(rom.size()==0x1000000);
    std::vector<uint8_t> memory(dora3::hw::ram_size,0xCD);
    uint8_t* rdram=memory.data(); test_ram=rdram;
    recomp::Configuration cfg{};
    cfg.input_callbacks.get_connected_device_info=[](int port) {
        return ultramodern::input::connected_device_info_t{
            port==0 ? ultramodern::input::Device::Controller : ultramodern::input::Device::None,
            ultramodern::input::Pak::None};
    };
    cfg.input_callbacks.get_input=[](int port,uint16_t* out,float* x,float* y) {
        if(test_busy) {
            test_busy=false;
            assert(dora3::hw::si_dma(test_ram,0,0x80000800)==-1);
        }
        *out=buttons; *x=1; *y=0; return port==0;
    };
    dora3_configure_hardware(cfg);
    save_context.save_buffer.resize(2048);
    for(unsigned i=0;i<2048;++i) save_context.save_buffer[i]=char((i*17)^0x5A);
    osCreateMesgQueue(rdram,queue_address,int32_t(0x80000200),128);
    recomp_context ctx{};
    dora3_disable_int(rdram,&ctx); assert(ctx.r2==1);
    dora3_disable_int(rdram,&ctx); assert(ctx.r2==0);
    ultramodern::enqueue_external_message(queue_address,1234,false,false);
    dequeue_external_messages(rdram);
    ultramodern::wait_for_external_message(rdram);
    ultramodern::wait_for_external_message_timed(rdram,1);
    ultramodern::check_running_queue(rdram);
    assert(TO_PTR(OSMesgQueue,queue_address)->validCount==0 && scheduling_checks==0);
    ctx.r4=0;dora3_restore_int(rdram,&ctx);
    assert(!dora3_interrupts_enabled());
    bool other_thread_enabled=false;
    std::thread other([&](){other_thread_enabled=dora3_interrupts_enabled();}); other.join();
    assert(other_thread_enabled);
    ctx.r4=1;dora3_restore_int(rdram,&ctx);
    assert(dora3_interrupts_enabled() && TO_PTR(OSMesgQueue,queue_address)->validCount==1 && scheduling_checks==1);
    assert(MEM_W(0,int32_t(0x80000200))==1234);

    std::array<uint8_t,64> packet{};
    auto write_packet=[&](){
        for(unsigned i=0;i<64;++i) rdram[(0x800+i)^3]=packet[i];
        ctx.r4=1;ctx.r5=int32_t(0x80000800);dora3_si_raw_dma(rdram,&ctx);assert(ctx.r2==0);
    };
    auto read_packet=[&](){
        ctx.r4=0;ctx.r5=int32_t(0x80000800);dora3_si_raw_dma(rdram,&ctx);assert(ctx.r2==0);
        for(unsigned i=0;i<64;++i) packet[i]=rdram[(0x800+i)^3];
    };
    packet[0]=1;packet[1]=4;packet[2]=1;packet[7]=0xFE;packet[63]=1;
    write_packet(); assert(si_events==1);
    test_busy=true;read_packet();assert(si_events==2 && !test_busy);
    assert(packet[1]==4 && packet[3]==0xA0 && packet[4]==0x10 && packet[5]>=81 && packet[5]<=82 && packet[6]==0);
    buttons=0x4000;read_packet();assert(packet[3]==0x40 && si_events==3);
    // Controller status, padding/skip, absent port and EEPROM identify.
    packet.fill(0);packet[0]=0xFF;packet[1]=1;packet[2]=3;packet[3]=0;
    packet[7]=1;packet[8]=3;packet[9]=0;packet[13]=0;packet[14]=0;
    packet[15]=1;packet[16]=3;packet[17]=0;packet[21]=0xFE;packet[63]=1;
    write_packet();read_packet();
    assert(packet[4]==5 && packet[5]==0 && packet[6]==0 && packet[8]==0x83);
    assert(packet[18]==0 && packet[19]==0xC0 && packet[20]==0);
    // All 256 EEPROM blocks, including final block and word-swapped DMA.
    for(unsigned block=0;block<256;++block) {
        packet.fill(0);packet[4]=2;packet[5]=8;packet[6]=4;packet[7]=uint8_t(block);packet[16]=0xFE;packet[63]=1;
        write_packet();read_packet();
        for(unsigned i=0;i<8;++i) assert(packet[8+i]==uint8_t(save_context.save_buffer[block*8+i]));
    }
    auto original=save_context.save_buffer;
    packet.fill(0);packet[4]=10;packet[5]=1;packet[6]=5;packet[7]=255;
    for(unsigned i=0;i<8;++i) packet[8+i]=uint8_t(0xD0+i);
    packet[17]=0xFE;packet[63]=1;write_packet();read_packet();assert(packet[16]==0);
    for(unsigned i=0;i<2048;++i) assert(uint8_t(save_context.save_buffer[i])==(i<2040 ? uint8_t(original[i]):uint8_t(0xD0+i-2040)));
    save_read(rdram,int32_t(0x80000A00),2040,8);
    for(unsigned i=0;i<8;++i) assert(rdram[(0xA00+i)^3]==0xD0+i);
    packet.fill(0xFF);packet[0]=63;packet[1]=63;packet[63]=1;write_packet();read_packet();assert(packet[1]==0x7F);
    const auto before=si_events;
    assert(dora3::hw::si_dma(rdram,2,0x80000800)==-1);
    assert(dora3::hw::si_dma(rdram,0,0x803FFFC8)==-1);
    assert(dora3::hw::si_dma(rdram,0,0x80000801)==-1 && si_events==before);
    // Explicit high/low size ABI, busy rejection, completion and zero length.
    ctx.r4=int32_t(0x80002000);ctx.r5=0xDEADBEEF;ctx.r6=0;ctx.r7=16;
    dora3_dp_set_buffer(rdram,&ctx);assert(ctx.r2==0 && submissions==1 && submitted_address==0x2000 && submitted_size==16);
    dora3_dp_set_buffer(rdram,&ctx);assert(int64_t(ctx.r2)==-1 && submissions==1);
    dora3::hw::dp_consumed();ctx.r6=1;dora3_dp_set_buffer(rdram,&ctx);assert(int64_t(ctx.r2)==-1);
    ctx.r6=0;ctx.r7=8;dora3_dp_set_buffer(rdram,&ctx);assert(ctx.r2==0 && submissions==2);
    dora3::hw::dp_consumed();assert(dora3::hw::dp_buffer(0x80002000,0)==0);
    assert(dora3::hw::dp_buffer(0x803FFFF8,16)==-1);
    ctx.r4=0x1237;dora3_sp_set_pc(rdram,&ctx);assert(ctx.r2==0 && dora3::hw::sp_pc()==0x234);
    dora3_sp_submitted();dora3_sp_submitted();dora3_sp_set_pc(rdram,&ctx);assert(int64_t(ctx.r2)==-1);
    dora3_sp_completed();dora3_sp_set_pc(rdram,&ctx);assert(int64_t(ctx.r2)==-1);
    dora3_sp_completed();dora3_sp_set_pc(rdram,&ctx);assert(ctx.r2==0);

    // Execute only the original font function, with its three bus-read hooks.
    for(unsigned i=0;i<0x58E80;++i) rdram[(0x25C00+i)^3]=rom[0x1000+i];
    constexpr uint32_t output=0x100000, size=4320;
    assert(dora3::hw::bus_read32(rdram,0xB0000B70)==(uint32_t(rom[0xB70])<<24|uint32_t(rom[0xB71])<<16|uint32_t(rom[0xB72])<<8|rom[0xB73]));
    ctx={};ctx.r4=int32_t(0x80000000|output);ctx.r29=int32_t(0x803FF000);
    dora3_pak_font_decode(rdram,&ctx);
    assert(ctx.r29==uint64_t(int64_t(int32_t(0x803FF000))));
    std::ifstream reference(argv[2],std::ios::binary);
    std::vector<uint8_t> expected{std::istreambuf_iterator<char>(reference),{}};
    assert(expected.size()==size);
    for(unsigned i=0;i<size;++i) assert(rdram[(output+i)^3]==expected[i]);
    for(unsigned i=0;i<16;++i) assert(rdram[(output-16+i)^3]==0xCD && rdram[(output+size+i)^3]==0xCD);
    std::ofstream report(argv[3]);
    report << "PASS: nested IE and real queued-event deferral; SI write/read events, busy/error paths, controller repeat polls; all256 EEPROM blocks and shared runtime storage; DP ABI/busy/completion; SP halt/PC; original font matches independent bitstream decoder. No game or renderer launched.\n";
    std::cout << "PASS hardware components\n";
}
