"""Validate player-state dispatch and coverage of the remaining scene1 code batch."""
import argparse,json,struct
from pathlib import Path
from inspect_rom import ROOT,read_rom
from overlay_layout import prepare_layout


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--rom',required=True,type=Path)
    args=parser.parse_args();rom=read_rom(args.rom)
    meta=json.loads((ROOT/'recomp/startup.json').read_text());evidence=json.loads((ROOT/'research/gameplay-core-candidate-map.json').read_text())
    sections,compiled,sdk,frontier=prepare_layout(meta,rom)
    def word(address):return struct.unpack_from('>I',rom,0x70A50+address-0x801B8ED0)[0]
    expected={0x801C85A4:0x24020043,0x801C85A8:0xACA20094,
              0x801C8EF4:0x8C420094,0x801C8EF8:0x000210C0,
              0x801C8EFC:0x3C018020,0x801C8F04:0x8C22A6B0,
              0x801C8F08:0x0040F809,0x801FA8C8:0x801C85C8}
    for address,value in expected.items():
        if word(address)!=value:raise ValueError(f'Player-state dispatcher changed at{address:08X}')
    if (3,0x801C85C8) not in compiled:raise ValueError('Observed missing callback not compiled in scene1')
    if len(sections[3]['compiled'])!=1303:raise ValueError('Main block candidate count changed; review coverage')
    for item in evidence['added']:
        index=item['section'];address=int(item['address'],16)
        if (index,address) not in compiled or compiled[index,address][1]!=item['size']:raise ValueError('Candidate missing/extent changed')
        for key in item['calls']:
            target=int(key,16)
            hits=[i for i,s in enumerate(sections) if s['vram']<=target<s['vram']+s['size']]
            dest=index if index in hits else hits[0] if len(hits)==1 else None
            if dest is None or ((dest,target) not in compiled and not (dest==0 and target in sdk)):
                raise ValueError(f'Missing/ambiguous batch dependency {index}:{address:08X}->{key}')
    print(f'PASS: original state43 dispatch;1303 main-block candidates;598 additions and references; {len(frontier)} project pending boundaries; no game launch')


if __name__=='__main__':main()
