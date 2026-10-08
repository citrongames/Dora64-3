"""ROM-qualified HP and inventory hooks; no generated C is edited manually."""
import hashlib
import struct
from pathlib import Path
import json


def cheat_hooks(sections,compiled,rom):
    root=Path(__file__).resolve().parents[1]
    evidence=json.loads((root/'research/cheats-rom-evidence.json').read_text())
    if hashlib.sha256(rom).hexdigest()!=evidence['rom_sha256']:
        raise ValueError('Unreviewed cheats ROM')
    health=evidence['health_table']
    offset=health['rom_offset'];size=health['stride']*health['slots']
    if hashlib.sha256(rom[offset:offset+size]).hexdigest()!=health['sha256']:
        raise ValueError('Unreviewed character maximum HP table')
    scene=next(s for s in sections if s['name']=='.scene1')
    for item in evidence['items']:
        pointer=struct.unpack_from('>I',rom,int(item['name_table_rom'],16))[0]
        start=scene['rom']+pointer-scene['vram']
        if pointer!=int(item['name_pointer'],16) or rom[start:rom.index(b'\0',start)].decode('cp932')!=item['name']:
            raise ValueError('Unreviewed inventory item identity')
    code={
        0x8002787C:'dora3_cheats_tick(rdram);',
        0x801B50F4:'dora3_cheats_counter_store(rdram, ctx);',
        0x801B5178:'dora3_cheats_counter_get(rdram, ctx);',
        0x801C0738:'dora3_cheats_health_store(rdram, ctx);',
        0x801C9364:'dora3_cheats_tick(rdram);',
    }
    for function in evidence['functions']:
        index=next(i for i,s in enumerate(sections) if s['name']==function['section'])
        section=sections[index];address=function['vram'];size=function['size']
        if section['rom']!=function['section_rom'] or tuple(compiled[(index,address)][:2])!=(function['name'],size):
            raise ValueError('Unreviewed cheats function boundary')
        offset=section['rom']+address-section['vram']
        if hashlib.sha256(rom[offset:offset+size]).hexdigest()!=function['sha256']:
            raise ValueError('Unreviewed cheats function body')
        for site,word in function['sites'].items():
            site=int(site,16);offset=section['rom']+site-section['vram']
            if struct.unpack_from('>I',rom,offset)[0]!=int(word,16):
                raise ValueError('Unreviewed cheats instruction')
            yield function['name'],site,code[address]
