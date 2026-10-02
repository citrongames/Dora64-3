"""Check the retirement guard with original registry functions; no GPU."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
from inspect_rom import ROOT
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path);p.add_argument('--sanitizer',choices=['address','thread'],default='address');a=p.parse_args();g=a.generated.resolve()
 receipt=json.loads((g/'generation-receipt.json').read_text())
 for base,group in [(ROOT,'inputs'),(g,'generated')]:
  for f,h in receipt[group].items():
   if sha(base/f)!=h:raise ValueError('Stale '+f)
 out=ROOT/'build-local/retirement-guard-check';out.mkdir(exist_ok=True)
 syms=(g/'startup.syms.toml').read_text();names=[];bodies=[]
 for addr in [0x80038AE4,0x80033A14,0x80038B10,0x80038B4C]:
  name=re.search(r'name = "([^"]+)", vram = 0x'+f'{addr:08X}',syms)[1];names.append(name)
  pattern=r'RECOMP_FUNC void '+name+r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
  matches=[m for f in (g/'RecompiledFuncs').glob('*.c') for m in re.findall(pattern,read_decompiled_source(f),re.S)]
  assert len(matches)==1;bodies.extend(matches)
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies))
 (out/'retirement-functions.h').write_text('extern "C" {\n'+'\n'.join(f'void {name}(uint8_t*,recomp_context*);' for name in names)+'\n}\n')
 rt=ROOT/'build-deps/lib/N64ModernRuntime';flags=['-O1','-g','-fsanitize='+a.sanitizer,'-pthread','-fno-omit-frame-pointer',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',out,rt/'N64Recomp/include']]]
 subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
 subprocess.run(['g++','-std=c++20',*flags,str(ROOT/'research/retirement-guard-check.cpp'),str(ROOT/'src/resource_retirement.cpp'),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1';env['TSAN_OPTIONS']='halt_on_error=1:exitcode=24'
 result=subprocess.check_output([str(out/'check')],env=env,text=True).strip()
 report={'scope':'Original registry and heap init with production retirement guard; recording free endpoint; no game/GPU',
 'result':result,'sanitizers':a.sanitizer+' exit0; overflow pointers accounted for; GPU/driver untested',
 'source_sha256':{str(f.relative_to(ROOT)):sha(f) for f in [ROOT/'src/resource_retirement.cpp',ROOT/'src/resource_retirement.hpp',ROOT/'research/retirement-guard-check.cpp',Path(__file__).resolve()]},'generation_receipt_sha256':sha(g/'generation-receipt.json')}
 (ROOT/('research/retirement-guard-'+a.sanitizer+'-check.json')).write_text(json.dumps(report,indent=2)+'\n');print(result)
if __name__=='__main__':main()
