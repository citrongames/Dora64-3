"""Original guest/component stress under AddressSanitizer + LeakSanitizer; no GPU."""
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
from inspect_rom import ROOT
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path)
    a=p.parse_args();g=a.generated.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,group in [(ROOT,'inputs'),(g,'generated')]:
        for f,h in receipt[group].items():
            if sha(base/f)!=h:raise ValueError('Stale input '+f)
    out=ROOT/'build-local/synchronization-check';out.mkdir(exist_ok=True)
    addresses=[0x800271B0,0x800339F0,0x80033A14,0x80033A88,0x80033BA4]
    symbols=(g/'startup.syms.toml').read_text();bodies=[];names=[]
    for address in addresses:
        name=re.search(r'name = "([^"]+)", vram = 0x'+f'{address:08X}',symbols)[1];names.append(name)
        pattern=r'RECOMP_FUNC void '+name+r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
        found=[x for f in (g/'RecompiledFuncs').glob('*.c') for x in re.findall(pattern,f.read_text(),re.S)]
        assert len(found)==1;bodies.extend(found)
    baseline=bodies[0].replace(names[0],'original_copy').replace('    dora3_begin_cpu_fb_copy();\n','').replace('    dora3_end_cpu_fb_copy();\n','')
    # Both added hooks lie outside the backward loop; no guest function call is locked.
    locked=bodies[0].split('dora3_begin_cpu_fb_copy();')[1].split('dora3_end_cpu_fb_copy();')[0]
    assert not re.search(r'\b(?:dora3_|os\w+_recomp|LOOKUP_FUNC)\w*\(',locked)
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies+[baseline]))
    declarations=['#pragma once','#include "recomp.h"','extern "C" {']+[f'void {name}(uint8_t*,recomp_context*);' for name in names+['original_copy']]+['}']
    (out/'sync-functions.h').write_text('\n'.join(declarations)+'\n')
    rt=ROOT/'build-deps/lib/N64ModernRuntime'
    include=[ROOT/'src',g,g/'RecompiledFuncs',out,*[rt/x for x in ['N64Recomp/include','ultramodern/include','librecomp/include','thirdparty','thirdparty/concurrentqueue']]]
    flags=['-O1','-g','-pthread','-fsanitize=address','-fno-omit-frame-pointer',*['-I'+str(x) for x in include]]
    subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
    sources=[ROOT/'research/synchronization-check.cpp',ROOT/'src/graphics_sync.cpp']
    subprocess.run(['g++','-std=c++20',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
    env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1';env['LSAN_OPTIONS']='exitcode=23'
    result=subprocess.check_output([str(out/'check')],env=env,text=True).strip()
    subprocess.run(['python3',str(ROOT/'tools/prepare_runtime_timing.py'),str(out/'runtime')],check=True)
    events=(out/'runtime/events-after-parse.cpp').read_text()
    assert events.count('periodic_present.acquire()')==1
    # Multiple release paths are intentional: completion and stale/native drops.
    # Exact worker behavior is executed by check_native_present, not text-counted here.
    assert events.count('OriginalReadLease original_read')==1
    assert 'ScreenUpdateAction{ current_vi_regs }' in events # Explicit CPU publication remains intact.
    report={'scope':'Component-only; ASan/LSan; no renderer/game; GPU leaks and full-scene lifecycle not validated','result':result,
      'sanitizers':'AddressSanitizer, LeakSanitizer: exit 0; no reported violations',
      'generation_receipt_sha256':sha(g/'generation-receipt.json'),
      'sources':{str(f.relative_to(ROOT)):sha(f) for f in sources+[ROOT/'src/graphics_sync.hpp',ROOT/'src/live_graphics.cpp',ROOT/'tools/prepare_runtime_timing.py',Path(__file__).resolve()]}}
    (ROOT/'research/synchronization-check.json').write_text(json.dumps(report,indent=2)+'\n');print(result)
if __name__=='__main__':main()
