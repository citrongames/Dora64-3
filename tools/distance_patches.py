"""Exact-ROM draw distance/storage patches. Generated game C is never edited.

The reviewed manifest includes indexed resource/actor roots missed before068.
When deriving new evidence, account for actual GPR destinations: SLL/SRA/SLT
overwrite their destination, while stores and MTC1 only read their first GPR.
Keeping a stale pre-shift constant can hide a relocated cache lookup.
"""
import json,hashlib,struct
from pathlib import Path
def reviewed(sections,compiled,rom):
    path=Path(__file__).resolve().parents[1]/'research/draw-distance-rom-evidence.json'
    evidence=json.loads(path.read_text())
    if hashlib.sha256(rom).hexdigest()!=evidence['rom_sha256']:raise ValueError('Unreviewed distance ROM')
    for f in evidence['functions']:
        index=next(i for i,s in enumerate(sections) if s['name']==f['section']);s=sections[index]
        if s['rom']!=f['section_rom'] or tuple(compiled[(index,f['vram'])][:2])!=(f['name'],f['size']):
            raise ValueError('Unreviewed distance function boundary')
        offset=s['rom']+f['vram']-s['vram']
        if hashlib.sha256(rom[offset:offset+f['size']]).hexdigest()!=f['rom_sha256']:raise ValueError('Unreviewed distance body')
        for site,word in f['sites'].items():
            offset=s['rom']+int(site,16)-s['vram']
            if struct.unpack_from('>I',rom,offset)[0]!=int(word,16):raise ValueError('Unreviewed distance instruction')
    return evidence
def distance_instructions(sections,compiled,rom):return reviewed(sections,compiled,rom)['instructions']
def distance_hooks(sections,compiled,rom):
    for h in reviewed(sections,compiled,rom)['hooks']:yield h['func'],h['vram'],h['text']
