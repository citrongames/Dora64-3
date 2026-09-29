"""Check native handoff and real deadline thread, without guest game or renderer."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sanitizer',choices=['address','thread'],default='address');a=p.parse_args()
 out=ROOT/'build-local'/('native-session-'+a.sanitizer);out.mkdir(exist_ok=True)
 runtime=ROOT/'build-deps/lib/N64ModernRuntime'
 sources=[ROOT/'src'/n for n in ['native_frame_time.cpp','native_frame_scheduler.cpp','native_frame_bridge.cpp','native_present.cpp','native_frame_producer.cpp']]+[ROOT/'research/native-session-check.cpp']
 flags=['-O1','-g','-pthread','-fno-omit-frame-pointer','-fno-strict-aliasing','-fsanitize='+a.sanitizer,'-I'+str(ROOT/'src'),'-I'+str(runtime/'N64Recomp/include')]
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,*map(str,sources),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',TSAN_OPTIONS='halt_on_error=1'),timeout=30).strip()
 tracked=sources+[ROOT/'src'/n for n in ['native_frame_session.hpp','native_frame_producer.hpp','native_presentation_gate.hpp','native_wake_signal.hpp','native_frame_scheduler.hpp','native_frame_bridge.hpp','native_present.hpp']]+[Path(__file__).resolve()]
 (ROOT/'research'/('native-session-'+a.sanitizer+'-check.json')).write_text(json.dumps({'scope':'Actual session and producer thread with synthetic consumer and sender; no game, GPU, VI or audio thread; timeouts are test watchdogs only',
 'result':result,'sanitizer':a.sanitizer,'source_sha256':{str(p.relative_to(ROOT)):sha(p) for p in tracked}},indent=2)+'\n')
 print(result)
if __name__=='__main__':main()
