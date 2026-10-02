"""Audit first image scene's actual drawing on synthetic RAM; never launches GPU."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path);p.add_argument('--rom',required=True,type=Path)
 a=p.parse_args();g=a.generated.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
 for base,group in [(ROOT,'inputs'),(g,'generated')]:
  for n,v in receipt[group].items():assert sha(base/n)==v,('Stale',n)
 rom=a.rom.read_bytes();assert hashlib.sha256(rom).hexdigest()==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
 out=ROOT/'build-local/native-scene34-draw-check';out.mkdir(exist_ok=True)
 # Resident floating point constants only. Keep ROM bytes and original bodies ignored.
 (out/'seed.bin').write_bytes(struct.pack('<32I',*struct.unpack_from('>32I',rom,0x1000+0x8007CD80-0x80025C00)))
 addresses={0x801C012C,0x801BFF24,0x8002F594,0x8002F6A4,0x8002A724,0x8002A844,
  0x80028370,0x800282EC,0x80028320,0x8002833C,0x80028344,0x8002834C,0x80028354,0x800289D8,0x80028688}
 bodies=[];names=[];found=set()
 for f in (g/'RecompiledFuncs').glob('*.c'):
  for body in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S):
   pc=re.search(r'// 0x([0-9A-F]+):',body)
   if pc and int(pc[1],16) in addresses:
    found.add(int(pc[1],16));bodies.append(body);names.append(re.search(r'void (\w+)\(',body)[1])
 assert found==addresses,addresses-found
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies))
 (out/'scene34-functions.h').write_text('extern "C" {\n'+'\n'.join(f'void {n}(uint8_t*,recomp_context*);' for n in names)+'\n}\n')
 runtime=ROOT/'build-deps/lib/N64ModernRuntime'
 flags=['-O1','-g','-fsanitize=address','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',out,runtime/'N64Recomp/include']]]
 subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
 sources=[ROOT/'src/native_frame_time.cpp',ROOT/'research/native-scene34-draw-check.cpp']
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check'),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1')).strip()
 tracked=sources+[ROOT/'src/native_frame_time.hpp',ROOT/'src/frame_lights.hpp',Path(__file__).resolve()]
 (ROOT/'research/native-scene34-draw-check.json').write_text(json.dumps({'scope':'Original scene34 image draw closure + native Fade; synthetic resource descriptors and memory; no original update/whole build/renderer or runtime scene adapter',
  'result':result,'sanitizers':'ASan/LSan','source_sha256':{str(f.relative_to(ROOT)):sha(f) for f in tracked},
  'generation_receipt_sha256':sha(g/'generation-receipt.json'),'original_functions':names},indent=2)+'\n')
 print(result)
if __name__=='__main__':main()
