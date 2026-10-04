"""Validate actual Original story pickups and all six flash effect lifetimes, without a game/GPU."""
from pathlib import Path
import argparse, hashlib, json, os, re, subprocess, sys
from decomp_annotations import read_decompiled_source

ROOT = Path(__file__).resolve().parents[1]
EXTRA_HELPERS = {0x801E7528, 0x801E756C, 0x801E75D0, 0x801E7604,
                 0x801E7A50, 0x801E7B44, 0x801E7B78, 0x801E7EB8,
                 0x801E80FC, 0x801E810C, 0x801E8210, 0x801E8238,
                 0x801E8404, 0x801E89CC, 0x801E8A94, 0x8005A140, 0x8005A300}
EXTRA_HELPERS.update({0x801EB8F8,0x801EB8A0})
ROOTS = ['dora3_s3_core_801ECC6C', 'dora3_s3_core_801ECBE4',
         'dora3_s3_core_801ED020', 'dora3_s3_core_801ECFB4',
         'dora3_s3_core_801ED244', 'dora3_s3_core_801ED228',
         'dora3_s3_core_801ED898', 'dora3_s3_core_801ED4A4']

ROOTS+= ['dora3_s3_core_801EC938', 'dora3_s3_core_801EC91C', 'dora3_s3_core_801EC994', 'dora3_s3_core_801EC978', 'dora3_s3_core_801EC9F0', 'dora3_s3_core_801EC9D4', 'dora3_s3_core_801ECAA8', 'dora3_s3_core_801ECA8C', 'dora3_s3_core_801EC8DC', 'dora3_s3_core_801EC8C0', 'dora3_s3_core_801ECA4C', 'dora3_s3_core_801ECA30', 'dora3_s3_core_801ECB6C', 'dora3_s3_core_801ECB1C', 'dora3_s3_core_801ECC6C', 'dora3_s3_core_801ECBE4', 'dora3_s3_core_801ECD3C', 'dora3_s3_core_801ECCEC', 'dora3_s3_core_801ECDD4', 'dora3_s3_core_801ECD8C', 'dora3_s3_core_801ECE6C', 'dora3_s3_core_801ECE24', 'dora3_s3_core_801ECF3C', 'dora3_s3_core_801ECEEC', 'dora3_s3_core_801ED020', 'dora3_s3_core_801ECFB4', 'dora3_s3_core_801ED07C', 'dora3_s3_core_801ED060', 'dora3_s3_core_801ED104', 'dora3_s3_core_801ED0BC', 'dora3_s3_core_801ED1E8', 'dora3_s3_core_801ED17C', 'dora3_s3_core_801ED244', 'dora3_s3_core_801ED228', 'dora3_s3_core_801ED2A0', 'dora3_s3_core_801ED284']

