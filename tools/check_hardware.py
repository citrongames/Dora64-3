"""Compile/run hardware components only; no game frontend or graphics device."""
from decomp_annotations import read_decompiled_source
import argparse, hashlib, json, re, struct, subprocess
from pathlib import Path
from inspect_rom import ROOT, read_rom

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--rom',type=Path,required=True)
p.add_argument('--generated',type=Path,required=True)
a=p.parse_args();rom=read_rom(a.rom);generated=a.generated.resolve()
out=ROOT/'build-local/hardware-check';out.mkdir(exist_ok=True)

# Independent sequential bitstream decoding, not a transcription of MIPS shifts.
def resident(address,size):return rom[0x1000+address-0x80025C00:0x1000+address-0x80025C00+size]
buffer=bytearray(4320);bits=0;source=rom[0xB70:]
for group,(first,count) in enumerate(zip(resident(0x80076FB8,8)[::2],resident(0x80076FB8,8)[1::2])):
    if group==3:source=resident(0x80076A80,0x538);bits=0
    for glyph in range(count):
        for row in range(14):
            value=0
            for i in range(13):
                value=(value<<1)|((source[bits//8]>>(7-bits%8))&1);bits+=1
            struct.pack_into('>H',buffer,(first+glyph)*32+4+row*2,value)
        bits+=2
buffer[3488:4288]=buffer[2240:3040]
buffer[3968:4128]=buffer[2880:3040]
masks=resident(0x80076FC0,16)
for glyph in range(124,149):
    base=8 if glyph>=144 else 0
    for row in range(4):
        pos=3488+(glyph-124)*32+row*2
        value=struct.unpack_from('>H',buffer,pos)[0]
        signed=lambda x:x if x<128 else x-256
        struct.pack_into('>H',buffer,pos,((value|signed(masks[base+row]))&signed(masks[base+row+4]))&65535)
(out/'font-reference.bin').write_bytes(buffer)
runtime=ROOT/'build-deps/lib/N64ModernRuntime'
includes=[runtime/name for name in ['librecomp/include','librecomp/include/librecomp','ultramodern/include','N64Recomp/include','thirdparty/concurrentqueue','thirdparty']]+[ROOT/'src',generated,generated/'RecompiledFuncs']
flags=['-O2','-g','-pthread','-ffunction-sections','-fdata-sections',*['-I'+str(x) for x in includes]]
subprocess.run(['python3',str(ROOT/'tools/prepare_runtime_timing.py'),str(out/'runtime')],check=True)
sources=[ROOT/'research/hardware-check.cpp',ROOT/'src/n64_hardware.cpp',ROOT/'src/n64_hardware_backend.cpp',
         runtime/'ultramodern/src/input.cpp',out/'runtime/mesgqueue.cpp',out/'runtime/scheduling.cpp']
pattern=r'RECOMP_FUNC void dora3_pak_font_decode\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)'
matches=[m for path in (generated/'RecompiledFuncs').glob('*.c') for m in re.findall(pattern,read_decompiled_source(path),re.S)]
assert len(matches)==1
# Extract this generated function verbatim to avoid unrelated game code in harness.
font=out/'font.c';font.write_text('#include "startup_hooks.h"\n'+matches[0])
subprocess.run(['gcc','-std=c17',*flags,'-c',str(font),'-o',str(out/'font.o')],check=True)
subprocess.run(['g++','-std=c++20',*flags,*map(str,sources),str(out/'font.o'),'-Wl,--gc-sections','-o',str(out/'hardware-check')],check=True)
subprocess.run([str(out/'hardware-check'),str(a.rom.resolve()),str(out/'font-reference.bin'),str(out/'result.txt')],check=True)
receipt={'scope':'Hardware component checks; no game or renderer launch; graphics device validation remains manual',
         'result':(out/'result.txt').read_text().strip(),'rom_sha256':hashlib.sha256(rom).hexdigest(),
         'font_bytes':len(buffer),'font_sha256':hashlib.sha256(buffer).hexdigest(),
         'sources':{str(x.relative_to(ROOT)):hashlib.sha256(x.read_bytes()).hexdigest() for x in sources[:3]+[ROOT/'tools/check_hardware.py',ROOT/'tools/prepare_runtime_timing.py']}}
(ROOT/'research/hardware-check.json').write_text(json.dumps(receipt,indent=2)+'\n')
