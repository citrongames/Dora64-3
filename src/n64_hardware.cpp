#include "n64_hardware.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

namespace dora3::hw {
namespace {
Backend backend{};
thread_local bool ie = true;
std::atomic_flag si_busy = ATOMIC_FLAG_INIT;
std::atomic<bool> dp_busy{false};
std::mutex sp_mutex;
unsigned sp_tasks = 0;
uint32_t pc = 0;
std::array<uint8_t,64> pif{};
std::array<int,5> channels{-1,-1,-1,-1,-1};

bool ram_range(uint32_t address, uint32_t size) {
    const uint32_t segment = address & 0xE0000000;
    const uint32_t offset = address & 0x1FFFFFFF;
    return (segment == 0 || segment == 0x80000000 || segment == 0xA0000000) &&
        offset <= ram_size && size <= ram_size-offset;
}
void parse_pif() {
    channels.fill(-1);
    unsigned port = 0, offset = 0;
    while (port < 5 && offset < 63) {
        const unsigned start = offset;
        const uint8_t tx = pif[offset++];
        if (tx == 0xFE) break;
        if (tx == 0xFF) continue;
        // No attached device has a stateful accessory; reset/skip consume a port.
        if (tx == 0 || tx == 0xFD) { ++port; continue; }
        if (offset >= 63) break;
        const unsigned rx = pif[offset++] & 63;
        const unsigned end = offset + (tx & 63) + rx;
        if ((tx & 63) == 0 || end > 63) {
            pif[start+1] = uint8_t(rx | 0x40);
            break;
        }
        channels[port++] = int(start);
        offset = end;
    }
}
void run_pif() {
    for (unsigned port = 0; port < 5; ++port) {
        if (channels[port] < 0) continue;
        const unsigned start = unsigned(channels[port]);
        if (pif[start] & 0xC0) continue;
        const unsigned tx = pif[start] & 63, rx = pif[start+1] & 63;
        if (!tx || start+2+tx+rx > 63) continue;
        const auto* request = &pif[start+2];
        auto* response = &pif[start+2+tx];
        pif[start+1] = uint8_t(rx);
        Controller controller{};
        if (port < 4 && backend.controller) controller = backend.controller(port);
        if (port < 4 && !controller.connected) { pif[start+1] |= 0x80; continue; }
        auto lengths = [&](unsigned send, unsigned receive) {
            if (tx == send && rx == receive) return true;
            pif[start+1] |= 0x40;
            return false;
        };
        switch (request[0]) {
        case 0x00: case 0xFF:
            if (lengths(1,3)) {
                // Wire order: standard controller 05 00; 16K EEPROM 00 C0.
                response[0] = port == 4 ? 0 : 5;
                response[1] = port == 4 ? 0xC0 : 0;
                response[2] = 0; // No accessory; synchronous EEPROM is ready.
            }
            break;
        case 0x01:
            if (port == 4) { pif[start+1] |= 0x80; break; }
            if (lengths(1,4)) {
                response[0] = uint8_t(controller.buttons >> 8);
                response[1] = uint8_t(controller.buttons);
                response[2] = uint8_t(controller.x);
                response[3] = uint8_t(controller.y);
            }
            break;
        case 0x04:
            if (port != 4) { pif[start+1] |= 0x80; break; }
            if (lengths(2,8) && (!backend.eeprom_read ||
                !backend.eeprom_read(request[1], std::span<uint8_t,8>(response,8)))) pif[start+1] |= 0x80;
            break;
        case 0x05:
            if (port != 4) { pif[start+1] |= 0x80; break; }
            if (lengths(10,1)) {
                if (backend.eeprom_write && backend.eeprom_write(request[1],
                    std::span<const uint8_t,8>(request+2,8))) response[0] = 0;
                else pif[start+1] |= 0x80;
            }
            break;
        default:
            // Controller Pak/rumble/RTC are not connected in this frontend.
            pif[start+1] |= 0x80;
            break;
        }
    }
}
}

void configure(Backend value) { backend = value; }
bool interrupts_enabled() { return ie; }
uint32_t disable_interrupts() { const uint32_t old = ie; ie = false; return old; }
void restore_interrupts(uint8_t* ram, uint32_t saved) {
    const bool was_enabled = ie;
    ie = ie || (saved & 1);
    if (!was_enabled && ie && backend.deliver_interrupts) backend.deliver_interrupts(ram);
}
int si_dma(uint8_t* ram, uint32_t direction, uint32_t address) {
    if (direction > 1 || (address & 7) || !ram_range(address,64)) return -1;
    if (si_busy.test_and_set()) return -1;
    const uint32_t offset = address & 0x1FFFFFFF;
    if (direction == 1) {
        for (unsigned i=0; i<64; ++i) pif[i] = ram[(offset+i)^3];
        if (pif[63] & 1) { pif[63] &= ~1; parse_pif(); }
    } else {
        // READ executes the configured Joybus channels again (libultra reuses
        // controller command buffers without a preceding WRITE every poll).
        run_pif();
        for (unsigned i=0; i<64; ++i) ram[(offset+i)^3] = pif[i];
    }
    si_busy.clear();
    if (backend.si_event) backend.si_event();
    return 0;
}
int dp_buffer(uint32_t address, uint64_t size) {
    if ((address & 7) || (size & 7) || size > ram_size || !ram_range(address,uint32_t(size))) return -1;
    bool idle = false;
    if (!dp_busy.compare_exchange_strong(idle,true)) return -1;
    if (size == 0) { dp_consumed(); return 0; }
    if (!backend.rdp_submit) { dp_consumed(); return -1; }
    backend.rdp_submit(address & 0x1FFFFFFF,uint32_t(size));
    return 0;
}
void dp_consumed() { dp_busy.store(false); }
void sp_submitted() { std::lock_guard lock(sp_mutex); ++sp_tasks; pc = 0; }
void sp_completed() {
    std::lock_guard lock(sp_mutex);
    if (sp_tasks == 0) { std::fputs("Unbalanced RSP completion\n",stderr); std::abort(); }
    --sp_tasks;
}
int sp_set_pc(uint32_t address) {
    std::lock_guard lock(sp_mutex);
    if (sp_tasks) return -1;
    pc = address & 0xFFC;
    return 0;
}
uint32_t sp_pc() { std::lock_guard lock(sp_mutex); return pc; }
uint32_t bus_read32(uint8_t* ram, uint32_t address) {
    const uint32_t physical = address & 0x1FFFFFFF;
    if (!(address & 3) && ram_range(address,4)) {
        uint32_t value;
        std::memcpy(&value,ram+physical,4);
        return value;
    }
    const auto rom = backend.rom ? backend.rom() : std::span<const uint8_t>{};
    if (!(address & 3) && physical >= 0x10000000 &&
        physical-0x10000000 <= rom.size() && rom.size()-(physical-0x10000000) >= 4) {
        const auto* p = rom.data()+(physical-0x10000000);
        return uint32_t(p[0])<<24 | uint32_t(p[1])<<16 | uint32_t(p[2])<<8 | p[3];
    }
    std::fprintf(stderr,"Unsupported N64 bus word read %08X\n",address);
    std::abort();
}
}
extern "C" bool dora3_interrupts_enabled() { return dora3::hw::interrupts_enabled(); }
extern "C" void dora3_sp_submitted() { dora3::hw::sp_submitted(); }
extern "C" void dora3_sp_completed() { dora3::hw::sp_completed(); }
