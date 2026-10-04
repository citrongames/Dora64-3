"""Verify native frame protocol plus original scheduler functions on synthetic RAM."""
from decomp_annotations import read_decompiled_source
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--generated', required=True, type=Path)
    p.add_argument('--sanitizer', choices=['address', 'thread'], default='address')
    args = p.parse_args()
    g = args.generated.resolve()
    receipt = json.loads((g / 'generation-receipt.json').read_text())
    for base, group in [(ROOT, 'inputs'), (g, 'generated')]:
        for name, expected in receipt[group].items():
            if sha(base / name) != expected: raise ValueError('Stale ' + name)
    out = ROOT / 'build-local' / ('native-frame-scheduler-' + args.sanitizer)
    out.mkdir(exist_ok=True)
    names = ['dora3_scheduler_framebuffer_ready', 'dora3_scheduler_task_complete',
             'dora3_fade_draw_dispatch', 'dora3_rgba_rectangle']
    bodies = []
    for name in names:
        pattern = r'RECOMP_FUNC void ' + name + r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
        matches = [match for f in (g / 'RecompiledFuncs').glob('*.c') for match in re.findall(pattern, read_decompiled_source(f), re.S)]
        if len(matches) != 1: raise ValueError('Missing/duplicate original ' + name)
        bodies.extend(matches)
    (out / 'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n' + '\n'.join(bodies))
    (out / 'native-scheduler-functions.h').write_text('extern "C" {\n' + '\n'.join(
        f'void {n}(uint8_t*,recomp_context*);' for n in names) + '\n}\n')
    runtime = ROOT / 'build-deps/lib/N64ModernRuntime'
    flags = ['-O1', '-g', '-pthread', '-fno-omit-frame-pointer', '-fno-strict-aliasing',
             '-fsanitize=' + args.sanitizer, *['-I'+str(x) for x in
             [ROOT/'src', g, g/'RecompiledFuncs', out, runtime/'N64Recomp/include']]]
    subprocess.run(['gcc', '-std=c17', *flags, '-c', str(out/'functions.c'), '-o', str(out/'functions.o')], check=True)
    sources = [ROOT/'research/native-loop-stubs.cpp', ROOT/'src/native_frame_time.cpp', ROOT/'src/native_frame_scheduler.cpp', ROOT/'research/native-frame-scheduler-check.cpp']
    sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
    subprocess.run(['g++', '-std=c++20', '-Wall', '-Wextra', '-Werror', *flags,
                    *map(str, sources), str(out/'functions.o'), '-o', str(out/'check')], check=True)
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', TSAN_OPTIONS='halt_on_error=1')
    result = subprocess.check_output([str(out/'check')], text=True, env=env).strip()
    tracked = sources + [ROOT/'src/native_frame_scheduler.hpp', ROOT/'src/native_frame_time.hpp', Path(__file__).resolve()]
    report = {'scope': 'Frame scheduling protocol only; original scheduler on synthetic RAM; SDK endpoints are stubs; no live event adapter',
              'result': result, 'sanitizer': args.sanitizer,
              'source_sha256': {str(f.relative_to(ROOT)): sha(f) for f in tracked},
              'generation_receipt_sha256': sha(g/'generation-receipt.json'),
              'original_functions': names}
    (ROOT/'research'/('native-frame-scheduler-'+args.sanitizer+'-check.json')).write_text(json.dumps(report, indent=2)+'\n')
    print(result)

if __name__ == '__main__': main()
