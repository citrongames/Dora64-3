"""Validate actual Original story pickups and all six flash effect lifetimes, without a game/GPU."""
from pathlib import Path
import argparse, hashlib, json, os, re, subprocess, sys
from decomp_annotations import read_decompiled_source

ROOT = Path(__file__).resolve().parents[1]
EXTRA_HELPERS = {0x801E7528, 0x801E756C, 0x801E75D0, 0x801E7604,
                 0x801E7A50, 0x801E7B44, 0x801E7B78, 0x801E7EB8,
                 0x801E80FC, 0x801E810C, 0x801E8210, 0x801E8238,
                 0x801E8404, 0x801E89CC, 0x801E8A94, 0x8005A140, 0x8005A300}
ROOTS = ['dora3_s3_core_801ECC6C', 'dora3_s3_core_801ECBE4',
         'dora3_s3_core_801ED020', 'dora3_s3_core_801ECFB4',
         'dora3_s3_core_801ED244', 'dora3_s3_core_801ED228',
         'dora3_s3_core_801ED898', 'dora3_s3_core_801ED4A4']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def bodies(generation):
    return {re.search(r'void (\w+)\(', body)[1]: body
            for path in (generation / 'RecompiledFuncs').glob('*.c')
            for body in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',
                                   read_decompiled_source(path), re.S)}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generated', type=Path, required=True)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--reference', type=Path, default=ROOT/'build-local/startup-v61')
    parser.add_argument('--city-reference', type=Path, default=ROOT/'build-local/startup-v189')
    parser.add_argument('--scope',choices=['original','native-source','effect-delay','effect-motion','effect-pool','pickup-contact'],default='original')
    args = parser.parse_args()
    generation = args.generated.resolve()
    diagnostic=args.scope in ['native-source','effect-pool','pickup-contact']
    contact=args.scope=='pickup-contact'
    pool=diagnostic
    motion=args.scope=='effect-motion'
    delay=args.scope in ['effect-delay','effect-motion']
    folder='native-pickup-contact-check' if contact else 'native-flash-pool-check' if pool else 'native-flash-motion-check' if motion else 'native-flash-delay-check' if delay else ('native-story-pickup-native-source-check' if diagnostic else 'native-story-pickup-source-check')
    report_name=folder+'.json'
    out=ROOT/'build-local'/folder
    out.mkdir(exist_ok=True)
    # Use the same verified real common pool, constructors, resource lists,
    # destructors, model implementation and frozen Oracle as the whole-world check.
    checker = ROOT/'tools/check_native_player_update.py'
    preparation = checker.read_text()
    preparation = preparation[:preparation.index("    subprocess.run(['gcc','-std=c17'")]
    preparation = preparation.replace("out=ROOT/'build-local/native-player-update-check'", "out=ROOT/'build-local/native-story-pickup-source-check'")
    preparation = preparation.replace("out=ROOT/('build-local/native-player-update-check' if a.scope=='full' else 'build-local/native-scene-frame-check')", "out=ROOT/'build-local/native-story-pickup-source-check'")
    preparation=preparation.replace("out=ROOT/'build-local/native-story-pickup-source-check'",f"out=ROOT/'build-local/{folder}'")
    sys.argv = [str(checker), '--generated', str(generation), '--rom', str(args.rom),
                '--reference', str(args.reference), '--city-reference', str(args.city_reference)]
    preparation += "\nmain()\n"
    exec(compile(preparation, str(checker), 'exec'), dict(__file__=str(checker), __name__='__main__'))
    current = bodies(generation)
    frozen = bodies(args.city_reference)
    base = (out/'functions.c').read_text()
    existing = set(re.findall(r'RECOMP_FUNC void (\w+)\(', base))
    fixture = ROOT/'research/native-story-pickup-source-check.cpp'
    supplied = set(re.findall(r'void (\w+)\(uint8_t\*', fixture.read_text())) | {
        'dora3_native_input_sample', 'dora3_trace', 'dora3_native_model_advance', 'dora3_native_model_morph_allowed'}
    selected = {}
    todo = list(ROOTS)
    while todo:
        name = todo.pop()
        if name in existing or name in supplied or name in selected:
            continue
        assert name in current and name in frozen, name
        body = current[name]
        pc = int(re.search(r'// 0x([0-9A-F]+):', body)[1], 16)
        assert 0x801EBA00 <= pc <= 0x801ED898 or pc in EXTRA_HELPERS, name
        if pc==0x801EC598:
            expected_sites={0x801EC6F4,0x801EC704,0x801EC72C,0x801EC740,0x801EC764,0x801EC79C,0x801EC7B8}
            sites={int(x,16) for x in re.findall(r'dora3_native_preview_hook\(rdram,ctx,0x([0-9A-F]+)u\)',body)}
            assert sites==expected_sites,(name,sites)
            clean=re.sub(r'^    (?:dora3_native_preview_hook\(rdram,ctx,0x[0-9A-F]+u\);|if \(dora3_native_preview_hook\(rdram,ctx,0x[0-9A-F]+u\)\).+|dora3_after_pickup_[0-9A-F]+:;)\n','',body,flags=re.M)
            assert clean==frozen[name],name
        if pc==0x801ED5F4:
            expected_sites={0x801ED5F4,0x801ED664,0x801ED6B0,0x801ED6E0,0x801ED70C,0x801ED72C,0x801ED74C,0x801ED76C,0x801ED778,0x801ED79C,0x801ED7B8,0x801ED7F4,0x801ED814,0x801ED840,0x801ED85C,0x801ED884}
            sites={int(x,16) for x in re.findall(r'dora3_native_preview_hook\(rdram,ctx,0x([0-9A-F]+)u\)',body)}
            assert sites==expected_sites,(name,sites)
            clean=re.sub(r'^    (?:dora3_native_preview_hook\(rdram,ctx,0x[0-9A-F]+u\);|if \(dora3_native_preview_hook\(rdram,ctx,0x[0-9A-F]+u\)\).+|dora3_after_flash_[0-9A-F]+:;)\n','',body,flags=re.M)
            assert clean.count('; dora3_native_preview_hook(rdram,ctx,0x801ED5F4u);')==1
            clean=clean.replace('; dora3_native_preview_hook(rdram,ctx,0x801ED5F4u);',';')
            assert clean==frozen[name],name
        if pc==0x801ED4AC:
            for hook in ['if (dora3_native_preview_hook(rdram,ctx,0x801ED520u)) goto L_801ED5E8;', 'dora3_native_preview_hook(rdram,ctx,0x801ED5E8u);']:
                assert body.count(hook)==1,(name,hook)
                body=body.replace('    '+hook+'\n','')
            assert body==frozen[name],name
            body=current[name]
        elif pc not in [0x801ED5F4,0x801EC598]:assert body == frozen[name] and 'dora3_native_preview_hook' not in body, name
        selected[name] = body
        todo.extend(re.findall(r'\b(\w+)\(rdram, ctx\)', body))
    assert set(ROOTS).issubset(existing | supplied | set(selected)),set(ROOTS)-existing-supplied-set(selected)
    (out/'functions.c').write_text(base+'\n'+'\n'.join(selected.values()))
    header = out/'player-update-functions.h'
    header.write_text(header.read_text()+'\nextern "C" {\n'+'\n'.join(
        f'void {name}(uint8_t*,recomp_context*);' for name in selected)+'\n}\n')
    if pool:
        references={name:frozen[name].replace('void '+name+'(', 'void reference_'+name+'(',1) for name in selected}
        for name,body in references.items():
            for dependency in selected:body=re.sub(r'\b'+dependency+r'(?=\(rdram, ctx\))','reference_'+dependency,body)
            references[name]=body
        with (out/'functions.c').open('a') as stream:
            stream.write('\n'+'\n'.join(f'void reference_{name}(uint8_t*,recomp_context*);' for name in selected)+'\n'+'\n'.join(references.values()))
        with header.open('a') as stream:stream.write('\nextern "C" {\n'+'\n'.join(f'void reference_{name}(uint8_t*,recomp_context*);' for name in selected)+'\n}\n')
    original_delay=frozen['dora3_s3_core_801ED4AC'].replace('void dora3_s3_core_801ED4AC(', 'void reference_dora3_s3_core_801ED4AC(')
    assert 'reference_dora3_s3_core_801ED4AC' not in base
    if not pool:
        with (out/'functions.c').open('a') as stream:stream.write('\n'+original_delay)
    if motion:
        original_motion=frozen['dora3_s3_core_801ED5F4'].replace('void dora3_s3_core_801ED5F4(', 'void reference_dora3_s3_core_801ED5F4(')
        def tail(body,label):
            part=body[body.index('    // 0x801E8D88:'):body.index('    // 0x801E8E28:')]
            part='\n'.join(line for line in part.splitlines() if 'dora3_native_preview_hook(rdram,ctx,0x801E8D88u)' not in line)+'\n'
            return f'RECOMP_FUNC void {label}(uint8_t* rdram,recomp_context* ctx) {{\nint c1cs=0;\n'+part+'L_801E8E7C:;\n}\n'
        assert 'dora3_s3_801E8BDC' in current and 'dora3_s3_801E8BDC' in frozen
        with (out/'functions.c').open('a') as stream:
            stream.write('\n'+original_motion+'\n'+tail(frozen['dora3_s3_801E8BDC'],'reference_flash_common_tail')+'\n'+tail(current['dora3_s3_801E8BDC'],'native_flash_common_tail'))
    flags = ['-O0' , '-g', '-fno-omit-frame-pointer', '-fno-strict-aliasing', *[
        '-I'+str(path) for path in [ROOT/'src', ROOT/'research', generation,
                                  generation/'RecompiledFuncs', out,
                                  ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    if diagnostic:flags+=['-DDORA3_STORY_SOURCE_NATIVE_CHECK','-DDORA3_STORY_EFFECT_POOL_CHECK']
    if contact:flags+=['-DDORA3_STORY_PICKUP_CONTACT_CHECK']
    if delay:flags+=['-DDORA3_STORY_EFFECT_DELAY_CHECK']
    if motion:flags+=['-DDORA3_STORY_EFFECT_MOTION_CHECK']
    subprocess.run(['gcc', '-std=c17', '-fsanitize=address', *flags, '-c',
                    str(out/'functions.c'), '-o', str(out/'functions.o')], check=True)
    sources = [ROOT/'src'/name for name in ['native_gameplay_preview.cpp',
        'native_input_timeline.cpp', 'native_simulation_time.cpp', 'native_camera_timeline.cpp',
        'native_model_animation.cpp', 'native_frame_time.cpp']]
    binary = out/'check'
    subprocess.run(['g++', '-std=c++20', '-Wall', '-Wextra', '-Werror',
                    '-Wno-unused-variable', '-Wno-unused-but-set-variable',
                    '-fsanitize=address,undefined', *flags, *map(str, sources),
                    str(fixture), str(out/'functions.o'), str(out/'boundaries.cpp'),
                    '-o', str(binary)], check=True)
    with (out/'result.log').open('w') as stream:
        result = subprocess.run([str(binary), str(out/'seed.bin')], stdout=stream,
            stderr=subprocess.STDOUT, text=True, env=dict(os.environ,
            ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1'))
    output = (out/'result.log').read_text().strip()
    print(output, flush=True)
    result.check_returncode()
    assert output.startswith('PASS: 96 actual Source mode3 contact' if contact else 'PASS: 168 connected Native flash pool' if pool else 'PASS: 288 Native flash motion' if motion else 'PASS: 144 Native flash delay' if delay else 'PASS: 168 ')
    tracked = sources+[fixture, checker, Path(__file__).resolve(),
                       ROOT/'research/native-npc-interaction-check.inc', ROOT/'recomp/startup.json']
    tracked+=sorted((ROOT/'src').glob('*.hpp'))
    report = dict(result=output, extra_original_functions=sorted(selected),
        source_sha256={str(path.relative_to(ROOT)):sha(path) for path in tracked},
        generation_receipt_sha256=sha(generation/'generation-receipt.json'),
        reference_receipt_sha256=sha(args.city_reference/'generation-receipt.json'),
        validated_schedules=dict(settings=24, producer_profiles=7, source_schedules=168,
            successful_item_lifetimes=96, flash_effect_lifetimes=576,
            already_set_quest_schedules=72, original_rates=[20,30],
            configured_native_rates=[60,73,120,144,165,360], new_native_ownership=False),
        limitations=['Source producer replay compares full guest RAM/context/heap/resources/commands/model count with frozen189; items and all six effects execute actual Original lifetimes and destructors on synthetic assets',
            'Only successful and already-set quest branches; full-pool failure and real-level dialogue progression remain unverified',
            'Render FPS is a setting matrix only; this check does not claim Native item trajectories'])
    if delay:
        report['scope']='Isolated effect-delay callback intervals; common pool ownership remains Source'
        report['validated_schedules']=dict(settings=24,selector_schedules=144,original_rates=[20,30],configured_native_rates=[60,73,120,144,165,360],new_native_ownership=False)
        report['source_sha256']['research/native-flash-delay-check.inc']=sha(ROOT/'research/native-flash-delay-check.inc')
        report['limitations']=['Actual generated delay body and descriptor constructor, synthetic assets; every end compared with frozen189 Original actor/physics/model/commands', 'Direct SimulationTimeline callback diagnostic; no native_gameplay_preview pool dispatch, movement-state10 ownership or renderer execution']
    if motion:
        report['scope']='Isolated generated flash motion and actual common coordinate tail; six descriptor effects at birth and after3 Source ticks'
        report['validated_schedules']=dict(settings=24,selector_birth_acquisition_schedules=288,original_rates=[20,30],configured_native_rates=[60,73,120,144,165,360],new_native_ownership=False)
        report['source_sha256']['research/native-flash-motion-check.inc']=sha(ROOT/'research/native-flash-motion-check.inc')
        report['limitations']=['Generated real ED5F4 callback, actual E810C endpoint probe, actual Original common-coordinate region and genuine constructors/destructors on synthetic resources', 'Live actor/physics/model and heap/resources/commands checked at Original endpoints; freed payload bytes are not observable owners', 'Direct SimulationTimeline callback diagnostic; no actual native_gameplay_preview pool ownership, Model advance in the excluded tail remainder, real-level collision or renderer execution']
    if pool:
        report['scope']='Actual Native kind130 pool update interleaved with real Source reward producer/item loops; paired frozen Original world lifetimes'
        report['source_sha256']['research/native-flash-pool-check.inc']=sha(ROOT/'research/native-flash-pool-check.inc')
        report['validated_schedules']=dict(settings=24,producer_profiles=7,connected_schedules=168,original_rates=[20,30],configured_native_rates=[60,73,120,144,165,360],new_native_ownership={'118':[10],'123':[10],'127':[10],'130':[10]})
        report['limitations']=['Actual Source producer, ordered Source delay states, Native flash state10 common pool, descriptor constructors, full model-advance tail and retirement on synthetic resources', 'Frame-bank ownership and loop activation are fixture flags; no real NativeRuntime/FrameSession/presentation/renderer in this scoped proof', 'Uncollected item states remain Source-owned; real-level collision and full pool exhaustion not yet covered']
    if contact:
        report['scope']='Actual Source mode3 contact after three uncollected state1 passes, Source initialization then connected Native item/effect state10; paired frozen Original'
        report['validated_schedules']['actual_mode3_contacts']=96
        report['validated_schedules']['source_idle_passes_before_contact']=3
        report['limitations']+=['Fixture external state1 command after genuine quest producer; no real-level item placement/contact selector or collection trigger claim']
    (ROOT/'research'/report_name).write_text(json.dumps(report,indent=2)+'\n')

if __name__ == '__main__':
    main()
