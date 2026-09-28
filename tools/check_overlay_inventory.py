"""Audit all known overlay loader ranges and every generated direct JAL destination."""
import argparse
import json
import re
import struct
from pathlib import Path
from inspect_rom import ROOT, read_rom
from overlay_layout import prepare_layout, pending_name


def check(meta, rom, generated):
    sections, compiled, sdk, frontier = prepare_layout(meta, rom)
    regions = {(s['rom'], s['vram'], s['size']): s for s in sections[1:]}
    fixed = json.loads((ROOT / 'research/layout-report-v2.json').read_text())['fixed_loads']
    rows = []
    for entry in fixed:
        expected = tuple(int(entry[k], 16) for k in ('rom_start', 'ram_start')) + (entry['size'],)
        loader = int(entry['loader'], 16)
        assert (0, loader) in compiled
        words = struct.unpack_from('>13I', rom, 0x1000 + loader - 0x80025C00)
        regs = [0] * 32
        calls = []
        for word in words:
            op, rs, rt, imm = word >> 26, (word >> 21) & 31, (word >> 16) & 31, word & 65535
            if op == 15: regs[rt] = imm << 16
            elif op == 9: regs[rt] = (regs[rs] + (imm if imm < 32768 else imm - 65536)) & 0xFFFFFFFF
            elif op == 0 and word & 63 == 35: regs[(word >> 11) & 31] = (regs[rs] - regs[rt]) & 0xFFFFFFFF
            elif op == 3: calls.append(0x80000000 | ((word & 0x3FFFFFF) << 2))
        assert calls == [0x8002730C] and (regs[4], regs[5], regs[6]) == expected
        assert expected in regions
        rows.append(dict(kind='fixed', loader=entry['loader'], section=regions[expected]['name'], rom=expected[0], ram=expected[1], size=expected[2]))
    # Loader801E4774 uses three pair tables. The third ends before byte-order data00010203.
    for address, word in {0x801E47A0:0x8C84C020, 0x801E47AC:0x8CC6C024, 0x801E47D4:0x8C84C0A8, 0x801E47E0:0x8CC6C0AC, 0x801E4808:0x8C84C100, 0x801E4814:0x8CC6C104}.items():
        assert struct.unpack_from('>I', rom, 0x70A50 + address - 0x801B8ED0)[0] == word
    empty = []
    for slot, (table, count) in enumerate(((0x801FC020,17), (0x801FC0A8,11), (0x801FC100,10))):
        previous_end = None
        for selector in range(1, count+1):
            pair = table + (selector-1)*8
            start, end = struct.unpack_from('>2I', rom, 0x70A50 + pair - 0x801B8ED0)
            assert start <= end and (previous_end is None or start == previous_end)
            previous_end = end
            if start == end:
                empty.append(dict(slot=slot, selector=selector, rom=start))
                continue
            key = (start, 0x8021D6D0 + slot*0x10000, end-start)
            assert key in regions, key
            section = regions[key]
            assert section['requires'] == ['.scene1', '.scene_common']
            assert section['overlay_group'] == f'child_slot{slot}'
            rows.append(dict(kind='child', slot=slot, selector=selector, pair=f'0x{pair:08X}', section=section['name'], rom=start, ram=key[1], size=key[2]))
    assert struct.unpack_from('>2I', rom, 0x70A50 + 0x801FC150 - 0x801B8ED0) == (0x00010203,0x04050607)
    assert len(fixed)==14 and len(rows)==49 and len(empty)==3 and len(regions)==49
    assert {x['section'] for x in rows} == {s['name'] for s in sections[1:]}
    source = '\n'.join(f.read_text() for f in (generated/'RecompiledFuncs').glob('*.c'))
    bodies = dict(re.findall(r'RECOMP_FUNC void (\w+)\([^\n]+\n(.*?)(?=RECOMP_FUNC void |\Z)', source, re.S))
    direct_count = 0
    for (index, address), (name, size, _) in compiled.items():
        section = sections[index]
        offset = section['rom'] + address - section['vram']
        body = bodies[name]
        for (word,) in struct.iter_unpack('>I', rom[offset:offset+size]):
            if word >> 26 != 3: continue
            target = 0x80000000 | ((word & 0x3FFFFFF) << 2)
            hits = [j for j,s in enumerate(sections) if s['vram'] <= target < s['vram']+s['size']]
            required = [j for j in hits if sections[j]['name'] in section.get('requires', [])]
            dest = index if index in hits else required[0] if len(required)==1 else hits[0] if len(hits)==1 else None
            assert dest is not None, (name, hex(target))
            expected = sdk[target][0]+'_recomp' if dest==0 and target in sdk else compiled[dest,target][0] if (dest,target) in compiled else pending_name(dest,target)
            assert expected+'(rdram, ctx)' in body or f'LOOKUP_FUNC(0x{target:08X})' in body, (name, expected)
            direct_count += 1
    header = (generated/'overlay_regions.hpp').read_text()
    by_name = {s['name']:i-1 for i,s in enumerate(sections)}
    for section in sections[1:]:
        for required in section.get('requires',[]):
            assert f"{{0x{section['rom']:X}, {by_name[required]}}}" in header
    repaired = compiled[41,0x80231320]
    assert repaired[1] == 0x54 and (41,0x8023136C) not in compiled
    assert '// 0x80231370:' in bodies[repaired[0]] and 'ctx->r29 = ADD32(ctx->r29, 0X18);' in bodies[repaired[0]]
    evidence = json.loads((ROOT/'research/remaining-candidate-map.json').read_text())
    for item in evidence['added']:
        i,a,n=item['section'],int(item['address'],16),item['size'];s=sections[i]
        words=struct.unpack_from('>'+str(n//4)+'I',rom,s['rom']+a-s['vram'])
        last=max(k for k,w in enumerate(words) if w)
        assert any(k>=last-1 and (w>>26==2 or w&0xFC1FFFFF==8) for k,w in enumerate(words)),('Unreviewed function tail',i,hex(a))
    assert {a for i,a in frontier} == {0x80045520,0x800586F0,0x80058A50,0x8005F1C0,0x8005F230,0x800604E0}
    return dict(scope='All ranges from14 fixed DMA loaders and3 known child tables; not proof of all game code or execution paths',fixed_ranges=14,child_nonempty_ranges=35,empty_entries=empty,registered_ranges=49,original_functions=len(compiled),sdk_mappings=len(sdk),pending=[f'0x{a:08X}' for _,a in sorted(frontier)],generated_direct_call_checks=direct_count,regions=rows,game_launched=False)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path)
    p.add_argument('--generated',required=True,type=Path)
    p.add_argument('--output',type=Path)
    args=p.parse_args()
    report=check(json.loads((ROOT/'recomp/startup.json').read_text()),read_rom(args.rom),args.generated)
    if args.output:args.output.write_text(json.dumps(report,indent=2)+'\n')
    print(f"PASS:14 fixed+35 child ranges,3 empty entries;{report['original_functions']} originals;{report['generated_direct_call_checks']} generated JAL checks;parent guards and repaired epilogue.6 hardware boundaries remain.")

if __name__=='__main__':main()
