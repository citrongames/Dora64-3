"""Execute the reviewed enemy closures against frozen Source callbacks; no game/GPU."""
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
    ap.add_argument('--resume-evidence',type=Path,help='Verified unchanged completed kind scopes; affected/incomplete kinds still execute')
    a=ap.parse_args();g=a.generated.resolve();rom=a.rom.read_bytes();ref=a.enemy_reference.resolve()
    out=ROOT/'build-local/native-enemy-check';out.mkdir(exist_ok=True)
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
    defs=[tuple(map(lambda x:int(x,0),row)) for row in re.findall(r'Definition\{(\d+),(0x\w+),(0x\w+),(0x\w+)\}',(ROOT/'src/native_enemy.cpp').read_text())]
    assert len(defs)==218 and {d[0] for d in defs}==set(range(264,479))|{257,258,259} and len(set(defs))==218
    byrom={s['rom']:(i,s) for i,s in enumerate(sections)}
    def symbol(rompc,pc):return compiled[byrom[rompc][0],pc][0]
    inventory=json.loads((ROOT/'build-local/continuation-20260930/enemy357-kind-inventory.json').read_text())['descriptors']
    roots=[];meta=[];ctor_roms={};spawning_constructors={}
    for kind,rompc,update,contact in defs:
        row=inventory[kind];ctor=[x for x in row['functions']['constructor'] if x['rom']==rompc]
        if kind==394:ctor=[x for x in row['functions']['constructor'] if x['section']=='.lake_slot2']
        if not ctor:ctor=[x for x in row['functions']['constructor'] if x['section']=='.scene1']
        assert len(ctor)==1 or not ctor and int(row['constructor'],16)==0,(kind,ctor)
        contact_rows=[x for x in row['functions']['contact'] if x['rom']==rompc]
        if not contact_rows:contact_rows=[x for x in row['functions']['contact'] if x['section']=='.scene1']
        assert len(contact_rows)==1,(kind,contact_rows)
        names=(ctor[0]['symbol'] if ctor else 'nullptr',symbol(rompc,update),contact_rows[0]['symbol']);roots+=[n for n in names if n!='nullptr']
        meta.append((kind,rompc,*names));ctor_roms[kind]=ctor[0]['rom'] if ctor else rompc
        spawning_constructors[kind]=names[0]!='nullptr' and bool(re.search(r'\b\w+_801E6968\(rdram, ctx\)',current[names[0]]))
    overlay=set();todo=roots.copy()
    while todo:
        name=todo.pop()
        if name in overlay:continue
        overlay.add(name);prefix=name.rsplit('_',1)[0]+'_'
        todo += [n for n in re.findall(r'\b(\w+)\(rdram, ctx\)',current[name]) if n.startswith(prefix) and n in current]
    common_contact_roots=['dora3_s3_801E50C0','dora3_s3_core_801E5828']
    proof=set(overlay)|set(common_contact_roots)|{n for n,pc,text in latest if 'dora3_native_enemy_' in text}
    for name in sorted(proof):
        assert name in frozen and clean(name,current[name],latest)==clean(name,frozen[name],previous),('Guest Source changed',name)
    print('Pinned complete Source bodies:',len(proof),'ROM-qualified enemy kinds:',len(defs),flush=True)
    # Reuse the existing actual actor/model/resource/collision constructors.
    # Preparation is separated from compilation and from the old story test.
    checker=ROOT/'tools/check_native_story_pickups.py';preparation=checker.read_text()
    preparation=preparation[:preparation.index('    flags = ')]
    preparation=preparation.replace("out=ROOT/'build-local'/folder","out=ROOT/'build-local/native-enemy-check'")
    preparation=preparation.replace('    report_name=folder+\'.json\'','    folder=\'native-enemy-check\'\n    report_name=folder+\'.json\'')
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
    parents={268: 267, 273: 272, 275: 274, 276: 274, 278: 277, 279: 277, 280: 277, 284: 283, 285: 283, 286: 283, 288: 287, 289: 287, 291: 290, 292: 290, 294: 293, 295: 293, 296: 293, 298: 297, 299: 297, 301: 300, 302: 300, 303: 300, 304: 300, 306: 305, 307: 305, 309: 308, 310: 308, 311: 308, 313: 312, 314: 312, 315: 312, 317: 316, 324: 323, 328: 327, 329: 327, 333: 332, 334: 332, 335: 332, 337: 336, 338: 336, 340: 339, 342: 341, 345: 343, 346: 343, 347: 343, 349: 348, 350: 348, 351: 348, 352: 348, 353: 348, 355: 354, 356: 354, 358: 357, 360: 359, 361: 359, 363: 362, 365: 364, 367: 366, 368: 366, 369: 366, 371: 370, 372: 370, 373: 370, 375: 374, 376: 374, 378: 377, 380: 379, 381: 379, 383: 382, 384: 382, 386: 385, 388: 387, 389: 387, 390: 387, 391: 387, 394: 393, 396: 395, 397: 395, 400: 399, 401: 399, 402: 399, 403: 399, 404: 399, 405: 399, 406: 399, 407: 399, 408: 399, 409: 399, 411: 410, 412: 410, 413: 410, 414: 410, 415: 410, 416: 410, 417: 410, 418: 410, 419: 410, 420: 410, 421: 410, 422: 410, 423: 410, 424: 410, 425: 410, 426: 410, 427: 410, 429: 428, 430: 428, 431: 428, 432: 428, 433: 428, 434: 428, 436: 435, 437: 435, 438: 435, 439: 435, 440: 435, 441: 435, 442: 435, 443: 435, 444: 435, 445: 435, 447: 446, 448: 446, 449: 446, 450: 446, 451: 446, 452: 446, 453: 446, 455: 454, 456: 454, 457: 454, 458: 454, 459: 454, 460: 454, 461: 454, 462: 454, 463: 454, 464: 454, 465: 454, 466: 454, 467: 454, 468: 454, 469: 454, 470: 454, 471: 454, 473: 472, 474: 472, 475: 472, 476: 472, 477: 472}
    launch_contracts=json.loads((ROOT/'research/native-enemy-launch-contracts.json').read_text())
    launch_by_kind={}
    for kind,rompc,*unused in meta:
        calls=[row for row in launch_contracts if row['kind']==kind and row['rom']==rompc]
        # Match every retained contract to exact canonical ROM instructions.
        for row in calls:
            assert row['function'] in current and row['function'] in frozen
            assert clean(row['function'],current[row['function']],latest)==clean(row['function'],frozen[row['function']],previous)
            asm=[list(match) for match in re.findall(r'// (0x[0-9A-F]+): (.*)',frozen[row['function']])]
            size=row['context_length']
            windows=[asm[j:j+size] for j in range(len(asm)-size+1) if asm[j][0]==row['context_start'] and asm[j+size-1][0]==row['context_end']]
            assert any(hashlib.sha256(json.dumps(window,separators=(',',':')).encode()).hexdigest()==row['context_sha256'] for window in windows),row
            assert [row['pc'],'jal         0x801'+('E70E0' if row['producer']=='E70E0' else 'E6968')] in asm
        producers={row['producer'] for row in calls}
        independent=producers=={'E6968'}
        selectors=[row['selector'] for row in calls if row['producer']=='E70E0' and row['selector'] is not None and 0<=row['selector']<23]
        launch_by_kind[kind]=(not independent,min(selectors) if selectors else 0)
    lines=['#pragma once','struct EnemyFixtureDefinition { unsigned kind,rom;recomp_func_t* create;recomp_func_t* reference_create;uint32_t script;unsigned constructor_rom,parent_kind;bool creates_actor;bool linked_spawn;unsigned link_index; };','static const EnemyFixtureDefinition enemy_fixture_definitions[]={']
    lines += [f'{{{k},0x{r:X}u,{ctor},{'enemy_reference_'+ctor if ctor!='nullptr' else 'nullptr'},ENEMY_SCRIPT_{k},0x{ctor_roms[k]:X}u,{parents.get(k,0)},{str(spawning_constructors[k]).lower()},{str(launch_by_kind[k][0]).lower()},{launch_by_kind[k][1]}}},' for k,r,ctor,update,contact in meta];lines+=['};']
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
    assert ids and scripts
    script_by_kind={}
    for rompc,value,pos in sorted(scripts):
        kind=struct.unpack_from('>H',rom,pos+2)[0]
        if (kind,rompc) not in script_by_kind:script_by_kind[kind,rompc]=value
    for kind,rompc,*unused in meta:
        placeholder='ENEMY_SCRIPT_'+str(kind)
        lines=[line.replace(placeholder,'0x%Xu'%script_by_kind.get((kind,ctor_roms[kind]),script_by_kind.get((kind,rompc),0))) for line in lines]
    lines+=['static const std::set<std::pair<unsigned,int32_t>> enemy_fixture_resource_ids{'+','.join('{%d,%d}'%pair for pair in sorted(ids))+'};']
    (out/'enemy-fixture-data.hpp').write_text('\n'.join(lines)+'\n')
    # Keep unselected direct ABI boundaries fatal. Additional actual calls
    # replace the old fixture's generated fatal definitions, never the game C.
    boundaries=(out/'boundaries.cpp').read_text()
    for name in selected:boundaries=re.sub(r'extern "C" void '+name+r'\(uint8_t\*,recomp_context\*\) \{[^\n]+\}\n?','',boundaries)
    for name in sorted(external-supplied-existing):
        if 'void '+name+'(' not in boundaries:boundaries+=f'\nextern "C" void {name}(uint8_t*,recomp_context*) {{std::fputs("Uncovered enemy boundary {name}\\n",stderr);std::abort();}}\n'
    (out/'boundaries.cpp').write_text(boundaries)
    flags=['-'+a.optimization,'-g','-fno-var-tracking-assignments','-fno-omit-frame-pointer','-fno-strict-aliasing','-DDORA3_ENEMY_CHECK','-DDORA3_STORY_EFFECT_POOL_CHECK',*['-I'+str(x) for x in [ROOT/'src',ROOT/'research',g,g/'RecompiledFuncs',out,ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    signature=c_signature(out,g,flags);cache=out/'functions.sha256'
    if not (out/'functions.o').exists() or not cache.exists() or cache.read_text()!=signature:
        subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
        cache.write_text(signature)
    sources=[ROOT/'src'/n for n in ['native_cutscene.cpp','native_enemy.cpp','native_gameplay_preview.cpp','native_input_timeline.cpp','native_simulation_time.cpp','native_camera_timeline.cpp','native_model_animation.cpp','native_frame_time.cpp']]+[ROOT/'research/native-gameplay-cutscene-fixture.cpp',fixture]
    binary=out/'check'
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
    subprocess.run(['g++','-fsanitize=address,undefined',*map(str,objects),str(out/'functions.o'),'-o',str(binary)],check=True)
    tracked=sources+[Path(__file__).resolve(),ROOT/'research/native-enemy-source-check.inc',ROOT/'research/native-enemy-launch-contracts.json',ROOT/'src/startup_runtime.cpp',ROOT/'tools/check_native_story_pickups.py',ROOT/'tools/check_native_player_update.py']+sorted((ROOT/'src').glob('*.hpp'))
    checked_inputs={x.relative_to(ROOT).as_posix():sha(x) for x in tracked}
    resume=None;requested={d[0] for d in defs}
    if a.resume_evidence:
        resume=json.loads(a.resume_evidence.read_text())
        assert resume['generation_receipt_sha256']==sha(g/'generation-receipt.json')
        eq=ROOT/'research/native-enemy-generation-equivalence-check.json'
        assert sha(eq)==resume['generation_equivalence_report_sha256']
        assert json.loads(eq.read_text())['previous_generation_receipt_sha256']==resume['previous_generation_receipt_sha256']
        retained=set(resume['retained_kinds']);requested=set(resume['rerun_kinds'])
        assert not retained&requested and retained|requested=={d[0] for d in defs}
        assert {k for k,rom,*unused in defs if rom in resume['affected_roms']}<=requested
        for n,h in resume['unchanged_runtime_compiled_inputs_sha256'].items():assert sha(ROOT/n)==h,n
        for n,h in resume['old_progress_sha256'].items():assert sha(ROOT/n)==h,n
        for key in ['old_suite_log','old_checker_binary','old_fixture']:
            r=resume[key];assert sha(ROOT/r['path'])==r['sha256'],key
        assert sha(ROOT/'research/native-enemy-source-check.inc')==resume['current_fixture_sha256']
        case=(ROOT/'research/native-enemy-source-check.inc').read_text();start=case.index('        std::vector<EnemySnapshot> oracle;');end=case.index('        if(progress){progress<<"PASS kind="')
        original_case=case[start:end].replace(resume.get('failure_only_diagnostic_block',''), '') if resume.get('failure_only_diagnostic_block') else case[start:end]
        if resume.get('source_cleanup_only_soundorago_block'):
            assert not retained&set(range(300,308))
            original_case=original_case.replace(resume['source_cleanup_only_soundorago_block'],'')
        assert hashlib.sha256(original_case.encode()).hexdigest()==resume['unchanged_eight_mode_case_body_sha256']
    run_dir=out/('replay-'+str(time.time_ns()));run_dir.mkdir()
    def run_shard(index):
        log=run_dir/('shard-'+str(index)+'.log')
        progress=run_dir/('progress-'+str(index)+'.log')
        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1',
                 DORA3_ENEMY_PROGRESS_PATH=str(progress),DORA3_ENEMY_SHARD_COUNT=str(a.workers),DORA3_ENEMY_SHARD_INDEX=str(index))
        if resume:env['DORA3_ENEMY_INCLUDE_KINDS']=','.join(map(str,sorted(requested)))
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
    print('Enemy replay: '+str(a.workers)+' isolated CPU workers, '+str(48*len(requested))+' fresh cases; '+str(218-len(requested))+' verified unchanged kinds retained; logs '+str(run_dir),flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
        pending=[pool.submit(run_shard,index) for index in range(a.workers)]
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
        assert set(states)=={d[0] for d in defs}
    executed_original=totals['original'];executed_native=totals['native']
    total_original=executed_original+12*(218-len(requested));total_native=executed_native+36*(218-len(requested))
    assert total_original==2616 and total_native==7848
    output=(f"PASS: enemy {total_original} frozen/current Original and {total_native} Native schedules across all218 kinds; "
            f"fresh replay={len(requested)} kinds/{executed_original+executed_native} cases, retained unchanged={218-len(requested)} kinds; {totals['endpoints']} fresh pool endpoints, "
            f"{totals['intermediate']} intervals, {totals['zero_dt']} zero-dt checks, {totals['contacts']} genuine contacts; "
            "218 kinds including all135 ordinary/all79 bosses and4 outbound owners, actual constructors/retirement on synthetic assets; no game/GPU.\n")
    output+='Enemy reached-state coverage: '+' '.join(str(kind)+'='+','.join(map(str,values))+',' for kind,values in sorted(states.items()))
    output+=f" rabbit landing transitions={totals['rabbit_floor']} ground hits={totals['rabbit_ground_hits']} stable ground={totals['rabbit_stable_ground']} fractional samples={totals['rabbit_substeps']}\n"
    (out/'result.log').write_text(output);print(output,flush=True)
    (run_dir/'summary.json').write_text(json.dumps(dict(totals=totals,shards=shards,checked_inputs=checked_inputs),indent=2)+'\n')
    report=dict(result=output.strip(),retained_generation_provenance=resume,fresh_replay_kinds=sorted(requested),retained_kinds=resume['retained_kinds'] if resume else [],generation_receipt_sha256=sha(g/'generation-receipt.json'),enemy_reference_receipt_sha256=sha(ref/'generation-receipt.json'),source_sha256=checked_inputs,optimization=a.optimization,replay_workers=a.workers,replay_summary_sha256=sha(run_dir/'summary.json'),source_body_proof=sorted(proof),reviewed_kinds=[x[0] for x in defs],synthetic_descriptor_resource_ids=sorted(ids),limitations=['Synthetic models, room geometry, sound queue and resource lookup; no real level/renderer/GPU','Synthetic room/model clips cannot establish visual quality in real levels','The coverage report records reached states; enumerating all kinds is not a claim that every AI branch is exercised'])
    (ROOT/'research/native-enemy-check.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
