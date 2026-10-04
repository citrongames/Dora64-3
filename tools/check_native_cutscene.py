"""Focused connected scene1 cutscene check. No player/NPC/pickup rerun or GPU."""
from pathlib import Path
import argparse,hashlib,json,os,re,struct,subprocess
from decomp_annotations import read_decompiled_source
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def extract(folder):
 return {re.search(r'void (\w+)\(',b)[1]:b for f in (folder/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for k in ['generated','reference','rom']:ap.add_argument('--'+k,type=Path,required=True)
 a=ap.parse_args();g=a.generated.resolve();ref=a.reference.resolve();rom=a.rom.read_bytes()
 assert hashlib.sha256(rom).hexdigest()==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
 receipt=json.loads((g/'generation-receipt.json').read_text());old=json.loads((ref/'generation-receipt.json').read_text())
 for base,group in [(ROOT,'inputs'),(g,'generated')]:
  for n,v in receipt[group].items():assert sha(base/n)==v,('Stale generated input',n)
 assert old['rom_sha256']==receipt['rom_sha256']
 for n,v in old['generated'].items():assert sha(ref/n)==v,n
 current=extract(g);baseline=extract(ref)
 motion_roots=['dora3_s3_801E3768','dora3_s3_801E0760','dora3_s3_801BBDDC','dora3_s0_8002F064']
 command_roots=['dora3_s3_801E4484','dora3_s3_801E3ECC','dora3_s3_801DE424','dora3_s3_801DE528','dora3_s3_801E3C10','dora3_s3_801E3C28','dora3_s3_801E3E94','dora3_s3_801E3E04','dora3_s3_801E4614']
 def closure(bodies,roots):
  out={};todo=roots.copy()
  while todo:
   n=todo.pop()
   if n in out:continue
   if n not in bodies:raise ValueError('Unexpected motion boundary '+n)
   out[n]=bodies[n]
   assert 'LOOKUP_FUNC' not in bodies[n],n
   todo.extend(x for x in re.findall(r'\b(\w+)\(rdram, ctx\)',bodies[n]) if x!='dora3_trace' and not x.startswith('dora3_native_'))
  return out
 motion=closure(current,motion_roots);expected_motion=closure(baseline,motion_roots);assert motion.keys()==expected_motion.keys()
 patched=dict(motion);original=dict(expected_motion)
 for n in command_roots:patched[n]=current[n];original[n]=baseline[n]
 assert all('dora3_native_' not in b for b in original.values())
 boundaries=set(x for b in patched.values() for x in re.findall(r'\b(\w+)\(rdram, ctx\)',b) if x not in patched and x!='dora3_trace' and not x.startswith('dora3_native_'))
 out=ROOT/'build-local/native-cutscene-check';out.mkdir(exist_ok=True)
 source='\n'.join(original.values());names=sorted(set(patched)|boundaries,key=len,reverse=True)
 for n in names:source=re.sub(r'\b'+n+r'\b','reference_'+n,source)
 declarations='\n'.join('RECOMP_FUNC void reference_'+n+'(uint8_t*,recomp_context*);' for n in names)
 defs=[]
 allowed={'dora3_memset','dora3_s0_80033C50','dora3_s1_801B5740','dora3_common_widgets_init'}
 for n in sorted(boundaries):
  for name in [n,'reference_'+n]:
   body=f'cutscene_fixture_boundary(rdram,ctx,"{n}");' if n in allowed else f'fprintf(stderr,"Unexpected command {n}\\n"); abort();'
   defs.append('RECOMP_FUNC void '+name+'(uint8_t* rdram,recomp_context* ctx){'+body+'}')
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n#include <stdio.h>\n#include <stdlib.h>\nvoid cutscene_fixture_boundary(uint8_t*,recomp_context*,const char*);\n'+declarations+'\n'+'\n'.join(patched.values())+'\n'+source+'\n'+'\n'.join(defs))
 seed=bytearray(0x400000);meta=json.loads((ROOT/'recomp/startup.json').read_text())
 for section in [{'rom':0x1000,'vram':0x80025C00,'size':0x8007FA00-0x80025C00},next(s for s in meta['overlays'] if s['name']=='.scene1')]:
  data=rom[section['rom']:section['rom']+section['size']];base=section['vram']&0xFFFFFF
  for i in range(0,len(data),4):seed[base+i:base+i+4]=data[i:i+4][::-1]
 (out/'seed.bin').write_bytes(seed)
 runtime=ROOT/'build-deps/lib/N64ModernRuntime';flags=['-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',runtime/'N64Recomp/include']]]
 function_stamp=hashlib.sha256((sha(out/'functions.c')+sha(g/'startup_hooks.h')+sha(g/'RecompiledFuncs/funcs.h')+repr(flags)).encode()).hexdigest()
 cached=out/'functions.sha256'
 if not (out/'functions.o').exists() or not cached.exists() or cached.read_text()!=function_stamp:
  subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
  cached.write_text(function_stamp)
 sources=[ROOT/'src/native_simulation_time.cpp',ROOT/'src/native_cutscene.cpp',ROOT/'research/native-cutscene-check.cpp']
 sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check'),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
 tracked=sources+[ROOT/'src/native_cutscene.hpp',ROOT/'src/native_simulation_time.hpp',ROOT/'src/native_scene_policy.cpp',ROOT/'src/native_game_loop.cpp',ROOT/'recomp/native-timing-rules.json',ROOT/'tools/native_timing_patches.py',ROOT/'tools/generate_startup.py',Path(__file__).resolve()]
 report=dict(scope='Connected scene1 participant/four-camera/opacity equation ownership and Source interpreter/camera command boundaries',result=result,motion_functions=sorted(motion),actual_command_functions=command_roots,external_boundaries=sorted(boundaries),allowed_counted_fixture_boundaries=sorted(allowed),source_sha256={str(x.relative_to(ROOT)):sha(x) for x in tracked},generation_receipt_sha256=sha(g/'generation-receipt.json'),reference_receipt_sha256=sha(ref/'generation-receipt.json'),limitations=['Synthetic controllers and original ROM math/constants; no real level/renderer or GPU','Actual motion closure has no allocator/audio/RNG/input/interpreter call; unselected opcodes fail-stop in fixture','Script resource-free/text-widget restoration are counted fixture boundaries, not complete allocator/UI integration','BB080 hooks statically pinned to Original ROM instructions; fixture runs actual interpreter and final geometry, not unrelated whole-world/HUD callbacks','Other overlays/minigames/2D/enemies/platforms remain separate blocks'])
 (ROOT/'research/native-cutscene-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(result)
if __name__=='__main__':main()
