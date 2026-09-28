#include "n64_hardware.hpp"
#include "recomp.h"
#include "ultramodern/ultramodern.hpp"
#include "librecomp/game.hpp"

void save_write_ptr(const void*, uint32_t, uint32_t);
void save_read(uint8_t*, int32_t, uint32_t, uint32_t);
void convert_to_n64_range(float, float, int8_t&, int8_t&);
void dequeue_external_messages(uint8_t*);
void dora3_enqueue_raw_rdp(uint32_t, uint32_t);
namespace {
ultramodern::input::callbacks_t input{};
dora3::hw::Controller read_controller(unsigned port) {
    dora3::hw::Controller result{};
    if (!input.get_connected_device_info ||
        input.get_connected_device_info(int(port)).connected_device != ultramodern::input::Device::Controller) return result;
    if (input.poll_input) input.poll_input();
    float x=0, y=0;
    result.connected = input.get_input && input.get_input(int(port),&result.buttons,&x,&y);
    if (result.connected) convert_to_n64_range(x,y,result.x,result.y);
    return result;
}
bool read_eeprom(uint8_t block, std::span<uint8_t,8> out) {
    if (recomp::get_save_type() != recomp::SaveType::Eep16k) return false;
    alignas(8) uint8_t buffer[8]{};
    save_read(buffer,int32_t(0x80000000),uint32_t(block)*8,8);
    for (unsigned i=0; i<8; ++i) out[i]=buffer[i^3];
    return true;
}
bool write_eeprom(uint8_t block, std::span<const uint8_t,8> in) {
    if (recomp::get_save_type() != recomp::SaveType::Eep16k) return false;
    save_write_ptr(in.data(),uint32_t(block)*8,8);
    return true;
}
void deliver_interrupts(uint8_t* ram) {
    if (ultramodern::is_game_thread()) {
        dequeue_external_messages(ram);
        ultramodern::check_running_queue(ram);
    }
}
}
void dora3_configure_hardware(const recomp::Configuration& cfg) {
    input=cfg.input_callbacks;
    dora3::hw::configure({recomp::get_rom,read_controller,read_eeprom,write_eeprom,
        ultramodern::send_si_message,dora3_enqueue_raw_rdp,deliver_interrupts});
}
extern "C" void dora3_si_raw_dma(uint8_t* ram, recomp_context* ctx) {
    ctx->r2 = dora3::hw::si_dma(ram,uint32_t(ctx->r4),uint32_t(ctx->r5));
}
extern "C" void dora3_dp_set_buffer(uint8_t*, recomp_context* ctx) {
    // MIPS o32 aligns the u64 second parameter to a2:a3, skipping a1.
    ctx->r2 = dora3::hw::dp_buffer(uint32_t(ctx->r4),(uint64_t(uint32_t(ctx->r6))<<32)|uint32_t(ctx->r7));
}
extern "C" void dora3_disable_int(uint8_t*, recomp_context* ctx) { ctx->r2=dora3::hw::disable_interrupts(); }
extern "C" void dora3_restore_int(uint8_t* ram, recomp_context* ctx) { dora3::hw::restore_interrupts(ram,uint32_t(ctx->r4)); }
extern "C" void dora3_sp_set_pc(uint8_t*, recomp_context* ctx) { ctx->r2=dora3::hw::sp_set_pc(uint32_t(ctx->r4)); }
extern "C" uint32_t dora3_bus_read32(uint8_t* ram, uint32_t address) { return dora3::hw::bus_read32(ram,address); }
