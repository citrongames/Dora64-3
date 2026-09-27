"""Run the bounded diagnostic runtime after exact ROM validation; save metadata/log only."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from inspect_rom import ROOT,read_rom

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
        'graphics_capture_saved':'GRAPHICS CAPTURE saved' in log,
    }

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',type=Path,required=True)
    p.add_argument('--executable',type=Path,required=True)
    p.add_argument('--run-dir',type=Path,required=True)
    a=p.parse_args();rom=read_rom(a.rom)
    directory=a.run_dir.resolve()
    if not directory.is_relative_to(ROOT/'build-local') or directory.exists():raise ValueError('Use a fresh build-local run directory')
    directory.mkdir(parents=True)
    exe=a.executable.resolve(strict=True)
    with (directory/'runtime.log').open('w') as log:
        try:
            result=subprocess.run([str(exe),str(a.rom.resolve()),str(directory/'state')],stdout=log,stderr=subprocess.STDOUT,timeout=25)
            exit_code=result.returncode
        except subprocess.TimeoutExpired:
            exit_code=124
    report={'scope':'diagnostic startup; RSP completion recorded separately; no rendered frame or audio device validation','rom_sha256':hashlib.sha256(rom).hexdigest(),
            'executable_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),'exit_code':exit_code,
            'meaning':{77:'unreviewed function reached',78:'diagnostic task/output boundary; inspect observations',79:'runtime timeout',80:'runtime error',81:'DMA verification failure',82:'audio RSP identity/execution failure',83:'private graphics capture write failure',124:'runner timeout'}.get(exit_code,'unexpected termination')}
    report['observations']=observations((directory/'runtime.log').read_text())
    (directory/'result.json').write_text(json.dumps(report,indent=2)+'\n')
    print((directory/'runtime.log').read_text());print(json.dumps(report,indent=2))
    if exit_code not in (77,78):raise SystemExit(1)

if __name__=='__main__':main()
