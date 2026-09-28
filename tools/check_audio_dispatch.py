"""Check the ROM audio-command table against registered native function bodies."""
import argparse,json,struct
from pathlib import Path
from inspect_rom import ROOT,read_rom


def check(rom,meta,evidence):
    def word(address):return struct.unpack_from('>I',rom,address-0x80024C00)[0]
    # Both dispatch paths establish 0x80 <= opcode < 0xAD before masking.
    for address,expected in {
        0x80047868:0x2C620080,0x800478C0:0x2C4300AD,
        0x800478FC:0x3043007F,0x80047914:0x8C426FD0,0x80047920:0x0040F809,
        0x80049884:0x2C430080,0x800498A4:0x2C4300AD,
        0x800498E0:0x3043007F,0x80049900:0x8C226FD0,0x8004990C:0x0040F809,
    }.items():
        if word(address)!=expected:raise ValueError(f'Dispatch instructions changed at {address:08X}')
    targets=[f'0x{word(0x80076FD0+i*4):08X}' for i in range(45)]
    if targets!=evidence['targets']:raise ValueError('Command table differs from recorded ROM analysis')
    for key in targets:
        if key not in meta['compiled'] or key not in meta['indirect_targets']:
            raise ValueError(f'Unregistered audio command target {key}')
    for entry in evidence['new_functions']:
        key=entry['address'];address=int(key,16);size=entry['size']
        if meta['compiled'][key][1]!=size or word(address+size-8)!=0x03E00008:
            raise ValueError(f'Function extent changed {key}')
        calls=set()
        for pc in range(address,address+size,4):
            insn=word(pc)
            if insn>>26==3:calls.add(f'0x{((pc+4)&0xF0000000)|((insn&0x3FFFFFF)<<2):08X}')
        for target in calls:
            if target not in meta['compiled'] and target not in meta['sdk']:
                raise ValueError(f'Missing direct dependency {key} -> {target}')
    return len(targets),len(evidence['new_functions'])


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom',required=True,type=Path)
    args=parser.parse_args()
    result=check(read_rom(args.rom),json.loads((ROOT/'recomp/startup.json').read_text()),
                 json.loads((ROOT/'research/audio-command-dispatch.json').read_text()))
    print(f'PASS: {result[0]} original audio command targets; {result[1]} added bodies and their direct dependencies; no game launch')


if __name__=='__main__':main()
