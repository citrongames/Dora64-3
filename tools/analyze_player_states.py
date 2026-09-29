"""Recover the player callback table as metadata, not distributable ROM data."""
import argparse,hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rom',type=Path,required=True)
    rom=p.parse_args().rom.read_bytes();digest=hashlib.sha256(rom).hexdigest()
    if digest!=json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']:raise ValueError('Unreviewed ROM')
    m=json.loads((ROOT/'recomp/startup.json').read_text());s=next(s for s in m['overlays'] if s['name']=='.scene1')
    base=0x801FA6B0;rows=[];unique={}
    for state in range(0x44):
        pair=struct.unpack_from('>II',rom,s['rom']+base-s['vram']+state*8)
        names=[]
        for address in pair:
            name,size,*_=s['compiled'][f'0x{address:08X}'];names.append(name)
            data=rom[s['rom']+address-s['vram']:s['rom']+address-s['vram']+size]
            direct=[];indirect=[]
            for j,(w,) in enumerate(struct.iter_unpack('>I',data)):
                pc=address+4*j
                if w>>26==3:direct.append({'site':f'0x{pc:08X}','target':f'0x{((pc+4)&0xF0000000)|((w&0x3FFFFFF)<<2):08X}'})
                if w>>26==0 and (w&63)==9:indirect.append(f'0x{pc:08X}')
            unique[f'0x{address:08X}']={'name':name,'size':size,'sha256':hashlib.sha256(data).hexdigest(),'direct_calls':direct,'indirect_calls':indirect}
        rows.append({'state':f'0x{state:02X}','table_address':f'0x{base+state*8:08X}',
            'first':f'0x{pair[0]:08X}','second':f'0x{pair[1]:08X}','handlers':names,
            'timing_conversion':'not_implemented','gameplay_role':'not_classified'})
    # Verify the observed contiguous table boundary; this alone is not a range
    # proof for every state setter or a semantic classification of the states.
    following=struct.unpack_from('>II',rom,s['rom']+base-s['vram']+0x44*8)
    if any(f'0x{x:08X}' in s['compiled'] for x in following):raise ValueError('Table boundary changed')
    report={'scope':'Verified pointer pairs and compiled handler bodies; timing and state semantics are NOT converted',
        'rom_sha256':digest,'script_sha256':sha(Path(__file__)), 'source_layout_sha256':sha(ROOT/'recomp/startup.json'),
        'section':s['name'],'table':'0x801FA6B0','dispatcher':'0x801C8B28','rows':rows,'unique_handlers':unique,
        'summary':{'states':len(rows),'unique_handlers':len(unique)},
        'limitations':['Table boundary is supported by contiguous valid pointers and following non-code data; all dynamic state writers still need review',
                       'State callbacks mix commands, counters and continuous motion; no blanket elapsed multiplier is implied',
                       'Other actors and minigames have separate dispatchers']}
    (ROOT/'research/player-state-inventory.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['summary']))
if __name__=='__main__':main()
