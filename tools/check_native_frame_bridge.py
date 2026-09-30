"""Run the hooked original game loop and pinned runtime queue on synthetic RAM."""
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
    out = ROOT / 'build-local' / ('native-frame-bridge-' + args.sanitizer)
    out.mkdir(exist_ok=True)
    subprocess.run(['python3', str(ROOT/'tools/prepare_runtime_timing.py'), str(out)], check=True)
    name = 'dora3_game_message_loop'
    pattern = r'RECOMP_FUNC void ' + name + r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
    bodies = [match for f in (g/'RecompiledFuncs').glob('*.c') for match in re.findall(pattern, f.read_text(), re.S)]
    if len(bodies) != 1: raise ValueError('Missing/duplicate original loop')
    if bodies[0].count('if (dora3_native_game_message(rdram, ctx)) goto L_800278E0;') != 1:
        raise ValueError('Native receive hook missing/duplicated')
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n' + bodies[0])
    runtime = ROOT/'build-deps/lib/N64ModernRuntime'
    flags = ['-O1', '-g', '-pthread', '-fno-omit-frame-pointer', '-fno-strict-aliasing',
             '-fsanitize='+args.sanitizer, *['-I'+str(x) for x in
             [ROOT/'src', g, g/'RecompiledFuncs', runtime/'N64Recomp/include',
              runtime/'ultramodern/include', runtime/'thirdparty', runtime/'thirdparty/concurrentqueue']]]
    subprocess.run(['gcc', '-std=c17', *flags, '-c', str(out/'functions.c'), '-o', str(out/'functions.o')], check=True)
    # This is the actual adapted runtime TU, not a rewritten mock queue. Its
    # thread scheduler endpoints alone are replaced by the single-thread harness.
    subprocess.run(['g++', '-std=c++20', *flags, '-c', str(out/'mesgqueue.cpp'), '-o', str(out/'queue.o')], check=True)
    sources = [ROOT/'research/native-loop-stubs.cpp', ROOT/'src/native_frame_time.cpp', ROOT/'src/native_frame_scheduler.cpp',
               ROOT/'src/native_frame_bridge.cpp', ROOT/'src/native_present.cpp', ROOT/'src/native_present_hooks.cpp', ROOT/'research/native-frame-bridge-check.cpp']
    subprocess.run(['g++', '-std=c++20', '-Wall', '-Wextra', '-Werror', *flags,
                    *map(str, sources), str(out/'functions.o'), str(out/'queue.o'), '-o', str(out/'check')], check=True)
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', TSAN_OPTIONS='halt_on_error=1')
    result = subprocess.check_output([str(out/'check')], text=True, env=env).strip()
    tracked = sources + [ROOT/'src/recomp_context_copy.hpp', ROOT/'src/native_frame_bridge.hpp', ROOT/'src/native_frame_scheduler.hpp',
                         ROOT/'tools/prepare_runtime_timing.py', Path(__file__).resolve()]
    report = {'scope': 'Hooked original game loop and actual adapted external/guest queues; synthetic RAM; scene callbacks and thread scheduler stubbed; no GPU or active native scene',
              'result': result, 'sanitizer': args.sanitizer,
              'source_sha256': {str(f.relative_to(ROOT)): sha(f) for f in tracked},
              'generation_receipt_sha256': sha(g/'generation-receipt.json'),
              'adapted_queue_sha256': sha(out/'mesgqueue.cpp'),
              'limitations': ['Thread scheduler endpoints and scene callbacks are stubs; no renderer',
                              'TSan cannot instrument atomic_thread_fence in the pinned concurrentqueue library',
                              'Requeue OOM is fail-fast; actual host allocation failure not injected']}
    (ROOT/'research'/('native-frame-bridge-'+args.sanitizer+'-check.json')).write_text(json.dumps(report, indent=2)+'\n')
    print(result)

if __name__ == '__main__': main()
