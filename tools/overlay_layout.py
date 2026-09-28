"""Section-qualified startup symbols; mutually exclusive overlays keep their identity."""
import struct

def prepare_layout(meta, rom):
    sections=[dict(name='.startup',rom=0x1000,vram=0x80025C00,size=0x58E80,
                   compiled=meta['compiled'],indirect_targets=meta.get('indirect_targets',{}))]
    sections.extend(meta.get('overlays',[]))
    compiled={}; sdk={int(k,16):v for k,v in meta['sdk'].items()}; frontier=set(); names=set(); symbols=set()
    for index,section in enumerate(sections):
        base,size=section['vram'],section['size']
        if (section['name'] in names or base%4 or size%4 or size<=0 or
            not 0x80000000<=base<base+size<=0x80400000 or section['rom']<0 or
            section['rom']%4 or section['rom']+size>len(rom)):
            raise ValueError('Invalid or duplicate section')
        names.add(section['name']); ranges=[]
        for key,value in section['compiled'].items():
            address=int(key,16);length=value[1]
            if (value[0] in symbols or address in sdk or address%4 or length<=0 or length%4 or
                not base<=address<address+length<=base+size):
                raise ValueError(f'Invalid function {section["name"]}:{key}')
            symbols.add(value[0]);compiled[index,address]=value;ranges.append((address,address+length))
        ranges.sort()
        if any(a[1]>b[0] for a,b in zip(ranges,ranges[1:])):raise ValueError('Overlapping functions within section')
    for i,left in enumerate(sections):
        for right in sections[i+1:]:
            if left['rom']<right['rom']+right['size'] and right['rom']<left['rom']+left['size']:
                raise ValueError('Overlapping ROM sections')
            if left['vram']<right['vram']+right['size'] and right['vram']<left['vram']+left['size']:
                group=left.get('overlay_group')
                if not group or group!=right.get('overlay_group') or left is sections[0]:
                    raise ValueError('RAM overlap needs explicit mutually exclusive overlay_group')
    def target_section(source,address):
        matches=[i for i,s in enumerate(sections) if s['vram']<=address<s['vram']+s['size']]
        if source in matches:return source
        if len(matches)!=1:raise ValueError(f'Ambiguous external call {sections[source]["name"]}:{address:08X}')
        return matches[0]
    for index,section in enumerate(sections):
        for key in section.get('indirect_targets',{}):
            addr=int(key,16);target=target_section(index,addr)
            if (target,addr) not in compiled and addr not in sdk:frontier.add((target,addr))
    for (index,addr),(_,size,_) in compiled.items():
        section=sections[index];offset=section['rom']+addr-section['vram']
        for (word,) in struct.iter_unpack('>I',rom[offset:offset+size]):
            if word>>26==3:
                address=((addr+4)&0xF0000000)|((word&0x3ffffff)<<2)
                target=target_section(index,address)
                if (target,address) not in compiled and address not in sdk:frontier.add((target,address))
    for address in sdk:
        if target_section(0,address)!=0:raise ValueError('SDK symbol outside resident section')
    return sections,compiled,sdk,frontier

def pending_name(index,address):return f'pending_s{index}_{address:08X}'
