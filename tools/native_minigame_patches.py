"""ROM-qualified Source arithmetic capture for the complete mini-game block.

Only explicit reviewed sites emit hooks. This module does not identify roles
from arbitrary stores or treat an inventory as a validated Native conversion.
"""
from collections import defaultdict
import hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DECLARATIONS='''extern void dora3_native_minigame_source(uint8_t*,recomp_context*,uint32_t,uint32_t,unsigned);
extern void dora3_native_minigame_instruction(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t,unsigned,uint32_t);
extern void dora3_native_minigame_call_return(uint8_t*,recomp_context*,uint32_t,uint32_t,unsigned);
extern void dora3_native_minigame_ground(uint8_t*,recomp_context*,uint32_t,unsigned);
extern void dora3_native_minigame_indirect(uint8_t*,recomp_context*,uint32_t,unsigned);
extern void dora3_native_minigame_counter(uint8_t*,recomp_context*,uint32_t,double);
extern void dora3_native_minigame_parameter(uint8_t*,recomp_context*);
extern int dora3_native_minigame_drawing(void);
extern double dora3_native_minigame_visual(uint8_t*,uint32_t);'''

def minigame_hooks(sections,compiled,rom):
    data=json.loads((ROOT/'research/native-minigame-reviewed.json').read_text())
    byname={s['name']:(i,s) for i,s in enumerate(sections)};hooks=defaultdict(list)
    declared=set()
    def body(record):
        i,s=byname[record['section']];pc=record.get('update',record.get('function_address'))
        name,size,*_=compiled[i,pc]
        actual=rom[s['rom']+pc-s['vram']:s['rom']+pc-s['vram']+size]
        if name!=record['function'] or hashlib.sha256(actual).hexdigest()!=record['function_sha256'] or s['rom']!=record['rom']:
            raise ValueError('Unreviewed minigame ROM/function identity')
        if name not in declared:
            hooks[name,pc].append(DECLARATIONS);declared.add(name)
        return name,s,pc,size
    for row in data['callbacks']:
        name,s,pc,size=body(row)
        hooks[name,pc].append(f'dora3_native_minigame_source(rdram,ctx,0x{s["rom"]:X},0x{pc:X},0);')
        for site in row['returns']:
            if not pc<=site<pc+size or struct.unpack_from('>I',rom,s['rom']+site-s['vram'])[0]!=0x03E00008:
                raise ValueError('Unreviewed minigame return')
            hooks[name,site].append(f'dora3_native_minigame_source(rdram,ctx,0x{s["rom"]:X},0x{pc:X},1);')
    for row in data['motion_sites']:
        name,s,pc,size=body(row);site=row['site'];word=row['word'];role=row['role']
        if not pc<=site<pc+size or role not in range(5) or struct.unpack_from('>I',rom,s['rom']+site-s['vram'])[0]!=word:
            raise ValueError('Unreviewed minigame motion instruction')
        predicates=[]
        for condition in row.get('command_if',[]):
            address,width,equal=condition['address'],condition['width'],condition['equal']
            if width not in (1,2,4) or address%width or not 0x80000000<=address<=0x80400000-width:
                raise ValueError('Unreviewed minigame command predicate')
            accessor={1:'MEM_BU',2:'MEM_HU',4:'MEM_W'}[width]
            predicates.append(f'({accessor}(0,(int32_t)0x{address:X})=={equal})')
        role_code=f'(({" && ".join(predicates)})?0:{role})' if predicates else str(role)
        owner_register=row.get('owner_register',0)
        if not 0<=owner_register<32:raise ValueError('Unreviewed minigame owner register')
        owner=f'(uint32_t)ctx->r{owner_register}' if owner_register else '0'
        hooks[name,site].append(f'dora3_native_minigame_instruction(rdram,ctx,0x{s["rom"]:X},0x{site:X},0x{word:X},{role_code},{owner});')
        if row.get('indirect_targets'):
            if word&0xFC00003F!=9 or row['indirect_targets']!=[0x801C4D74,0x801C4AF8,0x801C46B4]:
                raise ValueError('Unreviewed indirect minigame dispatch')
            hooks[name,site].append(f'dora3_native_minigame_indirect(rdram,ctx,0x{s["rom"]:X},0);')
            hooks[name,site+8].insert(0,f'dora3_native_minigame_indirect(rdram,ctx,0x{s["rom"]:X},1);')
        if row.get('return_target'):
            target=row['return_target']
            if word>>26!=3 or (((site+4)&0xF0000000)|((word&0x3FFFFFF)<<2))!=target:
                raise ValueError('Unreviewed minigame intrinsic call')
            captured=row.get('captured_target',False)
            if not isinstance(captured,bool):raise ValueError('Unreviewed minigame callee capture')
            hooks[name,site+8].insert(0,f'dora3_native_minigame_call_return(rdram,ctx,0x{s["rom"]:X},0x{target:X},{int(captured)});')
    for row in data.get('counter_operand_sites',[]):
        name,s,pc,size=body(row);site=row['site'];word=row['word']
        if not pc<=site<pc+size or struct.unpack_from('>I',rom,s['rom']+site-s['vram'])[0]!=word or row['rate']!=-1:
            raise ValueError('Unreviewed minigame counter operand')
        hooks[name,site].insert(0,f'dora3_native_minigame_counter(rdram,ctx,0x{word:X},-1);')
    if data.get('parameter_entry'):
        name,s,pc,size=body(data['parameter_entry'])
        hooks[name,pc].append('dora3_native_minigame_parameter(rdram,ctx);')
    for row in data.get('parameter_sites',[]):
        name,s,pc,size=body(row);site=row['site']
        if not pc<=site<pc+size or struct.unpack_from('>I',rom,s['rom']+site-s['vram'])[0]!=row['word']:
            raise ValueError('Unreviewed model parameter call')
        hooks[name,site].insert(0,'dora3_native_minigame_parameter(rdram,ctx);')
    for row in data.get('terrain_sites',[]):
        name,s,pc,size=body(row)
        if row['mode'] not in (1,4):raise ValueError('Unreviewed minigame terrain mode')
        hooks[name,pc].append(f'dora3_native_minigame_ground(rdram,ctx,0x{s["rom"]:X},{row["mode"]});')
    for row in data['integer_visual_sites']:
        name,s,pc,size=body(row);site=row['site'];word=row['word'];index=row['fpr'];wide=row['wide']
        if not pc<=site-4<site<pc+size or not 0<=index<32 or not isinstance(wide,bool) or row['address']!=0x801C3650:
            raise ValueError('Unreviewed minigame visual operand')
        actual=struct.unpack_from('>I',rom,s['rom']+site-4-s['vram'])[0]
        expected=(17<<26)|(20<<21)|(index<<11)|(index<<6)|(33 if wide else 32)
        if actual!=word or actual!=expected or row['scale'] not in (-1,1):
            raise ValueError('Unreviewed minigame visual conversion')
        destination=f'ctx->f{index}.'+('d' if wide else 'fl')
        hooks[name,site].append(f'if(dora3_native_minigame_drawing()) {destination}={row["offset"]}+({row["scale"]})*dora3_native_minigame_visual(rdram,0x{row["address"]:X});')
    return hooks
