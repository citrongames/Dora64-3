"""Read-only ROM identity and structural survey; no extracted assets in the report."""
import argparse
import hashlib
import json
import re
import struct
import unicodedata
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read_rom(path):
    lock = json.loads((ROOT / 'recomp/rom.lock.json').read_text(encoding='utf-8'))
    if path.stat().st_size != lock['size']:
        raise ValueError('Unsupported ROM size; expected the locked 16 MiB Japanese z64.')
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != lock['sha256']:
        raise ValueError('Unsupported ROM hash; input is never patched or normalized silently.')
    return data


def checksum_6102(data):
    # Standard CIC-6102 checksum over ROM [0x1000, 0x101000), modulo 2**32.
    mask = 0xffffffff
    t1 = t2 = t3 = t4 = t5 = t6 = 0xf8ca4ddc
    for (word,) in struct.iter_unpack('>I', data[0x1000:0x101000]):
        total = (t6 + word) & mask
        if total < t6:
            t4 = (t4 + 1) & mask
        t6 = total
        t3 ^= word
        shift = word & 31
        rotated = ((word << shift) | (word >> ((32 - shift) & 31))) & mask
        t5 = (t5 + rotated) & mask
        t2 ^= rotated if t2 > word else t6 ^ word
        t1 = (t1 + (t5 ^ word)) & mask
    return [f'{t6 ^ t4 ^ t3:08X}', f'{t5 ^ t2 ^ t1:08X}']


def inspect(data):
    title = data[0x20:0x34].decode('shift_jis').rstrip()
    checksums = [f'{v:08X}' for v in struct.unpack_from('>II', data, 0x10)]
    computed = checksum_6102(data)
    strings = []
    for match in re.finditer(rb'[\x20-\x7e]{12,}', data):
        if match.group().startswith((b'RSP Gfx ucode', b'RSP SW Version')):
            strings.append({'rom_offset':f'0x{match.start():08X}', 'text':match.group().decode()})
    signatures = {}
    for sig in [b'MIO0', b'Yay0', b'Yaz0', b'RNC\x01', b'RNC\x02', b'\x1f\x8b\x08', b'PK\x03\x04']:
        signatures[sig.hex()] = [f'0x{m.start():08X}' for m in re.finditer(re.escape(sig), data)]
    blocks = []
    for start in range(0x1000, len(data), 0x10000):
        part = data[start:min(start+0x10000,len(data))]
        words = [w[0] for w in struct.iter_unpack('>I', part)]
        prologues = sum((w & 0xffff8000) == 0x27bd8000 for w in words)
        returns = words.count(0x03e00008)
        if prologues > 10 or returns > 10:
            blocks.append({'rom_start':f'0x{start:08X}', 'rom_end':f'0x{start+len(part):08X}',
                           'stack_adjust_candidates':prologues,'jr_ra_words':returns})
    return {'schema':1,'size':len(data),
            'hashes':{name:hashlib.new(name,data).hexdigest() for name in ['sha256','sha1','md5']},
            'file_crc32':f'{zlib.crc32(data):08X}', 'byte_order':'big-endian z64',
            'title_shift_jis':title,'title_nfkc':unicodedata.normalize('NFKC',title),
            'cartridge_id':data[0x3b:0x3f].decode(), 'region_byte':f'0x{data[0x3e]:02X}',
            'revision_byte':data[0x3f],'entrypoint':f'0x{struct.unpack_from(">I",data,8)[0]:08X}',
            'ipl3_crc32':f'{zlib.crc32(data[0x40:0x1000]):08X}',
            'header_checksums':checksums,'computed_6102_checksums':computed,
            'checksum_matches':checksums==computed, 'microcode_strings':strings,
            'compression_magic_hits':signatures,'code_density_candidates':blocks,
            'limitations':['Strings do not establish active microcodes or exact code boundaries.',
                            'No magic hits does not prove that assets or overlays are uncompressed.',
                            'Code-density candidates are not executable sections or a function map.']}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom',type=Path,required=True)
    parser.add_argument('--output',type=Path)
    args=parser.parse_args()
    result=json.dumps(inspect(read_rom(args.rom)),ensure_ascii=False,indent=2)+'\n'
    if args.output:
        # Never overwrite the input or an earlier report.
        with args.output.open('x',encoding='utf-8') as stream:stream.write(result)
    else:print(result,end='')


if __name__=='__main__':main()
