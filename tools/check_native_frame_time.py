"""Native time component + two original resident routines on synthetic RAM. No game/GPU."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generated', type=Path, required=True)
    args = parser.parse_args()
    generated = args.generated.resolve()
    receipt = json.loads((generated / 'generation-receipt.json').read_text())
    for base, group in [(ROOT, 'inputs'), (generated, 'generated')]:
        for name, expected in receipt[group].items():
            if sha(base / name) != expected:
                raise ValueError('Stale generation: ' + name)
    out = ROOT / 'build-local/native-frame-time-check'
    out.mkdir(exist_ok=True)
    names = ['dora3_fade_tick', 'dora3_s0_8003948C']
    bodies = []
    for name in names:
        pattern = r'RECOMP_FUNC void ' + name + r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
        matches = [match for file in (generated / 'RecompiledFuncs').glob('*.c')
                   for match in re.findall(pattern, file.read_text(), re.S)]
        if len(matches) != 1:
            raise ValueError('Expected exactly one original body: ' + name)
        bodies.extend(matches)
    (out / 'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n' + '\n'.join(bodies))
    (out / 'native-time-functions.h').write_text('extern "C" {\n' + '\n'.join(
        f'void {name}(uint8_t*, recomp_context*);' for name in names) + '\n}\n')
    runtime = ROOT / 'build-deps/lib/N64ModernRuntime'
    flags = ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
             '-fno-strict-aliasing', *['-I' + str(p) for p in [ROOT / 'src', generated,
                 generated / 'RecompiledFuncs', out, runtime / 'N64Recomp/include']]]
    # Original generated MIPS shifts intentionally use signed arithmetic; UBSan on new code only.
    original_flags = [f for f in flags if f != '-fsanitize=address,undefined'] + ['-fsanitize=address']
    subprocess.run(['gcc', '-std=c17', *original_flags, '-c', str(out / 'functions.c'),
                    '-o', str(out / 'functions.o')], check=True)
    sources = [ROOT / 'src/native_frame_time.cpp', ROOT / 'research/native-frame-time-check.cpp']
    subprocess.run(['g++', '-std=c++20', '-Wall', '-Wextra', '-Werror', *flags,
                    *map(str, sources), str(out / 'functions.o'), '-o', str(out / 'check')], check=True)
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
    result = subprocess.check_output([str(out / 'check')], text=True, env=env).strip()
    tracked = sources + [ROOT / 'src/native_frame_time.hpp', Path(__file__).resolve()]
    report = {
        'scope': 'Isolated native timing/fade implementation; original ROM routine oracle on synthetic RAM; no live integration or GPU',
        'result': result,
        'sanitizers': 'ASan/LSan on component and original functions; UBSan on new C++ component/harness',
        'source_sha256': {str(p.relative_to(ROOT)): sha(p) for p in tracked},
        'generation_receipt_sha256': sha(generated / 'generation-receipt.json'),
        'original_functions': names,
        'remaining': ['Guest event delivery and graphics bank scheduling not connected',
                      'Physics, camera, scripts and per-scene callbacks not converted',
                      'Existing Manual remains RT64 interpolation in test-024; no new test EXE'],
    }
    (ROOT / 'research/native-frame-time-check.json').write_text(json.dumps(report, indent=2) + '\n')
    print(result)


if __name__ == '__main__':
    main()
