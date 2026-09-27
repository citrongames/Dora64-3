"""Optional SDK comparison against the user's existing Dora64 ROM; candidates only."""
import argparse, json, struct, tomllib
from pathlib import Path
from inspect_rom import read_rom

def normalized(data):
    result=[]
    for (word,) in struct.iter_unpack('>I',data):
        op=word>>26
        if op in (2,3): word &= 0xfc000000
        elif op in (9,13,15) or op>=32: word &= 0xffff0000
        result.append(word)
    return struct.pack('>'+str(len(result))+'I',*result)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path)
    p.add_argument('--donor',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path)
    a=p.parse_args()
    rom=read_rom(a.rom)
    donor=(a.donor/'doraemon.n64.jp.z64').read_bytes()
    baseline=json.loads((a.donor/'recomp/baseline.json').read_text())
    import hashlib
    if hashlib.sha256(donor).hexdigest()!=baseline['rom']['sha256']:raise ValueError('Donor ROM mismatch')
    metadata=tomllib.loads((a.donor/'recomp/doraemon.syms.toml').read_text())
    # The separately identified RSP boot starts at 0x3DA20; do not scan it as CPU.
    haystack=normalized(rom[0x1000:0x3da20])
    rows=[]
    for section in metadata['section']:
        for f in section.get('functions',[]):
            name,size=f['name'],f['size']
            if not name.startswith(('os','__os','al','__al','gu')) or size<24:continue
            offset=section['rom']+f['vram']-section['vram']
            needle=normalized(donor[offset:offset+size])
            hits=[];start=0
            while (pos:=haystack.find(needle,start))!=-1:
                if pos%4==0:hits.append(f'0x{pos+0x80025c00:08X}')
                start=pos+1
            if hits:rows.append({'name':name,'size':size,'targets':hits,'status':'candidate; normalized immediates and addresses; inspect before substitution'})
    a.output.write_text(json.dumps({'method':'SDK-only relocation-tolerant comparison; not game symbol transfer','matches':rows},indent=2)+'\n')
    print(json.dumps(rows,indent=2))

if __name__=='__main__':main()
