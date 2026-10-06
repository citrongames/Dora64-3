"""Execute the real verified-DMA overlay state fragments without a renderer."""
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def block(source,anchor):
    assert source.count(anchor)==1,anchor
    start=source.index(anchor);opening=source.index('{',start);depth=0
    tokens=re.finditer(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]',source[opening:],re.S)
    for token in tokens:
        if token[0]=='{':depth+=1
        elif token[0]=='}':
            depth-=1
            if not depth:return source[start:opening+token.end()]
    raise ValueError('Unclosed source block '+anchor)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--generated',required=True,type=Path);p.add_argument('--rom',required=True,type=Path)
    a=p.parse_args();g=a.generated.resolve();receipt=json.loads((g/'generation-receipt.json').read_text())
    assert sha(a.rom)==receipt['rom_sha256']
    for base,key in [(ROOT,'inputs'),(g,'generated')]:
        for n,v in receipt[key].items():assert sha(base/n)==v,('Stale',n)
    runtime=ROOT/'src/startup_runtime.cpp';source=runtime.read_text()
    anchors=['bool overlay_dependencies_active(uint32_t rom)', 'void unload_scene_code()',
             'extern "C" int dora3_overlay_loaded(uint32_t section_rom)',
             'extern "C" uint64_t dora3_overlay_generation(uint32_t section_rom)',
             'extern "C" void dora3_verify_dma(uint8_t* rdram)']
    functions=[block(source,x) for x in anchors]
    dma=block(source,'if(address==0x8002730C)')
    gate=block(source,'if(region.rom==section_rom && (!overlay_active[i] || !overlay_dependencies_active(section_rom)))')
    boot=block(source,'if(address==0x80025C00)')
    state='std::array<std::atomic<bool>,std::size(dora3_overlay_regions)> overlay_active{};'
    pending='struct PendingDma { uint32_t rom, ram, size; bool active; };'
    assert source.count(state)==source.count(pending)==1
    out=ROOT/'build-local/overlay-load-state-check';out.mkdir(exist_ok=True)
    generation='std::array<std::atomic<uint64_t>,std::size(dora3_overlay_regions)> overlay_generation{};\nstd::atomic<uint64_t> next_overlay_generation{1};'
    assert source.count(generation)==1
    extracted='\n'.join([state,generation,pending,'thread_local PendingDma pending_dma{};\nthread_local bool stream_dma_call=false;',*functions,
        'static void runtime_begin_dma(uint8_t* rdram,recomp_context* ctx) {\n'+dma+'\n}',
        'static void runtime_overlay_gate(uint32_t address,uint32_t section_rom) {\nfor(size_t i=0;i<overlay_active.size();++i) {const auto& region=dora3_overlay_regions[i];\n'+gate+'\n}\n}',
        'static void runtime_boot_reset() {\n'+boot+'\n}'])
    # The selected blocks never read these wrapper-only names, except boot/DMA
    # conditions whose literal address is supplied here; bodies remain exact.
    extracted=extracted.replace('static void runtime_begin_dma(uint8_t* rdram,recomp_context* ctx) {','static void runtime_begin_dma(uint8_t* rdram,recomp_context* ctx) {\nconst uint32_t address=0x8002730C;')
    extracted=extracted.replace('static void runtime_boot_reset() {','static void runtime_boot_reset() {\nconst uint32_t address=0x80025C00;')
    (out/'runtime-fragments.inl').write_text(extracted+'\n')
    fixture=ROOT/'research/overlay-load-state-check.cpp';binary=out/'check'
    subprocess.run(['g++','-std=c++20','-O1','-g','-Wall','-Wextra','-Werror','-Wno-unused-parameter',
        '-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-strict-aliasing',
        *['-I'+str(x) for x in [g,out,ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']],str(fixture),'-o',str(binary)],check=True)
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    result=subprocess.run([str(binary),str(a.rom),'positive'],text=True,capture_output=True,env=env)
    if result.returncode:print(result.stderr[-6000:]);result.check_returncode()
    negative={}
    for mode,code in [('missing-parent-load',85),('missing-parent-call',85),('before-load-call',85),('bad-bytes',81),('bad-bounds',81)]:
        run=subprocess.run([str(binary),str(a.rom),mode],text=True,capture_output=True,env=env)
        assert run.returncode==code,(mode,run.returncode,run.stderr[-2000:])
        negative[mode]={'exit':run.returncode,'diagnostic':run.stderr.splitlines()[-1]}
    report={'result':result.stdout.strip(),'generation_receipt_sha256':sha(g/'generation-receipt.json'),
        'source_sha256':{str(x.relative_to(ROOT)):sha(x) for x in [runtime,fixture,Path(__file__).resolve()]},
        'extracted_fragments_sha256':hashlib.sha256(extracted.encode()).hexdigest(),
        'executed_runtime_fragments':anchors+['actual pre-DMA invalidation block','actual call-before-load gate','actual boot reset block'],
        'negative_cases':negative,'sanitizers':'ASan/UBSan/LSan','game_launched':False,
        'limitations':['Runtime overlay registry load/unload endpoints are observations; no OS DMA scheduling, renderer, or full frontend linkage in this component check.']}
    (ROOT/'research/overlay-load-state-check.json').write_text(json.dumps(report,indent=2)+'\n')
    print(result.stdout.strip());print('PASS: all five invalid load/call cases reject with the required runtime diagnostic.')

if __name__=='__main__':main()
