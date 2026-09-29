"""Whole known-ROM timing inventory. Metadata only; no ROM bytes/disassembly output."""
import argparse,collections,hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
TARGETS={0x800280DC:'callback_pair',0x80027FEC:'callback_slot',0x800281B8:'dispatch',
 0x80028878:'fade_command',0x800288C0:'fade_command',0x800288EC:'fade_command',0x80028914:'fade_command',0x80028930:'fade_command',0x80028948:'fade_tick',
 0x8002A1B8:'animation_pose_and_time',0x800393C4:'animation_pose',0x8003948C:'animation_time',
 0x80039564:'animation_seek',0x80039578:'animation_pause',0x800395BC:'animation_clamp',
 0x800300C4:'input_select',0x80030FA8:'input_repeat',0x800314B4:'scene_select',
 0x80030AE8:'resource_load',0x80043390:'scene_unload',0x800271B0:'cpu_framebuffer_copy',
 0x80028474:'cross_frame_copy',0x8003CD48:'rumble',0x8003D39C:'controller_pak',0x8004281C:'sound_events'}
GLOBALS={0x801A39DC:'tick_divider',0x801AC694:'VI_count',0x801AC6B4:'game_tick',0x801A48C0:'frame_built',
 0x801B1430:'deferred_build',0x801B1400:'process_transition',0x801A48E0:'render_transition',0x801AA618:'outstanding_tasks'}
