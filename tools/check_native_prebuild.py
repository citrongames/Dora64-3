"""Run the connected CPU pre-build suite once after a completed implementation block.

No game, renderer or GPU is launched. --only repeats failed/affected checks;
passed independent checks retain their actual generation and source provenance.
"""
from pathlib import Path
import argparse, concurrent.futures, datetime, hashlib, json, subprocess, sys, threading, time

ROOT=Path(__file__).resolve().parents[1]
CHECKS=('ui cutscene model_animation frame_time simulation camera_control camera_modes '
        'script_camera effects gravity input player_motion process_time frame_bridge '
        'frame_scheduler graphics_pump loop scene34_draw session present scene_transition '
        'player_update scene_frame story_pickups enemy names overlay docs').split()
NO_GENERATED={'session','present','docs'}
ROM_CHECKS=set('ui cutscene camera_control camera_modes script_camera effects gravity player_motion process_time graphics_pump loop scene34_draw scene_transition player_update scene_frame story_pickups overlay'.split())
REFERENCE_CHECKS=set('ui cutscene simulation camera_control camera_modes script_camera effects gravity player_motion process_time scene_transition player_update scene_frame story_pickups'.split())
LEAF_REFERENCE=set('camera_control script_camera effects gravity process_time'.split())

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('generated','rom','reference','leaf-reference','city-reference','output-dir'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--enemy-reference',type=Path,default=ROOT/'build-local/startup-v356')
    parser.add_argument('--enemy-resume-evidence',type=Path)
    parser.add_argument('--jobs',type=int,choices=[1,2],default=2)
    parser.add_argument('--only',nargs='+',choices=CHECKS,help='Repeat only the explicitly affected checks in an existing run')
    a=parser.parse_args();g=a.generated.resolve();out=a.output_dir.resolve()
    assert out.is_relative_to(ROOT/'build-local'),'Logs and ROM paths must remain in ignored build-local'
    out.mkdir(parents=True,exist_ok=True)
    receipt=sha(g/'generation-receipt.json');path=out/'suite.json'
    lock=threading.Lock()
    if a.only:
        manifest=json.loads(path.read_text())
        assert manifest['generation_receipt_sha256']==receipt,'Retry cannot silently change generation'
        selected=a.only
    else:
        assert not path.exists(),'Use a fresh output directory for a full run, or --only for affected retries'
        selected=CHECKS
        manifest=dict(status='RUNNING',started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            generation_receipt_sha256=receipt,baseline_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            checks={},no_game_gpu=True,scope='Connected CPU suite; synthetic assets/effect boundaries, no visual or full-game completion claim')
    def command(key):
        special={'names':'check_decomp_names.py','overlay':'check_overlay_load_state.py','docs':'update_decomp_docs.py','scene_frame':'check_native_player_update.py','player_update':'check_native_player_update_partitions.py'}
        cmd=[sys.executable,str(ROOT/'tools'/special.get(key,'check_native_'+key+'.py'))]
        if key not in NO_GENERATED:cmd+=['--generated',str(g)]
        if key in ROM_CHECKS or key=='enemy':cmd+=['--rom',str(a.rom.resolve())]
        if key in REFERENCE_CHECKS or key=='enemy':cmd+=['--reference',str((a.leaf_reference if key in LEAF_REFERENCE else a.reference).resolve())]
        if key in {'scene_transition','player_update','scene_frame','story_pickups','enemy'}:cmd+=['--city-reference',str(a.city_reference.resolve())]
        if key in {'player_update','scene_frame','story_pickups'}:cmd+=['--optimization','O2']
        if key=='scene_frame':cmd+=['--scope','scene-frame']
        if key=='story_pickups':cmd+=['--scope','pickup-linked-batch']
        if key=='enemy':cmd+=['--enemy-reference',str(a.enemy_reference.resolve())]
        if key=='enemy' and a.enemy_resume_evidence:cmd+=['--resume-evidence',str(a.enemy_resume_evidence.resolve())]
        if key=='docs':cmd+=['--check']
        return cmd
    def save():path.write_text(json.dumps(manifest,indent=2)+'\n')
    for key in selected:
        previous=manifest['checks'].get(key)
        manifest['checks'][key]=dict(command=command(key),status='QUEUED',attempts=previous.get('attempts',[])+[{k:v for k,v in previous.items() if k!='attempts'}] if previous else [])
    save()
    def run(key):
        started=time.monotonic();record=manifest['checks'][key];cmd=record['command']
        log=out/(key+'-attempt'+str(len(record['attempts'])+1)+'.log')
        assert not log.exists(), 'Refuse to overwrite an existing attempt log: '+str(log)
        with lock:record.update(status='RUNNING',log=log.name);save();print('START '+key,flush=True)
        with log.open('w') as stream:result=subprocess.run(cmd,cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
        with lock:
            record.update(status='PASS' if result.returncode==0 else 'FAIL',exit_code=result.returncode,
                elapsed_seconds=round(time.monotonic()-started,2),log=log.name,log_sha256=sha(log))
            save();print(record['status']+' '+key,flush=True)
    # The story check prepares through player-update's fixture. Keep those
    # preparations sequential; their executables and output directories differ.
    connected={'player_update','scene_frame','story_pickups','scene_transition','enemy'}
    def run_connected():
        # Newly completed enemy block goes first. A failing connected check
        # leaves the expensive dependents queued until its cause is repaired.
        order=(["enemy"] if "enemy" in selected else [])+[key for key in selected if key in connected and key!="enemy"]
        for key in order:
            run(key)
            if manifest['checks'][key]['status']!='PASS':break
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as executor:
        futures=[executor.submit(run_connected)]
        futures += [executor.submit(run,key) for key in selected if key not in connected]
        for future in futures:future.result()
    assert sha(g/'generation-receipt.json')==receipt
    manifest['status']='PASS' if all(v['status']=='PASS' for v in manifest['checks'].values()) else 'FAIL'
    manifest['finished_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();save()
    print('PREBUILD '+manifest['status'],flush=True)
    return 0 if manifest['status']=='PASS' else 1
if __name__=='__main__':sys.exit(main())
