"""Connected minigame Source arithmetic and frame ownership on synthetic CPU RAM."""
from decomp_annotations import read_decompiled_source
from pathlib import Path
import argparse,hashlib,json,os,re,struct,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--generated',type=Path,required=True);p.add_argument('--rom',type=Path,required=True)
    p.add_argument('--reference',type=Path,required=True)
    a=p.parse_args();g=a.generated.resolve();out=ROOT/'build-local/native-minigame-check';out.mkdir(exist_ok=True)
    receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,key in ((ROOT,'inputs'),(g,'generated')):
        for name,digest in receipt[key].items():assert sha(base/name)==digest,('Stale generation',name)
    rom=a.rom.read_bytes();assert hashlib.sha256(rom).hexdigest()==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
    def bodies(directory):
        return {re.search(r'void (\w+)\(',b)[1]:b for f in (directory/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
    current=bodies(g);frozen=bodies(a.reference.resolve())
    data=json.loads((ROOT/'research/native-minigame-reviewed.json').read_text())
    assert len(data['callbacks'])==12 and data['motion_implemented_modules']==list(range(1,10))
    assert 'implementation_ready=true' in (ROOT/'src/native_minigame_registry.hpp').read_text()
    # An Original-disabled hook leaves the same MIPS instruction stream in every
    # reviewed body. Compare all695 frozen module bodies, not only reached cases.
    proofs=[]
    for name,body in current.items():
        if not re.match(r'dora3_s(?:20|21|22|23|24|25|26|27|28|29)_mini_',name):continue
        old=frozen[name]
        asm=lambda b:list(dict.fromkeys(re.findall(r'// (0x[0-9A-Fa-f]+:.*)',b)))
        assert asm(body)==asm(old),('Original instructions changed',name)
        proofs.append({'function':name,'original_instruction_sha256':hashlib.sha256('\n'.join(asm(body)).encode()).hexdigest()})
    assert len(proofs)==695,len(proofs)
    selected={};external=set()
    mini={n for n in current if re.match(r'dora3_s(?:20|21|22|23|24|25|26|27|28|29)_mini_',n)}
    pure={0x8002DA98,0x8002F064,0x80040104,0x800327A4,0x80031A0C,0x80031884,0x80031AA4,0x80031A58,0x800393C4,0x8003931C,0x80055BD0,0x80057A80,0x80055BC0}
    bypc={int(re.search(r'// 0x([0-9A-Fa-f]+):',b)[1],16):n for n,b in current.items() if re.search(r'// 0x([0-9A-Fa-f]+):',b) and n not in mini}
    roots=list(mini)+[bypc[pc] for pc in pure]
    # Follow pure SDK math; command routines in mini bodies are explicit
    # fixture boundaries. Source graphics/allocator endpoints fail if reached.
    todo=[(n,n not in mini) for n in roots]
    while todo:
        name,pure_path=todo.pop()
        if name in selected:continue
        if name not in current:external.add(name);continue
        selected[name]=current[name]
        for callee in re.findall(r'\b(\w+)\(rdram, ctx\)',current[name]):
            if callee in mini or pure_path or callee in {bypc[pc] for pc in pure}:todo.append((callee,pure_path or callee not in mini))
            else:external.add(callee)
    external-=selected.keys()
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(selected.values()))
    cases=[('mini3_retreat','dora3_s23_mini_801C5E1C'),('mini8_intro','dora3_s28_mini_801C763C'),('mini8_body','dora3_s28_mini_801C6CEC'),('mini8_shadow','dora3_s28_mini_801C7570'),('mini2_visual','dora3_s22_mini_801C5E74'),('mini2_follower','dora3_s22_mini_801C56C8'),('mini1_effect','dora3_s21_mini_801C4DD8'),('mini1_projected','dora3_s21_mini_801C9728'),('mini4_projected','dora3_s24_mini_801C50A0'),('mini2_oscillation','dora3_s22_mini_801C45D0'),('mini3_camera','dora3_s23_mini_801C4BB4'),('mini4_camera','dora3_s24_mini_801C3FCC'),('mini5_control','dora3_s25_mini_801C59E8'),('mini6_rotation','dora3_s26_mini_801C41F0'),('mini7_result','dora3_s27_mini_801C4E58'),('mini8_physics','dora3_s28_mini_801C4300'),('mini9_result','dora3_s29_mini_801C5514')]
    header='extern "C" {\n'+'\n'.join(f'void {n}(uint8_t*,recomp_context*);' for n in selected)+'\n}\n'
    header+='\n'.join(f'inline constexpr auto {alias}={name};' for alias,name in cases)
    header+='\ninline auto fixture_common(unsigned i){const std::array<recomp_func_t*,3> functions{dora3_s20_mini_801BD5B8,dora3_s20_mini_801BE430,dora3_s20_mini_801BF8CC};return functions.at(i);}\n'
    header+='inline recomp_func_t* fixture_function(uint32_t pc){switch(pc){case 0x801C4D74:return dora3_s28_mini_801C4D74;case 0x801C4AF8:return dora3_s28_mini_801C4AF8;case 0x801C46B4:return dora3_s28_mini_801C46B4;default:throw std::logic_error("Unreviewed fixture dispatch");}}\n'
    (out/'minigame-functions.h').write_text('#include <array>\n#include <stdexcept>\n'+header)
    seed=bytearray(0x400000)
    # Resident/overlay constants and jump tables are exact local ROM bytes,
    # never tracked. BSS remains synthetic and is initialized by each case.
    sections=json.loads((g/'sections.json').read_text()) if (g/'sections.json').exists() else None
    # Existing generation sections are kept in the project config.
    config=json.loads((ROOT/'recomp/sections.json').read_text()) if (ROOT/'recomp/sections.json').exists() else None
    if not config:
        config=json.loads((ROOT/'research/overlay-map.json').read_text()) if (ROOT/'research/overlay-map.json').exists() else None
    # Seed resident whole startup and common module. Each case's overlay
    # constants are installed separately by fixture_seed_module below.
    maps=[(0x1000,0x80025C00,0x60980),(0x18BDF0,0x801B8ED0,0xA3C0)]
    for offset,vram,size in maps:
        raw=rom[offset:offset+size]
        for n in range(0,len(raw),4):seed[(vram&0xFFFFFF)+n:(vram&0xFFFFFF)+n+4]=raw[n:n+4][::-1]
    (out/'seed.bin').write_bytes(seed)
    # Per-module fixture ROM installation retains common/address identity.
    overlay_sections=[]
    for callback in data['callbacks'][:9]:
        prefix=callback['function'].split('_mini_')[0]+'_mini_'
        pcs=[int(re.search(r'// 0x([0-9A-Fa-f]+):',b)[1],16) for n,b in current.items() if n.startswith(prefix)]
        # Exact overlay sizes from the canonical project section manifest.
        section=next(s for s in data['sections'] if s['rom']==callback['rom']);next_rom=section['rom']+section['size']
        raw=rom[callback['rom']:next_rom]
        swizzled=b''.join(raw[n:n+4][::-1] for n in range(0,len(raw),4))
        path=out/(str(callback['rom'])+'.bin');path.write_bytes(swizzled)
        overlay_sections.append({'rom':callback['rom'],'path':str(path)})
    (out/'modules.json').write_text(json.dumps(overlay_sections))
    # Stub declarations are generated only for recorded external boundaries.
    command_addresses={0x8002A47C,0x8002A4A4,0x8002A1B8,0x80028C74,0x8002A51C,0x80042A9C,0x800300C4,0x80042B90,0x80030FA8,0x800444C0,0x800445E0}
    command_names={bypc[pc] for pc in command_addresses}
    stub='#include "recomp.h"\n#include <stdexcept>\nvoid fixture_command();\nextern "C" {\n'
    # Hook symbols are provided by the real runtime or inactive enemy adapter.
    hooks=set(re.findall(r'\b(dora3_native_\w+)\s*\(', (ROOT/'src/native_model_animation.cpp').read_text()))
    for name in sorted(external):
        if name.startswith('dora3_native_') or name=='dora3_trace':continue
        action='fixture_command();' if name in command_names else f'throw std::logic_error("Unseeded external boundary: {name}");'
        stub+=f'void {name}(uint8_t*,recomp_context*){{{action}}}\n'
    stub+='}\n';(out/'boundaries.cpp').write_text(stub)
    includes=['-I'+str(x) for x in (ROOT/'src',g,g/'RecompiledFuncs',out,ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include')]
    flags=['-O1','-g','-fno-strict-aliasing','-fno-omit-frame-pointer',*includes]
    cache=out/'original-object.json'
    command=['gcc','-std=c17',*flags,'-fsanitize=address','-c',str(out/'functions.c'),'-o',str(out/'functions.o')]
    fingerprint=hashlib.sha256((sha(out/'functions.c')+sha(g/'generation-receipt.json')+subprocess.check_output(['gcc','--version'],text=True)+repr(command)).encode()).hexdigest()
    reuse=cache.exists() and (out/'functions.o').exists() and json.loads(cache.read_text())=={'input':fingerprint,'object':sha(out/'functions.o')}
    if not reuse:
        subprocess.run(command,check=True)
        cache.write_text(json.dumps({'input':fingerprint,'object':sha(out/'functions.o')}))

    sources=[ROOT/'src'/n for n in ('native_minigames.cpp','native_minigame_math.cpp','native_model_animation.cpp','native_texture_animation.cpp','native_simulation_time.cpp','native_frame_time.cpp')]+[ROOT/'research/native-minigame-check.cpp',ROOT/'research/native-inactive-enemy-fixture.cpp',out/'boundaries.cpp']
    toolchain=subprocess.check_output(['g++','--version'],text=True)
    objects=[]
    for i,source in enumerate(sources):
        obj=out/('cpp'+str(i)+'.o');dep=out/('cpp'+str(i)+'.d');record=out/('cpp'+str(i)+'.json')
        cmd=['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,'-fsanitize=address,undefined','-MMD','-MF',str(dep),'-c',str(source),'-o',str(obj)]
        inputs={}
        if dep.exists():
            tokens=dep.read_text().replace('\\\n',' ').split(':',1)[1].split()
            inputs={n:sha(Path(n)) for n in tokens if Path(n).exists()}
        reuse=obj.exists() and record.exists() and json.loads(record.read_text())=={'command':cmd,'toolchain':toolchain,'inputs':inputs,'object':sha(obj)}
        if not reuse:
            subprocess.run(cmd,check=True)
            tokens=dep.read_text().replace('\\\n',' ').split(':',1)[1].split()
            inputs={n:sha(Path(n)) for n in tokens}
            record.write_text(json.dumps({'command':cmd,'toolchain':toolchain,'inputs':inputs,'object':sha(obj)}))
        objects.append(obj)
    subprocess.run(['g++','-fsanitize=address,undefined',*map(str,objects),str(out/'functions.o'),'-o',str(out/'check')],check=True)

    result=subprocess.check_output([str(out/'check'),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
    tracked=[f for f in sources if f.is_relative_to(ROOT) and not f.is_relative_to(ROOT/'build-local')]+[ROOT/'src/native_minigames.hpp',ROOT/'src/native_minigame_math.hpp',ROOT/'src/native_minigame_registry.hpp',ROOT/'tools/native_minigame_patches.py',ROOT/'research/native-minigame-reviewed.json',Path(__file__).resolve()]
    report=dict(result=result,generation_receipt_sha256=sha(g/'generation-receipt.json'),source_sha256={f.relative_to(ROOT).as_posix():sha(f) for f in tracked},source_body_proof=proofs,callbacks=data['callbacks'],original_functions=sorted(selected),stub_boundaries=sorted(external),no_game_gpu=True,limitations=['Synthetic CPU state/resources and reached branches; visual/full-game QA remains user-owned.'])
    (ROOT/'research/native-minigame-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(result)
if __name__=='__main__':main()
