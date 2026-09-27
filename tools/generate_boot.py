"""Recompile only the verified entry sequence; this does not generate a playable port."""
import argparse, hashlib, json, subprocess
from pathlib import Path
from inspect_rom import ROOT, read_rom


def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom',required=True,type=Path)
    parser.add_argument('--output-dir',type=Path,default=ROOT/'build-local/boot')
    parser.add_argument('--kind',choices=['boot','decoder'],default='boot')
    args=parser.parse_args()
    rom=args.rom.resolve(strict=True)
    read_rom(rom)
    tool_root=ROOT/'build-tools/n64recomp'
    lock_path=ROOT/'tools/n64recomp/lock.json'
    lock=json.loads(lock_path.read_text())
    receipt=json.loads((tool_root/'build-receipt.json').read_text())
    if receipt['upstream_commit']!=lock['commit'] or receipt['patch_sha256']!=lock['patch_sha256']:
        raise ValueError('Tool receipt does not match the project lock')
    binary=Path(receipt['executables']['N64Recomp']['path'])
    if digest(binary)!=receipt['executables']['N64Recomp']['sha256']:
        raise ValueError('Recompiler binary hash mismatch')
    out=args.output_dir.resolve()
    if not out.is_relative_to((ROOT/'build-local').resolve()):
        raise ValueError('Generated game code must stay under ignored build-local/')
    out.mkdir(parents=True,exist_ok=False)
    syms=ROOT/f'recomp/{args.kind}.syms.toml'
    # Symbol-input entrypoint denotes ROM 0x1000, not an arbitrary probe function.
    config='[input]\n'+('entrypoint = 0x80025C00\n' if args.kind=='boot' else '')
    for key,path in [('symbols_file_path',syms),('rom_file_path',rom),('output_func_path',out/'RecompiledFuncs')]:
        config+=f'{key} = {json.dumps(path.as_posix())}\n'
    if args.kind=='decoder':
        config+='recomp_include = '+json.dumps('#include "probe_recomp.h"')+'\n\n[patches]\nignored = ["read_rom_dma"]\n'
    (out/'boot.toml').write_text(config)
    with (out/'N64Recomp.log').open('w') as log:
        subprocess.run([str(binary),str(out/'boot.toml')],cwd=out,stdout=log,stderr=subprocess.STDOUT,check=True)
    inputs=['recomp/rom.lock.json',f'recomp/{args.kind}.syms.toml','tools/n64recomp/lock.json',
            'tools/n64recomp/zero-register.patch','tools/generate_boot.py','tools/inspect_rom.py']
    report={'scope':args.kind+'-only; no renderer, audio or game loop',
            'rom_sha256':digest(rom),'tool_sha256':digest(binary),
            'inputs':{p:digest(ROOT/p) for p in inputs},
            'generated':{p.relative_to(out).as_posix():digest(p) for p in sorted((out/'RecompiledFuncs').glob('*')) if p.is_file()}}
    (out/'generation-receipt.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out)


if __name__=='__main__':main()
