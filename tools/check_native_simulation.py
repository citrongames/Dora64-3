"""Check original motion bodies, timing patches and shared simulation timeline without GPU."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',type=Path,required=True);p.add_argument('--reference',type=Path,required=True);a=p.parse_args()
    g=a.generated.resolve();ref=a.reference.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,group in [(ROOT,'inputs'),(g,'generated')]:
        for n,v in receipt[group].items():assert sha(base/n)==v,('Stale generated input',n)
    previous=json.loads((ref/'generation-receipt.json').read_text())
    assert previous['rom_sha256']==receipt['rom_sha256'] and not previous['frontier']
    for n,v in previous['generated'].items():assert sha(ref/n)==v,('Reference body changed',n)
    names={'dora3_s3_core_801CB6D4','dora3_s3_core_801CB400','dora3_s3_core_801C1664','dora3_s3_801C0F1C',
           'dora3_s28_mini_801C4300','dora3_s28_mini_801C42C8','dora3_s28_mini_801C7994'}
    def extract(directory):
        bodies={re.search(r'void (\w+)\(',b)[1]:b for p in (directory/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(p),re.S)}
        return {n:bodies[n] for n in sorted(names)}
    patched=extract(g);original=extract(ref)
    assert all('dora3_native_rate_' not in b for b in original.values())
    assert all('dora3_native_' in b for b in patched.values())
    out=ROOT/'build-local/native-simulation-check';out.mkdir(exist_ok=True)
    text='\n'.join(patched.values())+'\n'+'\n'.join(b.replace('void '+n+'(', 'void reference_'+n+'(') for n,b in original.items())
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+text)
    runtime=ROOT/'build-deps/lib/N64ModernRuntime'
    flags=['-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',runtime/'N64Recomp/include']]]
    subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
    sources=[ROOT/'src/native_simulation_time.cpp',ROOT/'research/native-simulation-check.cpp']
    sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
    sources.append(ROOT/'research/native-inactive-minigame-fixture.cpp')
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
    result=subprocess.check_output([str(out/'check')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
    tracked=sources+[ROOT/'src/native_simulation_time.hpp',ROOT/'recomp/native-timing-rules.json',ROOT/'tools/native_timing_patches.py',ROOT/'tools/generate_startup.py',Path(__file__).resolve()]
    report=dict(scope='Shared simulation timeline and explicit operand patches, not scene activation or full physics/collision integration',result=result,
        source_sha256={str(p.relative_to(ROOT)):sha(p) for p in tracked},generation_receipt_sha256=sha(g/'generation-receipt.json'),reference_receipt_sha256=sha(ref/'generation-receipt.json'),
        original_functions=sorted(names),limitations=['Synthetic memory; no game/GPU; seven selected original helpers, not all gameplay closures',
        'Gravity/displacement pairing additionally checked in native-gravity-check; full collision integration remains separate',
        'No scene is enabled by these operand hooks alone'])
    (ROOT/'research/native-simulation-check.json').write_text(json.dumps(report,indent=2)+'\n');print(result)
if __name__=='__main__':main()
