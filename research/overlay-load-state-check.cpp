#include "recomp.h"
#include "overlay_regions.hpp"
#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <vector>
static std::vector<uint8_t> fixture_rom;
static unsigned registered=0,unloaded=0;
namespace recomp {static std::span<const uint8_t> get_rom() {return fixture_rom;}}
static void load_overlays(uint32_t rom,int32_t ram,uint32_t size) {
    bool found=false;for(const auto& region:dora3_overlay_regions)
        if(region.rom==rom&&region.ram==uint32_t(ram)&&region.size==size)found=true;
    assert(found);++registered;
}
static void unload_overlays(int32_t ram,uint32_t size) {
    assert(uint32_t(ram)>=0x80000000&&size);++unloaded;
}
#include "runtime-fragments.inl"
static unsigned region_index(uint32_t rom) {
    for(unsigned i=0;i<std::size(dora3_overlay_regions);++i)
        if(dora3_overlay_regions[i].rom==rom)return i;
    std::abort();
}
static void begin(uint8_t* rdram,uint32_t rom,uint32_t ram,uint32_t size) {
    recomp_context c{};c.r4=int32_t(rom);c.r5=int32_t(ram);c.r6=size;
    runtime_begin_dma(rdram,&c);
    assert(pending_dma.active&&pending_dma.rom==rom&&pending_dma.ram==ram&&pending_dma.size==size);
}
static void copy(uint8_t* rdram,uint32_t rom,uint32_t ram,uint32_t size) {
    for(uint32_t i=0;i<size;++i)MEM_B(i,(gpr)(int32_t)ram)=int8_t(fixture_rom.at(rom+i));
}
static void load(uint8_t* rdram,unsigned index,bool parents=true) {
    const auto& region=dora3_overlay_regions[index];
    if(parents)for(const auto& dependency:dora3_overlay_dependencies)
        if(dependency.rom==region.rom&&!dora3_overlay_loaded(dora3_overlay_regions[dependency.required_index].rom))
            load(rdram,dependency.required_index);
    begin(rdram,region.rom,region.ram,region.size);
    assert(!dora3_overlay_loaded(region.rom));
    copy(rdram,region.rom,region.ram,region.size);dora3_verify_dma(rdram);
    assert(!pending_dma.active&&dora3_overlay_loaded(region.rom));
    runtime_overlay_gate(region.ram,region.rom);
}
int main(int argc,char** argv) {
    assert(argc==3);std::ifstream rom(argv[1],std::ios::binary);
    fixture_rom.assign(std::istreambuf_iterator<char>(rom),std::istreambuf_iterator<char>());
    assert(fixture_rom.size()>0x1D0000);
    std::vector<uint8_t> ram(0x400000);auto* rdram=ram.data();const std::string mode=argv[2];
    const auto city=region_index(0x132F80),common=region_index(0x5E400),scene=region_index(0x70A50);
    if(mode=="missing-parent-load") {load(rdram,city,false);assert(false);}
    if(mode=="before-load-call") {runtime_overlay_gate(0x8022D938,0x132F80);assert(false);}
    if(mode=="missing-parent-call") {
        load(rdram,city);overlay_active[common]=false;
        assert(!dora3_overlay_loaded(0x132F80));runtime_overlay_gate(0x8022D938,0x132F80);assert(false);
    }
    if(mode=="bad-bounds") {begin(rdram,0x132F80,0x803FFFF0,0x20);dora3_verify_dma(rdram);assert(false);}
    if(mode=="bad-bytes") {
        load(rdram,scene);const auto& region=dora3_overlay_regions[city];
        begin(rdram,region.rom,region.ram,region.size);copy(rdram,region.rom,region.ram,region.size);
        MEM_B(0,int32_t(region.ram))^=1;dora3_verify_dma(rdram);assert(false);
    }
    assert(mode=="positive");assert(!dora3_overlay_loaded(0x132F80)&&!dora3_overlay_loaded(0xFFFFFFFF));
    for(unsigned i=0;i<std::size(dora3_overlay_regions);++i) {
        unload_scene_code();load(rdram,i);
        for(unsigned j=0;j<std::size(dora3_overlay_regions);++j)
            assert(bool(dora3_overlay_loaded(dora3_overlay_regions[j].rom))==(overlay_active[j].load()&&overlay_dependencies_active(dora3_overlay_regions[j].rom)));
    }
    unload_scene_code();load(rdram,city);const auto registered_before=registered;
    // A reload invalidates the currently executable range before its DMA.
    const auto& region=dora3_overlay_regions[city];
    begin(rdram,region.rom,region.ram,region.size);assert(!dora3_overlay_loaded(0x132F80));
    copy(rdram,region.rom,region.ram,region.size);dora3_verify_dma(rdram);
    assert(dora3_overlay_loaded(0x132F80)&&registered==registered_before+1);
    // A different level overlays the exact same VRAM; city ownership must end.
    load(rdram,region_index(0x1398A0));assert(!dora3_overlay_loaded(0x132F80));
    load(rdram,city);begin(rdram,region.rom+4,region.ram+4,4);
    copy(rdram,region.rom+4,region.ram+4,4);dora3_verify_dma(rdram);
    assert(!dora3_overlay_loaded(0x132F80)); // Partial matching bytes cannot register code.
    load(rdram,city);overlay_active[common]=false;assert(!dora3_overlay_loaded(0x132F80));
    overlay_active[common]=true;assert(dora3_overlay_loaded(0x132F80));
    unload_scene_code();for(const auto& entry:dora3_overlay_regions)assert(!dora3_overlay_loaded(entry.rom));
    load(rdram,city);runtime_boot_reset();for(const auto& entry:dora3_overlay_regions)assert(!dora3_overlay_loaded(entry.rom));
    assert(registered>49&&unloaded>49);
    std::cout<<"PASS: 49 actual ROM overlay ranges, verified DMA/reload, identical VRAM replacement, partial overwrite, parent invalidation, scene unload and boot reset; ASan/UBSan/LSan; no game/GPU.\n";
}
