"""Reviewed terminal-pose presentation; never modify Source clip commands."""
import hashlib
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OBJECT_END = 0x8002A3B0
MODEL_END = 0x80039544


def model_terminal_hooks(sections, compiled, rom):
    review = json.loads((ROOT / 'recomp/model-terminal-poses.json').read_text())
    if hashlib.sha256(rom).hexdigest() != review['rom_sha256']:
        raise ValueError('Unreviewed terminal-pose ROM')
    resident = sections[0]
    if (resident['name'], resident['rom'], resident['vram']) != ('.startup', 0x1000, 0x80025C00):
        raise ValueError('Unreviewed model getter resident identity')
    # After the object getter returns, a0 is the selected component's model.
    # The common wrapper and leaf getter must retain that exact register ABI.
    for address, field in ((OBJECT_END, 'object_getter_sha256'), (MODEL_END, 'model_getter_sha256')):
        entry = compiled.get((0, address))
        if entry is None:
            raise ValueError('Missing common animation end getter')
        offset = resident['rom'] + address - resident['vram']
        if hashlib.sha256(rom[offset:offset + entry[1]]).hexdigest() != review[field]:
            raise ValueError('Model end getter register contract changed')
    by_section = {s['name']: (i, s) for i, s in enumerate(sections)}
    actual = set()
    for (index, address), (name, size, *_) in compiled.items():
        section = sections[index]
        offset = section['rom'] + address - section['vram']
        for relative, (word,) in enumerate(struct.iter_unpack('>I', rom[offset:offset + size])):
            site = address + relative * 4
            if word >> 26 == 3 and (((site + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)) in (OBJECT_END, MODEL_END):
                actual.add((section['name'], address, site))
    seen = set()
    for row in review['sites']:
        index, section = by_section[row['section']]
        address, site = row['function_address'], row['site']
        key = (row['section'], address, site)
        entry = compiled.get((index, address))
        if key in seen or entry is None or tuple(entry[:2]) != (row['function'], row['function_size']) or section['rom'] != row['rom']:
            raise ValueError('Unreviewed terminal-pose function/overlay')
        seen.add(key)
        offset = section['rom'] + address - section['vram']
        if hashlib.sha256(rom[offset:offset + entry[1]]).hexdigest() != row['function_sha256']:
            raise ValueError('Terminal-pose Source owner body changed')
        offset = section['rom'] + site - section['vram']
        if not address <= site < site + 8 < address + entry[1] or struct.unpack_from('>III', rom, offset) != (row['word'], row['delay_word'], row['return_word']):
            raise ValueError('Terminal-pose query/delay/return changed')
        if row['policy'] == 'terminal':
            if (row['word'] & 0x3FFFFFF) << 2 != (OBJECT_END & 0xFFFFFFF):
                raise ValueError('Terminal-pose hook needs the object getter ABI')
            # This hook precedes the return-site instruction and any existing
            # post-call capture; a0 is still the actual queried model pointer.
            yield entry[0], site + 8, 'dora3_native_model_terminal_sample(rdram,ctx);'
        elif row['policy'] not in ('continue', 'delegate'):
            raise ValueError('Unknown terminal-pose presentation policy')
    if seen != actual:
        raise ValueError('Animation end-query coverage changed; review all new/missing owners')
