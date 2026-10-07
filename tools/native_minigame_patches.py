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
extern void dora3_native_model_terminal_pose(uint8_t*,recomp_context*,unsigned);
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
    # Existing Source capture sites implement these exact circular camera
    # recurrences in native_minigames.cpp. Pin their role/body/store identity;
    # no additional instruction/dispatcher execution is generated here.
    circular={('.minigame_1',0x801C578C,0x801C58E8,0xE616001C,True),
              ('.minigame_1',0x801C5908,0x801C5A84,0xE614001C,False),
              ('.minigame_2',0x801C51B0,0x801C530C,0xE616001C,True),
              ('.minigame_2',0x801C532C,0x801C5494,0xE614001C,False)}
    seen_circular=set()
    for row in data.get('circular_camera_sites',[]):
        name,s,pc,size=body(row)
        key=(row['section'],pc,row['site'],row['word'],row['opposite'])
        if key not in circular or key in seen_circular or not any(
            r['function']==name and r['site']==row['site'] and r['word']==row['word'] and r['role']==2
            for r in data['motion_sites']):
            raise ValueError('Unreviewed circular minigame camera recurrence')
        if struct.unpack_from('>I',rom,s['rom']+row['site']-s['vram'])[0]!=row['word']:
            raise ValueError('Circular camera store changed')
        seen_circular.add(key)
    if seen_circular!=circular:raise ValueError('Missing circular minigame camera review')
    # Source end-query branches that pause/seek, retain a fixed pose or switch
    # one-shot clips. Pin every call/component0 delay; not a global end hook.
    terminal_owners={
        ('.minigame_3',0x801C5F34):(0x801C5F48,),
        ('.minigame_3',0x801C675C):(0x801C681C,),
        ('.minigame_3',0x801C69CC):(0x801C6AA8,0x801C6B58,0x801C6BD4,0x801C6C80,
            0x801C6D30,0x801C6DDC,0x801C6E9C,0x801C6F18,0x801C6FC8,0x801C7044,
            0x801C70F4,0x801C71A4),
        ('.minigame_7',0x801C45E4):(0x801C45FC,),
        ('.minigame_9',0x801C4C74):(0x801C4C8C,),
        ('.minigame_2',0x801C56C8):(0x801C57C8,),
        ('.minigame_4',0x801C4678):(0x801C46DC,),
        ('.minigame_4',0x801C5768):(0x801C57CC,),
        ('.minigame_4',0x801C7570):(0x801C75B4,),
        ('.minigame_4',0x801C75DC):(0x801C7770,),
        ('.minigame_4',0x801C779C):(0x801C780C,),
        ('.minigame_4',0x801C788C):(0x801C78C4,),
        ('.minigame_4',0x801C7970):(0x801C7B08,),
        ('.minigame_4',0x801C8164):(0x801C81F8,0x801C824C,0x801C826C,0x801C82C0,
            0x801C8314,0x801C8368,0x801C83D0,0x801C83F0,0x801C8444,0x801C8464,
            0x801C84B8,0x801C850C),
        ('.minigame_8',0x801C592C):(0x801C5980,),
        ('.minigame_8',0x801C6CEC):(0x801C6EB8,),
    }
    terminal={(section,pc,site) for (section,pc),sites in terminal_owners.items() for site in sites}
    seen_terminal=set()
    for row in data.get('terminal_pose_sites',[]):
        name,s,pc,size=body(row);site=row['site'];key=(row['section'],pc,site)
        expected=0x0C000000|((0x8002A3B0>>2)&0x03FFFFFF)
        offset=s['rom']+site-s['vram']
        if key not in terminal or key in seen_terminal or not pc<=site<site+4<pc+size or row['component']!=0 or row['word']!=expected or row['delay_word']!=0x00002821:
            raise ValueError('Unreviewed terminal minigame pose owner')
        if struct.unpack_from('>II',rom,offset)!=(expected,0x00002821):
            raise ValueError('Terminal minigame end query/component changed')
        seen_terminal.add(key)
        hooks[name,site].append('dora3_native_model_terminal_pose(rdram,ctx,0);')
    if seen_terminal!=terminal:raise ValueError('Missing terminal minigame pose review')
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
