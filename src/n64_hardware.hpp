#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <span>

namespace dora3::hw {
constexpr uint32_t ram_size = 0x400000;
constexpr uint32_t raw_rdp_task = 0x44335244;
struct Controller { bool connected; uint16_t buttons; int8_t x, y; };
// Frontend callbacks are installed before guest/worker threads start.
struct Backend {
    std::span<const uint8_t> (*rom)();
    Controller (*controller)(unsigned);
    bool (*eeprom_read)(uint8_t, std::span<uint8_t, 8>);
    bool (*eeprom_write)(uint8_t, std::span<const uint8_t, 8>);
    void (*si_event)();
    void (*rdp_submit)(uint32_t, uint32_t);
    void (*deliver_interrupts)(uint8_t*);
};
void configure(Backend backend);
// Initial boot or quiescent runtime reset; preserves installed backend/save callbacks.
void reset_session();
bool interrupts_enabled();
uint32_t disable_interrupts();
void restore_interrupts(uint8_t* ram, uint32_t saved);
int si_dma(uint8_t* ram, uint32_t direction, uint32_t address);
int dp_buffer(uint32_t address, uint64_t size);
void dp_consumed();
void sp_submitted();
void sp_completed();
int sp_set_pc(uint32_t address);
uint32_t sp_pc();
uint32_t bus_read32(uint8_t* ram, uint32_t address);
}

// These narrow hooks are also used by generated, hash-pinned runtime copies.
extern "C" bool dora3_interrupts_enabled();
extern "C" void dora3_sp_submitted();
extern "C" void dora3_sp_completed();
