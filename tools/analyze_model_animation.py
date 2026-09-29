"""Shared model APIs and their phase reachability over all locked known ROM code.
Metadata only. A direct closure is not proof of execution or of indirect coverage.
"""
import argparse, collections, hashlib, json, struct
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
APIS = {
 0x8002A1B8:'object_pose_then_time', 0x800393C4:'pose_channels', 0x8003948C:'advance_time',
 0x80039544:'end_pulse', 0x80039550:'duration', 0x8003955C:'time', 0x80039564:'seek_integer',
 0x80039578:'pause_command', 0x800395B0:'pause_query', 0x800395BC:'clamp_command',
 0x8003B560:'create_model', 0x8003B6BC:'bind_clip', 0x8003B744:'release_channels',
 0x8003B77C:'destroy_model', 0x80038BC8:'allocate_morph_pair', 0x80038C58:'retire_morph_pair',
 0x80038D58:'evaluate_morph_in_selected_bank', 0x80038FA8:'linear_channel',
 0x80039078:'cubic_channel', 0x8003B7D0:'object_system_reset',
 0x8002A3B0:'object_end_pulse', 0x8002A3D4:'object_duration', 0x8002A3F8:'object_time',
 0x8002A41C:'object_seek', 0x8002A454:'object_clamp', 0x8002A4A4:'object_pause',
 0x8002A170:'object_replace_clip', 0x8002A328:'object_select_clip',
}
def hx(v): return f'0x{v:08X}'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rom',required=True,type=Path);a=p.parse_args()
 rom=a.rom.read_bytes();digest=hashlib.sha256(rom).hexdigest()
 assert digest==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
 m=json.loads((ROOT/'recomp/startup.json').read_text())
 sections=[{'name':'.startup','rom':0x1000,'vram':0x80025C00,'compiled':m['compiled']}]+m['overlays']
 funcs={};secs={s['name']:s for s in sections}
 for sec in sections:
  for addr,(name,size,*_) in sec['compiled'].items():
   pc=int(addr,16);off=sec['rom']+pc-sec['vram'];data=rom[off:off+size]
   assert len(data)==size and size%4==0
   funcs[(sec['name'],pc)]={'name':name,'size':size,'sha256':hashlib.sha256(data).hexdigest(),'words':struct.unpack('>'+str(size//4)+'I',data)}
 def resolve(sec,pc):
  if(sec,pc) in funcs:return(sec,pc)
  if('.startup',pc) in funcs:return('.startup',pc)
  choices=[(s,pc) for s in secs[sec].get('requires',[]) if(s,pc) in funcs]
  return choices[0] if len(choices)==1 else None
 edges=collections.defaultdict(set);calls=[];indirect=collections.Counter()
 for key,f in funcs.items():
  sec,start=key
  for i,w in enumerate(f['words']):
   pc=start+i*4
   if w>>26==0 and w&63==9:indirect[key]+=1
   if w>>26!=3:continue
   target=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2);dest=resolve(sec,target)
   if dest:edges[key].add(dest)
   if target in APIS:calls.append({'section':sec,'function':f['name'],'function_address':hx(start),'site':hx(pc),'target':hx(target),'api':APIS[target]})
 def closure(root):
  seen=set();todo=[root]
  while todo:
   k=todo.pop()
   if k in seen:continue
   seen.add(k);todo.extend(edges[k]-seen)
  return seen
 # Recompute roots/closures from the broad inventory; verify its provenance first.
 inv=json.loads((ROOT/'research/timing-inventory.json').read_text())
 assert inv['rom_sha256']==digest
 assert inv['script_sha256']==hashlib.sha256((ROOT/'tools/analyze_timing_inventory.py').read_bytes()).hexdigest()
 phases=[];reaches=collections.defaultdict(set)
 for pair in inv['callback_installs']:
  if pair['kind']!='callback_pair':continue
  for phase,arg in [('update','a0'),('draw','a1')]:
   if not pair[arg]:continue
   root=resolve(pair['section'],int(pair[arg],16))
   if not root:continue
   seen=closure(root)
   found=sorted({APIS[pc] for sec,pc in seen if sec=='.startup' and pc in APIS})
   if not found:continue
   phases.append({'section':pair['section'],'install':pair['site'],'phase':phase,'callback':pair[arg],
    'model_apis':found,'indirect_sites':sum(indirect[k] for k in seen)})
   for k in seen:reaches[k].add(phase)
 for c in calls:c['direct_phase_reachability']=sorted(reaches[(c['section'],int(c['function_address'],16))])
 api_bodies=[{'address':hx(pc),'role':role,**{k:v for k,v in funcs[('.startup',pc)].items() if k!='words'}} for pc,role in APIS.items()]
 report={'scope':'All known code; direct calls and direct callback closures only. Indirect, alias and runtime identity remain unproven.',
  'rom_sha256':digest,'script_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
  'source_layout_sha256':hashlib.sha256((ROOT/'recomp/startup.json').read_bytes()).hexdigest(),
  'summary':{'functions':len(funcs),'sections':len(sections),'api_bodies':len(api_bodies),'direct_calls':dict(collections.Counter(c['api'] for c in calls))},
  'api_bodies':api_bodies,'direct_calls':calls,'callback_phase_closures':phases}
 (ROOT/'research/model-animation-inventory.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report['summary'],indent=2))
if __name__=='__main__':main()
