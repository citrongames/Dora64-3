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
    parser.add_argument('--scope',choices=['original','native-source'],default='original')
    args = parser.parse_args()
    generation = args.generated.resolve()
    diagnostic=args.scope=='native-source'
    folder='native-story-pickup-native-source-check' if diagnostic else 'native-story-pickup-source-check'
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
        assert body == frozen[name] and 'dora3_native_preview_hook' not in body, name
        selected[name] = body
        todo.extend(re.findall(r'\b(\w+)\(rdram, ctx\)', body))
    assert len(selected) == 38
    (out/'functions.c').write_text(base+'\n'+'\n'.join(selected.values()))
    header = out/'player-update-functions.h'
    header.write_text(header.read_text()+'\nextern "C" {\n'+'\n'.join(
        f'void {name}(uint8_t*,recomp_context*);' for name in selected)+'\n}\n')
    flags = ['-O0', '-g', '-fno-omit-frame-pointer', '-fno-strict-aliasing', *[
        '-I'+str(path) for path in [ROOT/'src', ROOT/'research', generation,
                                  generation/'RecompiledFuncs', out,
                                  ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    if diagnostic:flags+=['-DDORA3_STORY_SOURCE_NATIVE_CHECK']
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
    assert output.startswith('PASS: 168 ')
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
    if diagnostic:
        report['scope']='Source-owned items and six effects during actual Native gameplay time; no Native geometry adoption'
        report['validated_schedules']['real_native_intervals']=int(re.search(r'(\d+) real Native intervals',output)[1])
        report['validated_schedules']['source_item_constructors_under_native_banks']=96
        report['limitations'][-1]='Real native_gameplay_preview/input/simulation scopes interleave with Source updates; game-loop activation and bank ownership are diagnostic flags. No scheduler/session/presentation/renderer execution; no new Native item ownership'
    (ROOT/'research'/report_name).write_text(json.dumps(report,indent=2)+'\n')

if __name__ == '__main__':
    main()
