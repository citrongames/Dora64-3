"""Run the bounded diagnostic runtime after exact ROM validation; save metadata/log only."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from inspect_rom import ROOT,read_rom
from analyze_layout import decode_record

def observations(log):
    return {
        'pending_functions':sorted(set(re.findall(r'PENDING original function ([0-9A-F]{8})',log))),
        'dma_verified':re.findall(r'DMA VERIFIED (.*)',log),
        'rsp_tasks':re.findall(r'RSP TASK (.*)',log),
        'audio_rsp_completions':len(re.findall(r'^AUDIO RSP COMPLETED reason=1$',log,re.M)),
        'graphics_submissions':re.findall(r'GRAPHICS TASK (.*)',log),
        'pcm_boundary_reached':'PCM OUTPUT reached' in log,
        'pcm_buffers':[dict(zip(('samples','frames','nonzero_samples'),map(int,m)))
                       for m in re.findall(r'PCM OUTPUT reached samples=(\d+) frames=(\d+) nonzero=(\d+)',log)],
        'live_rt64_setup':re.findall(r'LIVE RT64 SETUP result=(\d+)',log),
        'live_rt64_submissions':re.findall(r'LIVE RT64 SUBMITTED (.*)',log),
        'live_rt64_drain':re.findall(r'LIVE RT64 DRAIN (.*)',log),
        'live_rt64_tasks':re.findall(r'LIVE RT64 PARSED (.*)',log),
        'graphics_capture_saved':'GRAPHICS CAPTURE saved' in log,
    }

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',type=Path,required=True)
    p.add_argument('--executable',type=Path,required=True)
    p.add_argument('--run-dir',type=Path,required=True)
    p.add_argument('--live',action='store_true',help='Allow RT64 setup time and record live research observations')
    a=p.parse_args();rom=read_rom(a.rom)
    directory=a.run_dir.resolve()
    if not directory.is_relative_to(ROOT/'build-local') or directory.exists():raise ValueError('Use a fresh build-local run directory')
    directory.mkdir(parents=True)
    exe=a.executable.resolve(strict=True)
    with (directory/'runtime.log').open('w') as log:
        try:
            result=subprocess.run([str(exe),str(a.rom.resolve()),str(directory/'state')],stdout=log,stderr=subprocess.STDOUT,timeout=180 if a.live else 25)
            exit_code=result.returncode
        except subprocess.TimeoutExpired:
            exit_code=124
    report={'scope':'diagnostic startup; RSP completion recorded separately; no rendered frame or audio device validation','rom_sha256':hashlib.sha256(rom).hexdigest(),
            'executable_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),'native_exit_code':exit_code,'exit_code':exit_code,
            'meaning':{77:'unreviewed function reached',78:'diagnostic task/output boundary; inspect observations',79:'runtime timeout',80:'runtime error',81:'DMA verification failure',82:'audio RSP identity/execution failure',83:'private graphics capture write failure',84:'live renderer setup/task failure',85:'overlay lifecycle failure',86:'resource capture or differential verification failure',124:'runner timeout'}.get(exit_code,'unexpected termination')}
    report['scope']='bounded live RT64 startup; original scene not yet validated; PCM measured/discarded' if a.live else report['scope']
    report['mode']='live_rt64' if a.live else 'diagnostic'
    report['observations']=observations((directory/'runtime.log').read_text())
    resource_checks=[]
    for file in sorted((directory/'state/resources').glob('*.bin')):
        offset=int(file.stem,16);actual=file.read_bytes()
        try:
            expected=decode_record(rom,offset)
            match=actual==expected;error=None
        except ValueError as exc:
            match=False;error=str(exc)
        resource_checks.append(dict(rom=f'{offset:08X}',bytes=len(actual),matches=match,
                                    sha256=hashlib.sha256(actual).hexdigest(),error=error))
    report['observations']['resource_checks']=resource_checks
    report['observations']['overlay_events']=re.findall(r'^OVERLAY (.*)',(directory/'runtime.log').read_text(),re.M)
    report['observations']['stream_full_refills']=len(re.findall(r'^STREAM DMA full refill',(directory/'runtime.log').read_text(),re.M))
    if any(not check['matches'] for check in resource_checks):
        exit_code=86;report['exit_code']=86;report['meaning']='resource differs from independent decoder'
    (directory/'result.json').write_text(json.dumps(report,indent=2)+'\n')
    print((directory/'runtime.log').read_text());print(json.dumps(report,indent=2))
    if exit_code not in (77,78):raise SystemExit(1)

if __name__=='__main__':main()
