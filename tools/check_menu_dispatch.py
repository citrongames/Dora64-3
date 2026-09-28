"""Verify shared-menu scene entrypoints and section-qualified dependency coverage."""
import argparse,json,struct
from pathlib import Path
from inspect_rom import ROOT,read_rom
from overlay_layout import prepare_layout


def check(rom,meta,evidence):
    sections,compiled,sdk,frontier=prepare_layout(meta,rom)
    def resident_word(address):return struct.unpack_from('>I',rom,address-0x80024C00)[0]
    for root in evidence['roots']:
        arm=int(root['arm'],16);target=int(root['target'],16)
        if resident_word(arm)!=0x0C010C3B or resident_word(arm+8)!=0x0C010C55:
            raise ValueError('Scene arm no longer loads common + shared menu')
        hi,lo=resident_word(arm+16),resident_word(arm+20)
        decoded=((hi&65535)<<16)+struct.unpack('>h',struct.pack('>H',lo&65535))[0]
        if hi!=0x3C04801C or lo>>16!=0x2484 or decoded!=target:
            raise ValueError('Scene callback differs from analysis')
        for scene in root['scene_ids']:
            if resident_word(0x8007DC20+(scene-1)*4)!=arm:raise ValueError('Scene switch differs')
        if (2,target) not in compiled or root['target'] not in sections[2]['indirect_targets']:
            raise ValueError('Menu root missing in its own ROM section')
    # The failing initializer installs both actual callbacks through800280DC.
    for address,expected in {0x801C0D20:0x3C04801C,0x801C0D24:0x24840BBC,
                             0x801C0D28:0x3C05801C,0x801C0D2C:0x24A50C88,
                             0x801C0D30:0x0C00A037}.items():
        if struct.unpack_from('>I',rom,0x635C0+address-0x801B8ED0)[0]!=expected:
            raise ValueError('Title callbacks changed')
    for item in evidence['added']:
        index=item['section'];address=int(item['address'],16)
        if (index,address) not in compiled or compiled[index,address][1]!=item['size']:
            raise ValueError(f'Function missing or extent changed: {index}:{address:08X}')
        for key in item['calls']:
            target=int(key,16)
            matches=[i for i,s in enumerate(sections) if s['vram']<=target<s['vram']+s['size']]
            dest=index if index in matches else matches[0] if len(matches)==1 else None
            if dest is None or ((dest,target) not in compiled and not (dest==0 and target in sdk)):
                raise ValueError(f'Missing or ambiguous dependency: {index}:{address:08X} -> {key}')
    if sdk.get(0x8004E780,[''])[0]!='osEepromLongWrite':raise ValueError('Missing EEPROM write binding')
    return len(evidence['roots']),len(evidence['added']),len(frontier)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--rom',required=True,type=Path)
    args=parser.parse_args()
    roots,added,pending=check(read_rom(args.rom),json.loads((ROOT/'recomp/startup.json').read_text()),
                            json.loads((ROOT/'research/menu-candidate-map.json').read_text()))
    print(f'PASS: {roots} menu roots; {added} added functions and references resolved in correct sections; EEPROM write binding; {pending} project pending boundaries; no game launch')


if __name__=='__main__':main()
