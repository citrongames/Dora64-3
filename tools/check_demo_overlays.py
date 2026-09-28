"""Check first attract-demo child overlays, ROM pairs and explicit parent identity."""
import argparse,json,struct
from pathlib import Path
from inspect_rom import ROOT,read_rom
from overlay_layout import prepare_layout


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--rom',required=True,type=Path)
    args=parser.parse_args();rom=read_rom(args.rom)
    meta=json.loads((ROOT/'recomp/startup.json').read_text())
    evidence=json.loads((ROOT/'research/demo-candidate-map.json').read_text())
    sections,compiled,sdk,frontier=prepare_layout(meta,rom)
    by_name={s['name']:i for i,s in enumerate(sections)}
    def scene_word(address):return struct.unpack_from('>I',rom,0x70A50+address-0x801B8ED0)[0]
    for expected in evidence['sections']:
        section=sections[by_name[expected['name']]]
        if any(section[k]!=expected[k] for k in ['rom','vram','size','requires']):raise ValueError('Overlay manifest differs')
        pair=int(expected['evidence']['pair_address'],16)
        if (scene_word(pair),scene_word(pair+4))!=(section['rom'],section['rom']+section['size']):raise ValueError('ROM pair differs')
        if section['requires']!=['.scene1']:raise ValueError('Child needs game parent')
    for address,expected in {0x801E47B0:0x3C058022,0x801E47B4:0x24A5D6D0,0x801E47B8:0x0C009CC3,
                             0x801E47E4:0x3C058023,0x801E47E8:0x24A5D6D0,0x801E47EC:0x0C009CC3,
                             0x801E4818:0x3C058024,0x801E481C:0x24A5D6D0,0x801E4820:0x0C009CC3}.items():
        if scene_word(address)!=expected:raise ValueError('DMA destination/call differs')
    for item in evidence['added']:
        index=item['section'];address=int(item['address'],16)
        if (index,address) not in compiled or compiled[index,address][1]!=item['size']:raise ValueError('Candidate body missing')
        for key in item['calls']:
            target=int(key,16)
            hits=[i for i,s in enumerate(sections) if s['vram']<=target<s['vram']+s['size']]
            required=[i for i in hits if sections[i]['name'] in sections[index].get('requires',[])]
            dest=index if index in hits else required[0] if len(required)==1 else hits[0] if len(hits)==1 else None
            if dest is None or ((dest,target) not in compiled and not (dest==0 and target in sdk)):
                raise ValueError(f'Missing dependency {index}:{address:08X} -> {key}')
    child=sections[by_name['.demo_slot0']]
    table=struct.unpack_from('>10I',rom,child['rom']+0x80224750-child['vram'])
    if not all(0x8022438C<=a<0x80224428 for a in table) or 0x80224404 not in table:raise ValueError('Switch table differs')
    if (4,0x80224404) in compiled or compiled[4,0x8022438C][1]!=0x9C:raise ValueError('False switch boundary returned')
    if (6,0x8023E9A0) not in compiled:raise ValueError('Observed missing function still absent')
    print(f'PASS:3 demo overlays;205 new bodies and references; required scene1 parent; corrected switch; {len(frontier)} project pending boundaries; no game launch')


if __name__=='__main__':main()
