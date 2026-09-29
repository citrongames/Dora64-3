"""Fingerprint the reviewed paired-vertex and renderer ownership path; no ROM output."""
import argparse,hashlib,json,struct
from pathlib import Path
from inspect_rom import ROOT,read_rom
from overlay_layout import prepare_layout
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rom',required=True,type=Path);a=p.parse_args()
 rom=read_rom(a.rom);sections,compiled,_,_=prepare_layout(json.loads((ROOT/'recomp/startup.json').read_text()),rom)
 reviewed={0x80038BC8:0x90,0x80038D58:0x1E0,0x80038C58:0x50,0x80038B10:0x3C,0x80038B4C:0x7C,
  0x8003931C:0xA8,0x8003AC14:0x314,0x8002F594:0x110,0x800314E4:0x78,0x80031588:0x68,
  0x80033BA4:0xAC,0x8002787C:0x20C}
 targets=set(reviewed);calls={f'0x{x:08X}':[] for x in targets}
 for (idx,addr),(name,size,*_) in compiled.items():
  s=sections[idx];off=s['rom']+addr-s['vram']
  for i,(w,) in enumerate(struct.iter_unpack('>I',rom[off:off+size])):
   pc=addr+4*i
   if w>>26==3:
    target=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
    if target in targets:calls[f'0x{target:08X}'].append({'section':s['name'],'section_rom':s['rom'],'caller':name,'site':f'0x{pc:08X}'})
 fingerprints={}
 for addr,size in reviewed.items():
  matches=[(idx,v) for (idx,start),v in compiled.items() if start==addr and sections[idx]['rom']==0x1000]
  assert len(matches)==1 and matches[0][1][1]==size,hex(addr)
  off=addr-0x80025C00+0x1000
  fingerprints[f'0x{addr:08X}']={'size':size,'sha256':hashlib.sha256(rom[off:off+size]).hexdigest()}
 hosts=['build-deps/lib/rt64/src/hle/rt64_rsp.cpp','build-deps/lib/rt64/src/hle/rt64_rdp.cpp',
  'build-deps/lib/rt64/src/hle/rt64_rdp_tmem.cpp','build-deps/lib/rt64/src/hle/rt64_state.cpp',
  'build-deps/lib/rt64/src/hle/rt64_present_queue.cpp','build-deps/lib/rt64/src/render/rt64_texture_cache.cpp']
 report={'scope':'Reviewed compiled original functions and pinned renderer source; direct JAL inventory, not exhaustive pointer analysis',
  'rom_sha256':hashlib.sha256(rom).hexdigest(),'reviewed_bodies':fingerprints,'direct_callers':calls,
  'renderer_sha256':{f:sha(ROOT/f) for f in hosts},
  'paired_vertex_layout':{'source':'object+0x20','count':'object+0x30','stride':16,'banks':['object+0x6C','object+0x70'],'bank_index':'0x801A9544'},
  'renderer_boundary':'RSP vertex memcpy and TMEM upload-owned vector; texture LoadOperations may reread RDRAM at FullSync inside send_dl; keep source alive for entire send_dl',
  'framebuffer_classification':{'0x8002F594':'RDP depth fill command builder, not CPU pixel writer','0x800314E4':'VI setup/bank pointers/heap init, not pixel clear','0x80031588':'pointer/dimension defaults, not pixel clear'},
  'limits':['No GPU execution','No proof of all CPU framebuffer writers','51 paired releases is a synthetic schedule, not observed gameplay','Retirement during pending parse still unresolved; no free-policy patch applied']}
 (ROOT/'research/graphics-ownership-audit.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS: reviewed',len(reviewed),'original bodies; paired retire has',len(calls['0x80038B10']),'direct sites; renderer fingerprints saved')
if __name__=='__main__':main()
