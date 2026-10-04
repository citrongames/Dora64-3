"""Check original scheduler native wake path on synthetic RAM, without GPU/game."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--generated',required=True,type=Path);p.add_argument('--rom',required=True,type=Path)
 a=p.parse_args();g=a.generated.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
 for base,group in [(ROOT,'inputs'),(g,'generated')]:
  for n,v in receipt[group].items():
   if sha(base/n)!=v:raise ValueError('Stale '+n)
 rom=a.rom.read_bytes()
 if hashlib.sha256(rom).hexdigest()!=json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']:raise ValueError('Wrong ROM')
 out=ROOT/'build-local/native-graphics-pump-check';out.mkdir(exist_ok=True)
 # Only the reviewed scheduler jump table is needed; never track these ROM bytes.
 words=struct.unpack_from('>7I',rom,0x1000+0x8007CD60-0x80025C00)
 (out/'seed.bin').write_bytes(struct.pack('<7I',*words))
 names=['dora3_scheduler_thread','dora3_scheduler_retrace','dora3_scheduler_enqueue','dora3_scheduler_select',
  'dora3_scheduler_launch','dora3_scheduler_request_yield','dora3_scheduler_framebuffer_ready',
  'dora3_graphics_task_submit','dora3_scheduler_sp_complete','dora3_scheduler_dp_complete','dora3_scheduler_task_complete']
 bodies=[]
 for name in names:
  matches=[m for f in (g/'RecompiledFuncs').glob('*.c') for m in re.findall(r'RECOMP_FUNC void '+name+r'\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)]
  if len(matches)!=1:raise ValueError('Missing/duplicate '+name)
  bodies+=matches
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(bodies))
 subprocess.run(['python3',str(ROOT/'tools/prepare_runtime_timing.py'),str(out)],check=True)
 runtime=ROOT/'build-deps/lib/N64ModernRuntime'
 flags=['-O1','-g','-pthread','-fsanitize=address','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in
  [ROOT/'src',g,g/'RecompiledFuncs',runtime/'N64Recomp/include',runtime/'ultramodern/include',runtime/'thirdparty',runtime/'thirdparty/concurrentqueue']]]
 subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
 subprocess.run(['g++','-std=c++20',*flags,'-c',str(out/'mesgqueue.cpp'),'-o',str(out/'queue.o')],check=True)
 sources=[ROOT/'research/native-loop-stubs.cpp']+[ROOT/'src'/n for n in ['native_frame_time.cpp','native_frame_scheduler.cpp','native_frame_bridge.cpp','native_graphics_pump.cpp','native_present.cpp','native_present_hooks.cpp']]+[ROOT/'research/native-graphics-pump-check.cpp']
 sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,*map(str,sources),str(out/'functions.o'),str(out/'queue.o'),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check'),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1')).strip()
 # Reintroduce the old inner-epilogue hook in an isolated fixture. The same
 # fast completion must catch premature reuse before the caller's final stores.
 bad=(out/'functions.c').read_text()
 marker='    // 0x80026DBC:'
 assert bad.count(marker)==1
 bad=bad.replace(marker,'    dora3_native_task_retired(rdram, ctx);\n'+marker)
 (out/'early-functions.c').write_text(bad)
 subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'early-functions.c'),'-o',str(out/'early-functions.o')],check=True)
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,*map(str,sources),str(out/'early-functions.o'),str(out/'queue.o'),'-o',str(out/'early-check')],check=True)
 import resource
 def no_core(): resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 rejected=subprocess.run([str(out/'early-check'),str(out/'seed.bin')],text=True,capture_output=True,
  env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1'),preexec_fn=no_core)
 assert rejected.returncode==-6 and '0x123BAD' in rejected.stderr,(rejected.returncode,rejected.stdout,rejected.stderr)
 negative={'result':'Expected SIGABRT: old 80026DBC hook presents before final caller timing store',
  'exit_code':rejected.returncode,'diagnostic':rejected.stderr.strip()}
 print(negative['result'])
 tracked=sources+[ROOT/'src/recomp_context_copy.hpp',ROOT/'src/native_graphics_pump.hpp',ROOT/'src/native_frame_bridge.hpp',ROOT/'tools/generate_startup.py',ROOT/'tools/prepare_runtime_timing.py',Path(__file__).resolve()]
 (ROOT/'research/native-graphics-pump-check.json').write_text(json.dumps({'scope':'Actual original scheduler/submit and adapted message queue on synthetic RAM; SDK hardware and thread scheduling stubbed; no active native scene or renderer',
  'result':result,'negative_control':negative,'sanitizers':'ASan/LSan','source_sha256':{str(f.relative_to(ROOT)):sha(f) for f in tracked},
  'generation_receipt_sha256':sha(g/'generation-receipt.json'),'original_functions':names},indent=2)+'\n')
 print(result)
if __name__=='__main__':main()