ROOTS+=['dora3_s3_core_801EC8F8', 'dora3_s3_core_801EC954', 'dora3_s3_core_801EC9B0', 'dora3_s3_core_801ECA68', 'dora3_s3_core_801EC89C', 'dora3_s3_core_801ECA0C', 'dora3_s3_core_801ECAC4', 'dora3_s3_core_801ECB88', 'dora3_s3_core_801ECC88', 'dora3_s3_core_801ECD58', 'dora3_s3_core_801ECDF0', 'dora3_s3_core_801ECE88', 'dora3_s3_core_801ECF58', 'dora3_s3_core_801ED03C', 'dora3_s3_core_801ED098', 'dora3_s3_core_801ED120', 'dora3_s3_core_801ED204', 'dora3_s3_core_801ED260', 'dora3_s3_core_801ED2BC', 'dora3_s3_core_801ED2CC', 'dora3_s3_core_801ED2C4']

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
    parser.add_argument('--scope',choices=['original','native-source','effect-delay','effect-motion','effect-pool','pickup-contact','pickup-idle-guards','periodic-source','periodic-producer','periodic-guards','producer-guards','pickup-uncollected-source','pickup-spin-native','pickup-spin-guards','pickup-arc-native','pickup-arc-guards','pickup-reveal-source','pickup-reveal-native','pickup-reveal-guards','pickup-settle-scatter-source','pickup-settle-native','pickup-settle-guards','pickup-scatter-native','pickup-scatter-guards','pickup-scatter-geometry','pickup-parent-source','pickup-parent-native','pickup-parent-guards','pickup-parent-guard-smoke','pickup-moving-idle-source','pickup-item-families-source','pickup-item-families-native','pickup-linked-batch'],default='original')
    parser.add_argument('--optimization',choices=['O0','O1','O2'],default='O0')
    args = parser.parse_args()
    moving_idle_source=args.scope=='pickup-item-families-native'
    linked_batch=args.scope=='pickup-linked-batch'
    if moving_idle_source or linked_batch:args.scope='pickup-settle-scatter-source'
    parent_guard=args.scope in ['pickup-parent-guards','pickup-parent-guard-smoke']
    parent_smoke=args.scope=='pickup-parent-guard-smoke'
    parent_native=args.scope in ['pickup-parent-native','pickup-parent-guards','pickup-parent-guard-smoke']
    parent_source=args.scope in ['pickup-parent-source','pickup-parent-native','pickup-parent-guards','pickup-parent-guard-smoke']
    if parent_source:args.scope='pickup-settle-scatter-source'
    scatter_geometry=args.scope=='pickup-scatter-geometry'
    scatter_guards=args.scope in ['pickup-scatter-guards','pickup-scatter-geometry']
    scatter_native=args.scope in ['pickup-scatter-native','pickup-scatter-guards','pickup-scatter-geometry']
    if scatter_native:args.scope='pickup-settle-guards' if scatter_guards else 'pickup-settle-native'
    generation = args.generated.resolve()
    diagnostic=args.scope in ['native-source','effect-pool','pickup-contact','pickup-idle-guards','periodic-source','periodic-producer','periodic-guards','producer-guards','pickup-uncollected-source','pickup-spin-native','pickup-spin-guards','pickup-arc-native','pickup-arc-guards','pickup-reveal-source','pickup-reveal-native','pickup-reveal-guards','pickup-settle-scatter-source','pickup-settle-native','pickup-settle-guards']
    settle_scatter_source=args.scope in ['pickup-settle-scatter-source','pickup-settle-native','pickup-settle-guards']
    settle_native=args.scope in ['pickup-settle-native','pickup-settle-guards']
    settle_guards=args.scope=='pickup-settle-guards'
    reveal_guards=args.scope=='pickup-reveal-guards'
    reveal_native=args.scope in ['pickup-reveal-native','pickup-reveal-guards']
    reveal_source=args.scope in ['pickup-reveal-source','pickup-reveal-native','pickup-reveal-guards']
    arc_guards=args.scope=='pickup-arc-guards'
    arc_native=args.scope in ['pickup-arc-native','pickup-arc-guards']
    spin_guards=args.scope=='pickup-spin-guards'
    spin_native=args.scope in ['pickup-spin-native','pickup-spin-guards','pickup-arc-native','pickup-arc-guards']
    uncollected_source=args.scope in ['pickup-uncollected-source','pickup-spin-native','pickup-spin-guards','pickup-arc-native','pickup-arc-guards','pickup-reveal-source','pickup-reveal-native','pickup-reveal-guards','pickup-settle-scatter-source','pickup-settle-native','pickup-settle-guards']
    producer_guard=args.scope=='producer-guards'
    producer=args.scope in ['periodic-producer','producer-guards','pickup-uncollected-source','pickup-spin-native','pickup-spin-guards','pickup-arc-native','pickup-arc-guards','pickup-reveal-source','pickup-reveal-native','pickup-reveal-guards','pickup-settle-scatter-source','pickup-settle-native','pickup-settle-guards']
    periodic_guard=args.scope=='periodic-guards'
    periodic=args.scope in ['periodic-source','periodic-producer','periodic-guards','producer-guards','pickup-uncollected-source','pickup-spin-native','pickup-spin-guards','pickup-arc-native','pickup-arc-guards','pickup-reveal-source','pickup-reveal-native','pickup-reveal-guards','pickup-settle-scatter-source','pickup-settle-native','pickup-settle-guards']
    guards=args.scope=='pickup-idle-guards'
    contact=args.scope in ['pickup-contact','pickup-idle-guards']
    pool=diagnostic
    motion=args.scope=='effect-motion'
    delay=args.scope in ['effect-delay','effect-motion']
    folder='native-pickup-settle-guard-check' if settle_guards else 'native-pickup-settle-check' if settle_native else 'native-pickup-settle-scatter-source-check' if settle_scatter_source else 'native-pickup-reveal-guard-check' if reveal_guards else 'native-pickup-reveal-check' if reveal_native else 'native-pickup-reveal-source-check' if reveal_source else 'native-pickup-arc-guard-check' if arc_guards else 'native-pickup-arc-check' if arc_native else 'native-pickup-spin-guard-check' if spin_guards else 'native-pickup-spin-check' if spin_native else 'native-pickup-uncollected-source-check' if uncollected_source else 'native-periodic-producer-guard-check' if producer_guard else 'native-periodic-effect-guard-check' if periodic_guard else 'native-periodic-producer-check' if producer else 'native-periodic-effect-source-check' if periodic else 'native-pickup-idle-guards-check' if guards else 'native-pickup-contact-check' if contact else 'native-flash-pool-check' if pool else 'native-flash-motion-check' if motion else 'native-flash-delay-check' if delay else ('native-story-pickup-native-source-check' if diagnostic else 'native-story-pickup-source-check')
    if scatter_native:folder='native-pickup-scatter-geometry-check' if scatter_geometry else 'native-pickup-scatter-guard-check' if scatter_guards else 'native-pickup-scatter-check'
    if parent_source:folder='native-pickup-parent-guard-smoke' if parent_smoke else 'native-pickup-parent-guard-check' if parent_guard else 'native-pickup-parent-check' if parent_native else 'native-pickup-parent-source-check'
    if moving_idle_source:folder='native-pickup-item-families-check'
    if linked_batch:folder='native-pickup-linked-batch-check'
    report_name=folder+'.json'
    out=ROOT/'build-local'/folder
    out.mkdir(exist_ok=True)
    # Use the same verified real common pool, constructors, resource lists,
    # destructors, model implementation and frozen Oracle as the whole-world check.
    checker = ROOT/'tools/check_native_player_update.py'
    preparation = checker.read_text()
    # Keep the scatter root in the story-specific selection so its independent
    # frozen reference is emitted; the whole-world checker still selects it.
    scatter_root="todo=['dora3_s3_core_801EBAD4','dora3_s3_core_801EBCC0','dora3_s3_801E89CC',"
    assert preparation.count(scatter_root)==1
    preparation=preparation.replace(scatter_root,"todo=['dora3_s3_801E89CC',")
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
    if periodic:todo+=['dora3_s3_core_801ED8EC','dora3_s3_core_801ED8F4','dora3_s3_core_801ED8FC']
    while todo:
        name = todo.pop()
        if name in existing or name in supplied or name in selected:
            continue
        assert name in current and name in frozen, name
        body = current[name]
        pc = int(re.search(r'// 0x([0-9A-F]+):', body)[1], 16)
        assert 0x801EBA00 <= pc <= 0x801ED898 or pc in EXTRA_HELPERS or (periodic and pc in [0x801ED8EC,0x801ED8F4,0x801ED8FC]), name
        if pc==0x801EBA9C:
            for hook in ['if (dora3_native_preview_hook(rdram,ctx,0x801EBAB4u)) goto dora3_after_pickup_idle_turn;', 'dora3_after_pickup_idle_turn:;']:
                assert body.count(hook)==1,(name,hook)
                body=body.replace('    '+hook+'\n','')
            assert body==frozen[name],name
            body=current[name]
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
        elif pc in [0x801ED2CC,0x801ED8FC,0x801EBCC0,0x801EBAD4,0x801EBEDC,0x801EBFC8,0x801EC0D0,0x801EC194,0x801EC2A8]:
            hook=f'; if (dora3_native_preview_hook(rdram,ctx,0x{pc:08X}u)) return;'
            assert body.count(hook)==1 and body.replace(hook,';')==frozen[name],name
        elif pc not in [0x801ED5F4,0x801EC598,0x801EBA9C]:assert body == frozen[name] and 'dora3_native_preview_hook' not in body, name
        selected[name] = body
        todo.extend(re.findall(r'\b(\w+)\(rdram, ctx\)', body))
    assert set(ROOTS).issubset(existing | supplied | set(selected)),set(ROOTS)-existing-supplied-set(selected)
    (out/'functions.c').write_text(base+'\n'+'\n'.join(selected.values()))
    header = out/'player-update-functions.h'
    header.write_text(header.read_text()+'\nextern "C" {\n'+'\n'.join(
        f'void {name}(uint8_t*,recomp_context*);' for name in selected)+'\n}\n')
    if pool:
        assert 'dora3_s3_core_801EBCC0' in selected and 'dora3_s3_core_801EBCC0' not in existing
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
    flags = ['-'+args.optimization , '-g', '-fno-omit-frame-pointer', '-fno-strict-aliasing', *[
        '-I'+str(path) for path in [ROOT/'src', ROOT/'research', generation,
                                  generation/'RecompiledFuncs', out,
                                  ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    if diagnostic:flags+=['-DDORA3_STORY_SOURCE_NATIVE_CHECK','-DDORA3_STORY_EFFECT_POOL_CHECK']
    if contact:flags+=['-DDORA3_STORY_PICKUP_CONTACT_CHECK']
    if guards:flags+=['-DDORA3_STORY_PICKUP_IDLE_GUARD_CHECK']
    if spin_guards:flags+=['-DDORA3_PICKUP_SPIN_GUARD_CHECK']
    if spin_native:flags+=['-DDORA3_PICKUP_SPIN_NATIVE_CHECK']
    if settle_scatter_source and not settle_guards:flags+=['-DDORA3_PICKUP_LINKED_BATCH_CHECK' if linked_batch else '-DDORA3_PICKUP_MOVING_IDLE_SOURCE_CHECK' if moving_idle_source else '-DDORA3_PICKUP_PARENT_SOURCE_CHECK' if parent_source else '-DDORA3_PICKUP_SETTLE_SCATTER_SOURCE_CHECK']
    if settle_guards and not scatter_guards:flags+=['-DDORA3_PICKUP_SETTLE_GUARD_CHECK']
    if scatter_guards:flags+=['-DDORA3_PICKUP_SCATTER_GEOMETRY_CHECK' if scatter_geometry else '-DDORA3_PICKUP_SCATTER_GUARD_CHECK']
    if parent_native:flags+=['-DDORA3_PICKUP_PARENT_NATIVE_CHECK']
    if parent_smoke:flags+=['-DDORA3_PICKUP_PARENT_GUARD_SMOKE']
    if settle_native:flags+=['-DDORA3_PICKUP_SETTLE_NATIVE_CHECK']
    if scatter_native:flags+=['-DDORA3_PICKUP_SCATTER_NATIVE_CHECK']
    if reveal_guards:flags+=['-DDORA3_PICKUP_REVEAL_GUARD_CHECK']
    if reveal_native:flags+=['-DDORA3_PICKUP_REVEAL_NATIVE_CHECK']
    if reveal_source:flags+=['-DDORA3_PICKUP_REVEAL_SOURCE_CHECK']
    if arc_guards:flags+=['-DDORA3_PICKUP_ARC_GUARD_CHECK']
    if arc_native:flags+=['-DDORA3_PICKUP_ARC_NATIVE_CHECK']
    if uncollected_source:flags+=['-DDORA3_PICKUP_UNCOLLECTED_SOURCE_CHECK']
    if producer_guard:flags+=['-DDORA3_PERIODIC_PRODUCER_GUARD_CHECK']
    if periodic:flags+=['-DDORA3_PERIODIC_EFFECT_GUARD_CHECK' if periodic_guard else '-DDORA3_PERIODIC_PRODUCER_CHECK' if producer else '-DDORA3_PERIODIC_EFFECT_SOURCE_CHECK']
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
    assert output.startswith('PASS: 72 Original and72 mixed Native/Source actual Native linked129 animation' if linked_batch else 'PASS: 18 Original and18 mixed Native/Source complete standard111..128 Native idle/collection family lifetimes' if moving_idle_source else ('PASS: 30 Original and30 mixed Native/Source guarded actual Parent130' if parent_smoke else 'PASS: 12960 Original and12960 mixed Native/Source guarded actual Parent130') if parent_guard else 'PASS: 864 Original and864 mixed Native/Source actual Parent130' if parent_source else 'PASS: 432 Original and432 mixed Native/Source scatter3 geometric surface-jump' if scatter_geometry else 'PASS: 4320 Original and4320 mixed Native/Source guarded initialized scatter3' if scatter_guards else 'PASS: 3456 Original and3456 mixed Native/Source guarded no-parent pickup2' if settle_guards else 'PASS: 432 Original and432 mixed Native/Source initialized no-parent pickup2/3' if scatter_native else 'PASS: 432 Original and432 mixed Native/Source initialized no-parent pickup2' if settle_native else 'PASS: 432 Original and432 mixed Native/Source Source-owned pickup2/3' if settle_scatter_source else 'PASS: 3456 Original and3456 mixed Native/Source guarded pickup4' if reveal_guards else 'PASS: 216 Original and216 mixed Native/Source Source-owned pickup4' if reveal_source else 'PASS: 3456 Original and3456 mixed Native/Source guarded pickup8' if arc_guards else 'PASS: 3456 Original and3456 mixed Native/Source guarded pickup7' if spin_guards else 'PASS: 432 Original and432 mixed Native/Source uncollected pickup states7/8' if uncollected_source else 'PASS: 3456 Original and3456 mixed Native/Source guarded pickup5/6' if producer_guard else 'PASS: 1152 Original and1152 mixed Native/Source periodic131' if periodic_guard else 'PASS: 216 Original and216 mixed Native/Source uncollected state5' if producer else 'PASS: 72 Original and72 mixed Native/Source periodic effect131' if periodic else 'PASS: 768 actual Source mode3 contacts' if guards else 'PASS: 384 actual Source mode3 contact' if contact else 'PASS: 168 connected Native flash pool' if pool else 'PASS: 288 Native flash motion' if motion else 'PASS: 144 Native flash delay' if delay else 'PASS: 168 ')
    tracked = sources+[fixture, checker, Path(__file__).resolve(),
                       ROOT/'research/native-npc-interaction-check.inc', ROOT/'recomp/startup.json']
    tracked+=sorted((ROOT/'src').glob('*.hpp'))
    report = dict(result=output, compile_optimization=args.optimization, extra_original_functions=sorted(selected),
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
        report['limitations']=['Actual Source producer, ordered Source delay states, Native flash state10 common pool, descriptor constructors, full model-advance tail and retirement on synthetic resources', 'Frame-bank ownership and loop activation are fixture flags; no real NativeRuntime/FrameSession/presentation/renderer in this scoped proof', 'Only stationary reviewed state1 and collected state10 have Native ownership; other item states, real-level collision and full pool exhaustion remain outside this proof']
    if contact:
        report['scope']='Stationary Native state1 and genuine Source mode3 contact, Source initialization then connected Native state10 item/effect lifetime; paired frozen Original'
        report['validated_schedules']['actual_mode3_contacts']=384
        report['validated_schedules']['connected_schedules']=672
        report['validated_schedules']['original_contact_periods']=[2,4,11,20]
        report['validated_schedules']['original_idle_periods_before_contact']=[1,3,10,19]
        report['validated_schedules']['new_native_ownership']={'118':[1,10],'123':[1,10],'127':[1,10],'130':[10]}
        report['validated_schedules']['idle_motion']='Only stationary state1 after actual first Source pass; measured fractional yaw changes and exact Original whole-pool endpoints'

        report['limitations']+=['Fixture external state1 command after genuine quest producer; no real-level item placement/contact selector or collection trigger claim']
    if guards:
        report['scope']='Actual common-pool pause/reentry/rejection/write/clip commands followed by genuine Source collection and complete six-effect lifetime, paired frozen Original'
        report['validated_schedules'].update(actual_mode3_contacts=768,connected_schedules=1344,
            original_contact_periods=[20],guard_profiles=8,
            original_idle_periods_before_contact='Varies with actual paused/Source-only periods')
        report['validated_schedules'].pop('successful_item_lifetimes',None)
        report['validated_schedules']['guard_commands']=['C9 pause5..20/resume21 and stale idle record',
            'moving state1 velocityX11 periods5..9/zero return10', 'state9 periods5..9/state1 return10',
            'update callback noop replacement5..9', 'contact callback noop replacement5..9',
            'external yaw175/359/-2 at5/7/9', 'external opacityC4 .25/C8=2 then .875/C8=1 at5/8',
            'actual model seek time0 at5 with run-token reset']
    if periodic:
        report['scope']='Actual Source kind131 descriptor and complete common-pool lifetime during Native player/world intervals; paired frozen Original'
        report['source_sha256']['research/native-periodic-effect-source-check.inc']=sha(ROOT/'research/native-periodic-effect-source-check.inc')
        report['validated_schedules']=dict(settings=24,source_schedules=72,mixed_schedules=72,heights=[80,250,400],
            original_rates=[20,30],configured_native_rates=[60,73,120,144,165,360],new_native_ownership=False)
        report['limitations']=['Actual descriptor801FC964, Original initialization/RNG/clip/seek/scale/velocity/pre-step threshold and complete genuine actor/model/heap retirement on synthetic assets',
            'Native player/world activity uses diagnostic frame-bank flags, not real session/presentation/renderer',
            'Isolated descriptor lifetime; actual state5 periodic producer, pool exhaustion and real-level placement remain outside this proof']
    if periodic:
        report['validated_schedules']['new_native_ownership']={'131':[1]}
        report['limitations'][0]='Actual descriptor801FC964 and ordered Source initialization/RNG/clip/seek/scale/pre-step destructor; Native pure initialized Y drift and model time, paired genuine resource lifetimes on synthetic assets'
    if producer:
        report['scope']='Actual item118/123/127 state5 periodic effect131 producer, Source state6 signal and Native idle followed by real Source contact/complete six flash130 lifetime, paired frozen Original'
        report['source_sha256']['research/native-periodic-producer-check.inc']=sha(ROOT/'research/native-periodic-producer-check.inc')
        report['validated_schedules']=dict(settings=24,source_schedules=216,mixed_schedules=216,heights=[80,250,400],item_kinds=[118,123,127],
            external_state6_period=40,actual_mode3_contact_period=100,periods=192,
            original_rates=[20,30],configured_native_rates=[60,73,120,144,165,360],new_native_ownership={'131':[1],'118':[5,6],'123':[5,6],'127':[5,6]},state5_state6_native_ownership=True)
        report['limitations']=['Actual descriptor constructors; Native pickup5/6 geometry/scale, initialized131 drift/model time and collected items/six130 flash effects; real Source5 counters/periodic producer/height stop, Source6 threshold/signal/sound, ordered131 retirement and full genuine model/heap/resource/command order on synthetic assets',
            'External state5/scale3.5/state6 and direct mode3 contact are fixture inputs; no real-level placement/contact selector or state selecting script claim',
            'Diagnostic loop/bank flags, no real session/presentation/renderer; full pool exhaustion outside this proof']
    if periodic_guard:
        report['scope']='Actual periodic131 external commands before/after full Native interval, whole Source endpoint and lifetime oracle'
        report['source_sha256']['research/native-periodic-effect-guard-check.inc']=sha(ROOT/'research/native-periodic-effect-guard-check.inc')
        report['validated_schedules'].update(source_schedules=1152,mixed_schedules=1152,heights=[250,400,800],guard_profiles=8,command_placements=['before Native period','after Native period before Source'],periods=192)
        report['limitations']=['Actual common pool, descriptor, model seek/flag commands, full actual retirement and resource/command order on synthetic assets', 'External fixture commands; no real-level script or renderer proof; commands within a Native interval and pool exhaustion outside this matrix']
    if producer_guard:
        report['scope']='Actual pickup118/123/127 Native5/6 motion/scale with eight external commands before/after Native intervals; whole Source pool/lifetime oracle'
        report['source_sha256']['research/native-periodic-producer-guard-check.inc']=sha(ROOT/'research/native-periodic-producer-guard-check.inc')
        report['validated_schedules'].update(source_schedules=3456,mixed_schedules=3456,guard_profiles=8,command_placements=['before Native period','after Native period before Source'])
        report['limitations']=['Actual constructors, periodic131 producer, real Source state6 signal, Native idle and real collection/six130 complete retirement on synthetic assets', 'Actor pause/moving/callback/Y/scale/blink/seek/opacity/model pause external fixture commands; no real script selector or renderer proof', 'Commands inside a Native interval and actual full-pool exhaustion remain outside this matrix']
    if uncollected_source:
        report['scope']='Actual uncollected pickup state7/8 complete Source lifetimes during existing Native player/world/idle/collection intervals, paired frozen Original'
        report['source_sha256']['research/native-pickup-uncollected-source-check.inc']=sha(ROOT/'research/native-pickup-uncollected-source-check.inc')
        report['validated_schedules']=dict(settings=24,source_schedules=432,mixed_schedules=432,initial_states=[7,8],state8_targets=[45,120,270],item_kinds=[118,123,127],periods=256,actual_mode3_contact_period=140,original_rates=[20,30],configured_native_rates=[60,73,120,144,165,360],new_native_ownership=False)
        report['limitations']=['Actual descriptor, common pool, state7/8 initializers and whole live actor/physics/model/heap/resource/command endpoints, real destructors on synthetic assets', 'State7/8 and arc BE/C0/44 values are external fixture commands; no real-level selecting script or renderer proof', 'This is a Source-only baseline for future state7/8 Native ownership; current idle/collected/130 ownership is preserved']
    if spin_native:
        report['scope']='Native initialized state7 yaw/speed/fade, Source private initialization/blink reset/destruction; paired actual Original lifetimes, state8 remains Source-only'
        report['validated_schedules']['new_native_ownership']={'118':[7],'123':[7],'127':[7]}
        report['limitations']=['Actual constructors/private callbacks/common pool and complete live actor/physics/model/heap/resource/command Original endpoints on synthetic assets', 'Only stationary initialized reviewed state7 with finite speed0..36/alpha0..1 and D8 sentinel0/1; other configurations remain Source-owned', 'External state/arc parameters, no real-level selecting script, renderer or full-pool exhaustion proof']
    if settle_scatter_source:
        report['scope']='Actual Source-only no-parent settle2 and scatter3 during Native world intervals; frozen Original complete lifetime baseline'
        report['source_sha256']['research/native-pickup-settle-scatter-source-check.inc']=sha(ROOT/'research/native-pickup-settle-scatter-source-check.inc')
        report['validated_schedules'].update(source_schedules=432,mixed_schedules=432,initial_states=[2,3],heights=[80,250,400],parent_index=-1,new_native_ownership=False)
        report['validated_schedules'].pop('state8_targets',None)
        report['limitations']=['Actual constructors, Source state2 gravity/probe and state3 RNG/collision helpers, common pool and whole live actor/physics/model/heap/resource/commands through genuine collection/retirement on synthetic room/assets', 'Explicit state/height/no-parent inputs; parent-follow branch and Native2/3 remain outside this baseline', 'No real-level selecting script/collision, renderer or full-pool exhaustion proof']
    if settle_native:
        report['scope']='Native initialized no-parent settle2 Y/velocity, actual Original Source probe/latch/state1 and complete retirement; scatter3 remains Source-owned'
        report['validated_schedules']['new_native_ownership']={'118':[2],'123':[2],'127':[2]}
        report['limitations']=['Actual constructors, gravity/probe, common pool and432 paired complete live actor/physics/model/heap/resource/command lifetimes on synthetic room/assets', 'Only initialized no-parent descending state2,gravity2,stationaryXZ,D8 sentinel0/1; parent-follow,scatter3 and incompatible configurations remain Source-owned', 'Explicit state/height/parent inputs; no real-level selector, dynamic platform integration, renderer or full-pool exhaustion proof']
    if settle_guards:
        report['source_sha256']['research/native-pickup-settle-guard-check.inc']=sha(ROOT/'research/native-pickup-settle-guard-check.inc')
        report['validated_schedules'].update(source_schedules=3456,mixed_schedules=3456,initial_states=[2],command_profiles=8,command_placements=['beforeNative','afterNative'])
        report['scope']='Initialized no-parent Native2 with eight before/after command guards and884736 actual Original full lifetime endpoints; parent-follow itself stays Source'
    if reveal_source:
        report['scope']='Actual Source-owned state4 reveal/angle90 state10/complete six-flash retirement during existing Native world intervals; baseline for future Native4'
        report['source_sha256']['research/native-pickup-reveal-source-check.inc']=sha(ROOT/'research/native-pickup-reveal-source-check.inc')
        report['validated_schedules'].update(source_schedules=216,mixed_schedules=216,initial_states=[4],heights=[80,250,400],new_native_ownership=False)
        report['validated_schedules'].pop('state8_targets',None);report['validated_schedules'].pop('actual_mode3_contact_period',None)
        report['limitations']=['Actual descriptor/private callbacks/common pool/Original trig/whole live actor/physics/model/heap/resource/commands and complete genuine resources on synthetic assets', 'Explicit state4/heights are fixture inputs; Native4 remains unclaimed, no real-level selector, renderer or full-pool exhaustion proof']
    if reveal_native:
        report['scope']='Native initialized state4 uniform reveal scale via actual Original trig; ordered Source integer half-angle/latch/angle90 collection transition, complete collected/six-flash retirement'
        report['validated_schedules']['new_native_ownership']={'118':[4],'123':[4],'127':[4]}
        report['limitations']=['Actual descriptor/common pool/Original trig and degreeA8 constants; complete live actor/physics/model/heap/resource/command endpoints and genuine resources on synthetic assets', 'Native4 only initialized stationary reviewed callbacks, half34 angle0..89 andD8 sentinel0/1; incompatible commands retain Source', 'External state4/heights inputs; no real-level script selector, renderer or full-pool exhaustion proof']
    if reveal_guards:
        report['scope']='Native initialized4 reveal scale with8 external-command guards before/afterNative; Source half-angle/latch/state10 and full genuine Original lifetimes'
        report['source_sha256']['research/native-pickup-reveal-guard-check.inc']=sha(ROOT/'research/native-pickup-reveal-guard-check.inc')
        report['validated_schedules'].update(source_schedules=3456,mixed_schedules=3456,guard_profiles=8,command_placements=['before Native period','after Native period before Source'])
    if arc_native:
        report['scope']='Native initialized state7 spin/fade and state8 two-phase XYZ/scale geometry; ordered Source angle/phase/latch/return and actual mode3 collection/full lifetimes'
        report['validated_schedules']['new_native_ownership']={'118':[7,8],'123':[7,8],'127':[7,8]}
        report['limitations']=['Actual descriptor/common pool/Original trig helpers/degree constants and whole live actor/physics/model/heap/resource/command endpoints; Source integer angle, phase switch, latch/state return and full retirement on synthetic assets', 'Native8 only initialized stationary phase0/1, phase0 angle0..89/phase1 angle0..target-1,target1..359,D8 sentinel0/1 with actual reviewed update/contact identity; other configurations remain Source-owned', 'External state and arc BE/C0/44 parameters; no real script selector, renderer or pool exhaustion proof']
    if arc_guards:
        report['scope']='Native initialized pickup8 two-phase geometry with8 external-command guards before/after Native intervals; Source integer angle/phase/latch/state, contact and full Original lifetimes'
        report['source_sha256']['research/native-pickup-arc-guard-check.inc']=sha(ROOT/'research/native-pickup-arc-guard-check.inc')
        report['validated_schedules'].update(rejected_phase_angle_raw_fields=True,source_schedules=3456,mixed_schedules=3456,initial_states=[8],guard_profiles=8,command_placements=['before Native period','after Native period before Source'])
    if spin_guards:
        report['scope']='Native pickup7 yaw/speed/fade with8 external-command guards before/after Native intervals, real Source init/blink reset/destructor and whole Original pool oracle'
        report['source_sha256']['research/native-pickup-spin-guard-check.inc']=sha(ROOT/'research/native-pickup-spin-guard-check.inc')
        report['validated_schedules'].update(source_schedules=3456,mixed_schedules=3456,initial_states=[7],heights=[80,250,400],guard_profiles=8,command_placements=['before Native period','after Native period before Source'])
        report['validated_schedules'].pop('state8_targets',None);report['validated_schedules'].pop('actual_mode3_contact_period',None)
    if parent_source:
        report['source_sha256']['research/native-pickup-parent-source-check.inc']=sha(ROOT/'research/native-pickup-parent-source-check.inc')
        pairs=30 if parent_smoke else 12960 if parent_guard else 864
        report['scope']='Native initialized attached2 child118/123/127 from real current Source-born flash130 parent pose; guarded complete frozen Original lifetimes'
        report['parent_source_baseline']=dict(paired_worlds=pairs,whole_endpoints=pairs*256,actual_parent_ctor=True,source_only_attached2=not parent_native,new_native_parent_ownership=parent_native,explicit_parent_kind=130)
        report['validated_schedules']=dict(settings=1 if parent_smoke else 24,source_schedules=pairs,mixed_schedules=pairs,initial_states=[2],heights=[80] if parent_smoke else [80,250,400],item_kinds=[118] if parent_smoke else [118,123,127],parent_index=30,parent_profiles=[0] if parent_smoke else [0,5],command_profiles=10,command_placements=['before Native','after Native before Source','mid Native interval'],periods=256,actual_mode3_contact_period=140,original_rates=[20] if parent_smoke else [20,30],configured_native_rates=[60] if parent_smoke else [60,73,120,144,165,360],new_native_ownership={'118':[2],'123':[2],'127':[2]})
        report['limitations']=['Real Parent130/child constructors, actual Original matrix closure and whole live pool/physics/model/heap/resource/command/RNG/candidate endpoints on synthetic room/assets','Child C9/VY/update callback/absolute pose/blink/real seek and model pause/detach-rebind, parent C9/update callback/real seek and model pause external inputs before/after Native; duplicate-dt requires no pose change','Only earlier live Source-born flash130 parent; player/other parent controllers, real-level scripts, renderer and pool exhaustion outside proof']
    if moving_idle_source:
        report['scope']='Actual standard111..128 generic descriptor/contact/complete-lifetime Native idle/collection'
        report['source_sha256']['research/native-pickup-moving-idle-source-check.inc']=sha(ROOT/'research/native-pickup-moving-idle-source-check.inc')
        report['validated_schedules']=dict(settings=1,paired_worlds=18,whole_endpoints=4608,periods=256,item_kinds=list(range(111,129)),heights=[250],initial_state=1,velocity_profiles=[[2,0,-3]],parent_index=-1,actual_contact_period=140,new_moving_idle_native_ownership=True)
        report['limitations']=['Actual descriptor/private idle yaw/common movement/model/opacity and full live pool/physics/models/heap/resources/commands on synthetic room/assets','Explicit constant velocities and direct actual Source mode3 contact inputs; no live selecting script/renderer/pool exhaustion proof','Native111..128 verified exact callback pairs; actual Source contact/quest/retirement order retained. Other states/full fps/guards/private script producer creation guards/real-level selectors not yet tested']
    if scatter_geometry:
        report['source_sha256']['research/native-pickup-scatter-geometry-check.inc']=sha(ROOT/'research/native-pickup-scatter-geometry-check.inc')
        report['native_scatter3_geometry']=dict(paired_worlds=432,whole_endpoints=110592,real_original_query_and_reflection=True,source_reaction_required_per_world=True,mesh='Synthetic floor elevations0/300 atX500;8 real floor/ceiling triangles; no query outcome injection')
    if scatter_guards and not scatter_geometry:
        report['source_sha256']['research/native-pickup-scatter-guard-check.inc']=sha(ROOT/'research/native-pickup-scatter-guard-check.inc')
        report['native_scatter3_guards']=dict(paired_worlds=4320,whole_endpoints=1105920,profiles=10,each_world_contact_and_boundary_reactions_required=True,geometric_wall_proof=False)
    if scatter_native and not scatter_guards:
        assert 'PASS: Native initialized standard scatter3' in output
        report['native_scatter3_candidate']=dict(new_native3_ownership=True,standard_even_source_speed=True,source_heading_count_rng_transition=True,wall_rebound_guard_proof_pending=True)
    if linked_batch:
        assert 'PASS: 117 Original and117 mixed Native/Source guarded linked129 animation lifetimes' in output
        assert output.endswith('PASS: linked Native animation batch;189 paired complete lifetimes/48384 exact Original endpoints; private commands and resources retain Source order; no game/GPU.')
        report['scope']='Connected Native129 animation batch: eight actual private producers and13 external command profiles; all complete item/linked/collection/retirement lifetimes'
        for name in ['native-pickup-linked-basic-check.inc','native-pickup-linked-guard-check.inc']:
            report['source_sha256']['research/'+name]=sha(ROOT/'research'/name)
        report['validated_schedules']=dict(settings=1,paired_worlds=189,whole_endpoints=48384,periods=256,basic_worlds=72,guard_worlds=117,item_kinds=[114,118,119,120,121,122,126,128],heights=[80,250,400],velocity_profiles=[[2,0,-3],[0,1,0],[-2,-1,1]],actual_parent_contact_period=140,actual_private_producers=True,command_profiles=13,command_placements=['before Native','after Native before Source','mid Native'],linked_effect_kind=129,linked_count_per_parent=3,new_native129_ownership=True)
        report['limitations']=['Actual Source constructors/RNG/private129 callbacks/common pool/automatic unlink and complete resources on synthetic assets','Native129 common animation only; real Source commands, initial state1/velocities/direct contact inputs. Broader settings and final whole-game integration remain pending; no live selecting script/renderer proof']
    (ROOT/'research'/report_name).write_text(json.dumps(report,indent=2)+'\n')

if __name__ == '__main__':
    main()
