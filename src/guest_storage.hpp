#pragma once
#include <cstdint>
#include <cstddef>
#include "recomp.h"
namespace dora3::distance {
inline constexpr uint32_t actor_base=0x804B1CD0u;
inline constexpr unsigned actor_capacity=1536;
inline constexpr unsigned common_begin=30,common_end=600,common_capacity=common_end-common_begin;
inline constexpr uint32_t game_ram_end=0x80800000u,game_ram_size=0x800000;
inline constexpr uint32_t sprites=0x80409CB0u,models2=0x80414900u,parts4=0x804339F0u;
inline constexpr uint32_t graphics68=0x80446C60u,graphics48=0x80469550u,graphics0c=0x8046C6E0u;
inline constexpr uint32_t resources=0x8047EC00u,spill_begin=0x80500000u;
inline constexpr unsigned sprite_capacity=2048,graphics_capacity=1024,resource_capacity=4096;
inline constexpr unsigned matrix_capacity=65536;
inline constexpr uint32_t matrix_bytes=matrix_capacity*64,display_list_bytes=0x400000;
void reset_storage();
void initialize_storage(uint8_t* ram);
uint32_t display_list_begin(unsigned bank);
uint32_t display_list_end(unsigned bank);
uint32_t task_for_display_list(uint32_t address);
bool frame_pointer(uint32_t address,uint64_t bytes);
uint32_t matrix_address(uint8_t* ram,unsigned index,uint32_t original);
uint32_t display_address(uint32_t address);
uint64_t heap_available(uint8_t* ram);
bool draw_budget(uint8_t* ram);
}
extern "C" {
void dora3_distance_alloc_begin(uint8_t*,recomp_context*);
void dora3_distance_alloc_end(uint8_t*,recomp_context*);
int dora3_distance_free(uint8_t*,recomp_context*);
int dora3_distance_free_address(uint8_t*,uint32_t);
void dora3_distance_heap_reset(uint8_t*,recomp_context*);
void dora3_distance_frame_begin(uint8_t*,recomp_context*);
void dora3_distance_frame_submit(uint8_t*,recomp_context*);
void dora3_distance_frame_usage(uint8_t*,recomp_context*);
uint32_t dora3_distance_matrix(uint8_t*,uint32_t,uint32_t);
int dora3_distance_model_draw(uint8_t*,recomp_context*);
int dora3_distance_draw_budget(uint8_t*);
uint32_t dora3_distance_display_address(uint32_t);
}
