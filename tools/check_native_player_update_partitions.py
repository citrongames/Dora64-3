"""Check full player coverage in four isolated camera/source-rate groups.
Exact-input completed groups retain their original execution; missing or
changed groups execute normally. --verify-cache-only cannot launch an oracle.

Each group retains all six Native FPS values, its 60 FPS comparison baselines,
Original state/resource/command oracles, and all final count/lifetime assertions.
The original tracked fixture and Native runtime remain untouched. Only the
diagnostic copy adds a checked group selector and expected setting count.
"""
from pathlib import Path
import argparse,ast,concurrent.futures,hashlib,json,os,re,subprocess,sys,time
from native_oracle_cache import atomic_json,compilation_inputs,digest,read_group,save_group,verify_inputs

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    overall_started=time.monotonic()
    parser=argparse.ArgumentParser(description=__doc__)
    for key in ['generated','rom','reference','city-reference']:parser.add_argument('--'+key,type=Path,required=True)
    parser.add_argument('--jobs',type=int,choices=[1,2,3,4],default=4)
    parser.add_argument('--optimization',choices=['O0','O1','O2'],default='O2')
    parser.add_argument('--force-run',action='store_true',help='Ignore completed cache groups and execute all four again')
    parser.add_argument('--verify-cache-only',action='store_true',help='Check inputs and retained PASS artifacts; refuse compilation or oracle execution')
    args=parser.parse_args();assert not (args.force_run and args.verify_cache_only);checker=ROOT/'tools/check_native_player_update.py'
    original=checker.read_text();fixture=ROOT/'research/native-player-update-check.cpp'
    source=fixture.read_text()
    argc='assert(argc==2);'
    loop='for(unsigned camera_mode:{12u,2u})for(unsigned divider:{2u,3u}) {'
    end='const unsigned npc_contact_cases=check_npc_action_contacts(image);assert(npc_contact_cases==3192);'
    assert source.count(argc)==source.count(loop)==source.count(end)==1
    derived='#include <cstdlib>\n'+source.replace(argc,'assert(argc==2||argc==3);const int partition=argc==3?std::atoi(argv[2]):-1;assert(partition>=-1&&partition<4);')
    derived=derived.replace(loop,loop+'\n        if(partition>=0&&partition!=int((camera_mode==12?0:2)+(divider==2?0:1)))continue;')
    # The contact oracle resets its own RDRAM/model lifetimes and already
    # enumerates both Source dividers and all six FPS values. It is independent
    # of camera-group selection: run the complete3192 matrix once in group0.
    # Every camera group still executes its six-setting/full-world assertions.
    shared='const unsigned npc_contact_cases=(partition<0||partition==0)?check_npc_action_contacts(image):0;assert(npc_contact_cases==(partition<0||partition==0?3192u:0u));'
    derived=derived.replace(end,'assert(partition<0?cases==24:cases==6);'+shared)
    out=ROOT/'build-local/native-player-update-partitions';out.mkdir(exist_ok=True)
    partition_fixture=out/'partitioned.cpp';partition_fixture.write_text(derived)
    prefix=original[:original.index('    result=subprocess.check_output')]
    location="out=ROOT/('build-local/native-player-update-check' if a.scope=='full' else 'build-local/native-scene-frame-check')"
    assert prefix.count(location)==1
    prefix=prefix.replace(location,"out=ROOT/'build-local/native-player-update-partitions'")
    assert prefix.count("+[ROOT/'research'/fixture_name]")==1
    prefix=prefix.replace("+[ROOT/'research'/fixture_name]","+[out/'partitioned.cpp']")
    compile_c="    subprocess.run(['gcc','-std=c17'"
    assert prefix.count(compile_c)==1
    prefix=prefix.replace(compile_c,"    flags+=['-I'+str(ROOT/'research')]\n"+compile_c)
    sys.argv=[str(checker),'--generated',str(args.generated),'--rom',str(args.rom),'--reference',str(args.reference),'--city-reference',str(args.city_reference),'--optimization',args.optimization]
    namespace=dict(__file__=str(checker),__name__='partition_preparation')
    # Prepare the exact original translation units before deciding what needs
    # to execute. Capture only the two compiler calls; no oracle is launched.
    preparation=ast.parse(prefix+'\n    globals().update(locals())\nmain()\n')
    class CaptureCompilation(ast.NodeTransformer):
        def visit_Call(self,node):
            self.generic_visit(node)
            if (isinstance(node.func,ast.Attribute) and isinstance(node.func.value,ast.Name)
                and node.func.value.id=='subprocess' and node.func.attr=='run'
                and node.args and isinstance(node.args[0],ast.List)
                and isinstance(node.args[0].elts[0],ast.Constant)
                and node.args[0].elts[0].value in ('gcc','g++')):
                node.func=ast.Name(id='capture_compilation',ctx=ast.Load())
            return node
    commands=[]
    namespace['capture_compilation']=lambda command,**kwargs:commands.append(command)
    exec(compile(ast.fix_missing_locations(CaptureCompilation().visit(preparation)),str(checker),'exec'),namespace)
    assert len(commands)==2 and [command[0] for command in commands]==['gcc','g++']
    roots=[(args.generated,'$generated'),(out,'$fixture'),(ROOT,'$repo')]
    inputs=compilation_inputs(commands,out/'seed.bin',roots,
        extra_files=[checker,ROOT/'recomp/native-timing-rules.json',ROOT/'research/player-state-inventory.json'])
    atomic_json(out/'cache-inputs.json',inputs)
    atomic_json(out/'compile-commands.json',commands)
    cache=ROOT/'build-local/native-player-oracle-cache'
    producer=dict(runner_sha256=sha(Path(__file__).resolve()),generation_receipt_sha256=sha(args.generated/'generation-receipt.json'),
                  inputs_sha256=digest(inputs),execution='fresh')
    assert sha(partition_fixture)==hashlib.sha256(derived.encode()).hexdigest()
    groups=[dict(id=i,camera_mode=12 if i<2 else 2,source_divider=2 if i%2==0 else 3,native_fps=[60,73,120,144,165,360],shared_npc_oracle=(i==0)) for i in range(4)]
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    retained={} if args.force_run else {group['id']:read_group(cache,inputs,group) for group in groups}
    missing=[group for group in groups if not retained.get(group['id'])]
    print('Full player inputs checked; retained '+str(4-len(missing))+'/4 groups; fresh '+str(len(missing))+'/4',flush=True)
    if args.verify_cache_only:
        assert not missing,'No exact completed cache for groups '+str([group['id'] for group in missing])+'; nothing compiled or executed'
    elif missing:
        for command in commands:subprocess.run(command,check=True)
        verify_inputs(inputs,roots)
    def run(group):
        saved=retained.get(group['id'])
        if saved:
            print('REUSE full player group'+str(group['id'])+' (exact inputs; original completed execution retained)',flush=True)
            return saved
        stamp=time.monotonic();log=out/('group-'+str(group['id'])+'.log')
        print('START full player group'+str(group['id'])+' camera'+str(group['camera_mode'])+' Source'+str(60//group['source_divider']),flush=True)
        with log.open('w') as stderr:
            process=subprocess.run([str(out/'check'),str(out/'seed.bin'),str(group['id'])],text=True,stdout=subprocess.PIPE,stderr=stderr,env=env)
        stdout=out/('group-'+str(group['id'])+'.stdout');stdout.write_text(process.stdout)
        verify_inputs(inputs,roots)
        record=save_group(cache,inputs,group,out/'check',stdout,log,process.returncode,round(time.monotonic()-stamp,2),producer)
        assert process.returncode==0,('Failed full player group',record,process.stdout[-1000:])
        print('PASS full player group'+str(group['id']),flush=True)
        return process.stdout.strip().splitlines(),dict(record,reused=False)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:completed=list(executor.map(run,groups))
    assert len(completed)==4
    verify_inputs(inputs,roots)
    footer=next(line.strip() for line in reversed(source.splitlines()) if line.strip().startswith('std::cout<<"PASS: '))
    parts=footer.removeprefix('std::cout').removesuffix(';').split('<<')[1:]
    tokens=[];pattern='';dynamic=0
    for part in parts:
        part=part.strip()
        if part.startswith('"'):
            literal=ast.literal_eval(part);tokens.append(('literal',literal));pattern+=re.escape(literal.rstrip('\n'))
        else:
            assert re.fullmatch(r'\w+(?:\*\d+)?',part),part
            tokens.append(('number',part));pattern+=r'(\d+)';dynamic+=1
    matches=[]
    for lines,record in completed:
        assert len(lines)>=1 and all(line.startswith('PASS:') for line in lines)
        match=re.fullmatch(pattern,lines[-1]);assert match,('Unexpected summary',record['group']['id'],lines[-1])
        values=list(map(int,match.groups()));assert len(values)==dynamic
        assert values[[v for k,v in tokens if k=='number'].index('cases')]==6
        matches.append(values)
    common=[lines[:-1] for lines,record in completed if record['group']['id']==0]
    assert len(common)==1 and len(common[0])==1 and common[0][0].startswith('PASS: 3192 real common NPC collision/B priority')
    assert all(not lines[:-1] for lines,record in completed if record['group']['id']!=0),'NPC matrix repeated in an unrelated camera group'
    shared_lines=common[0]
    totals=[sum(values[i] for values in matches) for i in range(dynamic)]
    index=0;merged=''
    for kind,value in tokens:
        if kind=='literal':merged+=value
        else:merged+=str(totals[index]);index+=1
    assert totals[[v for k,v in tokens if k=='number'].index('cases')]==24
    assert totals[[v for k,v in tokens if k=='number'].index('npc_contact_cases')]==3192
    # Reuse the existing full report schema with summed counters, not copied
    # historical claims. Every retained/fresh result comes from a completed
    # execution of all existing C++ final assertions.
    if args.verify_cache_only:
        verification=dict(result='PASS: exact-input completed cache covers all24 settings; no compilation or oracle re-execution',
            inputs_sha256=digest(inputs),total_settings=24,elapsed_seconds=round(time.monotonic()-overall_started,3),groups=[record for _,record in completed],
            retained_summary='\n'.join(shared_lines)+'\n'+merged.strip(),
            source_sha256={name:sha(ROOT/name) for name in ('tools/check_native_player_update_partitions.py','tools/native_oracle_cache.py')},
            original_full_report_unchanged=True,no_game_gpu=True)
        atomic_json(ROOT/'research/native-player-update-cache-check.json',verification)
        print(verification['result'],flush=True)
        return
    namespace['result']='\n'.join(shared_lines)+ '\n'+merged.strip()
    namespace['sources']=[fixture if path==partition_fixture else path for path in namespace['sources']]
    tail=original[original.index('    tracked='):original.rindex("if __name__=='__main__'")]
    exec(compile('def emit_report():\n'+tail+'\nemit_report()\n',str(checker),'exec'),namespace)
    report_path=ROOT/'research/native-player-update-check.json';report=json.loads(report_path.read_text())
    report['scope']='Full player/world/NPC/weapon/damage/death/recovery oracle; four isolated camera/source-rate groups, all24 settings and six Native FPS values'
    report['source_sha256']['tools/check_native_player_update_partitions.py']=sha(Path(__file__).resolve())
    report['source_sha256']['tools/native_oracle_cache.py']=sha(ROOT/'tools/native_oracle_cache.py')
    report['partitioned_execution']=dict(groups=[record for _,record in completed],total_settings=24,parallel_jobs=args.jobs,
        cache_inputs_sha256=digest(inputs),fresh_groups=len(missing),retained_groups=4-len(missing),
        original_fixture_sha256=sha(fixture),derived_fixture_sha256=sha(partition_fixture),preparation_checker_sha256=sha(checker),
        unchanged_original_oracles=True,all_final_cpp_assertions=True,shared_npc_contact_oracle=dict(group_id=0,cases=3192,executions=1,scope='Both Source dividers, all six FPS, twelve NPC kinds and all existing rejection/priority/held-B scenarios; independent seeded RDRAM/model lifetimes'),retained_as_an_uar_ubsan_lsan=True,
        diagnostic_generated_sha256={name:sha(out/name) for name in ['functions.c','player-update-functions.h','boundaries.cpp']})
    report['limitations'].append('Diagnostic fixture selects four camera/source-rate groups with six-setting guards; all24 settings and Source/Native oracles retained. The independently seeded3192 NPC contact matrix covers both dividers/all six FPS in group0 once; groups1..3 omit duplicate execution. Aggregation requires exactly3192 contacts and exactly one complete contact summary; no scenario coverage removed.')
    report['limitations'].append('Cached groups retain actual execution provenance and artifacts; exact compiler/header/generated-code/seed/sanitizer inputs checked before reuse. A retained PASS is not a new run. --force-run ignores reuse; --verify-cache-only cannot compile or execute the oracle.')
    report_path.write_text(json.dumps(report,indent=2)+'\n')
    print('PASS: complete full player coverage,24 settings; fresh groups='+str(len(missing))+'; retained completed groups='+str(4-len(missing))+'; same Original oracles, one complete independent3192 NPC matrix and sanitizers; no game/GPU.',flush=True)
if __name__=='__main__':main()
