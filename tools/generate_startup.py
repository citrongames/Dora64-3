"""Generate reviewed startup functions with explicit fail-stop boundaries."""
import argparse, hashlib, json, struct, subprocess
from pathlib import Path
from inspect_rom import ROOT, read_rom

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path)
    p.add_argument('--output-dir',required=True,type=Path)
    a=p.parse_args();rom=read_rom(a.rom);out=a.output_dir.resolve()
    if not out.is_relative_to(ROOT/'build-local'):raise ValueError('Keep generated code under build-local')
    out.mkdir(parents=True,exist_ok=False)
    meta=json.loads((ROOT/'recomp/startup.json').read_text())
    sections=[dict(name='.startup',rom=0x1000,vram=0x80025C00,size=0x58E80,
                   compiled=meta['compiled'],indirect_targets=meta.get('indirect_targets',{}))]
    sections.extend(meta.get('overlays',[]))
    compiled={};sdk={int(k,16):v for k,v in meta['sdk'].items()};frontier=set()
    names=set()
    for section in sections:
        base,size=section['vram'],section['size']
        if (section['name'] in names or base%4 or size%4 or size<=0 or
            not 0x80000000<=base<base+size<=0x80400000 or section['rom']<0 or
            section['rom']%4 or section['rom']+size>len(rom)):
            raise ValueError('Invalid or duplicate section')
        names.add(section['name'])
        ranges=[]
        for key,value in section['compiled'].items():
            address=int(key,16);length=value[1]
            if address in compiled or address in sdk or address%4 or length<=0 or length%4 or not base<=address<address+length<=base+size:
                raise ValueError(f'Invalid/ambiguous function {key}')
            compiled[address]=value;ranges.append((address,address+length))
        ranges.sort()
        if any(a[1]>b[0] for a,b in zip(ranges,ranges[1:])):raise ValueError('Overlapping functions')
    # Mutually exclusive overlays that share VRAM require a separate namespace;
    # this bootstrap currently supports only disjoint resident/common/scene code.
    for i,left in enumerate(sections):
        for right in sections[i+1:]:
            for key in ('rom','vram'):
                if left[key]<right[key]+right['size'] and right[key]<left[key]+left['size']:
                    raise ValueError(f'Overlapping section {key} ranges; review layout before generation')
    def section_for(address):
        matches=[section for section in sections if section['vram']<=address<section['vram']+section['size']]
        if len(matches)!=1:raise ValueError(f'Unknown/ambiguous section for {address:08X}; review before generation')
        return matches[0]
    for section in sections:
        for address in section.get('indirect_targets',{}):
            target=int(address,16)
            if target not in compiled and target not in sdk:frontier.add(target)
    for address,(_,size,_) in compiled.items():
        section=section_for(address);offset=section['rom']+address-section['vram']
        for (word,) in struct.iter_unpack('>I',rom[offset:offset+size]):
            if word>>26==3:
                target=((address+4)&0xF0000000)|((word&0x3ffffff)<<2)
                if target not in compiled and target not in sdk:frontier.add(target)
    functions={addr:(v[0],v[1]) for addr,v in compiled.items()}
    functions.update({addr:(v[0],4) for addr,v in sdk.items()})
    functions.update({addr:(f'pending_{addr:08X}',4) for addr in frontier})
    grouped={section['name']:[] for section in sections}
    for addr,(name,size) in sorted(functions.items()):
        grouped[section_for(addr)['name']].append(f'{{ name = "{name}", vram = 0x{addr:08X}, size = 0x{size:X} }},')
    lines=[]
    for section in sections:
        lines.extend(['[[section]]',f'name = "{section["name"]}"',f'rom = 0x{section["rom"]:X}',
                      f'vram = 0x{section["vram"]:X}',f'size = 0x{section["size"]:X}','functions = ['])
        lines.extend(grouped[section['name']]);lines.append(']')
    (out/'startup.syms.toml').write_text('\n'.join(lines)+'\n')
    registry=['#pragma once','#include <cstdint>',
              'struct Dora3OverlayRegion { uint32_t rom, ram, size; };',
              'static constexpr Dora3OverlayRegion dora3_overlay_regions[] = {']
    for section in sections[1:]:
        registry.append(f'    {{0x{section["rom"]:X},0x{section["vram"]:X},0x{section["size"]:X}}},')
    registry.append('};')
    (out/'overlay_regions.hpp').write_text('\n'.join(registry)+'\n')
    declarations=['#pragma once','#include "recomp.h"','#ifdef __cplusplus','extern "C" {','#endif','void dora3_trace(uint8_t*, recomp_context*, uint32_t);','void dora3_pending(uint32_t);','void dora3_verify_dma(uint8_t*);','void dora3_capture_resource(uint8_t*, recomp_context*);','void dora3_mark_stream_dma(void);']
    bodies=['#include "startup_hooks.h"']
    for addr in sorted(frontier):
        name=f'pending_{addr:08X}'
        declarations.append(f'void {name}(uint8_t*, recomp_context*);')
        bodies.append(f'void {name}(uint8_t* rdram, recomp_context* ctx) {{ dora3_pending(0x{addr:08X}); }}')
    declarations.extend(['#ifdef __cplusplus','}','#endif'])
    (out/'startup_hooks.h').write_text('\n'.join(declarations)+'\n')
    (out/'frontier.c').write_text('\n'.join(bodies)+'\n')
    config='[input]\nentrypoint = 0x80025C00\n'
    for key,value in [('symbols_file_path',out/'startup.syms.toml'),('rom_file_path',a.rom.resolve()),('output_func_path',out/'RecompiledFuncs')]:
        config+=f'{key} = {json.dumps(value.as_posix())}\n'
    config+='recomp_include = '+json.dumps('#include "startup_hooks.h"')+'\n[patches]\n'
    config+='ignored = '+json.dumps([f'pending_{addr:08X}' for addr in sorted(frontier)])+'\n'
    config+='hook = [\n'
    for addr,(name,size,evidence) in compiled.items():
        config+=f'{{ func = "{name}", before_vram = 0x{addr:08X}, text = "dora3_trace(rdram, ctx, 0x{addr:08X});" }},\n'
    if 0x8002730C in compiled:
        config+='{ func = "dora3_rom_dma", before_vram = 0x800273EC, text = "dora3_verify_dma(rdram);" },\n'
    if 0x80034000 in compiled:
        config+='{ func = "dora3_compressed_stream_read", before_vram = 0x8003407C, text = "dora3_mark_stream_dma();" },\n'
    if 0x80030AE8 in compiled:
        config+='{ func = "dora3_resource_load", before_vram = 0x80030BF8, text = "dora3_capture_resource(rdram, ctx);" },\n'
    config+=']\n';(out/'startup.toml').write_text(config)
    lock=json.loads((ROOT/'tools/n64recomp/lock.json').read_text())
    receipt=json.loads((ROOT/'build-tools/n64recomp/build-receipt.json').read_text())
    if receipt['upstream_commit']!=lock['commit'] or receipt['patch_sha256']!=lock['patch_sha256']:raise ValueError('Tool lock mismatch')
    binary=Path(receipt['executables']['N64Recomp']['path'])
    if sha(binary)!=receipt['executables']['N64Recomp']['sha256']:raise ValueError('Tool binary mismatch')
    with (out/'generation.log').open('w') as log:subprocess.run([str(binary),str(out/'startup.toml')],stdout=log,stderr=subprocess.STDOUT,check=True)
    inputs=['recomp/startup.json','recomp/rom.lock.json','tools/generate_startup.py','tools/inspect_rom.py','tools/n64recomp/lock.json','tools/n64recomp/zero-register.patch']
    tracked=[f for f in out.rglob('*') if f.is_file() and f.name!='generation.log']
    report={'scope':'reviewed resident/overlay functions; explicit stop on unreviewed calls',
            'rom_sha256':hashlib.sha256(rom).hexdigest(),'tool_sha256':sha(binary),'inputs':{x:sha(ROOT/x) for x in inputs},
            'generated':{x.relative_to(out).as_posix():sha(x) for x in tracked},'frontier':[f'0x{x:08X}' for x in sorted(frontier)],'sections':[{k:v for k,v in section.items() if k not in ('compiled','indirect_targets')} for section in sections]}
    (out/'generation-receipt.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'{len(compiled)} original functions; {len(sdk)} SDK replacements; {len(frontier)} explicit pending boundaries')

if __name__=='__main__':main()
