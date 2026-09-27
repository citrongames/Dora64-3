"""Run the bounded diagnostic runtime after exact ROM validation; save metadata/log only."""
import argparse, hashlib, json, subprocess
from pathlib import Path
from inspect_rom import ROOT,read_rom

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
    report={'scope':'diagnostic startup, no frame/audio/device validation','rom_sha256':hashlib.sha256(rom).hexdigest(),
            'executable_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),'exit_code':exit_code,
            'meaning':{77:'unreviewed function reached',78:'RSP/graphics/audio boundary reached; processing not validated',79:'runtime timeout',80:'runtime error',81:'DMA verification failure',124:'runner timeout'}.get(exit_code,'unexpected termination')}
    (directory/'result.json').write_text(json.dumps(report,indent=2)+'\n')
    print((directory/'runtime.log').read_text());print(json.dumps(report,indent=2))
    if exit_code not in (77,78):raise SystemExit(1)

if __name__=='__main__':main()
