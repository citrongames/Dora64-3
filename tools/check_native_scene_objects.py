"""Execute complete scene/NPC descriptor closures against frozen043 Source; no game/GPU.

The existing Source actor/model/parser fixture is reused. Each ROM-qualified
variant gets independent fresh memory; external assets and unrelated producers
remain explicit synthetic boundaries, with exact Source endpoint comparisons.
"""
from pathlib import Path
import argparse,concurrent.futures,hashlib,importlib.util,json,os,re,shlex,struct,subprocess,sys,time
from decomp_annotations import read_decompiled_source
from overlay_layout import prepare_layout
from native_timing_patches import timing_hooks
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def bodies(folder):
    return {re.search(r'void (\w+)\(',b)[1]:b for f in (folder/'RecompiledFuncs').glob('*.c')
            for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
def tokens(text):
    text=re.sub(r'//[^\n]*|/\*.*?\*/','',text,flags=re.S)
    return re.findall(r'"(?:\\.|[^"\\])*"|\w+|[^\s]',text)
def c_signature(out,g,flags):
    headers=sorted((g/'RecompiledFuncs').glob('*.h'))+[g/'startup_hooks.h']+sorted((ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include').glob('*.h'))
    return hashlib.sha256((sha(out/'functions.c')+''.join(sha(x) for x in headers)+repr(flags)).encode()).hexdigest()
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for key in ['generated','rom','reference','city-reference','enemy-reference']:ap.add_argument('--'+key,type=Path,required=True)
    ap.add_argument('--optimization',choices=['O0','O1','O2'],default='O2')
    ap.add_argument('--workers',type=int,choices=range(1,9),default=4)
    ap.add_argument('--completed-replay',type=Path,help='Retain an already completed whole run only when binary, seed, generated C and all actual compiler dependencies remain exact')
    ap.add_argument('--resume-evidence',type=Path,help='Verified unchanged completed kind scopes; affected/incomplete kinds still execute')
    a=ap.parse_args();g=a.generated.resolve();rom=a.rom.read_bytes();ref=a.enemy_reference.resolve()
    out=ROOT/'build-local/native-scene-object-check';out.mkdir(exist_ok=True)
    receipt=json.loads((g/'generation-receipt.json').read_text());old=json.loads((ref/'generation-receipt.json').read_text())
    assert receipt['rom_sha256']==old['rom_sha256']==hashlib.sha256(rom).hexdigest()
    for base,group in [(ROOT,receipt['inputs']),(g,receipt['generated']),(ref,old['generated'])]:
        for name,value in group.items():assert sha(base/name)==value,('Stale',name)
    sections,compiled,_,_=prepare_layout(json.loads((ROOT/'recomp/startup.json').read_text()),rom)
    current=bodies(g);frozen=bodies(ref)
    # Pin exact tooling inputs that actually produced the frozen reference.
    # HEAD already contains the first enemy group and cannot stand in for356.
    target=old['inputs']['tools/native_timing_patches.py']
    revisions=subprocess.check_output(['git','log','--format=%H','--','tools/native_timing_patches.py'],cwd=ROOT,text=True).splitlines()
    matches=[]
    for revision in revisions:
        data=subprocess.check_output(['git','show',revision+':tools/native_timing_patches.py'],cwd=ROOT)
        if hashlib.sha256(data).hexdigest()==target:matches.append((revision,data));break
    assert len(matches)==1,'Exact frozen-reference timing input must remain recoverable'
    revision,data=matches[0]
    rules=subprocess.check_output(['git','show',revision+':recomp/native-timing-rules.json'],cwd=ROOT)
    assert hashlib.sha256(rules).hexdigest()==old['inputs']['recomp/native-timing-rules.json']
    oldroot=out/'frozen-reference-timing';(oldroot/'recomp').mkdir(parents=True,exist_ok=True)
    (oldroot/'timing.py').write_bytes(data);(oldroot/'recomp/native-timing-rules.json').write_bytes(rules)
    spec=importlib.util.spec_from_file_location('enemy_frozen_timing',oldroot/'timing.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);module.ROOT=oldroot
    previous=module.timing_hooks(sections,compiled,rom);latest=timing_hooks(sections,compiled,rom)
    def clean(name,body,hooks):
        grouped={}
        for n,pc,text in hooks:
            if n==name:grouped.setdefault(text,set()).add(pc)
        for text,sites in grouped.items():
            bound=sum(max(body.count(f'// 0x{pc:08X}:'),body.count(f'L_{pc:08X}:')) for pc in sites)
            assert 1<=body.count(text)<=bound,(name,sites,text)
            body=body.replace('    '+text+'\n','') if '    '+text+'\n' in body else body.replace('; '+text,';')
        return tokens(body)
    raw=[tuple(int(x,0) for x in row) for row in re.findall(r'Definition\{(\d+),(0x\w+),(0x\w+),(0x\w+),(0x\w+)\}',(ROOT/'src/native_scene_objects.hpp').read_text())]
    defs=[row[:4] for row in raw]
    assert len(defs)==292 and len(set(defs))==292
    expected=set(range(29,71))|{78,79,81,109,110}|set(range(132,264))-{257,258,259}|set(range(479,592))
    assert {d[0] for d in defs}==expected and len(expected)==289
    byrom={s['rom']:(i,s) for i,s in enumerate(sections)}
    def symbol(rompc,pc):return compiled[byrom[rompc][0],pc][0]
    inventory=json.loads((ROOT/'build-local/continuation-20260930/enemy357-kind-inventory.json').read_text())['descriptors']
    roots=[];meta=[];ctor_roms={};spawning_constructors={}
    for kind,rompc,update,contact in defs:
        row=inventory[kind]
        ctor=[x for x in row['functions']['constructor'] if x['rom']==rompc]
        if not ctor:ctor=[x for x in row['functions']['constructor'] if x['section']=='.scene1']
        if not ctor:ctor=row['functions']['constructor']
        if kind in (586,589):ctor=[x for x in row['functions']['constructor'] if x['section']=='.mountain_slot2'] # actual FFF0 programs 8023E598/E5B4
        assert len(ctor)==1 or not ctor and int(row['constructor'],16)==0,(kind,hex(rompc),ctor)
        contact_rows=[x for x in row['functions']['contact'] if x['rom']==rompc]
        if not contact_rows:contact_rows=[x for x in row['functions']['contact'] if x['section']=='.scene1']
        assert len(contact_rows)==1,(kind,contact_rows)
        names=(ctor[0]['symbol'] if ctor else 'nullptr',symbol(rompc,update),contact_rows[0]['symbol']);roots+=[n for n in names if n!='nullptr']
        meta.append((kind,rompc,*names));ctor_roms[kind,rompc]=ctor[0]['rom'] if ctor else rompc
        spawning_constructors[kind,rompc]=names[0]!='nullptr' and bool(re.search(r'\b\w+_801E6968\(rdram, ctx\)',current[names[0]]))
    overlay=set();todo=roots.copy()
    while todo:
        name=todo.pop()
        if name in overlay:continue
        overlay.add(name);prefix=name.rsplit('_',1)[0]+'_'
        todo += [n for n in re.findall(r'\b(\w+)\(rdram, ctx\)',current[name]) if n.startswith(prefix) and n in current]
    common_contact_roots=['dora3_s3_801E50C0','dora3_s3_core_801E5828']
    proof=set(overlay)|set(common_contact_roots)|{n for n,pc,text in latest if 'dora3_native_enemy_' in text or 'dora3_native_scene_object_' in text}
    proof.update(('dora3_s3_core_801EB864','dora3_s3_core_801EB824'))
    for name in sorted(proof):
        assert name in frozen and clean(name,current[name],latest)==clean(name,frozen[name],previous),('Guest Source changed',name)
    print('Pinned complete Source bodies:',len(proof),'ROM-qualified scene variants:',len(defs),flush=True)
    # Reuse the existing actual actor/model/resource/collision constructors.
    # Preparation is separated from compilation and from the old story test.
    checker=ROOT/'tools/check_native_story_pickups.py';preparation=checker.read_text()
    preparation=preparation[:preparation.index('    flags = ')]
    preparation=preparation.replace("out=ROOT/'build-local'/folder","out=ROOT/'build-local/native-scene-object-check'")
    preparation=preparation.replace('    report_name=folder+\'.json\'','    folder=\'native-scene-object-check\'\n    report_name=folder+\'.json\'')
    saved=sys.argv;sys.argv=[str(checker),'--generated',str(g),'--rom',str(a.rom),'--reference',str(a.reference),'--city-reference',str(a.city_reference),'--scope','effect-pool']
    exec(compile(preparation+'\nmain()\n',str(checker),'exec'),dict(__file__=str(checker),__name__='enemy_preparation'));sys.argv=saved
    base=(out/'functions.c').read_text();existing=set(re.findall(r'RECOMP_FUNC void (\w+)\(',base))
    fixture=ROOT/'research/native-story-pickup-source-check.cpp'
    supplied=set(re.findall(r'void (\w+)\(uint8_t\*',fixture.read_text()))|{'dora3_trace'}
    selected={};external=set();todo=sorted(overlay)+common_contact_roots
    while todo:
        name=todo.pop()
        if name in existing|supplied|set(selected):continue
        if name not in current:external.add(name);continue
        if name.startswith('dora3_native_'):continue
        selected[name]=current[name];todo+=re.findall(r'\b(\w+)\(rdram, ctx\)',current[name])
    declarations='\n'.join('void enemy_reference_'+n+'(uint8_t*,recomp_context*);' for n in sorted(overlay))
    references=[]
    reference_symbols=re.compile(r'\b(?:'+'|'.join(map(re.escape,sorted(overlay,key=len,reverse=True)))+r')\b')
    for name in sorted(overlay):
        body=reference_symbols.sub(lambda match:'enemy_reference_'+match[0],frozen[name])
        references.append(body)
    (out/'functions.c').write_text(base+'\n'+declarations+'\n'+'\n'.join(selected.values())+'\n'+'\n'.join(references))
    with (out/'player-update-functions.h').open('a') as stream:
        stream.write('\nextern "C" {\n'+'\n'.join('void '+n+'(uint8_t*,recomp_context*);' for n in selected)+'\n'+declarations+'\n}\n')
    # ROM data is loaded separately per overlay; overlapping VRAM is never identity.
    parents={133:132,527:525,528:525,564:562}
    launch_by_kind={}
    for kind,rompc,*unused in meta:
        # Exact linked producer calls are distinguished from independent
        # parser calls by the full same-overlay Source closure.
        todo=[unused[1]];done=set();has_link=False;has_independent=False
        while todo:
            name=todo.pop()
            if name in done:continue
            done.add(name);prefix=name.rsplit('_',1)[0]+'_'
            has_link|=bool(re.search(r'\b\w+_801E70E0\(rdram, ctx\)',current[name]))
            has_independent|=bool(re.search(r'\b\w+_801E6968\(rdram, ctx\)',current[name]))
            todo+=[n for n in re.findall(r'\b(\w+)\(rdram, ctx\)',current[name]) if n.startswith(prefix) and n in current]
        launch_by_kind[kind,rompc]=(kind in parents and has_link and not has_independent,0)
    lines=['#pragma once','struct EnemyFixtureDefinition { unsigned case_id,kind,rom;recomp_func_t* create;recomp_func_t* reference_create;uint32_t script;unsigned constructor_rom,parent_kind;bool creates_actor;bool linked_spawn;unsigned link_index; };','static const EnemyFixtureDefinition enemy_fixture_definitions[]={']
    lines += [f'{{{case},{k},0x{r:X}u,{ctor},{"enemy_reference_"+ctor if ctor!="nullptr" else "nullptr"},ENEMY_SCRIPT_{k}_{r},0x{ctor_roms[k,r]:X}u,{parents.get(k,0)},{str(spawning_constructors[k,r]).lower()},{str(launch_by_kind[k,r][0]).lower()},{launch_by_kind[k,r][1]}}},' for case,(k,r,ctor,update,contact) in enumerate(meta)];lines+=['};']
    lines+=['static void enemy_fixture_overlay(std::vector<uint32_t>& memory,unsigned rom) {','switch(rom) {']
    for rompc in sorted({x[1] for x in defs}|set(ctor_roms.values())):
        s=byrom[rompc][1];data=rom[s['rom']:s['rom']+s['size']];assert len(data)%4==0
        values=struct.unpack('>'+str(len(data)//4)+'I',data)
        lines += [f'case 0x{rompc:X}: {{ static const uint32_t words[]={{'+','.join(f'0x{x:08X}u' for x in values)+'};',f'std::copy(std::begin(words),std::end(words),memory.begin()+0x{s["vram"]&0xFFFFFF:X}/4);return; }}']
    lines+=['default:std::abort();}}','static recomp_func_t* enemy_fixture_lookup(unsigned rom,uint32_t address,bool reference) {']
    for name in sorted(overlay):
        i,pc=next((key for key,value in compiled.items() if value[0]==name));s=sections[i]
        lines.append(f'if(rom==0x{s["rom"]:X}u&&address==0x{pc:08X}u)return reference?enemy_reference_{name}:{name};')
    lines+=['return nullptr;}']
    # Explicit ID set from Original descriptor scripts referenced by these
    # same-overlay closures (FFF1..FFF4), rather than a broad numeric range.
    ids=set();scripts=set()
    for name in sorted(overlay):
        i,pc=next(key for key,value in compiled.items() if value[0]==name);s=sections[i]
        asm=re.findall(r'// 0x[0-9A-F]+: (.*)',frozen[name]);regs={}
        for instruction in asm:
            m=re.fullmatch(r'lui\s+\$(\w+), 0x([A-F0-9]+)',instruction)
            if m:regs[m[1]]=int(m[2],16)<<16;continue
            m=re.fullmatch(r'addiu\s+\$(\w+), \$(\w+), (-?0x[A-F0-9]+)',instruction)
            if m and m[2] in regs:
                value=(regs[m[2]]+int(m[3],16))&0xFFFFFFFF;regs[m[1]]=value
                if s['vram']<=value<s['vram']+s['size'] and value%2==0:
                    pos=s['rom']+value-s['vram']
                    if struct.unpack_from('>H',rom,pos)[0]==0xFFF0:scripts.add((s['rom'],value,pos))
    core=byrom[0x70A50][1]
    # Shared effects and Boss7 mask are actual FFF0 descriptor programs.
    # The mask address is table-loaded by its parent, beyond the direct
    # lui/addiu walk above; pin its exact ROM identity and leading kind.
    for rompc,value,kind in ((0x70A50,0x801FC2E0,257),(0x70A50,0x801FC33C,258),
                             (0x70A50,0x801FC36C,259),(0x11D570,0x8021F124,474)):
        section=byrom[rompc][1];pos=section['rom']+value-section['vram']
        assert struct.unpack_from('>HH',rom,pos)==(0xFFF0,kind)
        scripts.add((rompc,value,pos))
    for rompc,value,pos in scripts:
        words=struct.unpack_from('>512H',rom,pos);j=0
        while j<512 and words[j]!=0xFFFF:
            op=words[j];j+=1
            if op==0xFFF0:j+=1
            elif 0xFFF1<=op<=0xFFF4:
                size=words[j];j+=1;assert size<128
                ids.update((op-0xFFF0,id) for id in words[j:j+size]);j+=size
            elif op==0xFFF5:continue
            else:raise ValueError(('Unknown descriptor resource opcode',hex(value),hex(op)))
        assert j<512

    # Complete canonical ROM programs include multi-actor FFF0 chains. Scan
    # only bounded, correctly terminated FFF1..FFF5 programs, never accept a
    # coincident marker as a descriptor. Keep the ROM identity in the report.
    for section in sections:
        for offset in range(0,section['size']-4,2):
            pos=section['rom']+offset
            if struct.unpack_from('>H',rom,pos)[0]!=0xFFF0:continue
            j=pos;resource_ids=[];valid=False
            for operation in range(40):
                if j+2>section['rom']+section['size']:break
                opcode=struct.unpack_from('>H',rom,j)[0];j+=2
                if opcode==0xFFFF:valid=True;break
                if opcode==0xFFF0:
                    if j+2>section['rom']+section['size'] or struct.unpack_from('>H',rom,j)[0]>=592:break
                    j+=2
                elif opcode==0xFFF5:continue
                elif 0xFFF1<=opcode<=0xFFF4:
                    if j+2>section['rom']+section['size']:break
                    count=struct.unpack_from('>H',rom,j)[0];j+=2
                    if count>=128 or j+count*2>section['rom']+section['size']:break
                    resource_ids += [(opcode-0xFFF0,x) for x in struct.unpack_from('>'+str(count)+'H',rom,j)]
                    j+=count*2
                else:break
            if valid:
                scripts.add((section['rom'],section['vram']+offset,pos));ids.update(resource_ids)
    assert ids and scripts
    script_by_kind={}
    for rompc,value,pos in sorted(scripts):
        kind=struct.unpack_from('>H',rom,pos+2)[0]
        if (kind,rompc) not in script_by_kind:script_by_kind[kind,rompc]=value

    synthetic=[];template_ids={(1,0x566),(2,0x567)}
    # Event actors receive resources from the cutscene stream at runtime.
    # The static return-only descriptor has no independent FFF0 program.
    # Construct those through the real parser using explicitly synthetic
    # descriptor programs and the same synthetic model/clip assets as above.
    # This proves ownership/model time, not real cutscene placement/assets.
    lines+=['static void scene_fixture_synthetic_templates(std::vector<uint32_t>& memory) {','auto* rdram=reinterpret_cast<uint8_t*>(memory.data());']
    for case,(kind,rompc,*unused) in enumerate(meta):
        script=script_by_kind.get((kind,ctor_roms[kind,rompc]),script_by_kind.get((kind,rompc),0))
        if not script:
            script=0x80265000+case*32
            values=[0xFFF0,kind,0xFFF1,1,0x566,0xFFF2,1,0x567,0xFFF5,0xFFFF]
            lines += [f'MEM_H({index*2},int32_t(0x{script:X}u))=int16_t(0x{value:X});' for index,value in enumerate(values)]
            synthetic.append(dict(kind=kind,rom=rompc,case_id=case))
        placeholder='ENEMY_SCRIPT_'+str(kind)+'_'+str(rompc)
        lines=[line.replace(placeholder,'0x%Xu'%script) for line in lines]
    lines+=['}'];ids.update(template_ids)
    lines+=['static const std::set<std::pair<unsigned,int32_t>> enemy_fixture_resource_ids{'+','.join('{%d,%d}'%pair for pair in sorted(ids))+'};']
    (out/'enemy-fixture-data.hpp').write_text('\n'.join(lines)+'\n')
    # Keep unselected direct ABI boundaries fatal. Additional actual calls
    # replace the old fixture's generated fatal definitions, never the game C.
    boundaries=(out/'boundaries.cpp').read_text()
    for name in selected:boundaries=re.sub(r'extern "C" void '+name+r'\(uint8_t\*,recomp_context\*\) \{[^\n]+\}\n?','',boundaries)
    for name in sorted(external-supplied-existing):
        if 'void '+name+'(' not in boundaries:boundaries+=f'\nextern "C" void {name}(uint8_t*,recomp_context*) {{std::fputs("Uncovered enemy boundary {name}\\n",stderr);std::abort();}}\n'
    (out/'boundaries.cpp').write_text(boundaries)
    flags=['-'+a.optimization,'-g','-fno-var-tracking-assignments','-fno-omit-frame-pointer','-fno-strict-aliasing','-DDORA3_ENEMY_CHECK','-DDORA3_SCENE_OBJECT_CHECK','-DDORA3_STORY_EFFECT_POOL_CHECK',*['-I'+str(x) for x in [ROOT/'src',ROOT/'research',g,g/'RecompiledFuncs',out,ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    signature=c_signature(out,g,flags);cache=out/'functions.sha256'
    if not (out/'functions.o').exists() or not cache.exists() or cache.read_text()!=signature:
        subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
        cache.write_text(signature)
    sources=[ROOT/'src'/n for n in ['native_cutscene.cpp','native_enemy.cpp','native_gameplay_preview.cpp','native_input_timeline.cpp','native_simulation_time.cpp','native_camera_timeline.cpp','native_model_animation.cpp','native_texture_animation.cpp','native_frame_time.cpp']]+[ROOT/'research/native-gameplay-cutscene-fixture.cpp',fixture]
    binary=out/'check'
    completed=json.loads(a.completed_replay.read_text()) if a.completed_replay else None
    if completed:
        assert completed['generation_receipt_sha256']==sha(g/'generation-receipt.json')
        assert completed['binary_sha256']==sha(binary) and completed['seed_sha256']==sha(out/'seed.bin')
        assert binary.stat().st_mtime_ns==completed['binary_mtime_ns']
        for name,digest in completed['compiled_inputs'].items():assert sha(Path(name))==digest,('Changed compiled dependency',name)
        for name,digest in completed['compiled_artifacts_sha256'].items():assert sha(Path(name))==digest,('Changed compiled artifact',name)
        for name,digest in completed['artifacts_sha256'].items():assert sha(Path(name))==digest,('Changed original replay log',name)

    compile_flags=['-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-variable','-Wno-unused-but-set-variable','-Wno-unused-function','-fsanitize=address,undefined',*flags]
    # Cache compilation only, never a scenario result. Exact source/header
    # content, flags and compiler version must match before reusing an object.
    object_dir=out/('cpp-objects-'+a.optimization);object_dir.mkdir(exist_ok=True)
    toolchain=subprocess.check_output(['g++','-dumpfullversion','-dumpversion'],text=True).strip()
    objects=[]
    for source in [*sources,out/'boundaries.cpp']:
        obj=object_dir/(source.stem+'.o');dep=obj.with_suffix('.d');record=obj.with_suffix('.json')
        def dependencies():
            names=shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]) if dep.exists() else []
            return {str(path):sha(path) for path in map(Path,names)}
        expected=dict(flags=compile_flags,toolchain=toolchain,inputs=dependencies())
        if not obj.exists() or not record.exists() or json.loads(record.read_text())!=expected:
            source_before=sha(source)
            subprocess.run(['g++',*compile_flags,'-MMD','-MF',str(dep),'-c',str(source),'-o',str(obj)],check=True)
            assert sha(source)==source_before,'Source changed during compilation'
            record.write_text(json.dumps(dict(flags=compile_flags,toolchain=toolchain,inputs=dependencies())))
        objects.append(obj)
    if not completed:
        subprocess.run(['g++','-fsanitize=address,undefined',*map(str,objects),str(out/'functions.o'),'-o',str(binary)],check=True)
    tracked=sources+[Path(__file__).resolve(),ROOT/'research/native-scene-object-source-check.inc',ROOT/'src/native_scene_objects.hpp',ROOT/'src/startup_runtime.cpp',ROOT/'tools/check_native_story_pickups.py',ROOT/'tools/check_native_player_update.py']+sorted({Path(name) for record in object_dir.glob('*.json') for name in json.loads(record.read_text())['inputs'] if Path(name).suffix=='.hpp' and Path(name).is_relative_to(ROOT/'src')})
    checked_inputs={x.relative_to(ROOT).as_posix():sha(x) for x in tracked}
    resume=None;requested=set(range(len(defs)))
    if a.resume_evidence:
        resume=json.loads(a.resume_evidence.read_text())
        equivalence=resume.get('input_equivalence',[])
        for item in equivalence:
            current_file=Path(item['path']);prior_file=ROOT/item['old_file']
            assert sha(current_file)==item['new_sha256'] and sha(prior_file)==item['old_sha256']
            text=current_file.read_text()
            for edit in reversed(item['edits']):
                assert text.count(edit['new'])==1;text=text.replace(edit['new'],edit['old'],1)
            assert text==prior_file.read_text()
        def verify_retained(record,live_fixture):
            assert record['generation_receipt_sha256']==sha(g/'generation-receipt.json')
            for name,value in record['unchanged_compiled_dependencies_sha256'].items():
                actual=sha(Path(name))
                if actual!=value:
                    matching=[e for e in equivalence if e['path']==name and e['old_sha256']==value and e['new_sha256']==actual]
                    assert len(matching)==1 and not set(record['retained_variants'])&set(matching[0]['affected_variants']),name
            for name,value in record['artifacts_sha256'].items():assert sha(ROOT/name)==value,name
            prior_fixture=ROOT/record['old_fixture'];assert sha(prior_fixture)==record['old_fixture_sha256']
            for edit in reversed(record.get('fixture_edits',[])):
                assert live_fixture.count(edit['new'])==1
                live_fixture=live_fixture.replace(edit['new'],edit['old'],1)
            assert live_fixture.count(record['new_line'])==1
            assert live_fixture.replace(record['new_line'],record['old_line'],1)==prior_fixture.read_text()
            retained=set(record['retained_variants']);assert not retained&set(record['generic_parser_variants'])
            groups={}
            for name in record['artifacts_sha256']:
                if Path(name).name.startswith('progress-'):
                    for case,divider,profile in re.findall(r'PASS case=(\d+) rom=\d+ kind=\d+ divider=(\d+) profile=(\d+)',(ROOT/name).read_text()):
                        groups.setdefault(int(case),set()).add((int(divider),int(profile)))
            executed={case for case,profiles in groups.items() if profiles=={(d,q) for d in (2,3) for q in (0,1,2)}}
            inherited=set()
            if prior:=record.get('prior_resume_evidence'):
                path=ROOT/prior['path'];assert sha(path)==prior['sha256']
                inherited=verify_retained(json.loads(path.read_text()),prior_fixture.read_text())
                assert inherited<=retained
            assert retained<=executed|inherited,'Retained variant lacks a complete six-profile execution'
            return retained
        requested-=verify_retained(resume,(ROOT/'research/native-scene-object-source-check.inc').read_text())

    run_dir=Path(completed['run_directory']) if completed else out/('replay-'+str(time.time_ns()))
    if not completed:run_dir.mkdir()
    def run_shard(index):
        log=run_dir/('shard-'+str(index)+'.log')
        progress=run_dir/('progress-'+str(index)+'.log')
        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1',
                 DORA3_ENEMY_PROGRESS_PATH=str(progress),DORA3_ENEMY_SHARD_COUNT=str(a.workers),DORA3_ENEMY_SHARD_INDEX=str(index))
        if resume:env['DORA3_ENEMY_INCLUDE_KINDS']=','.join(map(str,sorted(requested)))
        if completed:
            assert log.is_file() and progress.is_file()
            result=type('CompletedProcess',(),{'returncode':0})()
        else:
            with log.open('w') as stream:
                result=subprocess.run([str(binary),str(out/'seed.bin')],stdout=stream,stderr=subprocess.STDOUT,env=env)
        lines=log.read_text().splitlines();records=[x.removeprefix('ENEMY SHARD RESULT ') for x in lines if x.startswith('ENEMY SHARD RESULT ')]
        if result.returncode or len(records)!=1:
            raise RuntimeError('Enemy shard '+str(index)+' failed: '+str(log)+'\n'+'\n'.join(lines[-80:]))
        value=json.loads(records[0]);expected={kind for kind in requested if kind%a.workers==index}
        assert value['index']==index and value['count']==a.workers and len(value['kinds'])==len(expected) and set(value['kinds'])==expected
        assert value['original']==12*len(expected) and value['native']==value['births']==36*len(expected)
        assert {int(kind) for kind in value['states']}==expected
        return value
    shards=[];errors=[]
    print(('Retained completed scene replay; exact compiler inputs: ' if completed else 'Scene replay: ')+str(a.workers)+' isolated CPU workers, '+str(48*len(requested))+' fresh cases; '+str(len(defs)-len(requested))+' verified unchanged kinds retained; logs '+str(run_dir),flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
        pending=[pool.submit(run_shard,index) for index in range(a.workers) if any(case%a.workers==index for case in requested)]
        for future in concurrent.futures.as_completed(pending):
            try:shards.append(future.result())
            except Exception as error:errors.append(str(error))
    progress_text=''.join(x.read_text() for x in sorted(run_dir.glob('progress-*.log')))
    (out/'progress.log').write_text(progress_text)
    if errors:
        output='\n'.join(errors)+'\n';(out/'result.log').write_text(output);print(output,flush=True)
        raise RuntimeError('Full enemy replay failed; all worker logs preserved at '+str(run_dir))
    totals={key:sum(value[key] for value in shards) for key in ('original','native','births','endpoints','intermediate','zero_dt','moving','contacts','rabbit_floor','rabbit_substeps','rabbit_ground_hits','rabbit_stable_ground')}
    covered=[kind for value in shards for kind in value['kinds']]
    assert len(covered)==len(set(covered))==len(requested) and set(covered)==requested
    assert totals['original']==12*len(requested) and totals['native']==totals['births']==36*len(requested) and totals['moving']>0
    assert checked_inputs=={x.relative_to(ROOT).as_posix():sha(x) for x in tracked},'Inputs changed during replay'
    states={int(kind):values for value in shards for kind,values in value['states'].items()}
    if resume:
        states.update({int(k):v for k,v in resume['retained_states'].items()})
        assert set(states)==set(range(len(defs)))
    executed_original=totals['original'];executed_native=totals['native']
    total_original=executed_original+12*(len(defs)-len(requested));total_native=executed_native+36*(len(defs)-len(requested))
    assert total_original==12*len(defs) and total_native==36*len(defs)
    output=(f"PASS: scene {total_original} frozen/current Original and {total_native} Native schedules across all292 ROM variants / 289 kinds; "
            f"fresh replay={len(requested)} kinds/{executed_original+executed_native} cases, retained unchanged={len(defs)-len(requested)} kinds; {totals['endpoints']} fresh pool endpoints, "
            f"{totals['intermediate']} intervals, {totals['zero_dt']} zero-dt checks, {totals['contacts']} genuine contacts; "
            "289 scene/NPC kinds including platforms, doors, switches, traps, event actors and effects, actual constructors/retirement on synthetic assets; no game/GPU.\n")
    output+='Enemy reached-state coverage: '+' '.join(str(kind)+'='+','.join(map(str,values))+',' for kind,values in sorted(states.items()))
    output+=f" rabbit landing transitions={totals['rabbit_floor']} ground hits={totals['rabbit_ground_hits']} stable ground={totals['rabbit_stable_ground']} fractional samples={totals['rabbit_substeps']}\n"
    (out/'result.log').write_text(output);print(output,flush=True)
    (run_dir/'summary.json').write_text(json.dumps(dict(totals=totals,shards=shards,checked_inputs=checked_inputs),indent=2)+'\n')
    report=dict(completed_replay_provenance=completed,result=output.strip(),retained_generation_provenance=resume,fresh_replay_variants=[dict(case_id=i,kind=r[0],rom=r[1]) for i,r in enumerate(defs) if i in requested],retained_kinds=resume['retained_variants'] if resume else [],generation_receipt_sha256=sha(g/'generation-receipt.json'),enemy_reference_receipt_sha256=sha(ref/'generation-receipt.json'),source_sha256=checked_inputs,optimization=a.optimization,replay_workers=a.workers,replay_summary_sha256=sha(run_dir/'summary.json'),source_body_proof=sorted(proof),reviewed_kinds=[x[0] for x in defs],synthetic_descriptor_resource_ids=sorted(ids),synthetic_event_program_variants=synthetic,limitations=['Synthetic models, room geometry, sound queue and resource lookup; no real level/renderer/GPU','Synthetic room/model clips cannot establish visual quality in real levels','The coverage report records reached states; enumerating all kinds is not a claim that every AI branch is exercised'])
    (ROOT/'research/native-scene-object-check.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
