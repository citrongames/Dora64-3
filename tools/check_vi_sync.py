"""Stress the pinned runtime's real VI functions and retrace block, without graphics."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--baseline',action='store_true');p.add_argument('--sanitizer',choices=['thread','address','none'],default='thread');a=p.parse_args()
    out=ROOT/'build-local/vi-sync-check';out.mkdir(exist_ok=True)
    subprocess.run(['python3',str(ROOT/'tools/prepare_runtime_timing.py'),str(out/'runtime')],check=True)
    rt=ROOT/'build-deps/lib/N64ModernRuntime'
    source=rt/'ultramodern/src/events.cpp' if a.baseline else out/'runtime/events-after-parse.cpp'
    text=source.read_text();start=text.index('        {\n',text.index('// If the game hasn\'t started yet'))
    end=text.index('        // If the game has started, handle sending VI and AI events.',start)
    tick=text[start:end]
    if not a.baseline:
        assert 'std::scoped_lock lock{ events_context.message_mutex, events_context.vi.regs_mutex };' in tick
    harness=out/('baseline.cpp' if a.baseline else 'adapted.cpp')
    harness.write_text('#include "'+str(source)+'"\nvoid checked_vi_tick(){ const bool queue_periodic=false;\n'+tick+'}\n'+(ROOT/'research/vi-sync-main.cpp').read_text())
    includes=[ROOT/'src',*[rt/x for x in ['N64Recomp/include','ultramodern/include','librecomp/include','thirdparty','thirdparty/concurrentqueue']]]
    flags=['-std=c++20','-O1','-g','-pthread','-ffunction-sections','-fdata-sections','-fno-omit-frame-pointer',*['-I'+str(x) for x in includes]]
    if a.sanitizer!='none':flags+=['-fsanitize='+a.sanitizer]
    exe=out/('baseline' if a.baseline else 'check')
    subprocess.run(['g++',*flags,str(harness),str(rt/'ultramodern/src/misc_ultra.cpp'),'-Wl,--gc-sections','-o',str(exe)],check=True)
    env=os.environ.copy();env['TSAN_OPTIONS']='halt_on_error=1:exitcode=24';env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1'
    result=subprocess.run([str(exe)],env=env,text=True,capture_output=True)
    (out/('baseline-result.txt' if a.baseline else 'result.txt')).write_text(result.stdout+result.stderr)
    print(result.stdout+result.stderr)
    if result.returncode:raise SystemExit(result.returncode)
    if not a.baseline:
        report={'scope':'Real SDK VI state functions and exact retrace block only; no GPU/game threads',
          'sanitizer':a.sanitizer,'result':result.stdout.strip(),
          'sources':{str(f.relative_to(ROOT)):sha(f) for f in [ROOT/'tools/prepare_runtime_timing.py',ROOT/'research/vi-sync-main.cpp',Path(__file__).resolve()]},
          'adapted_events_sha256':sha(source)}
        (ROOT/'research/vi-sync-check.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
