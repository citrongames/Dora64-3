"""Execute the reviewed enemy closures against frozen Source callbacks; no game/GPU."""
from pathlib import Path
import argparse,hashlib,importlib.util,json,os,re,struct,subprocess,sys
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
    ap.add_argument('--optimization',choices=['O0','O1','O2'],default='O1')
    a=ap.parse_args();g=a.generated.resolve();rom=a.rom.read_bytes();ref=a.enemy_reference.resolve()
    out=ROOT/'build-local/native-enemy-check';out.mkdir(exist_ok=True)
    receipt=json.loads((g/'generation-receipt.json').read_text());old=json.loads((ref/'generation-receipt.json').read_text())
    assert receipt['rom_sha256']==old['rom_sha256']==hashlib.sha256(rom).hexdigest()
    for base,group in [(ROOT,receipt['inputs']),(g,receipt['generated']),(ref,old['generated'])]:
        for name,value in group.items():assert sha(base/name)==value,('Stale',name)
    sections,compiled,_,_=prepare_layout(json.loads((ROOT/'recomp/startup.json').read_text()),rom)
    current=bodies(g);frozen=bodies(ref)
    # Recover exact reviewed hooks from the last committed generator. No guest
    # instruction, call or arbitrary native-looking line is removed by this proof.
    oldroot=out/'committed-timing';(oldroot/'recomp').mkdir(parents=True,exist_ok=True)
    for path in ['tools/native_timing_patches.py','recomp/native-timing-rules.json']:
        data=subprocess.check_output(['git','show','HEAD:'+path],cwd=ROOT)
        target=oldroot/('timing.py' if path.startswith('tools/') else path);target.write_bytes(data)
    spec=importlib.util.spec_from_file_location('enemy_frozen_timing',oldroot/'timing.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);module.ROOT=oldroot
    previous=module.timing_hooks(sections,compiled,rom);latest=timing_hooks(sections,compiled,rom)
    def clean(name,body,hooks):
        for n,pc,text in hooks:
            if n==name:
                assert 1<=body.count(text)<=body.count(f'// 0x{pc:08X}:'),(name,hex(pc),text)
                body=body.replace('    '+text+'\n','') if '    '+text+'\n' in body else body.replace('; '+text,';')
        return tokens(body)
    defs=[tuple(map(lambda x:int(x,0),row)) for row in re.findall(r'Definition\{(\d+),(0x\w+),(0x\w+),(0x\w+)\}',(ROOT/'src/native_enemy.cpp').read_text())]
    assert len(defs)==25 and len(set(defs))==25
    byrom={s['rom']:(i,s) for i,s in enumerate(sections)}
    def symbol(rompc,pc):return compiled[byrom[rompc][0],pc][0]
    inventory=json.loads((ROOT/'build-local/continuation-20260930/enemy357-kind-inventory.json').read_text())['descriptors']
    roots=[];meta=[]
    for kind,rompc,update,contact in defs:
        row=inventory[kind];ctor=[x for x in row['functions']['constructor'] if x['rom']==rompc]
        assert len(ctor)==1,(kind,ctor)
        names=(ctor[0]['symbol'],symbol(rompc,update),symbol(rompc,contact));roots+=list(names)
        meta.append((kind,rompc,*names))
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
    for name in sorted(overlay):
        body=frozen[name]
        for dependency in sorted(overlay,key=len,reverse=True):body=re.sub(r'\b'+dependency+r'\b','enemy_reference_'+dependency,body)
        references.append(body)
    (out/'functions.c').write_text(base+'\n'+declarations+'\n'+'\n'.join(selected.values())+'\n'+'\n'.join(references))
    with (out/'player-update-functions.h').open('a') as stream:
        stream.write('\nextern "C" {\n'+'\n'.join('void '+n+'(uint8_t*,recomp_context*);' for n in selected)+'\n'+declarations+'\n}\n')
    # ROM data is loaded separately per overlay; overlapping VRAM is never identity.
    lines=['#pragma once','struct EnemyFixtureDefinition { unsigned kind,rom;recomp_func_t* create;recomp_func_t* reference_create;uint32_t script; };','static const EnemyFixtureDefinition enemy_fixture_definitions[]={']
    lines += [f'{{{k},0x{r:X}u,{ctor},enemy_reference_{ctor},ENEMY_SCRIPT_{k}}},' for k,r,ctor,update,contact in meta];lines+=['};']
    lines+=['static void enemy_fixture_overlay(std::vector<uint32_t>& memory,unsigned rom) {','switch(rom) {']
    for rompc in sorted({x[1] for x in defs}):
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
        lines=[line.replace(placeholder,'0x%Xu'%script_by_kind.get((kind,rompc),0)) for line in lines]
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
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-variable','-Wno-unused-but-set-variable','-Wno-unused-function','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),str(out/'boundaries.cpp'),'-o',str(binary)],check=True)
    with (out/'result.log').open('w') as stream:
        result=subprocess.run([str(binary),str(out/'seed.bin')],stdout=stream,stderr=subprocess.STDOUT,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'))
    output=(out/'result.log').read_text();print(output,flush=True);result.check_returncode();assert output.startswith('PASS: enemy')
    tracked=sources+[Path(__file__).resolve(),ROOT/'research/native-enemy-source-check.inc',ROOT/'tools/check_native_story_pickups.py',ROOT/'tools/check_native_player_update.py']+sorted((ROOT/'src').glob('*.hpp'))
    report=dict(result=output.strip(),generation_receipt_sha256=sha(g/'generation-receipt.json'),enemy_reference_receipt_sha256=sha(ref/'generation-receipt.json'),source_sha256={x.relative_to(ROOT).as_posix():sha(x) for x in tracked},source_body_proof=sorted(proof),reviewed_kinds=[x[0] for x in defs],synthetic_descriptor_resource_ids=sorted(ids),limitations=['Synthetic models, room geometry, sound queue and resource lookup; no real level/renderer/GPU','Unreviewed scripts/effects return actual failed allocation and are command-traced; not all death particles are instantiated','No claim that every enemy branch/boss or the whole variable FPS task is complete'])
    (ROOT/'research/native-enemy-check.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
