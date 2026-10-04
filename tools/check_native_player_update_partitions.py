"""Run every full player oracle in four isolated camera/source-rate groups.

Each group retains all six Native FPS values, its 60 FPS comparison baselines,
Original state/resource/command oracles, and all final count/lifetime assertions.
The original tracked fixture and Native runtime remain untouched. Only the
diagnostic copy adds a checked group selector and expected setting count.
"""
from pathlib import Path
import argparse,ast,concurrent.futures,hashlib,json,os,re,subprocess,sys,time

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for key in ['generated','rom','reference','city-reference']:parser.add_argument('--'+key,type=Path,required=True)
    parser.add_argument('--jobs',type=int,choices=[1,2,3,4],default=4)
    parser.add_argument('--optimization',choices=['O0','O1','O2'],default='O2')
    args=parser.parse_args();checker=ROOT/'tools/check_native_player_update.py'
    original=checker.read_text();fixture=ROOT/'research/native-player-update-check.cpp'
    source=fixture.read_text()
    argc='assert(argc==2);'
    loop='for(unsigned camera_mode:{12u,2u})for(unsigned divider:{2u,3u}) {'
    end='assert(check_npc_action_contacts(image)==3192);'
    assert source.count(argc)==source.count(loop)==source.count(end)==1
    derived='#include <cstdlib>\n'+source.replace(argc,'assert(argc==2||argc==3);const int partition=argc==3?std::atoi(argv[2]):-1;assert(partition>=-1&&partition<4);')
    derived=derived.replace(loop,loop+'\n        if(partition>=0&&partition!=int((camera_mode==12?0:2)+(divider==2?0:1)))continue;')
    derived=derived.replace(end,'assert(partition<0?cases==24:cases==6);'+end)
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
    exec(compile(prefix+'\n    globals().update(locals())\nmain()\n',str(checker),'exec'),namespace)
    assert sha(partition_fixture)==hashlib.sha256(derived.encode()).hexdigest()
    groups=[dict(id=i,camera_mode=12 if i<2 else 2,source_divider=2 if i%2==0 else 3,native_fps=[60,73,120,144,165,360]) for i in range(4)]
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    def run(group):
        stamp=time.monotonic();log=out/('group-'+str(group['id'])+'.log')
        print('START full player group'+str(group['id'])+' camera'+str(group['camera_mode'])+' Source'+str(60//group['source_divider']),flush=True)
        with log.open('w') as stderr:
            process=subprocess.run([str(out/'check'),str(out/'seed.bin'),str(group['id'])],text=True,stdout=subprocess.PIPE,stderr=stderr,env=env)
        stdout=out/('group-'+str(group['id'])+'.stdout');stdout.write_text(process.stdout)
        record=dict(**group,exit_code=process.returncode,elapsed_seconds=round(time.monotonic()-stamp,2),stdout_sha256=sha(stdout),stderr_sha256=sha(log))
        (out/('group-'+str(group['id'])+'.json')).write_text(json.dumps(record,indent=2)+'\n')
        assert process.returncode==0,('Failed full player group',record,process.stdout[-1000:])
        print('PASS full player group'+str(group['id']),flush=True)
        return process.stdout.strip().splitlines(),record
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:completed=list(executor.map(run,groups))
    assert len(completed)==4
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
        assert len(lines)>=2 and all(line.startswith('PASS:') for line in lines)
        match=re.fullmatch(pattern,lines[-1]);assert match,('Unexpected summary',record['id'],lines[-1])
        values=list(map(int,match.groups()));assert len(values)==dynamic
        assert values[[v for k,v in tokens if k=='number'].index('cases')]==6
        matches.append(values)
    assert all(lines[:-1]==completed[0][0][:-1] for lines,_ in completed),'Common NPC oracle summaries differ'
    totals=[sum(values[i] for values in matches) for i in range(dynamic)]
    index=0;merged=''
    for kind,value in tokens:
        if kind=='literal':merged+=value
        else:merged+=str(totals[index]);index+=1
    assert totals[[v for k,v in tokens if k=='number'].index('cases')]==24
    # Reuse the existing full report schema with summed counters, not copied
    # historical results. Every group ran all existing C++ final assertions.
    namespace['result']='\n'.join(completed[0][0][:-1])+ '\n'+merged.strip()
    namespace['sources']=[fixture if path==partition_fixture else path for path in namespace['sources']]
    tail=original[original.index('    tracked='):original.rindex("if __name__=='__main__'")]
    exec(compile('def emit_report():\n'+tail+'\nemit_report()\n',str(checker),'exec'),namespace)
    report_path=ROOT/'research/native-player-update-check.json';report=json.loads(report_path.read_text())
    report['scope']='Full player/world/NPC/weapon/damage/death/recovery oracle; four isolated camera/source-rate groups, all24 settings and six Native FPS values'
    report['source_sha256']['tools/check_native_player_update_partitions.py']=sha(Path(__file__).resolve())
    report['partitioned_execution']=dict(groups=[record for _,record in completed],total_settings=24,parallel_jobs=args.jobs,
        original_fixture_sha256=sha(fixture),derived_fixture_sha256=sha(partition_fixture),preparation_checker_sha256=sha(checker),
        unchanged_original_oracles=True,all_final_cpp_assertions=True,retained_as_an_uar_ubsan_lsan=True,
        diagnostic_generated_sha256={name:sha(out/name) for name in ['functions.c','player-update-functions.h','boundaries.cpp']})
    report['limitations'].append('Diagnostic fixture adds only an independent camera/source-rate selector and six-setting count guard; Source/Native oracles unchanged. Groups reset original image/resources per setting; each retains all six FPS baselines. Common3192 NPC oracle runs in each isolated process and is not summed as new distinct cases.')
    report_path.write_text(json.dumps(report,indent=2)+'\n')
    print('PASS: complete full player suite,24 settings across four isolated groups; same Original oracles and all sanitizers; no game/GPU.',flush=True)
if __name__=='__main__':main()
