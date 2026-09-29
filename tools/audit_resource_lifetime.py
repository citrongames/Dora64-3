"""ROM-qualified direct-call inventory for resource/VI/raw-RDP lifetime review."""
import argparse,collections,hashlib,json,struct
from pathlib import Path
from inspect_rom import ROOT,read_rom
from overlay_layout import prepare_layout
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rom',required=True,type=Path);a=p.parse_args()
 rom=read_rom(a.rom);meta=json.loads((ROOT/'recomp/startup.json').read_text());sections,compiled,sdk,frontier=prepare_layout(meta,rom)
 targets={0x80030C44:'resource_release',0x80033C50:'heap_free_wrapper',0x80033BA4:'heap_free',
  0x80043390:'scene_memory_clear',0x800586F0:'raw_dp_submit',0x800595F0:'current_framebuffer',
  0x80059630:'next_framebuffer',0x800340E8:'compressed_resource_load',0x800314E4:'video_and_heap_init'}
 calls=collections.defaultdict(list)
 for (idx,addr),(name,size,*_) in compiled.items():
  s=sections[idx];off=s['rom']+addr-s['vram']
  for i,(word,) in enumerate(struct.iter_unpack('>I',rom[off:off+size])):
   if word>>26==3:
    target=((addr+4)&0xF0000000)|((word&0x3ffffff)<<2)
    if target in targets:calls[targets[target]].append({'section':s['name'],'section_rom':s['rom'],'site':f'0x{addr+4*i:08X}','function':name})
 reviewed={0x80030AE8:0x15C,0x80030C44:0x7C,0x80030D00:0xBC,0x80038B10:0x3C,
  0x80038B4C:0x7C,0x80026CEC:0x54,0x80026E24:0x108,0x8002FE20:0x144,0x80043390:0x40}
 fingerprints={f'0x{addr:08X}':{'size':size,'sha256':hashlib.sha256(rom[0x1000+addr-0x80025C00:0x1000+addr-0x80025C00+size]).hexdigest()} for addr,size in reviewed.items()}
 report={'scope':'All configured original bodies/direct JAL sites; not proof of indirect call reachability or all memory writers',
  'rom_sha256':hashlib.sha256(rom).hexdigest(),'original_bodies':len(compiled),'overlay_regions':len(sections)-1,
  'calls':dict(calls),'reviewed_bodies':fingerprints,
  'findings':{'resource_registry':{'address':'0x801AEC00','entries':512,'stride':20,'expiry_ticks':16},
   'deferred_free_registry':{'address':'0x800DD420','entries':100,'stride':8,'expiry_ticks':16},
   'expiry_clock':'Both tick routines run from retrace branch before outstanding graphics task check',
   'raw_rdp':'Only direct caller: scheduler 0x80026EF0; ordinary graphics producer uses flags 0x63; indirect reachability not excluded'},
  'open':['Timer expiry is not a host parse/GPU fence','Other CPU framebuffer producers and dynamic DL/texture reference graph not fully mapped']}
 (ROOT/'research/resource-lifetime-audit.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS: inventoried',len(compiled),'bodies;',len(sections)-1,'overlay regions;',dict((k,len(v)) for k,v in calls.items()))
if __name__=='__main__':main()