def hx(v):return None if v is None else f'0x{v:08X}'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rom',required=True,type=Path);a=p.parse_args();rom=a.rom.read_bytes()
 digest=hashlib.sha256(rom).hexdigest();assert digest==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
 meta=json.loads((ROOT/'recomp/startup.json').read_text());sections=[{'name':'.startup','rom':0x1000,'vram':0x80025C00,'compiled':meta['compiled']}]+meta['overlays']
 funcs={};section_names={s['name']:s for s in sections}
 for sec in sections:
  for addr,(name,size,*_) in sec['compiled'].items():
   pc=int(addr,16);offset=sec['rom']+pc-sec['vram'];data=rom[offset:offset+size];assert len(data)==size and size%4==0
   funcs[(sec['name'],pc)]={'name':name,'words':struct.unpack('>'+str(size//4)+'I',data),'sha256':hashlib.sha256(data).hexdigest()}
 def resolve(sec,pc):
  if (sec,pc) in funcs:return(sec,pc)
  if ('.startup',pc) in funcs:return('.startup',pc)
  choices=[(s,pc) for s in section_names[sec].get('requires',[]) if(s,pc) in funcs]
  return choices[0] if len(choices)==1 else None
 pairs=[];calls=[];references=[];summary=[];edges=collections.defaultdict(set);indirect={};risks={}
 for (sec,start),f in sorted(funcs.items()):
  words=f['words'];regs={0:0};pair_pending=None;clear_after=-1;branch_targets=set();branch_end=set();counts=collections.Counter();function_risks=set()
  for i,w in enumerate(words):
   op=w>>26
   if op in {1,4,5,6,7,20,21,22,23} or op==17 and (w>>21)&31==8:
    disp=w&65535;disp=disp-65536 if disp&32768 else disp;branch_targets.add(start+4*i+4+4*disp);branch_end.add(i+2)
   elif op==2:branch_targets.add(((start+4*i+4)&0xf0000000)|((w&0x3ffffff)<<2));branch_end.add(i+2)
  for i,w in enumerate(words):
   pc=start+4*i;op=w>>26;rs=(w>>21)&31;rt=(w>>16)&31;rd=(w>>11)&31;fun=w&63;u=w&65535;imm=u-65536 if u&32768 else u
   if pc in branch_targets or i in branch_end:regs={0:0}
   # Local basic-block constants only; no joins, heap aliasing or indirect proof.
   if op==15:regs[rt]=u<<16
   elif op in {9,13}:
    val=regs.get(rs);regs[rt]=None if val is None else ((val+imm) if op==9 else(val|u))&0xffffffff
   elif op==0 and fun in {0,33,37}:
    x,y=regs.get(rs),regs.get(rt)
    regs[rd]=None if x is None or y is None else ((y<<((w>>6)&31)) if fun==0 else(x+y) if fun==33 else(x|y))&0xffffffff
   elif op in {32,33,35,36,37,39,40,41,43,49,53,57,61}:
    base=regs.get(rs);address=None if base is None else(base+imm)&0xffffffff
    if address in GLOBALS:
     kind='write' if op in {40,41,43,57,61} else'read'
     references.append({'section':sec,'function':f['name'],'site':hx(pc),'global':GLOBALS[address],'kind':kind,'value':regs.get(rt) if kind=='write' else None})
     function_risks.add(GLOBALS[address])
    if op in {40,41,43,57,61}:counts['stores']+=1
    if op in {32,33,35,36,37,39}:regs[rt]=None
   elif op==0:
    if fun==9:counts['indirect_calls']+=1
    if fun not in {8,9,12,13,16,17,18,19,24,25,26,27}:regs[rd]=None
   elif op in {8,10,11,12,14}:regs[rt]=None
   if op==17 and rs in {16,17}:counts['float_ops']+=1
   if op in {4,5,6,7,1,20,21,22,23}:counts['integer_branches']+=1
   if op==3:
    target=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2);resolved=resolve(sec,target)
    if resolved:edges[(sec,start)].add(resolved)
    if target in TARGETS:
     calls.append({'section':sec,'function':f['name'],'site':hx(pc),'target':hx(target),'category':TARGETS[target]});function_risks.add(TARGETS[target])
    pair_pending=(target,pc) if target in {0x800280DC,0x80027FEC} else None;clear_after=i+1
   elif i==clear_after:
    if pair_pending:
     target,site=pair_pending;pairs.append({'section':sec,'caller':f['name'],'site':hx(site),'kind':TARGETS[target],
      'a0':hx(regs.get(4)),'a1':hx(regs.get(5)),'a2':hx(regs.get(6)),
      'status':'local constants; validate live identity and state; unknown args remain null'})
    for reg in list(regs):
     if reg not in {0,16,17,18,19,20,21,22,23,28,29,30}:regs[reg]=None
    pair_pending=None
   regs[0]=0
  indirect[(sec,start)]=counts['indirect_calls'];risks[(sec,start)]=function_risks
  summary.append({'section':sec,'address':hx(start),'name':f['name'],'sha256':f['sha256'],**counts,'timing_categories':sorted(function_risks)})
 closures=[]
 for pair in pairs:
  if pair['kind']!='callback_pair':continue
  for phase,arg in [('update','a0'),('draw','a1')]:
   value=pair[arg];root=resolve(pair['section'],int(value,16)) if value else None
   if not root:continue
   seen=set();todo=[root]
   while todo:
    key=todo.pop()
    if key in seen:continue
    seen.add(key);todo.extend(edges[key]-seen)
   closures.append({'section':pair['section'],'install':pair['site'],'phase':phase,'callback':value,
    'direct_closure_functions':len(seen),'indirect_call_sites':sum(indirect[k] for k in seen),
    'categories':sorted(set().union(*(risks[k] for k in seen))),
    'status':'static direct closure; indirect and data-driven paths not certified'})
 stats={'functions':len(funcs),'sections':len(sections),'overlay_sections':len(sections)-1,
  'callback_installs':len(pairs),'resolved_pair_arguments':sum(p['kind']=='callback_pair' and p['a0'] is not None and p['a1'] is not None for p in pairs),
  'indirect_call_sites':sum(indirect.values()),'timing_calls':dict(collections.Counter(c['category'] for c in calls)),
  'timing_global_references':len(references)}
 report={'scope':'Inventory of every known compiled ROM body, not semantic conversion or complete recovery of indirect callbacks',
  'rom_sha256':digest,'script_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'summary':stats,
  'callback_installs':pairs,'callback_closures':closures,'timing_calls':calls,'global_references':references,'functions':summary}
 (ROOT/'research/timing-inventory.json').write_text(json.dumps(report,indent=2)+'\n')
 lines=['# Общая карта перевода времени','', 'Автоматический статический инвентарь по закреплённому ROM. Числа показывают объём найденного кода, а не процент готовности native FPS.','',
  f'Охвачено {stats["functions"]} функций в {stats["sections"]} секциях; найдено {len(pairs)} вызовов установки callbacks и {stats["indirect_call_sites"]} косвенных вызовов.','',
  '## Общие подсистемы','', '| Подсистема | Прямых вызовов |','|---|---:|']
 lines += [f'| {k} | {v} |' for k,v in stats['timing_calls'].items()]
 lines += ['', '## Секции и границы callbacks','', '| Секция | Функций | Установок callbacks | Косвенных вызовов |','|---|---:|---:|---:|']
 for sec in sections:
  name=sec['name'];lines.append(f'| {name} | {len(sec["compiled"])} | {sum(p["section"]==name for p in pairs)} | {sum(indirect[k] for k in indirect if k[0]==name)} |')
 lines += ['', '## Как использовать','',
  'Сначала общий loop: независимый графический deadline, настоящие завершения, Original/Manual, пауза и смена сцен. Затем общие системы fade, model time/pose, transform/movement, camera, events и input. Только остаточные правила относятся к отдельным сценам и минииграм.',
  '', 'Каждая найденная пара update/draw имеет прямую closure в research/timing-inventory.json. Наличие stores/float operations — признак для разбора, не доказательство зависимости от FPS. Косвенные вызовы, таблицы callbacks, aliasing и семантика событий остаются открытыми; локальные константы не заменяют контроль потока.',
  '', 'Не следует увеличивать частоту всего dispatcher: он меняет integer waits и состояния process slots. Физике нужен общий шаг времени с сохранением частоты дискретных событий; анимации — порядок pose/time и однократность crossings; вводу — разделение held/edge/repeat; загрузке — drain до освобождения ресурсов.']
 (ROOT/'docs/timing-inventory.md').write_text('\n'.join(lines)+'\n')
 print(json.dumps(stats,ensure_ascii=False))
if __name__=='__main__':main()
