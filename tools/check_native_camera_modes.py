"""Check original camera control and transition functions with native time coefficients."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ['generated','reference','rom']:p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();g=a.generated.resolve();ref=a.reference.resolve();rom=a.rom.read_bytes()
    assert hashlib.sha256(rom).hexdigest()==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
    receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,group in [(ROOT,'inputs'),(g,'generated')]:
        for n,v in receipt[group].items():assert sha(base/n)==v,('Stale generated input',n)
    old=json.loads((ref/'generation-receipt.json').read_text())
    assert old['rom_sha256']==receipt['rom_sha256']
    for n,v in old['generated'].items():assert sha(ref/n)==v,n
    def extract(folder):
        bodies={re.search(r'void (\w+)\(',b)[1]:b for p in (folder/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC\s+void \w+\([^\n]+\n.*?(?=RECOMP_FUNC\s+void |\Z)',read_decompiled_source(p),re.S)}
        todo=['dora3_s3_core_801D5594','dora3_s3_801D5864', 'dora3_s3_801D7294', 'dora3_s3_801D8578', 'dora3_s3_801D8A24', 'dora3_s3_801D8FF0', 'dora3_s3_801DB720', 'dora3_s3_801D9B00', 'dora3_s3_801D9C94', 'dora3_s3_801DA06C', 'dora3_s3_801DAB58', 'dora3_s3_801DA588', 'dora3_s3_801DAF50', 'dora3_s3_801DB3D0'];out={}
        while todo:
            name=todo.pop()
            if name in out:continue
            if name not in bodies:raise ValueError('Unexpected external effect call '+name)
            out[name]=bodies[name]
            todo.extend(n for n in re.findall(r'\b(\w+)\(rdram, ctx\)',bodies[name]) if n not in {'dora3_trace','dora3_native_zoom_update','dora3_native_rectangle_update'})
        return out
    patched=extract(g);original=extract(ref);assert patched.keys()==original.keys()
    assert all('dora3_native_' not in body for body in original.values())
    declarations='\n'.join('RECOMP_FUNC void reference_'+n+'(uint8_t*,recomp_context*);' for n in original)
    originals='\n'.join(original.values())
    for n in sorted(original,key=len,reverse=True):originals=re.sub(r'\b'+n+r'\b','reference_'+n,originals)
    out=ROOT/'build-local/native-camera-modes-check';out.mkdir(exist_ok=True)
    (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+declarations+'\n'+'\n'.join(patched.values())+'\n'+originals)
    seed=bytearray(0x400000);m=json.loads((ROOT/'recomp/startup.json').read_text())
    for s in [{'rom':0x1000,'vram':0x80025C00,'size':0x8007FA00-0x80025C00},next(s for s in m['overlays'] if s['name']=='.scene1')]:
        data=rom[s['rom']:s['rom']+s['size']];base=s['vram']&0xFFFFFF
        for i in range(0,len(data),4):seed[base+i:base+i+4]=data[i:i+4][::-1]
    (out/'seed.bin').write_bytes(seed)
    runtime=ROOT/'build-deps/lib/N64ModernRuntime';flags=['-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',runtime/'N64Recomp/include']]]
    subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
    sources=[ROOT/'src/native_simulation_time.cpp',ROOT/'src/native_camera_timeline.cpp',ROOT/'research/native-camera-modes-check.cpp']
    sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
    result=subprocess.check_output([str(out/'check'),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
    tracked=sources+[ROOT/'src/native_simulation_time.hpp',ROOT/'src/native_camera_timeline.hpp',ROOT/'recomp/native-timing-rules.json',ROOT/'tools/native_timing_patches.py',ROOT/'tools/generate_startup.py',Path(__file__).resolve()]
    report=dict(scope='Thirteen ordinary camera roots: anchored equation endpoint equivalence, shared fractional samples, external-write ownership and Source-end shake commands; no scene activation',result=result,original_functions=sorted(patched),source_sha256={str(p.relative_to(ROOT)):sha(p) for p in tracked},generation_receipt_sha256=sha(g/'generation-receipt.json'),reference_receipt_sha256=sha(ref/'generation-receipt.json'),
        limitations=['Synthetic state and original ROM constants, no GPU; replay constants remain ignored locally','Whole-tick equivalence compares owned state, excluding scratch stack/registers of replaced leaf functions','Synthetic actors and no-map/room geometry through original query; no real level, mode-switch input or changing-player trajectory verification'])
    (ROOT/'research/native-camera-modes-check.json').write_text(json.dumps(report,indent=2)+'\n');print(result)
if __name__=='__main__':main()
