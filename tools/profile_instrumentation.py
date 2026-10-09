"""Observational C wrappers around unchanged ROM-qualified game bodies."""
import json,re,hashlib
from pathlib import Path

def instrument(generated,sections,compiled,rom):
    root=Path(__file__).resolve().parents[1]
    manifest=json.loads((root/'research/performance-profile-functions.json').read_text())
    if hashlib.sha256(rom).hexdigest()!=manifest['rom_sha256']:
        raise ValueError('Unreviewed profiler ROM')
    # Catalog format is reviewed by the existing annotation pass; the explicit
    # semantic alias in the profile manifest identifies its generated spelling.
    wrapped=[]
    files={p:p.read_text() for p in generated.glob('*.c')}
    for item in manifest['functions']:
        index=next(i for i,s in enumerate(sections) if s['name']==item['section'])
        section=sections[index];address=item['vram']
        name,size=compiled[index,address][:2]
        if name!=item['name'] or size!=item['size']:
            raise ValueError('Unreviewed profiler boundary')
        offset=section['rom']+address-section['vram']
        if hashlib.sha256(rom[offset:offset+size]).hexdigest()!=item['function_rom_sha256']:
            raise ValueError('Unreviewed profiler body')
        aliases={name,item['semantic_name']}
        found=[]
        for file,body in files.items():
            for alias in aliases:
                match=re.search(r'RECOMP_FUNC void '+re.escape(alias)+r'\(uint8_t\* rdram, recomp_context\* ctx\) \{',body)
                if match:found.append((file,alias,match))
        if len(found)!=1:raise ValueError('Ambiguous profiler function '+name)
        file,alias,match=found[0];body=files[file]
        private=alias+'_profile_body'
        body=body[:match.start()]+'RECOMP_FUNC void '+private+'(uint8_t* rdram, recomp_context* ctx) {'+body[match.end():]
        context='    dora3_profile_context(rdram);\n' if item.get('capture_context') else ''
        wrapper=f'''\n/* Observational wrapper: the complete generated body and every return remain above. */
RECOMP_FUNC void {alias}(uint8_t* rdram, recomp_context* ctx) {{
{context}    const uint64_t profile_ticket = dora3_profile_begin({item['tag']});
    {private}(rdram, ctx);
    dora3_profile_end(profile_ticket);
}}
'''
        files[file]=body+wrapper
        wrapped.append({'name':name,'semantic_name':alias,'tag':item['tag'],'file':file.name})
    for file,body in files.items():file.write_text(body)
    (generated/'profile-wrappers.json').write_text(json.dumps({'scope':'Observation only; original generated bodies unmodified','wrapped':wrapped},indent=2)+'\n')
    return wrapped
