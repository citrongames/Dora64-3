"""Recompile the audio microcode located independently in Doraemon 3."""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
from inspect_rom import ROOT,read_rom

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path)
    p.add_argument('--output-dir',required=True,type=Path)
    a=p.parse_args();rom=read_rom(a.rom);out=a.output_dir.resolve()
    if not out.is_relative_to(ROOT/'build-local'):raise ValueError('Use ignored build-local')
    meta=json.loads((ROOT/'recomp/audio-rsp.json').read_text())
    start,size=meta['text_offset'],meta['text_size']
    if hashlib.sha256(rom[start:start+size]).hexdigest()!=meta['text_sha256']:raise ValueError('Microcode identity mismatch')
    targets=struct.unpack_from('>'+str(meta['dispatch_count'])+'H',rom,meta['data_offset']+meta['dispatch_offset'])
    if any(t%4 or not meta['text_address']<=t<meta['text_address']+size for t in targets):raise ValueError('Invalid dispatch table')
    lock=json.loads((ROOT/'tools/n64recomp/lock.json').read_text())
    receipt=json.loads((ROOT/'build-tools/n64recomp/build-receipt.json').read_text())
    if receipt['upstream_commit']!=lock['commit'] or receipt['patch_sha256']!=lock['patch_sha256']:raise ValueError('Tool lock mismatch')
    binary=Path(receipt['executables']['RSPRecomp']['path'])
    if sha(binary)!=receipt['executables']['RSPRecomp']['sha256']:raise ValueError('Tool binary mismatch')
    out.mkdir(parents=True,exist_ok=False)
    config='\n'.join(f'{key} = {meta[key]}' for key in ('text_offset','text_size','text_address'))+'\n'
    config+='rom_file_path = '+json.dumps(a.rom.resolve().as_posix())+'\n'
    config+='output_file_path = '+json.dumps((out/'dora3_aspMain.cpp').as_posix())+'\n'
    config+='output_function_name = "dora3_aspMain"\nextra_indirect_branch_targets = '+json.dumps(targets)+'\n'
    (out/'audio.toml').write_text(config)
    with (out/'generation.log').open('w') as log:subprocess.run([str(binary),str(out/'audio.toml')],stdout=log,stderr=subprocess.STDOUT,check=True)
    inputs=['recomp/audio-rsp.json','recomp/rom.lock.json','tools/generate_audio_rsp.py','tools/inspect_rom.py','tools/n64recomp/lock.json','tools/n64recomp/zero-register.patch']
    report={'scope':'audio microcode generated; real task/audio not yet executed','inputs':{f:sha(ROOT/f) for f in inputs},
            'generated':{'dora3_aspMain.cpp':sha(out/'dora3_aspMain.cpp')},'tool_sha256':sha(binary),'dispatch_targets':targets}
    (out/'generation-receipt.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Audio RSP generated from verified text and 16 ROM dispatch entries')

if __name__=='__main__':main()
