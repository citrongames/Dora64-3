"""Original R/C prefix isolation for the optional PC camera."""
import hashlib,struct
def camera_hooks(sections,compiled,rom):
    i=next(i for i,s in enumerate(sections) if s['name']=='.scene1')
    s=sections[i];address=0x801DBD58
    name,size,_=compiled[i,address]
    offset=s['rom']+address-s['vram']
    if size!=584 or hashlib.sha256(rom[offset:offset+size]).hexdigest()!='df59215484a9babe96b411ff98d3f1e98d410844f714ac632c42709b58cb4e85':
        raise ValueError('Unreviewed ordinary camera dispatch body')
    site=0x801DBD84
    if struct.unpack_from('>I',rom,s['rom']+site-s['vram'])[0]!=0x30620010:
        raise ValueError('Unreviewed camera selected-input prefix')
    yield name,site,'extern int dora3_modern_camera_prepare(uint8_t*); if (dora3_modern_camera_prepare(rdram)) ctx->r3 &= ~0x1C;'
