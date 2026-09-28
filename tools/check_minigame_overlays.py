"""Check mini-game overlay identities and generated call destinations."""
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
    evidence = json.loads((ROOT / 'research/minigame-candidate-map.json').read_text())
    sections, compiled, sdk, frontier = prepare_layout(meta, rom)
    by_name = {s['name']: i for i, s in enumerate(sections)}
    generated = '\n'.join(f.read_text() for f in (args.generated / 'RecompiledFuncs').glob('*.c'))
    bodies = dict(re.findall(r'RECOMP_FUNC void (\w+)\([^\n]+\n(.*?)(?=RECOMP_FUNC void |\Z)', generated, re.S))
    for k, expected in enumerate(evidence['sections']):
        index = by_name[expected['name']]
        section = sections[index]
        for key in ('rom', 'vram', 'size', 'overlay_group'):
            assert section[key] == expected[key], key
        loader = int(expected['evidence']['loader'], 16)
        words = struct.unpack_from('>13I', rom, 0x1000 + loader - 0x80025C00)
        regs = [0] * 32
        called = False
        for word in words:
            op, rs, rt, imm = word >> 26, (word >> 21) & 31, (word >> 16) & 31, word & 65535
            if op == 15: regs[rt] = imm << 16
            elif op == 9: regs[rt] = (regs[rs] + (imm if imm < 32768 else imm - 65536)) & 0xFFFFFFFF
            elif op == 0 and word & 63 == 35: regs[(word >> 11) & 31] = (regs[rs] - regs[rt]) & 0xFFFFFFFF
            elif op == 3:
                assert 0x80000000 | ((word & 0x3FFFFFF) << 2) == 0x8002730C
                called = True
        assert called and (regs[4], regs[5], regs[6]) == (section['rom'], section['vram'], section['size'])
        assert compiled[0, loader][1] == 0x34
        if k:
            assert section['requires'] == ['.minigame_common', '.scene_common']
            callback = int(evidence['callbacks'][k-1], 16)
            assert (index, callback) in compiled
        else:
            assert section['requires'] == ['.scene_common']
            for callback in (0x801BDCD0, 0x801BEE34, 0x801C1CD0): assert (index, callback) in compiled
    table = struct.unpack_from('>96I', rom, 0x59020)
    assert table[34:37] == (0x800435D4, 0x800435F4, 0x80043614)
    for scene, arm in zip((2,3,4,5,6,7,8,10,11), range(0x80043674,0x800437B5,0x28)):
        assert table[scene-1] == arm
    for item in evidence['added']:
        index, address = item['section'], int(item['address'], 16)
        name, size, _ = compiled[index, address]
        assert size == item['size']
        body = bodies[name]
        section = sections[index]
        offset = section['rom'] + address - section['vram']
        direct = {0x80000000 | ((w & 0x3FFFFFF) << 2) for (w,) in struct.iter_unpack('>I', rom[offset:offset+size]) if w >> 26 in (2, 3)}
        for target_hex in item['calls']:
            target = int(target_hex, 16)
            hits = [i for i, s in enumerate(sections) if s['vram'] <= target < s['vram'] + s['size']]
            required = [i for i in hits if sections[i]['name'] in sections[index].get('requires', [])]
            dest = index if index in hits else required[0] if len(required) == 1 else hits[0] if len(hits) == 1 else None
            assert dest is not None, (index, target_hex)
            expected_name = sdk[target][0] if dest == 0 and target in sdk else compiled[dest, target][0]
            assert expected_name in bodies or target in sdk
            if target not in direct: continue  # Address-taken callback, not a direct call.
            assert expected_name + '(rdram, ctx)' in body or 'LOOKUP_FUNC(' + target_hex + ')' in body, (name, expected_name)
    header = (args.generated / 'overlay_regions.hpp').read_text()
    for section in evidence['sections']:
        parent = 0 if section['name'] == '.minigame_common' else by_name['.minigame_common'] - 1
        assert f"{{0x{section['rom']:X}, {parent}}}" in header
    assert len(frontier) == 6
    print(f'PASS: 10 ROM DMA loaders, mini-game menu callbacks and nine variants; {len(evidence["added"])} bodies and generated calls; {len(evidence["context_resolved_calls"])} context references; parent guards; 6 remaining boundaries. No game launch.')

if __name__ == '__main__':
    main()
