"""Exercise real clip/bone/morph/lifetime routines on synthetic RAM, without game/GPU."""
from decomp_annotations import read_decompiled_source
import argparse, hashlib, json, os, re, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',type=Path,required=True)
    g=p.parse_args().generated.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,key in [(ROOT,'inputs'),(g,'generated')]:
        for name,value in receipt[key].items():
            if sha(base/name)!=value:raise ValueError('Stale generation: '+name)
    bodies={re.search(r'void (\w+)\(',b)[1]:b for f in (g/'RecompiledFuncs').glob('*.c')
            for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
    by_address={int(re.search(r'// 0x([0-9A-F]+):',b)[1],16):n for n,b in bodies.items() if re.search(r'// 0x([0-9A-F]+):',b) and 'dora3_trace(rdram, ctx,' in b and ', 0x1000);' in b}
    roots=[0x8003B560,0x8003B6BC,0x8003B77C,0x800393C4,0x8003948C,0x80039544,0x80039564,0x80039578,0x800395BC,0x80038BC8,0x8003997C,0x8003B7D0,0x801C4650]
    stop={by_address[a] for a in [0x80033B68,0x80033C50,0x80038B10]}
    stop.update({'dora3_s3_core_801C120C','dora3_s3_core_801C1664'})
    by_address[0x801C4650]='dora3_s3_core_801C4650'
    todo=[by_address[a] for a in roots];selected={};external=set()
    while todo:
        name=todo.pop()
        if name in selected:continue
        if name not in bodies or name in stop:external.add(name);continue
        selected[name]=bodies[name];todo+=re.findall(r'\b(\w+)\(rdram, ctx\)',bodies[name])
    out=ROOT/'build-local/native-model-animation-check';out.mkdir(exist_ok=True)
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(selected.values()))
    (out/'model-functions.h').write_text('extern "C" {\n'+'\n'.join(f'void {n}(uint8_t*,recomp_context*);' for n in selected)+'\n}\n')
    rt=ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include'
    includes=['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',out,rt]]
    flags=['-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*includes]
    subprocess.run(['gcc','-std=c17',*flags,'-fsanitize=address','-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
    sources=[ROOT/'src/native_model_animation.cpp',ROOT/'src/native_simulation_time.cpp',ROOT/'src/native_frame_time.cpp',ROOT/'research/native-model-animation-check.cpp']
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,'-fsanitize=address,undefined',*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
    result=subprocess.check_output([str(out/'check')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
    tracked=sources+[ROOT/'src/recomp_context_copy.hpp',ROOT/'src/native_model_animation.hpp',ROOT/'src/native_simulation_time.hpp',ROOT/'src/native_frame_time.hpp',Path(__file__).resolve()]
    report={'scope':'Actual original constructors, bind/seek/pause/clamp, linear/Hermite pose, morph and destruction; synthetic assets and allocator; no renderer or scene activation',
        'result':result,'sanitizers':'ASan/LSan original and new code; UBSan new C++ only','original_functions':sorted(selected),'stub_boundaries':sorted(external),
        'source_sha256':{str(f.relative_to(ROOT)):sha(f) for f in tracked},'generation_receipt_sha256':sha(g/'generation-receipt.json'),
        'remaining':['Unreviewed phase1 commands, matrix consumers and full scene coverage still need integration audit',
                     'This component does not convert movement, camera, collision, AI, scripts or all 2D systems']}
    (ROOT/'research/native-model-animation-check.json').write_text(json.dumps(report,indent=2)+'\n');print(result)
if __name__=='__main__':main()
