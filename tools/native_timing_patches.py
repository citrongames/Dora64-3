"""Validate semantic timing annotations before emitting optional continuous-step hooks."""
import collections,hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
APPROVED_UI_SITES={('ui_source_end', '.startup', 2147647928, 2147648108, 2411659308): {},
 ('ui_draw_boundary', '.scene_common', 2149281308, 2149281308, 666763216): {'end': 0},
 ('ui_draw_boundary', '.scene_common', 2149281308, 2149281796, 2411659304): {'end': 1},
 ('ui_draw_boundary', '.scene_common', 2149285324, 2149285324, 1006796828): {'end': 0},
 ('ui_draw_boundary', '.scene_common', 2149285324, 2149286716, 2411659340): {'end': 1},
 ('ui_draw_boundary', '.scene34', 2149310204, 2149310204, 666763072): {'end': 0},
 ('ui_draw_boundary', '.scene34', 2149310204, 2149313048, 2411659420): {'end': 1},
 ('ui_draw_boundary', '.scene34', 2149327556, 2149327556, 1006796828): {'end': 0},
 ('ui_draw_boundary', '.scene34', 2149327556, 2149328028, 65011720): {'end': 1},
 ('ui_command_once', '.scene_common', 2149278648, 2149278672, 201396528): {},
 ('ui_velocity', '.scene34', 2149301484, 2149301564, 1174536192): {'velocity': 0,
                                                                   'acceleration': 2,
                                                                   'base': 4,
                                                                   'offset': 28},
 ('ui_velocity', '.scene34', 2149301936, 2149302116, 1174409344): {'velocity': 2,
                                                                   'acceleration': 0,
                                                                   'base': 5,
                                                                   'offset': 24},
 ('ui_velocity', '.scene34', 2149302652, 2149302924, 1174536192): {'velocity': 0,
                                                                   'acceleration': 2,
                                                                   'base': 3,
                                                                   'offset': 28},
 ('ui_displacement', '.scene34', 2149301484, 2149301612, 1174536192): {'register': 2,
                                                                       'base': 4,
                                                                       'offset': 28},
 ('ui_displacement', '.scene34', 2149301936, 2149302132, 1174536192): {'register': 2,
                                                                       'base': 5,
                                                                       'offset': 24},
 ('ui_displacement', '.scene34', 2149302652, 2149302976, 1174536192): {'register': 2,
                                                                       'base': 3,
                                                                       'offset': 28},
 ('ui_displacement', '.scene34', 2149302652, 2149303256, 1174413568): {'register': 0,
                                                                       'base': 3,
                                                                       'offset': 24},
 ('ui_displacement', '.scene34', 2149303488, 2149303592, 1174536193): {'register': 2,
                                                                       'base': 3,
                                                                       'offset': 28},
 ('ui_displacement', '.scene34', 2149303488, 2149303612, 3294756892): {'register': 0,
                                                                       'base': 3,
                                                                       'offset': 28},
 ('ui_displacement', '.scene34', 2149303488, 2149303680, 1174536192): {'register': 2,
                                                                       'base': 3,
                                                                       'offset': 28},
 ('ui_rate', '.scene34', 2149302652, 2149303096, 1176633345): {'register': 2, 'double': True},
 ('ui_rate', '.scene34', 2149302652, 2149303156, 1176633345): {'register': 2, 'double': True},
 ('ui_rate', '.scene34', 2149302652, 2149303176, 1174544640): {'register': 2, 'double': False},
 ('ui_rate', '.scene34', 2149302652, 2149303212, 1176633344): {'register': 2, 'double': True},
 ('ui_rate', '.scene34', 2149302652, 2149303228, 1174544641): {'register': 2, 'double': False},
 ('ui_rate', '.scene34', 2149323960, 2149324100, 1174417792): {'register': 0, 'double': False},
 ('ui_rate', '.scene34', 2149323960, 2149324116, 1174413568): {'register': 0, 'double': False},
 ('ui_rate', '.scene34', 2149323960, 2149324304, 1174409344): {'register': 0, 'double': False},
 ('ui_selection_y_drift', '.scene34', 2149323960, 2149324084, 1006714752): {},
 ('ui_source_begin', '.startup', 2147647928, 2147647928, 666763216): {},
 ('ui_domain', '.scene_common', 2149276420, 2149276420, 666763232): {'domain': 1},
 ('ui_domain', '.scene1', 2149311984, 2149311988, 2221113626): {'domain': 2},
 ('ui_domain', '.scene34', 2149301484, 2149301484, 266304): {'domain': 4},
 ('ui_domain', '.scene34', 2149301936, 2149301936, 266304): {'domain': 4},
 ('ui_domain', '.scene34', 2149302652, 2149302652, 266304): {'domain': 4},
 ('ui_domain', '.scene34', 2149303488, 2149303488, 266304): {'domain': 4},
 ('ui_domain', '.scene34', 2149319976, 2149319976, 1006862364): {'domain': 8},
 ('ui_domain', '.scene34', 2149323960, 2149323960, 1006796823): {'domain': 16},
 ('ui_domain', '.scene34', 2149325456, 2149325456, 1006796828): {'domain': 32},
 ('ui_domain', '.scene34', 2149325984, 2149325984, 1006862364): {'domain': 64},
 ('ui_draw_boundary', '.scene_common', 2149278648, 2149278648, 1006796828): {'end': 0},
 ('ui_draw_boundary', '.scene_common', 2149278648, 2149279372, 65011720): {'end': 1}}
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
    event_reads=set()
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
        elif op=='player_reverse_yaw_gain':
            # 230C has already selected the shortest angular error and divided
            # it by ten. Replace only that temporal gain; +180 reorientations
            # elsewhere are instantaneous state commands.
            if rule['section']!='.scene1' or address!=0x801C230C or site!=0x801C23A0 or word!=0x3C028022:
                raise ValueError('Unreviewed player reverse yaw gain')
            text='if (dora3_native_simulation_active()) ctx->f14.fl = (float)((double)ctx->f14.fl * dora3_native_approach_coefficient_f64(0.1) / 0.1);'
        elif op in {'approach_f64_source','retention_f64_source','scale_f64_source'}:
            reg=fpr(rule,'register').replace('.fl','.d')
            helper={'approach_f64_source':'dora3_native_approach_coefficient_f64','retention_f64_source':'dora3_native_retention_f64','scale_f64_source':'dora3_native_rate_f64'}[op]
            text=f'{reg} = {helper}({reg});'
        elif op=='approach_multiply_f64':
            dst=fpr(rule,'destination').replace('.fl','.d');err=fpr(rule,'error_register').replace('.fl','.d');alpha=fpr(rule,'coefficient_register').replace('.fl','.d')
            expected=(17<<26)|(17<<21)|(rule['coefficient_register']<<16)|(rule['error_register']<<11)|(rule['destination']<<6)|2
            if word!=expected:raise ValueError('Unreviewed approach multiplication')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ {dst} = {err} * dora3_native_approach_coefficient_f64({alpha}); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='projectile_growth_threshold':
            approved={0x801CC398:(0x801CC33C,0,1,0x80210820),0x801CC86C:(0x801CC7F4,2,5,0x80210840)}
            spec=(address,rule['value_register'],rule['target'],int(rule['coefficient_address'],16))
            if rule['section']!='.scene1' or approved.get(site)!=spec or word!=((17<<26)|(17<<21)|(rule['value_register']<<16)|(4<<11)|60):
                raise ValueError('Unreviewed projectile scale threshold')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ union {{ uint64_t bits; double value; }} gain; gain.bits=LD(0,(int32_t){rule["coefficient_address"]}u); c1cs = dora3_native_tick_end_approach_f64(ctx->f{rule["value_register"]}.d,{rule["target"]}.0,gain.value) > ctx->f4.d; goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='player_zero_shot_birth':
            if rule['section']!='.scene1' or address!=0x801C120C or (site,word) not in {(0x801C12D0,0x0C074429),(0x801C12D8,0x8FBF0014)}:
                raise ValueError('Unreviewed zero-cue constructor boundary')
            text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op in {'player_scripted_turn','player_scripted_stop'}:
            allowed={(0x801C84F8,0x801C8550,0x0C00C683,'player_scripted_turn'):0x801C8558,
                     (0x801C7CE4,0x801C7D04,0x0C00C683,'player_scripted_turn'):0x801C7D0C,
                     (0x801C7F98,0x801C7FB0,0x0C00C683,'player_scripted_turn'):0x801C7FB8,
                     (0x801C84F8,0x801C8574,0xAC400000,'player_scripted_stop'):0x801C8578,
                     (0x801C875C,0x801C8774,0xE4400000,'player_scripted_stop'):0x801C8778}
            if rule['section']!='.scene1' or (address,site,word,op) not in allowed:
                raise ValueError('Unreviewed scripted player turn/stop command')
            end=allowed[address,site,word,op]
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) goto {label};'
            hooks[name,end].insert(0,label+':;')
        elif op=='player_flight_command':
            allowed={(0x801C1A3C,0x801C1A3C,0x1480000C),(0x801C1ADC,0x801C1ADC,0x3C018022),(0x801C0D88,0x801C0E24,0x8FBF0018)}
            if rule['section']!='.scene1' or (address,site,word) not in allowed:
                raise ValueError('Unreviewed coupled player flight closure')
            call=f'dora3_native_player_flight_hook(rdram,ctx,0x{site:08X}u)'
            text=f'if ({call}) return;' if site==address else call+';'
        elif op=='projectile_bounce_clock':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801CBFDC,0x801CC0B0,0x0807305B):
                raise ValueError('Unreviewed bounce impulse commit')
            text='dora3_native_preview_hook(rdram,ctx,0x801CC0B0u);'
        elif op=='player_weapon_command_exit':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801C554C,0x801C565C,0x0C070599):
                raise ValueError('Unreviewed player weapon command exit')
            # This callback has only saved RA at sp18. A post-pose command
            # must not execute scalar drive or either subsequent angle store.
            text='if (dora3_native_simulation_active() && dora3_native_player_weapon_command_only()) { ctx->r31=MEM_W(0x18,ctx->r29); ctx->r29=ADD32(ctx->r29,0x20); return; }'
        elif op=='projectile_terminal_after_pose':
            if rule['section']!='.scene1' or address not in {0x801CDAE4,0x801CE074,0x801CE620,0x801CEBA8,0x801CF1F0,0x801CF7AC,0x801CFDB4,0x801D0398,0x801D0BC0} or site!=address or word!=0x27BDFFE8:
                raise ValueError('Unreviewed terminal projectile callback')
            text='if (dora3_native_simulation_active()) return;'
        elif op=='projectile_chain_retire_after_pose':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801CD2C8,0x801CD340,0x02202021):
                raise ValueError('Unreviewed projectile chain retirement')
            label=f'dora3_after_chain_retire_{site:08X}'
            text=f'if (dora3_native_simulation_active()) goto {label};'
            hooks[name,0x801CD380].insert(0,label+':;')
        elif op=='projectile_surface_hit_boundary':
            spec=(address,site,word,rule['physical_register'],int(rule['end'],16))
            if rule['section']!='.scene1' or spec not in {
                (0x801CBFDC,0x801CC05C,0xA6020034,16,0x801CC060),
                (0x801CBFDC,0x801CC07C,0xA6020034,16,0x801CC080),
                (0x801CBFDC,0x801CC0E4,0xA6020034,16,0x801CC0E8),
                (0x801CBFDC,0x801CC104,0xA6020034,16,0x801CC108),
                (0x801CBFDC,0x801CC12C,0xA6020034,16,0x801CC130),
                (0x801CC5D4,0x801CC670,0x8E0600A4,17,0x801CC6A4),(0x801CD5A4,0x801CD650,0x96020034,16,0x801CD6AC),
                (0x801CCAAC,0x801CCDDC,0x820200DA,16,0x801CCE20),
                (0x801CDCB8,0x801CDD4C,0x824200DA,18,0x801CDD90),
                (0x801CE338,0x801CE3DC,0x820200DA,16,0x801CE424),
                (0x801CE954,0x801CE9F8,0x820200DA,16,0x801CEA40),
                (0x801CED88,0x801CEE24,0x820200DA,16,0x801CEE68),
                (0x801CF52C,0x801CF5D8,0x820200DA,16,0x801CF620),
                (0x801D0F48,0x801D103C,0x820200DA,16,0x801D108C)}:
                raise ValueError('Unreviewed projectile surface-hit command')
            label=f'dora3_after_surface_hit_{site:08X}'
            text=f'if (!dora3_native_simulation_tick_end() && MEM_H(0xD4,ctx->r{rule["physical_register"]})==-1) goto {label};'
            hooks[name,int(rule['end'],16)].insert(0,label+':;')
        elif op in {'projectile_opacity_positive','projectile_opacity_linear'}:
            approved={'projectile_opacity_positive':(0x801CD8C8,0x4604003C),
                      'projectile_opacity_linear':(0x801CD8E8,0x46220001)}
            if rule['section']!='.scene1' or address!=0x801CD86C or (site,word)!=approved[op]:
                raise ValueError('Unreviewed projectile opacity clock')
            owner='(uint32_t)MEM_W(0x74,ctx->r17)'
            scalar='(uint32_t)ADD32(ctx->r16,0x58)'
            label=f'dora3_after_timing_{site:08X}'
            if op=='projectile_opacity_positive':
                statement=f'c1cs=dora3_native_model_scalar_positive(rdram,{owner},{scalar},ctx->f4.fl);'
            else:
                statement=f'ctx->f0.d=(double)dora3_native_model_scalar_linear(rdram,{owner},{scalar},ctx->f4.fl,-ctx->f2.d);'
            text=f'if (dora3_native_simulation_active()) {{ {statement} goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op in {'projectile_interaction_skip','projectile_interaction_reach','projectile_interaction_bind','projectile_interaction_guard','projectile_interaction_birth','projectile_interaction_retire'}:
            approved={'projectile_interaction_skip':(0x801D05A8,0x801D05DC,0x0C072DC2),
                      'projectile_interaction_reach':(0x801D05A8,0x801D06E4,0x86020034),
                      'projectile_interaction_bind':(0x801D05A8,0x801D0750,0xAE020048),
                      'projectile_interaction_guard':(0x801D05A8,0x801D08A4,0x8E100048),
                      'projectile_interaction_retire':(0x801D05A8,0x801D091C,0x0C00A8EC),
                      'projectile_interaction_birth':(0x801D03D0,0x801D0594,0x8FBF0030)}
            if rule['section']!='.scene1' or (address,site,word)!=approved[op]:raise ValueError('Unreviewed projectile interaction site')
            label=f'dora3_after_timing_{site:08X}'
            if op=='projectile_interaction_skip':
                text=f'if (dora3_native_interaction_skip()) goto {label};';hooks[name,0x801D0940].insert(0,label+':;')
            elif op=='projectile_interaction_reach':text='if (dora3_native_interaction_enabled()) ctx->f22.fl=dora3_native_interaction_reach(rdram,ctx);'
            elif op=='projectile_interaction_bind':text='if (dora3_native_interaction_enabled()) dora3_native_interaction_bind(rdram,ctx);'
            elif op=='projectile_interaction_guard':text='if (dora3_native_interaction_enabled() && !dora3_native_interaction_target_valid(rdram,ctx)) MEM_W(0x48,ctx->r16)=0;'
            elif op=='projectile_interaction_retire':
                text=f'if (dora3_native_interaction_geometry_only() || !dora3_native_simulation_tick_end()) goto {label};';hooks[name,0x801D0940].insert(0,label+':;')
                hooks[name,site+8].insert(0,'if (dora3_native_simulation_active() && dora3_native_preview_hook(rdram,ctx,0x801D0924u)) ctx->r2=1;')
            else:text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op=='projectile_charge_birth':
            if rule['section']!='.scene1' or (address,site,word) not in {(0x801CF7E4,0x801CFA80,0x8FBF0030),(0x801CFDEC,0x801D0074,0x8FBF0030)}:
                raise ValueError('Unreviewed charge birth command')
            text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op in {'projectile_charge_alpha','projectile_charge_display_alpha','projectile_charge_speed','projectile_attachment_offset'}:
            approved={
                (0x801CFA94,'projectile_charge_alpha'):(0x801CFB80,0x46220001,0x58,0x80210880),
                (0x801D0088,'projectile_charge_alpha'):(0x801D0174,0x46220001,0x58,0x80210890),
                (0x801CFA94,'projectile_charge_display_alpha'):(0x801CFB64,0x02202021,0x58,0x80210880),
                (0x801D0088,'projectile_charge_display_alpha'):(0x801D0158,0x02202021,0x58,0x80210890),
                (0x801CFA94,'projectile_charge_speed'):(0x801CFBE8,0x46020000,0,0),
                (0x801D0088,'projectile_charge_speed'):(0x801D01DC,0x46020000,0,0),
                (0x801CFA94,'projectile_attachment_offset'):(0x801CFC24,0x0C072DB5,0,0),
                (0x801D0088,'projectile_attachment_offset'):(0x801D0218,0x0C072DB5,0,0),
            }
            spec=approved.get((address,op))
            if rule['section']!='.scene1' or not spec or (site,word)!=spec[:2]:raise ValueError('Unreviewed charge/attachment clock')
            offset,coefficient=spec[2:];owner='(uint32_t)MEM_W(0x74,ctx->r17)';scalar=f'(uint32_t)ADD32(ctx->r16,0x{offset:X})';label=f'dora3_after_timing_{site:08X}'
            if op=='projectile_charge_display_alpha':
                text=f'if (dora3_native_simulation_active()) {{ union {{ int32_t bits; float value; }} alpha; alpha.bits=MEM_W(0x58,ctx->r16); union {{ uint64_t bits; double value; }} gain; gain.bits=LD(0,(int32_t)0x{coefficient:08X}u); alpha.value=dora3_native_model_scalar_previous(rdram,{owner},{scalar},alpha.value,-gain.value); ctx->r6=alpha.bits; }}'
            else:
                if op=='projectile_charge_alpha':statement=f'ctx->f0.d=(double)dora3_native_model_scalar_linear(rdram,{owner},{scalar},(float)ctx->f0.d,-ctx->f2.d);';end=site+4
                elif op=='projectile_charge_speed':statement=f'ctx->f0.fl=dora3_native_model_scalar_linear(rdram,{owner},{scalar},ctx->f0.fl,(double)ctx->f2.fl);';end=site+4
                else:statement='ctx->r5=ctx->r16; dora3_native_attached_projectile_offset(rdram,ctx,dora3_s3_core_801CB6D4);';end=site+8
                text=f'if (dora3_native_simulation_active()) {{ {statement} goto {label}; }}';hooks[name,end].insert(0,label+':;')
        elif op in {'projectile_homing_motion','projectile_homing_sample','projectile_homing_target','projectile_homing_end'}:
            samples={0x801CCB54:(0xC60C0058,12,5),0x801CCC0C:(0xC620009C,0,0),
                     0x801CCC18:(0xC62000A4,0,2),0x801CCC24:(0xC62000A0,0,1),
                     0x801CCC38:(0xC60E0004,14,3),0x801CCD24:(0xC60E0058,14,4)}
            if rule['section']!='.scene1' or address!=0x801CCAAC:raise ValueError('Unreviewed homing motion function')
            label=f'dora3_after_timing_{site:08X}'
            if op=='projectile_homing_motion' and site==0x801CCADC and word==0x0C072CBE:
                statement='dora3_native_homing_motion(rdram,ctx);';end=0x801CCAF8
            elif op=='projectile_homing_target' and site==0x801CCBBC and word==0x0C072E1C:
                statement='dora3_native_homing_target(rdram,ctx);';end=site+8
            elif op=='projectile_homing_end' and site==0x801CCE20 and word==0x8FBF0024:
                text='if (dora3_native_simulation_active()) dora3_native_homing_end(rdram,ctx);'
                hooks[name,site].append(text);continue
            elif op=='projectile_homing_sample' and site in samples and word==samples[site][0]:
                _,register,field=samples[site];statement=f'ctx->f{register}.fl=dora3_native_homing_sample(rdram,ctx,{field});';end=site+4
            else:raise ValueError('Unreviewed homing motion site')
            text=f'if (dora3_native_simulation_active()) {{ {statement} goto {label}; }}';hooks[name,end].insert(0,label+':;')
        elif op=='projectile_homing_wave':
            if rule['section']!='.scene1' or address!=0x801CCAAC or site!=0x801CCAF8 or word!=0x86020036:
                raise ValueError('Unreviewed short-angle oscillator')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ float angle=dora3_native_model_angle_short(rdram,(uint32_t)MEM_W(0x74,ctx->r17),(uint32_t)ADD32(ctx->r16,0x36),40.0f); union {{ uint64_t bits; double value; }} radians; radians.bits=LD(0,(int32_t)0x80210858u); ctx->r3=MEM_H(0x36,ctx->r16); ctx->r2=ctx->r3; ctx->f12.d=(double)angle*radians.value; goto {label}; }}'
            hooks[name,0x801CCB44].insert(0,label+':;')
        elif op=='projectile_homing_turn':
            approved={0x801CCCD8:(0x4,0x801CCCE0),0x801CCDC4:(0x58,0x801CCDCC)}
            if rule['section']!='.scene1' or address!=0x801CCAAC or site not in approved or word!=0x0C00C683:
                raise ValueError('Unreviewed target angular command')
            offset,end=approved[site];label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ ctx->f0.fl=dora3_native_model_latched_turn(rdram,(uint32_t)MEM_W(0x74,ctx->r17),(uint32_t)ADD32(ctx->r16,0x{offset:X}),ctx->f12.fl,ctx->f14.fl); goto {label}; }}'
            hooks[name,end].insert(0,label+':;')
        elif op=='projectile_homing_age':
            if rule['section']!='.scene1' or address!=0x801CCAAC or site!=0x801CCBF4 or word!=0x24430001:
                raise ValueError('Unreviewed target tracking age')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ ctx->r3=ADD32(ctx->r2,dora3_native_simulation_tick_end()?1:0); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='projectile_age_threshold':
            if rule['section']!='.scene1' or address!=0x801CD86C or site!=0x801CD8B0 or word!=0x28420015:
                raise ValueError('Unreviewed projectile age threshold')
            # Store age only at tick end; the fade for this tick uses its
            # post-increment age, exactly as the Original whole-tick callback.
            text='if (!dora3_native_simulation_tick_end()) ctx->r2=ADD32(ctx->r2,1);'
        elif op=='capped_approach_multiply_f64':
            dst=fpr(rule,'destination').replace('.fl','.d');err=fpr(rule,'error_register').replace('.fl','.d');alpha=fpr(rule,'coefficient_register').replace('.fl','.d')
            expected=(17<<26)|(17<<21)|(rule['coefficient_register']<<16)|(rule['error_register']<<11)|(rule['destination']<<6)|2
            limit=rule['limit']
            if word!=expected or limit not in (3,5):raise ValueError('Unreviewed capped camera approach')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ {dst} = dora3_native_capped_approach_f64({err}, {alpha}, {limit}.0); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='timed_angle_load_call':
            delay=struct.unpack_from('>I',rom,s['rom']+site+4-s['vram'])[0]
            if word!=0x0C00C683 or delay!=int(rule['delay_word'],16) or delay>>26!=49 or (delay>>16)&31!=14:raise ValueError('Unreviewed camera angle load')
            base=(delay>>21)&31;off=(delay&65535);off=off-65536 if off>=32768 else off
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ ctx->f14.u32l = MEM_W({off}, ctx->r{base}); ctx->f14.fl = dora3_native_rate_f32(ctx->f14.fl); dora3_s0_80031A0C(rdram,ctx); goto {label}; }}'
            hooks[name,site+8].insert(0,label+':;')
        elif op=='camera_follow_gain':
            if address!=0x801D8578 or site not in (0x801D87CC,0x801D8814) or word!=0x46340002:raise ValueError('Unreviewed coupled camera gain')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_rate_f32(1.0f) != 1.0f && ctx->f2.d > 0.0 && ctx->f2.d*ctx->f20.d < 1.0) {{ ctx->f0.d = dora3_native_capped_approach_f64(ctx->f0.d/ctx->f2.d, ctx->f2.d*ctx->f20.d, (double)MEM_BU(2, MEM_W(0,(int32_t)0x8021D6C0)) + 3.0); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='camera_preserve_z_fraction':
            if address!=0x801D8578 or site!=0x801D860C or word!=0xC6040008:raise ValueError('Unreviewed camera Z save')
            text='uint32_t dora3_camera_z_bits = MEM_W(8,ctx->r16);'
            hooks[name,0x801D8754].append('if (dora3_native_rate_f32(1.0f) != 1.0f) { union { uint32_t bits; float value; } z; z.bits = dora3_camera_z_bits; ctx->f0.fl = z.value + (ctx->f0.fl - z.value) * dora3_native_rate_f32(1.0f); }')
        elif op=='camera_shake_decay_gate':
            if address!=0x801D55EC or site!=0x801D5654 or word!=0x3C018022:raise ValueError('Unreviewed shake decay')
            text='if (!dora3_native_simulation_tick_end()) goto L_801D5670;'
        elif op=='tick_addiu':
            reg=rule['register'];step=rule.get('step',1)
            charge=rule['section']=='.scene1' and (address,site,reg,step) in {(0x801CFA94,0x801CFC30,2,3),(0x801D0088,0x801D0224,2,3)}
            if not isinstance(reg,int) or not 1<=reg<32 or (step not in (-1,1) and not charge) or word!=((9<<26)|(reg<<21)|(reg<<16)|(step&65535)):raise ValueError('Unreviewed discrete increment')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (!dora3_native_simulation_tick_end()) goto {label};'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='rate_arithmetic_f32':
            dst=fpr(rule,'destination');pos=fpr(rule,'position_register');vel=fpr(rule,'velocity_register')
            code=rule['subtract'];expected=(17<<26)|(16<<21)|(rule['velocity_register']<<16)|(rule['position_register']<<11)|(rule['destination']<<6)|int(code)
            if not isinstance(code,bool) or word!=expected:raise ValueError('Unreviewed time arithmetic')
            label=f'dora3_after_timing_{site:08X}';sign='-' if code else '+'
            text=f'if (dora3_native_simulation_active()) {{ {dst} = {pos} {sign} dora3_native_rate_f32({vel}); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='ordinary_camera_dispatch':
            target=((site+4)&0xF0000000)|((word&0x3FFFFFF)<<2)
            delay=struct.unpack_from('>I',rom,s['rom']+site+4-s['vram'])[0]
            if address!=0x801DBD58 or word>>26!=3 or delay!=0x02002021:raise ValueError('Unreviewed ordinary camera dispatcher call')
            name_target=compiled[i,target][0]
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ ctx->r4=ctx->r16; dora3_native_camera_dispatch(rdram,ctx,0x{target:08X}u,{name_target}); goto {label}; }}'
            hooks[name,site+8].insert(0,label+':;')
        elif op=='model_frame_event':
            read=int(rule['read_site'],16);reg=rule['register'];other=rule['other_register'];scale=rule['threshold_scale']
            if not address<=read<site or scale not in (1,3):raise ValueError('Invalid animation event provenance')
            getter=struct.unpack_from('>I',rom,s['rom']+read-s['vram'])[0]
            rs=(word>>11)&31;rt=(word>>16)&31
            if getter!=0x0C00A8FE or (word&0xFFE007FF)!=0x46000032 or {rs,rt}!={reg,other}:raise ValueError('Unreviewed clip-time equality')
            var=f'dora3_animation_read_{read:08X}'
            if (name,read) not in event_reads:
                event_reads.add((name,read))
                hooks[name,address].append(f'uint32_t {var}_model=0; float {var}_time=0.0f;')
                hooks[name,read+8].insert(0,f'{var}_model=(uint32_t)ctx->r4; {var}_time=ctx->f0.fl;')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ c1cs=dora3_native_model_event(rdram,{var}_model,{var}_time,ctx->f{other}.fl*{scale}.0f,0x{i:02X}{site:08X}ull); goto {label}; }}'
            if rule.get('source_start_consumed'):
                if rule['section']!='.scene1' or (address,site) not in {(0x801C4F7C,0x801C4F9C),(0x801C533C,0x801C535C)}:
                    raise ValueError('Unreviewed acquired initial shot event')
                text=text.replace('dora3_native_model_event','dora3_native_model_start_event')
            if rule.get('post_player_pose'):
                if (rule['section'],address,site) not in {('.scene1',0x801C4948,0x801C496C),('.scene1',0x801C4650,0x801C4674),('.scene1',0x801C4B5C,0x801C4B80),('.scene1',0x801C554C,0x801C5584),('.scene1',0x801C554C,0x801C5644)}:raise ValueError('Unreviewed post-pose player event')
                text=text.replace('c1cs=dora3_native_model_event','c1cs=dora3_native_defer_player_shot() ? 0 : dora3_native_model_event')
            if rule.get('post_weapon_level_command'):
                if (rule['section'],address,site) not in {('.scene1',0x801C554C,0x801C5584),('.scene1',0x801C554C,0x801C5644)}:
                    raise ValueError('Unreviewed paused weapon level command')
                text=text.replace('dora3_native_model_event(rdram',f'dora3_native_player_weapon_command_only() ? ({var}_time==ctx->f{other}.fl*{scale}.0f) : dora3_native_model_event(rdram')
            hooks[name,site+4].insert(0,label+':;')
        elif op=='ground_preview_boundary':
            action={0x801BB080:'',0x801C8B28:'return;',0x801C8F10:'return;',0x801C8F34:'return;',0x801C9048:'return;',0x801DBD7C:'goto L_801DBE78;',0x801DBE78:'goto L_801DBF88;'}[site]
            prefix='dora3_native_cutscene_source(rdram,ctx,0); ' if site==0x801BB080 else ''
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            text=prefix+(f'if ({call}) {{ {action} }}' if action else call+';')
        elif op=='projectile_phase_callback':
            if rule['section']!='.scene1' or address!=0x801CBD0C or site!=0x801CBDA4 or word!=0x0C00A8EC:
                raise ValueError('Unreviewed projectile phase callback')
            text='if (dora3_native_simulation_active()) goto L_801CBE2C;'
        elif op=='projectile_phase_boundary':
            if rule['section']!='.scene1' or address!=0x801D10D8 or site!=0x801D117C or word!=0x02202021:
                raise ValueError('Unreviewed projectile clip boundary')
            text='if (dora3_native_simulation_active()) { int phase_result=dora3_native_preview_hook(rdram,ctx,0x801D117Cu); if (phase_result==2) goto dora3_projectile_exit; if (phase_result==1) goto dora3_projectile_next; }'
        elif op=='native_pickup_idle_turn':
            if rule['section']!='.scene1' or address!=0x801EBA9C or site!=0x801EBAB4 or word!=0x0C00C683:
                raise ValueError('Unreviewed pickup idle turn instruction')
            text='if (dora3_native_preview_hook(rdram,ctx,0x801EBAB4u)) goto dora3_after_pickup_idle_turn;'
            hooks[name,0x801EBABC].insert(0,'dora3_after_pickup_idle_turn:;')
        elif op=='native_reward_opacity_end_gate':
            if rule['section']!='.scene1' or address!=0x801E8BDC or site!=0x801E8CD4 or word!=0xA06000C8:
                raise ValueError('Unreviewed reward opacity command end')
            # The original store is a delay slot of j L_801E8D08. Suppression
            # jumps to that same destination, preserving both generated paths.
            text='if (dora3_native_preview_hook(rdram,ctx,0x801E8CD4u)) goto L_801E8D08;'
        elif op=='native_reward_common_opacity':
            allowed={0x801E8CB0:0x46340000,0x801E8CE8:0x46340001}
            if rule['section']!='.scene1' or address!=0x801E8BDC or site not in allowed or word!=allowed[site]:
                raise ValueError('Unreviewed reward opacity instruction')
            label=f'dora3_after_reward_opacity_{site:08X}'
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) goto {label};'
            hooks[name,site+4].insert(0,label+':;')
        elif op in {'native_pickup_motion_input' ,'native_pickup_motion_replace','native_pickup_motion_retire_gate'}:
            allowed={0x801EC6F4:(0x3C028022,'native_pickup_motion_input',0x801EC6F8),0x801EC704:(0x0C00C683,'native_pickup_motion_replace',0x801EC70C),0x801EC72C:(0x0C07A043,'native_pickup_motion_replace',0x801EC734),0x801EC740:(0x4614003E,'native_pickup_motion_input',0x801EC744),0x801EC764:(0x46220001,'native_pickup_motion_replace',0x801EC768),0x801EC79C:(0x4622003E,'native_pickup_motion_retire_gate',0x801EC7A0),0x801EC7B8:(0x8FBF0020,'native_pickup_motion_input',0x801EC7BC)}
            if rule['section']!='.scene1' or address!=0x801EC598 or site not in allowed or (word,op)!=allowed[site][:2]:
                raise ValueError('Unreviewed collected pickup motion instruction')
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            label=f'dora3_after_pickup_{site:08X}'
            if op=='native_pickup_motion_input':text=call+';'
            else:
                text=('if ('+call+') { c1cs=0; goto '+label+'; }') if op=='native_pickup_motion_retire_gate' else ('if ('+call+') goto '+label+';')
                hooks[name,allowed[site][2]].insert(0,label+':;')
        elif op in {'native_flash_motion_input','native_flash_motion_replace','native_flash_motion_retire_gate'}:
            allowed={0x801ED5F4:(0x3C038022,'native_flash_motion_input',0x801ED5F8),0x801ED664:(0x3C038022,'native_flash_motion_input',0x801ED668),0x801ED6B0:(0x0C00C683,'native_flash_motion_replace',0x801ED6B8),0x801ED6E0:(0x46346302,'native_flash_motion_input',0x801ED6E4),0x801ED70C:(0x46020002,'native_flash_motion_input',0x801ED710),0x801ED72C:(0x46346302,'native_flash_motion_input',0x801ED730),0x801ED74C:(0x46020002,'native_flash_motion_input',0x801ED750),0x801ED76C:(0x46020000,'native_flash_motion_replace',0x801ED770),0x801ED778:(0x4604103E,'native_flash_motion_input',0x801ED77C),0x801ED79C:(0x46220001,'native_flash_motion_replace',0x801ED7A0),0x801ED7B8:(0x4624103C,'native_flash_motion_retire_gate',0x801ED7BC),0x801ED7F4:(0x0C07A043,'native_flash_motion_replace',0x801ED7FC),0x801ED814:(0x46220000,'native_flash_motion_replace',0x801ED818),0x801ED840:(0x4600203C,'native_flash_motion_input',0x801ED844),0x801ED85C:(0x46220000,'native_flash_motion_replace',0x801ED860),0x801ED884:(0x8FBF001C,'native_flash_motion_input',0x801ED888)}
            if rule['section']!='.scene1' or address!=0x801ED5F4 or site not in allowed or (word,op)!=allowed[site][:2]:
                raise ValueError('Unreviewed flash motion instruction')
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            label=f'dora3_after_flash_{site:08X}'
            if op=='native_flash_motion_input':text=call+';'
            else:
                text=f'if ({call}) goto {label};' if op=='native_flash_motion_replace' else 'if ('+call+') { c1cs=0; goto '+label+'; }'
                hooks[name,allowed[site][2]].insert(0,label+':;')
        elif op=='native_flash_delay':
            allowed={(0x801ED520,0x3C038022),(0x801ED5E8,0x8FBF0018)}
            if rule['section']!='.scene1' or address!=0x801ED4AC or (site,word) not in allowed:
                raise ValueError('Unreviewed flash delay count/clip boundary')
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            text=f'if ({call}) goto L_801ED5E8;' if site==0x801ED520 else call+';'
        elif op=='native_object_boundary':
            sites={
                0x801D10D8:(0x801D10D8,''),
                0x801D1130:(0x801D10D8,'goto dora3_projectile_next;'),
                0x801D1160:(0x801D10D8,'goto dora3_projectile_exit;'),
                0x801E8BDC:(0x801E8BDC,''),
                0x801E8C08:(0x801E8BDC,'ctx->r18=30; goto dora3_object_after_reset;'),
                0x801E8C9C:(0x801E8BDC,'goto L_801E8E7C;'),
                0x801E8D88:(0x801E8BDC,'goto dora3_object_exit;'),
            }
            owner,action=sites[site]
            if site==0x801D1130:
                hooks[name,0x801D1188].insert(0,'dora3_projectile_next:;')
                hooks[name,0x801D118C].insert(0,'dora3_native_projectile_scope_end();')
            if site==0x801D1160:hooks[name,0x801D119C].insert(0,'dora3_projectile_exit:; dora3_native_projectile_scope_end();')
            if site==0x801E8C08:hooks[name,0x801E8C10].insert(0,'dora3_object_after_reset:;')
            if site==0x801E8D88:hooks[name,0x801E8EA0].insert(0,'dora3_object_exit:; dora3_native_object_scope_end();')
            if rule['section']!='.scene1' or address!=owner:raise ValueError('Unreviewed object ownership boundary')
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            text=f'if ({call}) {{ {action} }}' if action else call+';'
        elif op=='native_city_command':
            allowed={(0x8022D8B0,0x8022D8F0,0x1440000E),(0x8022DB34,0x8022DB88,0x0C06E831)}
            if rule['section']!='.level1_slot1' or (address,site,word) not in allowed:
                raise ValueError('Unreviewed city state/sprite command boundary')
            text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op=='native_pickup_sequence':
            if rule['section']!='.scene1' or address!=site or site not in [0x801EBCC0,0x801EBAD4,0x801EBEDC,0x801EBFC8,0x801EC0D0,0x801EC194,0x801EC2A8] or word!=0x3C038022:raise ValueError('Unreviewed pickup2/4/5/6/7/8 callback')
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) return;'
        elif op=='native_linked_pickup_effect':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801ED2CC,0x801ED2CC,0x3C038022):raise ValueError('Unreviewed linked pickup129 callback')
            text='if (dora3_native_preview_hook(rdram,ctx,0x801ED2CCu)) return;'
        elif op=='native_periodic_effect':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801ED8FC,0x801ED8FC,0x3C038022):raise ValueError('Unreviewed periodic131 callback')
            text='if (dora3_native_preview_hook(rdram,ctx,0x801ED8FCu)) return;'
        elif op=='native_object_scope_end':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801E8BDC,0x801E8E7C,0x3C028022):
                raise ValueError('Unreviewed common object scope continuation')
            text='dora3_native_preview_hook(rdram,ctx,0x801E8E7Cu); dora3_native_object_scope_end();'
        elif op=='player_damage_counter_phase':
            targets={(0x801C6084,0x801C6094,0x94820044):'L_801C60F0',
                     (0x801C67C8,0x801C67D8,0x94820044):'L_801C6834'}
            if rule['section']!='.scene1' or (address,site,word) not in targets:
                raise ValueError('Unreviewed heavy damage counter/impulse phase')
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) goto {targets[address,site,word]};'
        elif op=='death_resource_barrier':
            if rule['section']!='.scene1' or (address,site,word) not in {(0x801EF5B0,0x801EF5B0,0x27BDFFE0),(0x801C9BE0,0x801C9BE0,0x27BDFFE8),(0x801B9FDC,0x801B9FDC,0x27BDFFE8)}:
                raise ValueError('Unreviewed death resource replacement boundary')
            text='dora3_native_loop_resource_barrier(rdram, ctx);'
        elif op=='player_death_first_phase':
            allowed={(0x801C6FCC,0x3C048022),(0x801C7160,0x3C088022),(0x801C7338,0x3C038022)}
            if rule['section']!='.scene1' or site!=address or (address,word) not in allowed:
                raise ValueError('Unreviewed death first timer/state/fade closure')
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) return;'
        elif op=='player_death_constructor_phase':
            allowed={(0x801C6F58,0x801C6FC0),(0x801C7104,0x801C7154),(0x801C72DC,0x801C732C),
                     (0x801C7430,0x801C7494),(0x801C74D8,0x801C7528),(0x801C756C,0x801C75BC),(0x801C7600,0x801C7648)}
            if rule['section']!='.scene1' or (address,site) not in allowed or word!=0x8FBF0010:
                raise ValueError('Unreviewed death constructor period')
            text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op=='player_hover_counter_phase':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801C6F30,0x801C6F38,0x94430044):
                raise ValueError('Unreviewed hover reaction counter')
            text='if (dora3_native_preview_hook(rdram,ctx,0x801C6F38u)) goto dora3_after_hover_counter;'
            hooks[name,0x801C6F4C].insert(0,'dora3_after_hover_counter:;')
        elif op=='player_hover_command':
            allowed={(0x801C8118,0x801C8118,0x27BDFFE8),(0x801C0E3C,0x801C0EA8,0x8FBF0018)}
            if rule['section']!='.scene1' or (address,site,word) not in allowed:
                raise ValueError('Unreviewed hover command/projection closure')
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            text=f'if ({call}) return;' if address==site else call+';'
        elif op=='player_idle_phase':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801C8B28,0x801C8E58,0x3C028022):
                raise ValueError('Unreviewed player idle pre-command phase')
            text='if (dora3_native_preview_hook(rdram,ctx,0x801C8E58u)) goto L_801C8EAC;'
        elif op=='player_surface_phase':
            targets={(0x801C85C8,0x801C85D8,0x84620042):'L_801C86A8',
                     (0x801C85C8,0x801C86DC,0x94620044):'L_801C8750'}
            if rule['section']!='.scene1' or (address,site,word) not in targets:
                raise ValueError('Unreviewed special surface initializer/recovery count')
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) goto {targets[address,site,word]};'
        elif op=='player_surface_action_phase':
            targets={(0x801C7C7C,0x801C7CB4,0x3C038022):'L_801C7CD8',
                     (0x801C7DAC,0x801C7DE4,0x3C028022):'L_801C7E64',
                     (0x801C7F3C,0x801C7F74,0x3C038022):'L_801C7F8C'}
            if rule['section']!='.scene1' or (address,site,word) not in targets:
                raise ValueError('Unreviewed surface first command phase')
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) goto {targets[address,site,word]};'
        elif op=='player_surface_repeat_entry':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801C2E94,0x801C2FC8,0x0C071F51):
                raise ValueError('Unreviewed repeated F4 constructor')
            label=f'dora3_after_surface_entry_{site:08X}'
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) goto {label};'
            hooks[name,site+8].insert(0,label+':;')
        elif op=='player_surface_constructor_phase':
            allowed={(0x801C7C20,0x801C7C6C),(0x801C7D44,0x801C7D9C),(0x801C7EE0,0x801C7F2C),(0x801C7A2C,0x801C7AAC)}
            if rule['section']!='.scene1' or (address,site) not in allowed or word!=0x8FBF0010:
                raise ValueError('Unreviewed surface/door constructor phase')
            text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op=='player_door_first_phase':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801C7AB8,0x801C7AB8,0x27BDFFE8):
                raise ValueError('Unreviewed door counter/scene command closure')
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) return;'
        elif op=='player_wait_damage_phase':
            targets={(0x801C6AA8,0x801C6AC8,0x84820044):'L_801C6AFC',
                     (0x801C6B08,0x801C6B20,0x84830044):'L_801C6BC0'}
            if rule['section']!='.scene1' or (address,site,word) not in targets:
                raise ValueError('Unreviewed waiting damage decrement/one-shot kick')
            # C6B28's delay slot writes CC=1 even when no impulse runs.
            # C8B28 clears CC at each update; skipping that store would select
            # its default animation rate3 on repeated native parts.
            prefix='MEM_W(0xCC,ctx->r4)=0x3F800000; ' if site==0x801C6B20 else ''
            text=f'if (dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) {{ {prefix}goto {targets[address,site,word]}; }}'
        elif op=='player_delayed_damage_phase':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801C63F8,0x801C6418,0x94620044):
                raise ValueError('Unreviewed delayed hazard counter/kick/drive')
            text='{ const int phase = dora3_native_preview_hook(rdram,ctx,0x801C6418u); if (phase==1) goto L_801C64F4; if (phase==2) goto L_801C64D0; }'
        elif op=='player_command_boundary':
            allowed={(0x801C8B28,0x801C8D18):0x24C2FFFE,(0x801C6D8C,0x801C6E00):0x8FBF0010,
                     (0x801C5D90,0x801C5DB0):0x30420002,(0x801C5EF0,0x801C5F7C):0x8FBF0010,
                     (0x801C6020,0x801C6064):0x30420002,(0x801C6270,0x801C62C8):0x8FBF0010,
                     (0x801C5A38,0x801C5A58):0x30420002,(0x801C5C78,0x801C5D04):0x8FBF0010,
                     (0x801C637C,0x801C63D8):0x30420002,(0x801C6764,0x801C67A8):0x30420002,
                     (0x801C6658,0x801C66B0):0x8FBF0010,(0x801C69B4,0x801C6A0C):0x8FBF0010,(0x801C63F8,0x801C6500):0x8FBF0010,(0x801BF828,0x801BFE18):0x8FBF003C,
                     (0x801C6AA8,0x801C6AE8):0x30420002,(0x801C6B08,0x801C6BCC):0x8FBF0010,
                     (0x801C6C68,0x801C6CF4):0x8FBF0010,(0x801C5910,0x801C5950):0x30420002,(0x801C6A38,0x801C6A9C):0x8FBF0010,(0x801C858C,0x801C85BC):0x304200FE,(0x801C6EA4,0x801C6F24):0x8FBF0010,(0x801C6E28,0x801C6E98):0x8FBF0018,(0x801C6EA4,0x801C6EDC):0x28420015,(0x801C3898,0x801C38D8):0x30420002}
            if rule['section']!='.scene1' or allowed.get((address,site))!=word:
                raise ValueError('Unreviewed player command boundary')
            text=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op=='native_object_axis_displacement':
            if rule['section']!='.scene1' or address!=0x801E8BDC or site not in (0x801E8E00,0x801E8E10,0x801E8E20) or word!=0x46020000:
                raise ValueError('Unreviewed common object axis displacement')
            calculation='dora3_native_gravity_displacement_f32(ctx->f2.fl,(uint32_t)ctx->r2+0x14u)' if site==0x801E8E10 else 'dora3_native_rate_f32(ctx->f2.fl)'
            text=f'if (dora3_native_simulation_active() && !dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)) ctx->f2.fl={calculation};'
        elif op=='native_orbit_object':
            approved={
                0x801EEFC8:(0x801EEFC8,None),
                0x801EED78:(0x801EECF4,0x801EEDD8),
                0x801EEE00:(0x801EECF4,0x801EEE1C),
                0x801EEE40:(0x801EECF4,0x801EEE5C),
                0x801EF038:(0x801EEFC8,0x801EF054),
                0x801EF080:(0x801EEFC8,0x801EF0B4),
                0x801EEEF8:(0x801EEE80,0x801EEF00),
                0x801EEF1C:(0x801EEE80,None),
                0x801EEF50:(0x801EEE80,0x801EEF58),
                0x801EEF70:(0x801EEE80,0x801EEF78),
                0x801EEF90:(0x801EEE80,0x801EEFB4),
                0x801EEFB4:(0x801EEE80,None),
            }
            if rule['section']!='.scene1' or site not in approved or address!=approved[site][0]:
                raise ValueError('Unreviewed orbital object timing site')
            _,end=approved[site];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_timing_{site:08X}'
                text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=call+';'
        elif op=='native_dialogue_actor':
            approved={
                0x80223FBC:('.level1_slot0',0x80223FBC,None),
                0x8022402C:('.level1_slot0',0x80223FBC,0x80224048),
                0x80224074:('.level1_slot0',0x80223FBC,0x802240A8),
                0x80223E5C:('.level1_slot0',0x80223DE4,0x80223E64),
                0x80223E80:('.level1_slot0',0x80223DE4,None),
                0x80223F44:('.level1_slot0',0x80223DE4,0x80223F4C),
                0x80223F64:('.level1_slot0',0x80223DE4,0x80223F6C),
                0x80223F84:('.level1_slot0',0x80223DE4,0x80223FA8),
                0x80223FA8:('.level1_slot0',0x80223DE4,None),
                0x8023DE6C:('.level1_slot2',0x8023DE08,0x8023DE74),
                0x8023DE90:('.level1_slot2',0x8023DE08,None),
                0x8023DEC4:('.level1_slot2',0x8023DE08,0x8023DECC),
                0x8023DEE4:('.level1_slot2',0x8023DE08,0x8023DEEC),
                0x8023DF04:('.level1_slot2',0x8023DE08,0x8023DF28),
                0x8023DF28:('.level1_slot2',0x8023DE08,None),
            }
            if site not in approved or (rule['section'],address)!=approved[site][:2]:
                raise ValueError('Unreviewed city dialogue actor timing site')
            end=approved[site][2];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_timing_{site:08X}'
                text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=call+';'
        elif op=='native_linked_city':
            approved={
                0x80230F70:(0x80230F70,None),
                0x80231030:(0x80230F70,None),
                0x8023098C:(0x8023098C,None),
                0x802309B4:(0x8023098C,None),
                0x80230D0C:(0x80230C7C,2149780756),
                0x80230F0C:(0x80230C7C,None),
            }
            if rule['section']!='.level1_slot1' or site not in approved or approved[site][0]!=address:
                raise ValueError('Unreviewed linked city callback/geometry site')
            end=approved[site][1];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_timing_{site:08X}';text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=f'if ({call}) return;' if site==address else call+';'
        elif op=='native_slot09_dialogue':
            approved={2149777600: (2149777600, None), 2149778308: (2149777600, None), 2149777732: (2149777600, 2149777740), 2149775648: (2149775648, None), 2149776288: (2149775648, None), 2149775748: (2149775648, 2149775756), 2149768120: (2149768120, None), 2149768872: (2149768120, None), 2149768264: (2149768120, 2149768272), 2149770488: (2149770488, None), 2149770996: (2149770488, None), 2149770612: (2149770488, 2149770620), 2149771576: (2149771576, None), 2149772168: (2149771576, None), 2149771700: (2149771576, 2149771708), 2149773908: (2149773908, None), 2149774952: (2149773908, None), 2149774032: (2149773908, 2149774040)}
            approved.update({0x8022FC1C:(0x8022FBAC,0x8022FC24),0x8022FC38:(0x8022FBAC,0x8022FC40),0x8022FE40:(0x8022FBAC,None)})
            if rule['section']!='.remaining_slot1_09' or site not in approved or approved[site][0]!=address:
                raise ValueError('Unreviewed slot09 dialogue boundary')
            end=approved[site][1];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_slot09_dialogue_{site:08X}'
                text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=f'if ({call}) return;' if site==address else call+';'
        elif op=='native_slot09_scene_publication':
            if rule['section']!='.remaining_slot1_09' or (address,site,word)!=(0x8022F254,0x8022F380,0x0C06D3EF):
                raise ValueError('Unreviewed sceneFD quest/request tail')
            text='if (dora3_native_preview_hook(rdram,ctx,0x8022F380u)) goto L_8022F668;'
        elif op=='native_slot09_decision':
            approved={2149769140: (2149769124, 2149769380), 2149769380: (2149769124, None), 2149772196: (2149772180, 2149772344), 2149772344: (2149772180, None), 2149774980: (2149774964, 2149775104), 2149775104: (2149774964, None), 2149778520: (2149778504, 2149778620), 2149778620: (2149778504, None), 2149776992: (2149776972, 2149777144), 2149777144: (2149776972, None)}
            if rule['section']!='.remaining_slot1_09' or site not in approved or approved[site][0]!=address:
                raise ValueError('Unreviewed slot09 proposal boundary')
            end=approved[site][1];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_slot09_decision_{site:08X}'
                text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=call+';'
        elif op=='native_city_special_dialogue':
            approved={0x80232070:(0x80232070,None),0x802320A4:(0x80232070,None),0x802320B0:(0x802320B0,None),0x802322E0:(0x802320B0,None),0x8023229C:(0x802320B0,0x802322A0)}
            if rule['section']!='.level1_slot1' or site not in approved or approved[site][0]!=address:
                raise ValueError('Unreviewed special city dialogue proposal/counter site')
            end=approved[site][1];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_timing_{site:08X}';text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=f'if ({call}) return;' if site==address else call+';'
        elif op=='native_city_dialogue':
            approved={2149769384: (2149769384, None), 2149770212: (2149769384, None), 2149769572: (2149769384, 2149769580), 2149771784: (2149771784, None), 2149772512: (2149771784, None), 2149771916: (2149771784, 2149771924), 2149774040: (2149774040, None), 2149774768: (2149774040, None), 2149774172: (2149774040, 2149774180), 2149776320: (2149776320, None), 2149777064: (2149776320, None), 2149776448: (2149776320, 2149776456), 2149778596: (2149778596, None), 2149779324: (2149778596, None), 2149778728: (2149778596, 2149778736), 2149782652: (2149782652, None), 2149783348: (2149782652, None), 2149782776: (2149782652, 2149782784), 2149784916: (2149784916, None), 2149785560: (2149784916, None), 2149785040: (2149784916, 2149785048), 2149787744: (2149787744, None), 2149788472: (2149787744, None), 2149787876: (2149787744, 2149787884)}
            if rule['section']!='.level1_slot1' or site not in approved or approved[site][0]!=address:
                raise ValueError('Unreviewed city dialogue proposal/geometry site')
            end=approved[site][1];call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            if end:
                label=f'dora3_after_timing_{site:08X}';text=f'if ({call}) goto {label};';hooks[name,end].insert(0,label+':;')
            else:text=f'if ({call}) return;' if site==address else call+';'
        elif op=='native_city_patrol_decision':
            approved={
                0x8022DE70:0x8022DE70,
                0x8022DF44:0x8022DE70,
                0x8022DF50:0x8022DF50,
                0x8022E09C:0x8022DF50,
                0x8022E7D0:0x8022E7D0,
                0x8022E8A4:0x8022E7D0,
                0x8022E8B0:0x8022E8B0,
                0x8022E9FC:0x8022E8B0,
                0x8022F0A0:0x8022F0A0,
                0x8022F174:0x8022F0A0,
                0x8022F180:0x8022F180,
                0x8022F2CC:0x8022F180,
                0x8022F988:0x8022F988,
                0x8022FA5C:0x8022F988,
                0x8022FA68:0x8022FA68,
                0x8022FBB4:0x8022FA68,
                0x8023026C:0x8023026C,
                0x80230340:0x8023026C,
                0x8023034C:0x8023034C,
                0x80230498:0x8023034C,
                0x80231270:0x80231270,
                0x80231344:0x80231270,
                0x80231350:0x80231350,
                0x8023146C:0x80231350,
                0x80231B1C:0x80231B1C,
                0x80231BF0:0x80231B1C,
                0x80231BFC:0x80231BFC,
                0x80231D48:0x80231BFC,
                0x80232654:0x80232654,
                0x80232728:0x80232654,
                0x80232734:0x80232734,
                0x80232850:0x80232734,
            }
            if rule['section']!='.level1_slot1' or approved.get(site)!=address:
                raise ValueError('Unreviewed city wait/patrol decision boundary')
            call=f'dora3_native_preview_hook(rdram,ctx,0x{site:08X}u)'
            text=f'if ({call}) return;' if site==address else call+';'
        elif op=='tick_counter_store':
            reg=rule['register'];step=rule['step']
            if word>>26 not in (40,41,43) or (word>>16)&31!=reg or step not in (-1,1):raise ValueError('Unreviewed timer store')
            text=f'if (!dora3_native_simulation_tick_end()) ctx->r{reg}=ADD32(ctx->r{reg}, {-step});'
        elif op in {'tick_region','tick_start_region'}:
            end=int(rule['end'],16)
            if not site<end<address+size:raise ValueError('Unreviewed timer region')
            label=f'dora3_after_tick_{site:08X}'
            gate='start' if op=='tick_start_region' else 'end'
            text=f'if (!dora3_native_simulation_tick_{gate}()) goto {label};'
            hooks[name,end].insert(0,label+':;')
        elif op=='player_drive':
            if address!=0x801C1664 or site!=0x801C1754:raise ValueError('Unreviewed drive closure')
            text='if (dora3_native_simulation_active()) { union { uint32_t bits; float value; } slope; slope.bits=MEM_W(0x24,MEM_W(0,(int32_t)0x8021D6C0)); if (!dora3_native_preview_hook(rdram,ctx,0x801C1754u)) ctx->f0.fl=dora3_native_drive_f32(ctx->f0.fl,ctx->f4.fl,ctx->f2.fl,slope.value,(float)MEM_H(8,MEM_W(0,(int32_t)0x8021D648)),(uint32_t)ctx->r3); MEM_W(0,ctx->r3)=ctx->f0.u32l; goto L_801C1818; }'
        elif op=='player_drive_projection':
            allowed={0x801C0DD0:(16,2,0x10,False),0x801C0DF8:(17,2,0x18,False),0x801C0E20:(16,20,0x14,True),0x801C0E84:(17,2,0x10,False),0x801C0EA4:(16,2,0x18,False)}
            base,reg,offset,negative=allowed[site]
            if word!=((57<<26)|(base<<21)|(reg<<16)|offset):raise ValueError('Unreviewed drive projection store')
            sign='-' if negative else ''
            text=f'dora3_native_project_drive_f32(ctx->f{reg}.fl,{sign}ctx->f0.fl,(uint32_t)MEM_W(0,ctx->r{base}),(uint32_t)ctx->r{base},(uint32_t)ctx->r{base}+{offset});'
        elif op=='player_horizontal_displacement':
            if address!=0x801BF828 or site not in (0x801BFA18,0x801BFA24):raise ValueError('Unreviewed horizontal displacement')
            offset=0x18 if site==0x801BFA18 else 0x10
            text=f'ctx->f4.fl=dora3_native_gravity_displacement_f32(ctx->f4.fl,(uint32_t)MEM_W(0,(int32_t)0x8021D4B0)+{offset}); dora3_native_preview_hook(rdram,ctx,0x{site:08X}u);'
        elif op=='gravity_subtract':
            allowed={('.scene1',0x801C0FA8):(2,-55),('.scene1',0x801CB408):(4,-50),('.minigame_8',0x801C42D0):(4,-50)}
            pair=allowed.get((rule['section'],site))
            if pair!=(rule['base_register'],rule['terminal']) or word!=0x46020001:raise ValueError('Unreviewed gravity subtraction')
            label=f'dora3_after_timing_{site:08X}'
            calculation='dora3_native_projectile_gravity_f32(rdram,ctx)' if (rule['section'],site)==('.scene1',0x801CB408) else f'dora3_native_gravity_f32(ctx->f0.fl, ctx->f2.fl, {pair[1]}.0f, (uint32_t)ctx->r{pair[0]} + 0x14u)'
            if (rule['section'],site)==('.scene1',0x801C0FA8):
                text=f'if (dora3_native_simulation_active()) {{ if (!dora3_native_preview_hook(rdram,ctx,0x801C0FA8u)) ctx->f0.fl = {calculation}; goto {label}; }}'
            else:text=f'if (dora3_native_simulation_active()) {{ ctx->f0.fl = {calculation}; goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='gravity_displacement_source':
            allowed={('.scene1',0x801BF8A8):(0,3),('.scene1',0x801BFDA4):(0,3),('.scene1',0x801E8E10):(2,2),('.scene1',0x801CB6EC):(2,5),('.minigame_8',0x801C4318):(2,5)}
            pair=allowed.get((rule['section'],site))
            if pair!=(rule['register'],rule['base_register']):raise ValueError('Unreviewed gravity displacement')
            reg=fpr(rule,'register');text=f'{reg} = dora3_native_gravity_displacement_f32({reg}, (uint32_t)ctx->r{pair[1]} + 0x14u);'
            if (rule['section'],site)==('.scene1',0x801BF8A8):text+=' dora3_native_preview_hook(rdram,ctx,0x801BF8A8u);'
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
        elif op=='cutscene_follow_eye_basis':
            if rule['section']!='.scene1' or address!=0x801DFFEC or (site,word) not in {(0x801E0038,0x46020000),(0x801E0050,0x46020000),(0x801E0064,0x46020000)}:raise ValueError('Unreviewed following eye basis')
            # Original repeated orbit recurrence subtracts the follow offset
            # from the old eye vector before rotation. This per-period basis
            # correction needs fractional time just like its angular drift.
            text='if (dora3_native_cutscene_equations_active()) ctx->f2.fl=dora3_native_rate_f32(ctx->f2.fl);'
        elif op=='cutscene_jump_gravity':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801E810C,0x801E8138,0x46001081):raise ValueError('Unreviewed scripted gravity')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_cutscene_equations_active()) {{ ctx->f2.fl=dora3_native_cutscene_gravity_f32(ctx->f2.fl,ctx->f0.fl,(uint32_t)ADD32(ctx->r16,0x14)); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op=='cutscene_jump_query':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801E7EB8,0x801E7EE0,0x46002100):raise ValueError('Unreviewed scripted jump query')
            text='if (dora3_native_cutscene_equations_active()) ctx->f0.fl=dora3_native_cutscene_query_velocity((uint32_t)ctx->r2,ctx->f0.fl);'
        elif op=='cutscene_axis_displacement':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801E32E0,0x801E330C,0x46020000):raise ValueError('Unreviewed scripted axis displacement')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ ctx->f0.fl += dora3_native_cutscene_axis_drift(rdram,(uint32_t)ctx->r4,(uint32_t)ctx->r5,(uint32_t)(ctx->r3-ctx->r4),ctx->f2.fl,(uint32_t)MEM_W(0x60,ctx->r3)); goto {label}; }}'
            hooks[name,site+4].insert(0,label+':;')
        elif op.startswith('ui_'):
            approved=APPROVED_UI_SITES.get((op,rule['section'],address,site,word))
            if approved is None or any(rule.get(k)!=v for k,v in approved.items()):raise ValueError('Unreviewed UI boundary/equation')
            if op=='ui_source_end':text='dora3_native_ui_source(rdram,ctx);'
            elif op=='ui_source_begin':text='dora3_native_ui_source_begin(rdram,ctx);'
            elif op=='ui_domain':text=f'dora3_native_ui_domain(rdram,{rule["domain"]});'
            elif op=='ui_draw_boundary':text=f'dora3_native_ui_draw_boundary(rdram,0x{address:08X}u,{rule["end"]});'
            elif op=='ui_command_once':
                label=f'dora3_after_ui_sound_{site:08X}'
                text=f'if (!dora3_native_ui_command_once(0x{address:08X}u)) goto {label};'
                hooks[name,site+8].insert(0,label+':;')
            elif op=='ui_velocity':
                dst=(word>>6)&31;vel=rule['velocity'];acc=rule['acceleration'];base=rule['base'];offset=rule['offset']
                label=f'dora3_after_ui_{site:08X}'
                text=f'if (dora3_native_ui_equations_active()) {{ ctx->f{dst}.fl=dora3_native_ui_velocity_f32(ctx->f{vel}.fl,ctx->f{acc}.fl,(uint32_t)ctx->r{base}+{offset}); goto {label}; }}'
                hooks[name,site+4].insert(0,label+':;')
            elif op=='ui_displacement':
                text=f'if (dora3_native_ui_equations_active()) ctx->f{rule["register"]}.fl=dora3_native_ui_displacement_f32(ctx->f{rule["register"]}.fl,(uint32_t)ctx->r{rule["base"]}+{rule["offset"]});'
            elif op=='ui_rate':
                field='d' if rule['double'] else 'fl';helper='f64' if rule['double'] else 'f32'
                text=f'ctx->f{rule["register"]}.{field}=dora3_native_ui_rate_{helper}(ctx->f{rule["register"]}.{field});'
            elif op=='ui_selection_y_drift':
                text='if (dora3_native_ui_equations_active()) { union { uint64_t bits; double value; } gain; gain.bits=LD(0,(int32_t)0x801C6340u); ctx->f2.d=dora3_native_ui_selection_y_f64(ctx->f2.d,ctx->f6.fl,gain.value); }'
            else:raise ValueError('Unknown UI operation')
        elif op=='cutscene_source_end':
            if rule['section']!='.scene1' or (address,site,word)!=(0x801BB080,0x801BB320,0x8FBF0018):raise ValueError('Unreviewed scripted Source endpoint')
            text='dora3_native_cutscene_source(rdram,ctx,1);'
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
            hooks[name,site+4].insert(0,label+':;')
        elif op=='timed_angle_call':
            # This reviewed call saves unscaled velocity in its delay slot.
            # Execute that store before passing displacement to the same angle helper.
            delay=struct.unpack_from('>I',rom,s['rom']+site+4-s['vram'])[0]
            base=rule['base_register'];off=rule['velocity_offset']
            if word!=0x0C00C683 or delay!=int(rule['delay_word'],16) or delay!=((57<<26)|(base<<21)|(14<<16)|off):
                raise ValueError('Unreviewed angle call/delay slot')
            label=f'dora3_after_timing_{site:08X}'
            text=f'if (dora3_native_simulation_active()) {{ MEM_W({off}, ctx->r{base}) = ctx->f14.u32l; ctx->f14.fl = dora3_native_displacement_f32(ctx->f14.fl, {acc(rule)}); dora3_s0_80031A0C(rdram, ctx); goto {label}; }}'
            hooks[name,site+8].insert(0,label+':;')
        else:raise ValueError('Unreviewed timing rule operation')
        hooks[name,site].append(text)
    return [(name,site,' '.join(text)) for (name,site),text in sorted(hooks.items())]
