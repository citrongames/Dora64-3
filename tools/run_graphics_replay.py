"""Replay a private first-task snapshot against pinned RT64; never a gameplay test."""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
from inspect_rom import ROOT,read_rom

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',type=Path,required=True)
    p.add_argument('--snapshot',type=Path,required=True)
    p.add_argument('--executable',type=Path,required=True)
    p.add_argument('--run-dir',type=Path,required=True)
    a=p.parse_args();rom=read_rom(a.rom)
    snapshot=a.snapshot.resolve(strict=True)
    if not snapshot.is_relative_to(ROOT/'build-local'):raise ValueError('Snapshot must stay in ignored build-local')
    raw=snapshot.read_bytes()
    if len(raw)!=140+0x400000:raise ValueError('Capture length mismatch')
    h=struct.unpack_from('<35I',raw)
    if h[:4]!=(0x44335231,1,0x400000,0x01020304) or h[4]!=1:raise ValueError('Unsupported snapshot header')
    for field,size_field in [(8,9),(10,11)]:
        addr=h[field]&0x1fffffff;size=h[size_field]
        if addr<0x25c00 or addr+size>0x7ea80:raise ValueError('Microcode outside resident ROM mapping')
        original=bytes(raw[140+((addr+i)^3)] for i in range(size))
        if original!=rom[addr-0x24c00:addr-0x24c00+size]:raise ValueError('Captured microcode differs from verified ROM')
    out=a.run_dir.resolve()
    if not out.is_relative_to(ROOT/'build-local') or out.exists():raise ValueError('Use fresh ignored build-local run directory')
    out.mkdir(parents=True)
    exe=a.executable.resolve(strict=True)
    with (out/'replay.log').open('w') as log:
        try:
            code=subprocess.run([str(exe),str(snapshot),str(out/'rt64')],stdout=log,stderr=subprocess.STDOUT,timeout=180).returncode
        except subprocess.TimeoutExpired:code=124
    log=(out/'replay.log').read_text(errors='replace')
    report={
        'scope':'Immutable first graphics task replay; original pre-completion VI retained; no live game or visible-frame claim',
        'rom_sha256':hashlib.sha256(rom).hexdigest(),'snapshot_sha256':sha(snapshot),
        'executable_sha256':sha(exe),'exit_code':code,'log_sha256':sha(out/'replay.log'),
        'task':{'text':f'{h[8]:08X}','text_size':h[9],'data':f'{h[10]:08X}','data_size':h[11],
                'display_list':f'{h[16]:08X}','display_list_bytes':h[17],'framebuffer':f'{h[34]:08X}'},
        'rt64_setup_succeeded':'RT64 SETUP result=0' in log,
        'display_list_parsed':'RT64 DISPLAY LIST PARSED' in log,
        'audible_output_verified':False,'visible_game_frame_verified':False,
        'inputs_sha256':{f:sha(ROOT/f) for f in ['src/graphics_replay.cpp','research/graphics-replay/CMakeLists.txt','dependencies.lock.json','tools/run_graphics_replay.py']}
    }
    (out/'result.json').write_text(json.dumps(report,indent=2)+'\n')
    print(log[-8000:]);print(json.dumps(report,indent=2))
    if code or not report['display_list_parsed']:raise SystemExit(1)

if __name__=='__main__':main()
