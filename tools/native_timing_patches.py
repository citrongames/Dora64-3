"""Validate semantic timing annotations before emitting optional continuous-step hooks."""
import collections,hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def timing_hooks(sections,compiled,rom):
    data=json.loads((ROOT/'recomp/native-timing-rules.json').read_text())
    if data['version']!=1 or hashlib.sha256(rom).hexdigest()!=data['rom_sha256']:
        raise ValueError('Unreviewed timing rule ROM/schema')
    lookup={s['name']:(i,s) for i,s in enumerate(sections)};seen=set();hooks=collections.defaultdict(list)
    def fpr(rule,key):
        value=rule[key]
        if not isinstance(value,int) or not 0<=value<32 or value%2:raise ValueError('Invalid float rate register')
        return f'ctx->f{value}.fl'
    def acc(rule):
        base=rule['base_register'];offset=rule['acceleration_offset']
        if not isinstance(base,int) or not 0<=base<32 or not isinstance(offset,int) or not -32768<=offset<32768:raise ValueError('Invalid acceleration address')
        return f'(uint32_t)MEM_W({offset}, ctx->r{base})'
    for rule in data['rules']:
        i,s=lookup[rule['section']];address=int(rule['function'],16);site=int(rule['site'],16)
        name,size,*_=compiled[i,address]
        if name!=rule['function_name'] or not address<=site<address+size or (i,site) in seen:
            raise ValueError('Invalid/duplicate timing rule function or site')
        seen.add((i,site));offset=s['rom']+address-s['vram']
        if hashlib.sha256(rom[offset:offset+size]).hexdigest()!=rule['function_sha256']:
            raise ValueError('Unreviewed timing function body')
        word=struct.unpack_from('>I',rom,s['rom']+site-s['vram'])[0]
        if word!=int(rule['expected_word'],16):raise ValueError('Unreviewed timing instruction')
        op=rule['operation']
        if op=='scale_f32_source':
            reg=fpr(rule,'register');text=f'{reg} = dora3_native_rate_f32({reg});'
        elif op=='gravity_subtract':
            allowed={('.scene1',0x801C0FA8):(2,-55),('.scene1',0x801CB408):(4,-50),('.minigame_8',0x801C42D0):(4,-50)}
            pair=allowed.get((rule['section'],site))
            if pair!=(rule['base_register'],rule['terminal']) or word!=0x46020001:raise ValueError('Unreviewed gravity subtraction')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ ctx->f0.fl = dora3_native_gravity_f32(ctx->f0.fl, ctx->f2.fl, {pair[1]}.0f, (uint32_t)ctx->r{pair[0]} + 0x14u); goto {label}; }}'
            hooks[name,site+4].append(label+':;')
        elif op=='gravity_displacement_source':
            allowed={('.scene1',0x801BF8A8):(0,3),('.scene1',0x801BFDA4):(0,3),('.scene1',0x801E8E10):(2,2),('.scene1',0x801CB6EC):(2,5),('.minigame_8',0x801C4318):(2,5)}
            pair=allowed.get((rule['section'],site))
            if pair!=(rule['register'],rule['base_register']):raise ValueError('Unreviewed gravity displacement')
            reg=fpr(rule,'register');text=f'{reg} = dora3_native_gravity_displacement_f32({reg}, (uint32_t)ctx->r{pair[1]} + 0x14u);'
        elif op=='process_slot_lifecycle':
            if address!=0x800281B8 or site!=0x80028208 or word!=0x8E220000:raise ValueError('Unreviewed process slot boundary')
            text='if (!dora3_native_process_lifecycle_allowed(rdram, ctx)) { MEM_W(0, (int32_t)0x80171590) = (int32_t)ctx->r18; goto L_80028258; }'
        elif op=='native_effect_entry':
            allowed={('.scene1',0x801BE5F0):'dora3_native_rectangle_update',('.scene1',0x801D56F4):'dora3_native_zoom_update'}
            target=allowed.get((rule['section'],address))
            if site!=address or rule['handler']!=target or not target:raise ValueError('Unreviewed native effect closure')
            text=f'if ({target}(rdram, ctx)) return;'
        elif op=='tick_end_entry':
            if site!=address:raise ValueError('Timer gate is not an entry hook')
            text='if (!dora3_native_simulation_tick_end()) return;'
        elif op=='tick_entry':
            if site!=address:raise ValueError('Timer gate is not an entry hook')
            text='if (!dora3_native_simulation_tick_start()) return;'
        elif op=='displacement_f32_source':
            reg=fpr(rule,'register');text=f'{reg} = dora3_native_displacement_f32({reg}, {acc(rule)});'
        elif op=='add_displacement_f32':
            dst=fpr(rule,'destination');pos=fpr(rule,'position_register');vel=fpr(rule,'velocity_register')
            expected=(17<<26)|(16<<21)|(rule['velocity_register']<<16)|(rule['position_register']<<11)|(rule['destination']<<6)
            if word!=expected or site+4>=address+size:raise ValueError('Unreviewed kick/drift addition')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ {dst} = {pos} + dora3_native_displacement_f32({vel}, {acc(rule)}); goto {label}; }}'
            hooks[name,site+4].append(label+':;')
        elif op=='timed_angle_call':
            # This reviewed call saves unscaled velocity in its delay slot.
            # Execute that store before passing displacement to the same angle helper.
            delay=struct.unpack_from('>I',rom,s['rom']+site+4-s['vram'])[0]
            base=rule['base_register'];off=rule['velocity_offset']
            if word!=0x0C00C683 or delay!=int(rule['delay_word'],16) or delay!=((57<<26)|(base<<21)|(14<<16)|off):
                raise ValueError('Unreviewed angle call/delay slot')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ MEM_W({off}, ctx->r{base}) = ctx->f14.u32l; ctx->f14.fl = dora3_native_displacement_f32(ctx->f14.fl, {acc(rule)}); dora3_s0_80031A0C(rdram, ctx); goto {label}; }}'
            hooks[name,site+8].append(label+':;')
        else:raise ValueError('Unreviewed timing rule operation')
        hooks[name,site].append(text)
    return [(name,site,' '.join(text)) for (name,site),text in sorted(hooks.items())]
