"""ROM-qualified layer boundaries; generated C is never edited by hand."""
from pathlib import Path
import hashlib,json,struct
def widescreen_hooks(sections,compiled,rom):
    evidence=json.loads((Path(__file__).resolve().parents[1]/'research/widescreen-hooks.json').read_text())
    # A JAL continuation can also be reached by a branch that skips the call.
    # Keep a per-invocation flag in the generated C frame, not shared guest/global
    # state: only the path that entered a call scope may close it. This also
    # preserves enclosing scopes and works on repeated loop visits/recursion.
    declarations={}
    for site in evidence.get('sites',[]):
        i=next(i for i,s in enumerate(sections) if s['name']==site['section'])
        a=int(site['address'],16);name=compiled[i,a][0]
        declarations.setdefault((name,a),[]).append(f"int dora3_wide_call_{site['enter']} = 0;")
    for (name,a),items in declarations.items():
        yield name,a,' '.join(items)
    for body in evidence['bodies']+evidence.get('sites',[]):
        i=next(i for i,s in enumerate(sections) if s['name']==body['section'])
        s=sections[i];a=int(body['address'],16);name,size,_=compiled[i,a]
        data=rom[s['rom']+a-s['vram']:s['rom']+a-s['vram']+size]
        if size!=body['size'] or hashlib.sha256(data).hexdigest()!=body['sha256']:
            raise ValueError('Unreviewed widescreen function: '+name)
        kind=body['role']
        if 'enter' in body:
            enter=int(body['enter'],16);leave=int(body['leave'],16)
            if not (a<=enter<a+size-8 and leave==enter+8 and struct.unpack_from('>I',data,enter-a)[0]>>26==3):
                raise ValueError('Widescreen scope is not an original direct call: '+name)
            flag=f'dora3_wide_call_{enter:08X}'
            yield name,enter,f'{flag}=1; extern void dora3_wide_enter(uint8_t*,recomp_context*,unsigned); dora3_wide_enter(rdram,ctx,{kind});'
            yield name,leave,f'if ({flag}) {{ {flag}=0; extern void dora3_wide_leave(uint8_t*,recomp_context*,unsigned); dora3_wide_leave(rdram,ctx,{kind}); }}'
            continue
        yield name,a,f'extern void dora3_wide_enter(uint8_t*,recomp_context*,unsigned); dora3_wide_enter(rdram,ctx,{kind});'
        if kind==10:continue
        for off in range(0,size-4,4):
            if struct.unpack_from('>I',data,off)[0]==0x03E00008:
                yield name,a+off,f'extern void dora3_wide_leave(uint8_t*,recomp_context*,unsigned); dora3_wide_leave(rdram,ctx,{kind});'
