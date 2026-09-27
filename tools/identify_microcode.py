"""Compare aligned resident ROM ranges to the pinned RT64 XXH3 microcode database."""
import argparse,ctypes,ctypes.util,json,re
from pathlib import Path
from inspect_rom import ROOT,read_rom

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--rom',required=True,type=Path)
p.add_argument('--rt64',type=Path,default=ROOT/'build-deps/lib/rt64')
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();rom=read_rom(a.rom)
source=(a.rt64/'src/gbi/rt64_gbi.cpp').read_text()
library=ctypes.util.find_library('xxhash')
if not library:raise SystemExit('Install the host xxhash shared library to run this optional analysis.')
lib=ctypes.CDLL(library);lib.XXH3_64bits.argtypes=[ctypes.c_void_p,ctypes.c_size_t];lib.XXH3_64bits.restype=ctypes.c_uint64
swapped=bytearray(len(rom))
for i in range(4):swapped[i::4]=rom[3-i::4]
buffer=(ctypes.c_ubyte*len(swapped)).from_buffer(swapped)
names=['F3DEX2_FIFO_2_08','S2DEX2_FIFO_2_08','L3DEX2_FIFO_2_08','F3DEX_1_23','F3DLX_1_23','F3DLX_1_23_REJ','F3DLP_1_23_REJ','L3DEX_1_23','S2DEX_1_07']
patterns={}
for length,hashval,candidates in re.findall(r'GBISegment\{\s*(0x[\da-fA-F]+),\s*(0x[\da-fA-F]+)ULL,\s*\{([^}]+)',source):
    ids=re.findall(r'&(\w+)',candidates)
    if any(n in names for n in ids):patterns.setdefault(int(length,16),{})[int(hashval,16)]=ids
matches=[]
for length,hashes in patterns.items():
    for offset in range(0x38000,0x59e80-length+1,16):
        value=lib.XXH3_64bits(ctypes.byref(buffer,offset),length)
        if value in hashes:matches.append({'rom_offset':hex(offset),'hash_length':hex(length),'xxh3':f'{value:016X}','rt64_candidates':hashes[value]})
with a.output.open('x') as f:json.dump({'scope':'aligned resident range scan; not active-task confirmation','matches':matches},f,indent=2);f.write('\n')
print(json.dumps(matches,indent=2))
