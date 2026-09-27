"""Compare decoded bytes to the original MIPS decoder recompiled into native C."""
import argparse,json,subprocess,hashlib
from pathlib import Path
from inspect_rom import read_rom
from analyze_layout import decode_record

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--rom',type=Path,required=True)
p.add_argument('--probe',type=Path,required=True)
p.add_argument('--layout',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();rom=read_rom(a.rom)
records=[r for r in json.loads(a.layout.read_text())['record_survey']['records'] if r['compressed']]
chosen={0,len(records)-1,max(range(len(records)),key=lambda i:records[i]['decoded_size'])}
chosen.update(round(i*(len(records)-1)/9) for i in range(10))
result=[]
for i in sorted(chosen):
    entry=records[i];off=int(entry['offset'],16)
    reference=subprocess.run([str(a.probe.resolve()),str(a.rom.resolve()),hex(off)],check=True,stdout=subprocess.PIPE,timeout=15).stdout
    decoded=decode_record(rom,off)
    if reference!=decoded:raise RuntimeError(f"Decoder mismatch at {off:X}")
    result.append({'offset':hex(off),'size':len(decoded),'sha256':hashlib.sha256(decoded).hexdigest()})
with a.output.open('x') as f:json.dump({'matches':result,'scope':'sampled records; original decoder with synchronous DMA adapter'},f,indent=2);f.write('\n')
print(f'PASS: {len(result)} real compressed records byte-identical to native original decoder; no extracted bytes saved.')
