"""Exact-ROM text adapters. Keep generated C disposable."""
from pathlib import Path
import json,hashlib
def localization_hooks(sections,compiled,rom):
    for entry in json.loads((Path(__file__).resolve().parents[1]/'research/localization-hooks.json').read_text()):
        i=next(i for i,s in enumerate(sections) if s['name']==entry['section'])
        s=sections[i];address=entry['function'];name,size,_=compiled[i,address]
        off=s['rom']+address-s['vram']
        if size!=entry['size'] or hashlib.sha256(rom[off:off+size]).hexdigest()!=entry['sha256']:
            raise ValueError('Unreviewed localization function: '+name)
        hook='dora3_localization_'+entry['role']
        declaration=f'extern {"int" if entry["returns"] else "void"} {hook}(uint8_t*,recomp_context*); '
        call=f'{hook}(rdram,ctx)'
        yield name,entry['site'],declaration+(f'if ({call}) return;' if entry['returns'] else call+';')
