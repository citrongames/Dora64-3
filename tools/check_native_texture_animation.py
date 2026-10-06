"""Actual Source bitmap binding/material builder and Native RDP scroll ownership; CPU only."""
from pathlib import Path
import argparse,hashlib,json,os,re,subprocess
from decomp_annotations import read_decompiled_source
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--generated',type=Path,required=True);a=ap.parse_args();g=a.generated.resolve()
 receipt=json.loads((g/'generation-receipt.json').read_text())
 for base,key in ((ROOT,'inputs'),(g,'generated')):
  for name,digest in receipt[key].items():assert sha(base/name)==digest,('Stale texture generation',name)
 bodies={re.search(r'void (\w+)\(',b)[1]:b for f in (g/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
 todo=['dora3_s0_80028D50','dora3_s0_80028C74','dora3_s0_8003A3B8','dora3_s3_801BBFF4'];selected={}
 while todo:
  name=todo.pop()
  if name in selected or name=='dora3_trace' or name.startswith('dora3_native_'):continue
  assert name in bodies,('Unseeded texture external',name);selected[name]=bodies[name]
  todo.extend(re.findall(r'\b(\w+)\(rdram, ctx\)',bodies[name]))
 out=ROOT/'build-local/native-texture-animation-check';out.mkdir(exist_ok=True)
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(selected.values()))
 (out/'texture-functions.h').write_text('extern "C" {\n'+'\n'.join('void '+n+'(uint8_t*,recomp_context*);' for n in selected)+'\n}\n')
 flags=['-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(p) for p in (ROOT/'src',g,g/'RecompiledFuncs',out,ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include')]]
 subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
 sources=[ROOT/'src/native_texture_animation.cpp',ROOT/'src/native_simulation_time.cpp',ROOT/'research/native-texture-animation-check.cpp',ROOT/'research/native-inactive-enemy-fixture.cpp',ROOT/'research/native-inactive-minigame-fixture.cpp']
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
 tracked=sources+[ROOT/'src/native_texture_animation.hpp',ROOT/'src/native_game_loop.cpp',ROOT/'src/native_ui.cpp',ROOT/'tools/native_timing_patches.py',ROOT/'recomp/native-timing-rules.json',Path(__file__).resolve()]
 report=dict(result=result,generation_receipt_sha256=sha(g/'generation-receipt.json'),source_sha256={f.relative_to(ROOT).as_posix():sha(f) for f in tracked},actual_functions=sorted(selected),no_game_gpu=True,limitations=['Synthetic bitmap/material assets with actual Original Source channel/binding/material commands; no rendered water or full-game QA','ROM texture inventory exact translations is separate analysis, not all reached runtime paths','Synthetic frame tickets verify guest bank isolation and references, not actual GPU retirement'])
 assert result.startswith('PASS:');(ROOT/'research/native-texture-animation-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(result)
if __name__=='__main__':main()
