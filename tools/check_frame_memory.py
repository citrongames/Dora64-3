"""Check frame light ownership with original generated functions; no renderer."""
from decomp_annotations import read_decompiled_source
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from inspect_rom import ROOT, read_rom

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path)
    p.add_argument('--generated',required=True,type=Path)
    a=p.parse_args();rom=read_rom(a.rom);generated=a.generated.resolve()
    receipt=json.loads((generated/'generation-receipt.json').read_text())
    for base,group in [(ROOT,'inputs'),(generated,'generated')]:
        for name,expected in receipt[group].items():
            if sha(base/name)!=expected:raise ValueError('Stale generation: '+name)
    out=ROOT/'build-local/frame-memory-check';out.mkdir(exist_ok=True)
    bodies=[]
    for name in ['dora3_graphics_task_submit','dora3_s0_80027B84']:
        pattern=r'RECOMP_FUNC void '+name+r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
        matches=[m for path in (generated/'RecompiledFuncs').glob('*.c') for m in re.findall(pattern,read_decompiled_source(path),re.S)]
        if len(matches)!=1:raise ValueError('Missing original function: '+name)
        bodies.append(matches[0])
    submit=bodies[0]
    if not submit.index('dora3_capture_frame_lights(')<submit.index('MEM_W(0XC, ctx->r8)')<submit.index('osSendMesg_recomp('):
        raise ValueError('Publication hook/delay-slot order changed')
    source=out/'functions.c'
    source.write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies))
    runtime=ROOT/'build-deps/lib/N64ModernRuntime'
    includes=[runtime/'N64Recomp/include',ROOT/'src',generated,generated/'RecompiledFuncs']
    flags=['-O2','-g','-pthread',*['-I'+str(x) for x in includes]]
    subprocess.run(['gcc','-std=c17',*flags,'-c',str(source),'-o',str(out/'functions.o')],check=True)
    sources=[ROOT/'research/native-loop-stubs.cpp', ROOT/'research/frame-memory-check.cpp',ROOT/'src/frame_lights.cpp']
    subprocess.run(['g++','-std=c++20',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
    result=subprocess.check_output([str(out/'check')],text=True).strip()
    # Fingerprints identify the reviewed ROM bodies; no ROM bytes in the report.
    reviewed={0x8002787C:0x20C,0x80026D40:0x8C,0x800271B0:0x15C,
              0x8002FE20:0x144,0x8002FF64:0x158,0x80027B84:0x3C,0x80031588:0x68}
    report={'scope':'Static frame lifetime audit and non-render component check; not gameplay/GPU validation',
            'result':result,'rom_sha256':hashlib.sha256(rom).hexdigest(),
            'reviewed_resident_bodies':{f'0x{address:08X}':{'size':size,'sha256':hashlib.sha256(rom[0x1000+address-0x80025C00:0x1000+address-0x80025C00+size]).hexdigest()} for address,size in reviewed.items()},
            'sources':{str(path.relative_to(ROOT)):sha(path) for path in sources+[ROOT/'src/frame_lights.hpp',ROOT/'src/live_graphics.cpp',ROOT/'tools/generate_startup.py',Path(__file__).resolve()]},
            'generation_receipt_sha256':sha(generated/'generation-receipt.json')}
    (ROOT/'research/frame-memory-check.json').write_text(json.dumps(report,indent=2)+'\n')
    print(result)
if __name__=='__main__':main()
