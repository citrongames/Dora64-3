"""Check cave overlay identities and generated call destinations."""
import argparse
import json
import re
import struct
from pathlib import Path
from inspect_rom import ROOT, read_rom
from overlay_layout import prepare_layout


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom', required=True, type=Path)
    p.add_argument('--generated', required=True, type=Path)
    args = p.parse_args()
    rom = read_rom(args.rom)
    meta = json.loads((ROOT / 'recomp/startup.json').read_text())
    evidence = json.loads((ROOT / 'research/cave-candidate-map.json').read_text())
    sections, compiled, sdk, frontier = prepare_layout(meta, rom)
    by_name = {s['name']: i for i, s in enumerate(sections)}
    generated = '\n'.join(f.read_text() for f in (args.generated / 'RecompiledFuncs').glob('*.c'))
    bodies = dict(re.findall(r'RECOMP_FUNC void (\w+)\([^\n]+\n(.*?)(?=RECOMP_FUNC void |\Z)', generated, re.S))
    for slot, expected in enumerate(evidence['sections']):
        s = sections[by_name[expected['name']]]
        for key in ('rom', 'vram', 'size', 'overlay_group'):
            assert s[key] == expected[key], key
        pair = int(expected['evidence']['pair_address'], 16)
        assert struct.unpack_from('>2I', rom, 0x70A50 + pair - 0x801B8ED0) == (s['rom'], s['rom'] + s['size'])
        demo = sections[by_name[f'.demo_slot{slot}']]
        assert demo['vram'] == s['vram'] and demo['rom'] != s['rom']
        assert demo['overlay_group'] == s['overlay_group'] == f'child_slot{slot}'
        assert s['requires'] == ['.scene1', '.scene_common']
        # Both variants must keep separate native entrypoints at the same RAM address.
        assert compiled[by_name[demo['name']], s['vram']][0] != compiled[by_name[s['name']], s['vram']][0]
    for item in evidence['added']:
        index, address = item['section'], int(item['address'], 16)
        name, size, _ = compiled[index, address]
        assert size == item['size']
        body = bodies[name]
        for target_hex in item['calls']:
            target = int(target_hex, 16)
            hits = [i for i, s in enumerate(sections) if s['vram'] <= target < s['vram'] + s['size']]
            required = [i for i in hits if sections[i]['name'] in sections[index].get('requires', [])]
            dest = index if index in hits else required[0] if len(required) == 1 else hits[0] if len(hits) == 1 else None
            assert dest is not None, (index, target_hex)
            expected_name = sdk[target][0] if dest == 0 and target in sdk else compiled[dest, target][0]
            assert expected_name + '(rdram, ctx)' in body or 'LOOKUP_FUNC(' + target_hex + ')' in body, (name, expected_name)
    assert (12, 0x8023E410) in compiled
    assert compiled[11, 0x80230310][1] == 0x180
    assert (11, 0x80230480) not in compiled
    header = (args.generated / 'overlay_regions.hpp').read_text()
    for rom_start in ('EBAC0', '142670', '150C60'):
        assert '{0x' + rom_start + ', 2}' in header
    print(f'PASS: 3 cave variants isolated from demo; 117 new bodies and generated calls; 3 parent-context references; repaired tail; {len(frontier)} pending boundaries. No game launch.')


if __name__ == '__main__':
    main()
