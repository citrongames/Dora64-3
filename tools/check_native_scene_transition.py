"""Verify the real Source room resource barrier after an entered Native frame, no game/GPU."""
from pathlib import Path
import argparse,hashlib,json,os,re,subprocess,sys
from decomp_annotations import read_decompiled_source

ROOT=Path(__file__).resolve().parents[1]
ROOM_ROOTS=['dora3_s3_801BB080','dora3_s3_801BA0F4','dora3_s3_801B9FDC']

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generated',type=Path,required=True)
    parser.add_argument('--rom',type=Path,required=True)
    parser.add_argument('--reference',type=Path,required=True)
    parser.add_argument('--city-reference',type=Path,required=True)
    args=parser.parse_args();g=args.generated.resolve()
    out=ROOT/'build-local/native-scene-transition-check';out.mkdir(exist_ok=True)
    # Reuse the checked real pool/constructor/player closure and ROM seeds.
    # This runs preparation only, never the full gameplay binary. All game C
    # is extracted afresh from verified generated input, never copied from
    # an earlier diagnostic executable or a persistent research source.
    checker=ROOT/'tools/check_native_player_update.py'
    preparation=checker.read_text().split("    subprocess.run(['gcc','-std=c17'",1)[0]
    location="out=ROOT/('build-local/native-player-update-check' if a.scope=='full' else 'build-local/native-scene-frame-check')"
    assert preparation.count(location)==1
    preparation=preparation.replace(location,"out=ROOT/'build-local/native-scene-transition-check'")
    selection="fixture_name='native-player-update-check.cpp' if a.scope=='full' else 'native-scene-frame-check.cpp'"
    assert preparation.count(selection)==1
    preparation=preparation.replace(selection,"fixture_name='native-scene-transition-check.cpp'")
    sys.argv=[str(checker),'--generated',str(g),'--rom',str(args.rom),'--reference',str(args.reference),'--city-reference',str(args.city_reference)]
    exec(compile(preparation+'\nmain()\n',str(checker),'exec'),dict(__file__=str(checker),__name__='__main__'))
    bodies={m[1]:m[0] for f in (g/'RecompiledFuncs').glob('*.c') for m in re.finditer(r'RECOMP_FUNC\s+void (\w+)\([^\n]+\n.*?(?=RECOMP_FUNC\s+void |\Z)',read_decompiled_source(f),re.S)}
    selected={name:bodies[name] for name in ROOM_ROOTS}
    fixture=ROOT/'research/native-scene-transition-check.cpp'
    text=fixture.read_text()
    known=set(re.findall(r'void (\w+)\(', (out/'player-update-functions.h').read_text()))|set(re.findall(r'void (\w+)\(',text))|set(ROOM_ROOTS)
    known.update(re.findall(r'void (\w+)\(', (out/'boundaries.cpp').read_text()))
    missing={name for body in selected.values() for name in re.findall(r'\b(\w+)\(rdram, ctx\)',body)}-known
    (out/'room-boundaries.cpp').write_text('#include "recomp.h"\n#include <cstdio>\n#include <cstdlib>\n'+'\n'.join(f'extern "C" void {name}(uint8_t*,recomp_context*) {{std::fputs("Unexpected room boundary {name}\\n",stderr);std::abort();}}' for name in sorted(missing)))
    (out/'room-functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(selected.values()))
    # Rename only diagnostic routing entry points and the wall clock. The
    # real Runtime/FrameSession/Bridge/Ledger/Policy/Scheduler implementation
    # performs activation, publication, retirement and barrier draining.
    loop=(ROOT/'src/native_game_loop.cpp').read_text()
    for name in ['gameplay_active','owns_frame_banks','owns_fade','fade_command','resource_barrier']:
        pattern=r'(extern "C" (?:int|void) )dora3_native_loop_'+name+r'\('
        loop,count=re.subn(pattern,r'\1fixture_real_loop_'+name+'(',loop);assert count==1,name
    (out/'loop.cpp').write_text(loop)
    bridge=(ROOT/'src/native_frame_bridge.cpp').read_text()
    for before,after in [('const FrameBinding* active_frame_binding() {','const FrameBinding* fixture_real_active_frame_binding() {'),('Nanoseconds monotonic_time() {','Nanoseconds fixture_unused_wall_clock() {')]:
        assert bridge.count(before)==1
        bridge=bridge.replace(before,after)
    (out/'bridge.cpp').write_text(bridge)
    flags=['-DDORA3_FIXTURE_REAL_LOOP','-O0','-g','-pthread','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(path) for path in [ROOT/'src',ROOT/'research',g,g/'RecompiledFuncs',out,ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    for source in ['functions','room-functions']:
        subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/f'{source}.c'),'-o',str(out/f'{source}.o')],check=True)
    runtime_names=['native_cutscene.cpp','native_ui.cpp','native_texture_animation.cpp','native_effects.cpp','native_gameplay_preview.cpp','native_enemy.cpp','native_input_timeline.cpp','native_simulation_time.cpp','native_camera_timeline.cpp','native_model_animation.cpp','native_frame_time.cpp','native_frame_scheduler.cpp','native_scene_policy.cpp','native_present.cpp','native_process_time.cpp']
    sources=[ROOT/'research/native-gameplay-cutscene-fixture.cpp',ROOT/'research/native-ui-menu-fatal-fixture.cpp',ROOT/'research/native-scene-transition-menu-matrix-fixture.cpp']+[ROOT/'src'/name for name in runtime_names]+[out/'loop.cpp',out/'bridge.cpp',fixture]
    binary=out/'check'
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-variable','-Wno-unused-but-set-variable','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),str(out/'boundaries.cpp'),str(out/'room-functions.o'),str(out/'room-boundaries.cpp'),'-o',str(binary)],check=True)
    result=subprocess.check_output([str(binary),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
    assert '32 slot09 Native edge schedules' in result and 'settings=8' in result and '3192 real common NPC collision/B priority' in result
    tracked=[ROOT/'src'/name for name in runtime_names+['native_game_loop.cpp','native_frame_bridge.cpp']]+[fixture,checker,Path(__file__).resolve(),ROOT/'recomp/native-timing-rules.json',ROOT/'research/native-npc-interaction-check.inc',ROOT/'research/native-player-motion-check.cpp',ROOT/'research/player-state-inventory.json']
    tracked+=sorted((ROOT/'src').glob('*.hpp'))
    tracked.append(ROOT/'research/native-scene-transition-menu-matrix-fixture.cpp')
    report=dict(result=result,scope='Actual Source BB080 -> BA0F4 -> B9FDC first room resource barrier after an entered Native FD frame; no game/GPU',source_sha256={path.relative_to(ROOT).as_posix():sha(path) for path in tracked},generation_receipt_sha256=sha(g/'generation-receipt.json'),reference_receipt_sha256=sha(args.reference/'generation-receipt.json'),city_reference_receipt_sha256=sha(args.city_reference/'generation-receipt.json'),original_room_functions=ROOM_ROOTS,unvisited_fatal_boundaries=sorted(missing),diagnostic_generated_sha256={name:sha(out/name) for name in ['functions.c','room-functions.c','loop.cpp','bridge.cpp']},validated=dict(settings=8,native_schedules=32,original_rates=[20,30],native_fps=[60,73],camera_modes=[12,2],pending_phase_before_barrier=2,banks=2,received_messages=6,deferred_fifo=3,context_and_scratch_preserved=True),limitations=['Synthetic assets and graphics/presentation endpoints; no game/renderer/GPU','Stops after actual first resource barrier, before original destructors and subsequent room DMA/decode/construction','Empty projectile pool here; active/newborn projectile continuation is checked separately by the scene-frame scope','Tests exact command routing and two-bank/early guest completion/late retirement/late host read/FIFO/context; no actual GPU retirement'])
    (ROOT/'research/native-scene-transition-check.json').write_text(json.dumps(report,indent=2)+'\n')
    print(result)

if __name__=='__main__':main()
