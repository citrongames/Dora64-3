"""Check original paired vertex buffers and their retirement on synthetic RDRAM; no GPU."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
from inspect_rom import ROOT
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path);a=p.parse_args();g=a.generated.resolve()
 receipt=json.loads((g/'generation-receipt.json').read_text())
 for base,group in [(ROOT,'inputs'),(g,'generated')]:
  for f,h in receipt[group].items():
   if sha(base/f)!=h:raise ValueError('Stale '+f)
 out=ROOT/'build-local/paired-vertex-check';out.mkdir(exist_ok=True)
 syms=(g/'startup.syms.toml').read_text();names=[];bodies=[]
 for addr in [0x80038BC8,0x80038D58,0x80038C58,0x80038B10,0x80038B4C]:
  name=re.search(r'name = "([^"]+)", vram = 0x'+f'{addr:08X}',syms)[1];names.append(name)
  pattern=r'RECOMP_FUNC void '+name+r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
  matches=[m for f in (g/'RecompiledFuncs').glob('*.c') for m in re.findall(pattern,read_decompiled_source(f),re.S)]
  assert len(matches)==1;bodies.extend(matches)
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies))
 (out/'retirement-functions.h').write_text('extern "C" {\n'+'\n'.join(f'void {name}(uint8_t*,recomp_context*);' for name in names)+'\n}\n')
 rt=ROOT/'build-deps/lib/N64ModernRuntime';flags=['-O1','-g','-fsanitize=address','-fno-omit-frame-pointer',*['-I'+str(x) for x in [g,g/'RecompiledFuncs',out,rt/'N64Recomp/include']]]
 subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
 subprocess.run(['g++','-std=c++20',*flags,str(ROOT/'research/paired-vertex-check.cpp'),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1'
 result=subprocess.check_output([str(out/'check')],env=env,text=True).strip()
 report={'scope':'Original generated paired allocation/morph/destruction and timer; synthetic allocation/free endpoints; no game/GPU',
 'result':result,'sanitizers':'ASan/LSan exit0; guest dropped request is a semantic observation, not a host allocator leak',
 'source_sha256':{str(f.relative_to(ROOT)):sha(f) for f in [ROOT/'research/paired-vertex-check.cpp',Path(__file__).resolve()]},'generation_receipt_sha256':sha(g/'generation-receipt.json')}
 (ROOT/'research/paired-vertex-check.json').write_text(json.dumps(report,indent=2)+'\n');print(result)
if __name__=='__main__':main()
