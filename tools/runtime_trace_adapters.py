"""Qualified functional entry adapters retained without research tracing."""
import json, struct
from inspect_rom import ROOT
def functional_trace_entries(sections,compiled,rom):
    data=json.loads((ROOT/'research/runtime-trace-adapters.json').read_text())
    entries=set()
    for adapter in data['adapters']:
        section_rom,address=adapter['section_rom'],adapter['address']
        index=next(i for i,s in enumerate(sections) if s['rom']==section_rom)
        section=sections[index]
        if (index,address) not in compiled:
            raise ValueError('Missing functional runtime entry')
        offset=section_rom+address-section['vram']
        if struct.unpack_from('>I',rom,offset)[0]!=adapter['word']:
            raise ValueError('Unreviewed functional runtime entry')
        entries.add((index,address))
    return entries
