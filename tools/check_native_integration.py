"""Connected native session lifecycle proof; no game/renderer/GPU."""
from pathlib import Path
import argparse,hashlib,json,os,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path);a=p.parse_args();g=a.generated.resolve()
    receipt=json.loads((g/'generation-receipt.json').read_text())
    for root,key in [(ROOT,'inputs'),(g,'generated')]:
        for n,h in receipt[key].items():assert sha(root/n)==h,('Stale',n)
    names=['native_game_loop.cpp','native_scene_policy.cpp','native_model_animation.cpp','native_ui.cpp','native_frame_time.cpp','native_frame_scheduler.cpp','native_frame_bridge.cpp','native_present.cpp','native_simulation_time.cpp']
    sources=[ROOT/'src'/n for n in names]+[ROOT/'research/native-integration-check.cpp']
    out=ROOT/'build-local/native-integration-check';out.mkdir(exist_ok=True)
    cmd=['g++','-std=c++20','-O1','-g','-Wall','-Wextra','-Werror','-pthread','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in (ROOT/'src',g/'RecompiledFuncs',ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include')],*map(str,sources),'-o',str(out/'check')]
    subprocess.run(cmd,check=True)
    run=subprocess.run([str(out/'check')],text=True,capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'))
    if run.returncode:print(run.stdout);print(run.stderr[-9000:]);run.check_returncode()
    tracked=sources+[ROOT/'src'/n for n in ('native_frame_session.hpp','native_scene_policy.hpp','native_model_animation.hpp','native_minigames.hpp','native_game_loop.hpp','native_presentation_gate.hpp','recomp_context_copy.hpp')]+[Path(__file__).resolve()]
    report={'result':run.stdout.strip(),'generation_receipt_sha256':sha(g/'generation-receipt.json'),'source_sha256':{x.relative_to(ROOT).as_posix():sha(x) for x in tracked},'no_game_gpu':True,'scope':'Real shared loop/policy/model/UI/session/bridge/scheduler/gate and ledger; synthetic scene clock probes/guest completions, actual family behavior covered by separate full checks','limitations':['No real game/renderer/GPU or actual DMA/level transition in this component; scene callbacks are explicit time probes.']}
    assert report['result'].startswith('PASS:')
    (ROOT/'research/native-integration-check.json').write_text(json.dumps(report,indent=2)+'\n');print(report['result'])
if __name__=='__main__':main()
