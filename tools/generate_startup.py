"""Generate reviewed startup functions with explicit fail-stop boundaries."""
import argparse, hashlib, json, struct, subprocess
from pathlib import Path
from inspect_rom import ROOT, read_rom
from overlay_layout import prepare_layout, pending_name
from local_jump_tables import reviewed_local_jumps, verify_generated_local_jumps

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path)
    p.add_argument('--output-dir',required=True,type=Path)
    a=p.parse_args();rom=read_rom(a.rom);out=a.output_dir.resolve()
    if not out.is_relative_to(ROOT/'build-local'):raise ValueError('Keep generated code under build-local')
    out.mkdir(parents=True,exist_ok=False)
    meta=json.loads((ROOT/'recomp/startup.json').read_text())
    sections,compiled,sdk,frontier=prepare_layout(meta,rom)
    local_jumps=reviewed_local_jumps(meta,sections,compiled,rom)
    functions={key:(v[0],v[1]) for key,v in compiled.items()}
    functions.update({(0,addr):(v[0],4) for addr,v in sdk.items()})
    functions.update({(index,addr):(pending_name(index,addr),4) for index,addr in frontier})
    grouped={section['name']:[] for section in sections}
    for (index,addr),(name,size) in sorted(functions.items()):
        grouped[sections[index]['name']].append(f'{{ name = "{name}", vram = 0x{addr:08X}, size = 0x{size:X} }},')
    lines=[]
    for section in sections:
        lines.extend(['[[section]]',f'name = "{section["name"]}"',f'rom = 0x{section["rom"]:X}',
                      f'vram = 0x{section["vram"]:X}',f'size = 0x{section["size"]:X}','functions = ['])
        lines.extend(grouped[section['name']]);lines.append(']')
    (out/'startup.syms.toml').write_text('\n'.join(lines)+'\n')
    registry=['#pragma once','#include <cstdint>','#include <array>',
              'struct Dora3OverlayRegion { uint32_t rom, ram, size; };',
              'static constexpr Dora3OverlayRegion dora3_overlay_regions[] = {']
    for section in sections[1:]:
        registry.append(f'    {{0x{section["rom"]:X},0x{section["vram"]:X},0x{section["size"]:X}}},')
    registry.append('};')
    dependencies=[(section['rom'],next(i for i,s in enumerate(sections[1:]) if s['name']==name))
                  for section in sections[1:] for name in section.get('requires',[])]
    registry.extend(['struct Dora3OverlayDependency { uint32_t rom, required_index; };',
                     f'static constexpr std::array<Dora3OverlayDependency, {len(dependencies)}> dora3_overlay_dependencies = {{{{'])
    registry.extend(f'    {{0x{rom:X}, {index}}},' for rom,index in dependencies)
    registry.append('}};')
    (out/'overlay_regions.hpp').write_text('\n'.join(registry)+'\n')
    declarations=['#pragma once','#include "recomp.h"','#ifdef __cplusplus','extern "C" {','#endif','void dora3_trace(uint8_t*, recomp_context*, uint32_t, uint32_t);','void dora3_pending(uint32_t);','void dora3_verify_dma(uint8_t*);','void dora3_capture_resource(uint8_t*, recomp_context*);','void dora3_mark_stream_dma(void);']
    native_names=[v[0] for v in meta.get('native',{}).values()]
    declarations.extend(['void dora3_native_task_register(uint8_t*, recomp_context*);',
                         'void dora3_native_task_retired(uint8_t*, recomp_context*);',
                         'void dora3_native_guest_completion(uint8_t*, recomp_context*);',
                         'int dora3_native_framebuffer_ready(uint32_t);',
                         'int dora3_native_completion_during_pause(uint8_t*, recomp_context*);'])
    declarations.extend(['int dora3_native_scheduler_message(uint8_t*, recomp_context*);',
                         'int dora3_native_scheduler_pumping(void);',
                         'void dora3_native_task_submitted(uint8_t*, recomp_context*);'])
    declarations.append('int dora3_native_game_message(uint8_t*, recomp_context*);')
    declarations.append('uint32_t dora3_bus_read32(uint8_t*, uint32_t);')
    declarations.append('void dora3_capture_frame_lights(uint8_t*, uint32_t);')
    declarations.extend(['void dora3_begin_cpu_fb_copy(void);','void dora3_end_cpu_fb_copy(void);'])
    declarations.extend(f'void {name}(uint8_t*, recomp_context*);' for name in native_names)
    bodies=['#include "startup_hooks.h"']
    for index,addr in sorted(frontier):
        name=pending_name(index,addr)
        declarations.append(f'void {name}(uint8_t*, recomp_context*);')
        bodies.append(f'void {name}(uint8_t* rdram, recomp_context* ctx) {{ dora3_pending(0x{addr:08X}); }}')
    declarations.extend(['#ifdef __cplusplus','}','#endif'])
    (out/'startup_hooks.h').write_text('\n'.join(declarations)+'\n')
    (out/'frontier.c').write_text('\n'.join(bodies)+'\n')
    config='[input]\nentrypoint = 0x80025C00\n'
    for key,value in [('symbols_file_path',out/'startup.syms.toml'),('rom_file_path',a.rom.resolve()),('output_func_path',out/'RecompiledFuncs')]:
        config+=f'{key} = {json.dumps(value.as_posix())}\n'
    config+='recomp_include = '+json.dumps('#include "startup_hooks.h"')+'\n[patches]\n'
    config+='ignored = '+json.dumps([pending_name(index,addr) for index,addr in sorted(frontier)]+native_names)+'\n'
    config+='hook = [\n'
    for (index,addr),(name,size,evidence) in compiled.items():
        hook=f'dora3_trace(rdram, ctx, 0x{addr:08X}, 0x{sections[index]["rom"]:X});'
        if index==0 and addr==0x80026CEC:
            hook+=' if (dora3_native_framebuffer_ready((uint32_t)ctx->r4)) { ctx->r2 = ctx->r4; return; }'
        config+=f'{{ func = "{name}", before_vram = 0x{addr:08X}, text = {json.dumps(hook)} }},\n'
    if (0,0x80045520) in compiled:
        for load,word,displacement in [(0x8004552C,0x8CC7FFFC,-4),(0x80045574,0x8CC7FFFC,-4),(0x800455DC,0x8CC70000,0)]:
            assert struct.unpack_from('>I',rom,0x1000+load-0x80025C00)[0]==word
            hook=f'ctx->r7 = (int32_t)dora3_bus_read32(rdram, (uint32_t)ctx->r6 + ({displacement})); goto dora3_after_{load:08X};'
            config+=f'{{ func = "dora3_pak_font_decode", before_vram = 0x{load:08X}, text = {json.dumps(hook)} }},\n'
            config+=f'{{ func = "dora3_pak_font_decode", before_vram = 0x{load+4:08X}, text = "dora3_after_{load:08X}:;" }},\n'
    if (0,0x800268A8) in compiled and (0,0x80026A28) in compiled:
        for address,word in [(0x800268DC,0x26240040),(0x800268EC,0x3C02801B),
                             (0x80026A50,0xAE22027C),(0x80026A54,0x26240078),
                             (0x80026AEC,0x8E300260),(0x80026B14,0x8FBF0028)]:
            if struct.unpack_from('>I',rom,0x1000+address-0x80025C00)[0]!=word:
                raise ValueError('Unreviewed native scheduler pump hook')
        hooks=[('dora3_scheduler_thread',0x800268DC,'dora3_scheduler_receive:;'),
               ('dora3_scheduler_thread',0x800268EC,'if (dora3_native_scheduler_message(rdram, ctx)) goto dora3_scheduler_receive;'),
               ('dora3_scheduler_retrace',0x80026A50,'if (dora3_native_scheduler_pumping()) goto dora3_scheduler_after_count;'),
               ('dora3_scheduler_retrace',0x80026A54,'dora3_scheduler_after_count:;'),
               ('dora3_scheduler_retrace',0x80026AEC,'if (dora3_native_scheduler_pumping()) goto L_80026B14;')]
        for name,address,hook in hooks:
            config+=f'{{ func = "{name}", before_vram = 0x{address:08X}, text = {json.dumps(hook)} }},\n'
    for address,word in [(0x80026BEC,0x8E220274),(0x80026C94,0x8E220274),(0x80027A6C,0x08009E38),
                         (0x80026CEC,0x27BDFFE0)]:
        if struct.unpack_from('>I',rom,0x1000+address-0x80025C00)[0]!=word:
            raise ValueError('Unreviewed native present hook')
    present_hooks=[('dora3_scheduler_sp_complete',0x80026BEC,'dora3_native_task_retired(rdram, ctx);'),
                   ('dora3_scheduler_dp_complete',0x80026C94,'dora3_native_task_retired(rdram, ctx);'),
                   ('dora3_game_message_loop',0x80027A6C,'dora3_native_guest_completion(rdram, ctx);')]
    for name,address,hook in present_hooks:
        config+=f'{{ func = "{name}", before_vram = 0x{address:08X}, text = {json.dumps(hook)} }},\n'
    if (0,0x8002787C) in compiled:
        # Native OSMesg is a value, not a pointer. Intercept before the original LH.
        for address, word in [(0x800278E0,0x3C04801B),(0x800278EC,0x0C0160F8),
                              (0x800278F4,0x3C02801B),(0x80027918,0x84500000)]:
            if struct.unpack_from('>I',rom,0x1000+address-0x80025C00)[0]!=word:
                raise ValueError('Unreviewed native game-message hook')
        name=compiled[(0,0x8002787C)][0]
        hook='if (dora3_native_game_message(rdram, ctx)) goto L_800278E0; if (dora3_native_completion_during_pause(rdram, ctx)) goto L_80027914;'
        config+=f'{{ func = "{name}", before_vram = 0x800278F4, text = {json.dumps(hook)} }},\n'
    if (0,0x8002FE20) in compiled:
        # Capture before publishing OSScTask, not at delayed RSP submission.
        if struct.unpack_from('>I',rom,0x1000+0x8002FF58-0x80025C00)[0]!=0x8FBF0010:
            raise ValueError('Unreviewed native task post-publication hook')
        config+='{ func = "dora3_graphics_task_submit", before_vram = 0x8002FF58, text = "dora3_native_task_submitted(rdram, ctx);" },\n'
        for address, word in [(0x8002FF48,0x01002821),(0x8002FF50,0x0C016144),(0x8002FF54,0xAD02000C)]:
            if struct.unpack_from('>I',rom,0x1000+address-0x80025C00)[0]!=word:
                raise ValueError('Unreviewed frame publication hook')
        name=compiled[(0,0x8002FE20)][0]
        hook='dora3_capture_frame_lights(rdram, (uint32_t)ctx->r8); dora3_native_task_register(rdram, ctx);'
        config+=f'{{ func = "{name}", before_vram = 0x8002FF50, text = {json.dumps(hook)} }},\n'
    if (0,0x800271B0) in compiled:
        # No guest calls or exits between these hooks except the common epilogue.
        for address,word in [(0x800272BC,0x10C0000E),(0x800272F0,0x1460FFF4),(0x800272F8,0x8FBF0020)]:
            if struct.unpack_from('>I',rom,0x1000+address-0x80025C00)[0]!=word:
                raise ValueError('Unreviewed CPU framebuffer loop')
        name=compiled[(0,0x800271B0)][0]
        for address,hook in [(0x800272BC,'dora3_begin_cpu_fb_copy();'),(0x800272F8,'dora3_end_cpu_fb_copy();')]:
            config+=f'{{ func = "{name}", before_vram = 0x{address:08X}, text = {json.dumps(hook)} }},\n'
    for jump in local_jumps:
        config+=f'{{ func = "{jump["func"]}", before_vram = 0x{jump["load"]:08X}, text = {json.dumps(jump["text"])} }},\n'
    if (0,0x8002730C) in compiled:
        config+='{ func = "dora3_rom_dma", before_vram = 0x800273EC, text = "dora3_verify_dma(rdram);" },\n'
    if (0,0x80034000) in compiled:
        config+='{ func = "dora3_compressed_stream_read", before_vram = 0x8003407C, text = "dora3_mark_stream_dma();" },\n'
    if (0,0x80030AE8) in compiled:
        config+='{ func = "dora3_resource_load", before_vram = 0x80030BF8, text = "dora3_capture_resource(rdram, ctx);" },\n'
    config+=']\n';(out/'startup.toml').write_text(config)
    lock=json.loads((ROOT/'tools/n64recomp/lock.json').read_text())
    receipt=json.loads((ROOT/'build-tools/n64recomp/build-receipt.json').read_text())
    if receipt['upstream_commit']!=lock['commit'] or receipt['patch_sha256']!=lock['patch_sha256']:raise ValueError('Tool lock mismatch')
    binary=Path(receipt['executables']['N64Recomp']['path'])
    if sha(binary)!=receipt['executables']['N64Recomp']['sha256']:raise ValueError('Tool binary mismatch')
    with (out/'generation.log').open('w') as log:subprocess.run([str(binary),str(out/'startup.toml')],stdout=log,stderr=subprocess.STDOUT,check=True)
    verify_generated_local_jumps(out/'RecompiledFuncs',local_jumps)
    inputs=['tools/local_jump_tables.py','recomp/startup.json','recomp/rom.lock.json','tools/generate_startup.py','tools/overlay_layout.py','tools/inspect_rom.py','tools/n64recomp/lock.json','tools/n64recomp/zero-register.patch']
    tracked=[f for f in out.rglob('*') if f.is_file() and f.name!='generation.log']
    report={'scope':'reviewed resident/overlay functions; explicit stop on unreviewed calls',
            'rom_sha256':hashlib.sha256(rom).hexdigest(),'tool_sha256':sha(binary),'inputs':{x:sha(ROOT/x) for x in inputs},
            'generated':{x.relative_to(out).as_posix():sha(x) for x in tracked},'frontier':[{'section':sections[i]['name'],'address':f'0x{x:08X}'} for i,x in sorted(frontier)],'sections':[{k:v for k,v in section.items() if k not in ('compiled','indirect_targets')} for section in sections]}
    (out/'generation-receipt.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'{len(compiled)} original functions; {len(sdk)-len(native_names)} SDK replacements; {len(native_names)} native hardware replacements; {len(frontier)} explicit pending boundaries')

if __name__=='__main__':main()
