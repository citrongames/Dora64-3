"""Exercise native input ownership through real original VI routines, no game/GPU."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path)
    g=p.parse_args().generated.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,key in [(ROOT,'inputs'),(g,'generated')]:
        for name,value in receipt[key].items():assert sha(base/name)==value,('Stale',name)
    bodies={re.search(r'void (\w+)\(',b)[1]:b for f in (g/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
    names=['dora3_controller_accumulate','dora3_controller_latch']
    out=ROOT/'build-local/native-input-check';out.mkdir(exist_ok=True)
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies[n] for n in names))
    flags=['-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
    sources=[ROOT/'src/native_input_timeline.cpp',ROOT/'research/native-input-check.cpp']
    sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
    sources.append(ROOT/'research/native-inactive-minigame-fixture.cpp')
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
    result=subprocess.check_output([str(out/'check')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
    tracked=sources+[ROOT/'src/native_input_timeline.hpp',Path(__file__).resolve()]
    (ROOT/'research/native-input-check.json').write_text(json.dumps(dict(result=result,original_functions=names,source_sha256={str(x.relative_to(ROOT)):sha(x) for x in tracked},generation_receipt_sha256=sha(g/'generation-receipt.json'),sanitizers='ASan/LSan; UBSan new C++'),indent=2)+'\n')
    print(result)
if __name__=='__main__':main()
