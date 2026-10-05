"""Exercise cache invalidation/retirement guards without running game oracles."""
from pathlib import Path
import hashlib,json,subprocess,tempfile,time
from native_oracle_cache import compilation_inputs,digest,read_group,save_group,verify_inputs

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    stamp=time.monotonic()
    parent=ROOT/'build-local';parent.mkdir(exist_ok=True)
    # Leave diagnostics in ignored build-local, including negative cases.
    out=Path(tempfile.mkdtemp(prefix='oracle-cache-guards-',dir=parent))
    source=out/'fixture.cpp';header=out/'fixture.hpp';seed=out/'seed.bin';binary=out/'check'
    header.write_text('#define CACHE_VALUE 3\n')
    source.write_text('#include "fixture.hpp"\n#include <cstdio>\nint main(){std::puts("PASS: mini fixture");std::puts("PASS: complete guard fixture");\n#ifdef CACHE_FAIL\nreturn CACHE_VALUE;\n#else\nreturn 0;\n#endif\n}\n')
    seed.write_bytes(b'input-v1')
    commands=[['g++','-std=c++20','-O2','-I'+str(out),str(source),'-o',str(binary)]]
    roots=[(out,'$fixture')]
    inputs=compilation_inputs(commands,seed,roots)
    assert '$fixture/fixture.hpp' in inputs['files_sha256'],'Compiler dependency scan omitted included header'
    subprocess.run(commands[0],check=True)
    stdout=out/'stdout';stderr=out/'stderr'
    with stdout.open('w') as out_stream,stderr.open('w') as err_stream:
        process=subprocess.run([str(binary)],stdout=out_stream,stderr=err_stream)
    assert process.returncode==0
    group=dict(id=0,camera_mode=12,source_divider=2,native_fps=[60,73,120,144,165,360])
    cache=out/'cache';producer=dict(execution='mini_fixture_cache_guards_only')
    assert read_group(cache,inputs,group) is None
    save_group(cache,inputs,group,binary,stdout,stderr,0,0,producer)
    cached=read_group(cache,inputs,group);assert cached and cached[1]['reused']
    header.write_text('#define CACHE_VALUE 4\n')
    changed=compilation_inputs(commands,seed,roots)
    assert digest(changed)!=digest(inputs) and read_group(cache,changed,group) is None
    try:verify_inputs(inputs,roots)
    except AssertionError:pass
    else:raise AssertionError('Edit during execution accepted')
    header.write_text('#define CACHE_VALUE 3\n');verify_inputs(inputs,roots)
    seed.write_bytes(b'input-v2')
    changed=compilation_inputs(commands,seed,roots)
    assert digest(changed)!=digest(inputs) and read_group(cache,changed,group) is None
    seed.write_bytes(b'input-v1')
    assert read_group(cache,inputs,dict(group,native_fps=[60,73])) is None
    record_file=Path(cached[1]['cache_record']);run=record_file.parent
    original=(run/'stdout.log').read_bytes();(run/'stdout.log').write_bytes(b'PASS: truncated')
    assert read_group(cache,inputs,group) is None
    (run/'stdout.log').write_bytes(original)
    original=(run/'check').read_bytes();(run/'check').write_bytes(original+b'corrupt')
    assert read_group(cache,inputs,group) is None
    (run/'check').write_bytes(original)
    original=record_file.read_bytes();record_file.write_text('{}')
    assert read_group(cache,inputs,group) is None
    record_file.write_bytes(original)
    assert read_group(cache,inputs,group)
    failed_commands=[commands[0][:-2]+['-DCACHE_FAIL']+commands[0][-2:]]
    failed_inputs=compilation_inputs(failed_commands,seed,roots)
    assert digest(failed_inputs)!=digest(inputs)
    subprocess.run(failed_commands[0],check=True)
    with stdout.open('w') as out_stream,stderr.open('w') as err_stream:
        process=subprocess.run([str(binary)],stdout=out_stream,stderr=err_stream)
    assert process.returncode==3
    save_group(cache,failed_inputs,group,binary,stdout,stderr,3,0,producer)
    assert read_group(cache,failed_inputs,group) is None
    assert read_group(cache,inputs,group),'Failure must preserve previous exact-input success'
    assert read_group(cache,inputs,dict(group,id=1)) is None,'Incomplete/absent group accepted'
    result=dict(result='PASS: cache hit, real included-header/seed/flag invalidation, concurrent-edit guard, settings mismatch, corrupt log/binary/record rejection, real subprocess failure and incomplete group rejection',
        elapsed_seconds=round(time.monotonic()-stamp,3),no_game_oracle_executed=True,no_game_gpu=True,
        source_sha256={name:sha(ROOT/name) for name in ('tools/native_oracle_cache.py','tools/check_native_oracle_cache.py','tools/check_native_player_update_partitions.py')},
        diagnostic_directory=out.relative_to(ROOT).as_posix(),limitations=['Mini C++ fixture tests orchestration/cache safety only; gameplay proofs remain the actual completed043 reports'])
    (ROOT/'research/native-oracle-cache-check.json').write_text(json.dumps(result,indent=2)+'\n')
    print(result['result'],'in',result['elapsed_seconds'],'seconds')
if __name__=='__main__':main()
