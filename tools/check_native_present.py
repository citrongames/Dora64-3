"""Check native frame ownership and real runtime snapshot backend without a GPU."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--sanitizer',choices=['address','thread'],default='address');args=parser.parse_args()
 out=ROOT/'build-local'/('native-present-'+args.sanitizer+'-check');out.mkdir(exist_ok=True)
 subprocess.run(['python3',str(ROOT/'tools/prepare_runtime_timing.py'),str(out)],check=True)
 runtime=ROOT/'build-deps/lib/N64ModernRuntime'
 original=runtime/'ultramodern/src/events.cpp'
 # prepare_runtime_timing has already verified the exact pinned source hash.
 text=original.read_text()
 state=text[text.index('struct ViState {'):text.index('static struct {')]
 methods=text[text.index('        ViState* get_next_state()'):text.index('    } vi;')]
 (out/'original-vi.h').write_text(state+'\nstruct ViOracle { int cur_state=0,field=0; ViState states[2]{}; ViRegs regs{};\n'+methods+'};\n')
 adapted=(out/'events-after-parse.cpp').read_text()
 start=adapted.index('bool dora3_enqueue_native_present(')
 end=adapted.index('\n}',start)+2
 (out/'native-present-backend.h').write_text(adapted[start:end]+'\n')
 start=adapted.index('            else if (const auto* screen_update_action =')
 end=adapted.index('            else if (const auto* config_action =',start)
 body=adapted[start:end].replace('else if','if',1)
 (out/'native-present-worker.h').write_text('static void process_action(const std::variant<ScreenUpdateAction>& action) { for (int once=0;once<1;++once) {\n'+body+'} }\n')
 sources=[ROOT/'research/native-loop-stubs.cpp']+[ROOT/'src'/n for n in ['native_frame_time.cpp','native_frame_scheduler.cpp','native_frame_bridge.cpp','native_present.cpp']]+[ROOT/'research/native-present-check.cpp']
 flags=['-O1','-g','-pthread','-fno-omit-frame-pointer','-fno-strict-aliasing','-fsanitize='+args.sanitizer,*['-I'+str(p) for p in [ROOT/'src',out,runtime/'ultramodern/include',runtime/'N64Recomp/include',runtime/'thirdparty']]]
 sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,*map(str,sources),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')).strip()
 tracked=sources+[ROOT/'src/recomp_context_copy.hpp',ROOT/'src/native_present.hpp',ROOT/'src/native_vi_snapshot.hpp',ROOT/'tools/prepare_runtime_timing.py',Path(__file__).resolve()]
 (ROOT/'research'/('native-present-'+args.sanitizer+'-check.json')).write_text(json.dumps({'scope':'Ownership protocol and extracted actual native enqueue backend versus pinned update_vi; extracted actual screen worker with fake renderer, no game/GPU',
  'result':result,'sanitizer':args.sanitizer,'source_sha256':{str(p.relative_to(ROOT)):sha(p) for p in tracked},
  'original_events_sha256':sha(original),'adapted_events_sha256':sha(out/'events-after-parse.cpp')},indent=2)+'\n')
 print(result)
if __name__=='__main__':main()
