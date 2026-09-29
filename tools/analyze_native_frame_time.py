"""Fingerprint native-time hook candidates and direct callers against the locked ROM."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
TARGETS = {
    0x8002A1B8: 'model pose evaluation then time advance wrapper',
    0x800393C4: 'model channel and morph evaluation at current clip time',
    0x8002A41C: 'float to integer model seek wrapper',
    0x80028948: 'fade update',
    0x8003948C: 'model time advance',
    0x8003955C: 'model time getter',
    0x80039564: 'model time integer seek',
    0x80039578: 'model pause bit command',
    0x800395BC: 'model forward clamp bit command',
    0x80026CEC: 'framebuffer current/next equality gate',
    0x80026D40: 'scheduler task completion and framebuffer swap',
    0x80026A28: 'scheduler retrace and client broadcast',
    0x800281B8: 'process phase dispatch and integer waits',
    0x801BB730: 'scene1 phase0 callback',
    0x801BB848: 'scene1 phase1 callback',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    lock = json.loads((ROOT / 'recomp/rom.lock.json').read_text())
    digest = hashlib.sha256(rom).hexdigest()
    if digest != lock['sha256']:
        raise ValueError('ROM hash differs from locked revision')
    meta = json.loads((ROOT / 'recomp/startup.json').read_text())
    sections = [{'name': '.startup', 'rom': 0x1000, 'vram': 0x80025C00, 'compiled': meta['compiled']}] + meta['overlays']
    bodies, callers = [], []
    for section in sections:
        for key, (name, size, *_) in section['compiled'].items():
            address = int(key, 16)
            offset = section['rom'] + address - section['vram']
            data = rom[offset:offset + size]
            if len(data) != size or size % 4:
                raise ValueError('Invalid function range: ' + name)
            if address in TARGETS and (section['name'] == '.startup' or
                                       section['name'] == '.scene1' and address >= 0x801BB000):
                bodies.append({'section': section['name'], 'address': f'0x{address:08X}',
                               'function': name, 'purpose': TARGETS[address],
                               'rom_offset': offset, 'size': size,
                               'sha256': hashlib.sha256(data).hexdigest()})
            for i, word in enumerate(struct.unpack('>' + str(size // 4) + 'I', data)):
                if word >> 26 != 3:
                    continue
                target = ((address + i * 4 + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
                if target in {0x8003948C, 0x80039564, 0x80039578, 0x800395BC}:
                    callers.append({'section': section['name'], 'caller': name,
                                    'site': f'0x{address + i * 4:08X}', 'target': f'0x{target:08X}'})
    if len(bodies) != len(TARGETS):
        raise ValueError('Not all reviewed candidates have unique verified ROM bodies')
    report = {'scope': 'Static direct-call inventory of known compiled bodies; indirect/data-driven calls not covered',
              'rom_sha256': digest, 'functions': bodies, 'animation_direct_calls': callers,
              'coverage': {'time_fade_animation': 'component checked; not connected to guest state',
                           'main_gameplay': 'not converted', 'menu_2d': 'not converted',
                           'minigames': 'not converted', 'native_scheduler': 'not connected'}}
    (ROOT / 'research/native-frame-time-analysis.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'ROM verified; {len(bodies)} candidate bodies; {len(callers)} direct animation control calls mapped')


if __name__ == '__main__':
    main()
