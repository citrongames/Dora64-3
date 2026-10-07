"""Static GIMO TEX_EDGE texture audit. No game code, tests or renderer execution."""
from pathlib import Path
import argparse, struct, collections, json, hashlib
from analyze_layout import decode_record
from inspect_rom import ROOT as root, read_rom
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--rom',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
args=parser.parse_args()
rom=read_rom(args.rom)
inventory=[];decoded={}
# SDK42E30 uses adjacent table entries as record bounds. Final entries are sentinels.
banks=[(0,0x80071B80,0x1C5DE0,3129),(50,0x80074C68,0x56FC40,324),(51,0x8007517C,0x682240,1523)]
for bank,table,base,count in banks:
    for rid in range(count):
        offset=struct.unpack_from('>I',rom,0x1000+table-0x80025C00+4*rid)[0]+base
        d=decode_record(rom,offset)
        inventory.append({'bank':bank,'id':rid,'rom':offset,'size':len(d),'type':d[:4].hex()})
        if d.startswith(b'GIMO'):decoded[(bank,rid)]=d
materials=[];textures=[];unknown=[];counts=collections.Counter()
def pixels(data,offset,w,h,fmt,siz,palette):
    n=w*h
    if fmt==2 and siz==0:
        indices=[(data[offset+i//2]>>(4 if i%2==0 else 0))&15 for i in range(n)]
        p=[struct.unpack_from('>H',data,palette+2*i)[0] for i in range(16)]
        values=[p[i] for i in indices]
        return [(v>>11&31,v>>6&31,v>>1&31,(v&1)*255) for v in values]
    if fmt==2 and siz==1:
        values=[struct.unpack_from('>H',data,palette+2*data[offset+i])[0] for i in range(n)]
        return [(v>>11&31,v>>6&31,v>>1&31,(v&1)*255) for v in values]
    if fmt==0 and siz==2:
        values=struct.unpack_from('>'+str(n)+'H',data,offset)
        return [(v>>11&31,v>>6&31,v>>1&31,(v&1)*255) for v in values]
    if fmt==0 and siz==3:
        return [tuple(data[offset+4*i:offset+4*i+4]) for i in range(n)]
    if fmt==3 and siz==1:
        return [(v>>4,v>>4,v>>4,(v&15)*17) for v in data[offset:offset+n]]
    return None
for row in inventory:
    if row['type']!='47494d4f':continue
    bank,rid=row['bank'],row['id'];d=decoded[(bank,rid)]
    u=lambda at:struct.unpack_from('>I',d,at)[0]
    parts,desc,segment=u(8),u(12),u(0x44)
    counts['models']+=1
    for part in range(parts):
        at=desc+28*part;mat=u(at);billboard=u(at+12)==33
        for m in range(1000):
            ma=mat+44*m
            if ma+44>len(d):raise ValueError((bank,rid,part,ma))
            dl=u(ma)
            if dl==0:break
            kind=u(ma+4);counts[f'material_{kind}']+=1
            if kind!=13:continue
            counts['cutout_materials']+=1
            material={'bank':bank,'id':f'{rid:04X}','part':part,'material':m,'billboard':billboard,'dl':f'{dl:X}'}
            materials.append(material)
            image=None;palette=None;tile=None;found=set();pending=[dl];seen=set()
            while pending:
                a=pending.pop()
                if a in seen:continue
                seen.add(a)
                for commands in range(40000):
                    if a+8>len(d):unknown.append(dict(material,reason='DL outside resource'));break
                    w0,w1=struct.unpack_from('>II',d,a);a+=8;op=w0>>24
                    if op==0xdf:break
                    if op==0xde:
                        if w1>>24==segment:pending.append(w1&0xffffff)
                        else:unknown.append(dict(material,reason='external nested DL',address=f'{w1:08X}'))
                        if w0&0x10000:break
                    if op==0xfd:image=w1
                    if op==0xf0:palette=image
                    if op==0xf5 and (w1>>24&7)==0:tile=((w0>>21)&7,(w0>>19)&3)
                    if op!=0xf2 or (w1>>24&7)!=0 or tile is None:continue
                    w=((w1>>12&0xfff)-(w0>>12&0xfff))//4+1;h=((w1&0xfff)-(w0&0xfff))//4+1
                    key=(image,palette,tile,w,h)
                    if key in found:continue
                    found.add(key)
                    info=dict(material,width=w,height=h,format=tile[0],size=tile[1],image=f'{image or 0:08X}',palette=f'{palette or 0:08X}')
                    if image is None or image>>24!=segment or (tile[0]==2 and (palette is None or palette>>24!=segment)):
                        unknown.append(dict(info,reason='external bitmap segment'));continue
                    try:pix=pixels(d,image&0xffffff,w,h,*tile,palette&0xffffff if palette else 0)
                    except (IndexError,struct.error):unknown.append(dict(info,reason='sample outside resource'));continue
                    if pix is None:unknown.append(dict(info,reason='unsupported static format'));continue
                    if len(pix)!=w*h:raise ValueError(info)
                    opaque=sum(v[3]>0 for v in pix);transparent=len(pix)-opaque
                    if not (opaque and transparent):continue
                    edge=bad=hidden=0
                    for y in range(h):
                        for x in range(w):
                            p=pix[y*w+x]
                            if p[3]:continue
                            hidden+=any(p[:3])
                            neighbours=[pix[ny*w+nx] for nx,ny in [(x-1,y),(x+1,y),(x,y-1),(x,y+1)] if 0<=nx<w and 0<=ny<h and pix[ny*w+nx][3]]
                            if neighbours:
                                edge+=1
                                bad+=any(p[:3]!=q[:3] for q in neighbours)
                    info.update(transparent=transparent,opaque=opaque,transparent_rgb_nonzero=hidden,edge_transparent=edge,edge_rgb_mismatch=bad)
                    textures.append(info)
report={'rom_sha256':hashlib.sha256(rom).hexdigest(),'resource_banks':[{'bank':b,'table':f'{t:08X}','rom_base':f'{base:08X}','records':n} for b,t,base,n in banks],'bmp_resources':sum(r['type'].startswith('424d50') for r in inventory),'scope':'Static decoded GIMO material13 TEX_EDGE lists in all three ROM banks; no runtime/test/game/render execution; catalog includes models also usable in menus. External bitmap segments are separately listed, never claimed decoded.',
        'counts':dict(counts),'cutout_material_count':len(materials),'mixed_alpha_textures':textures,'unresolved':unknown}
report['unique_mixed_alpha_textures']=len({(t['bank'],t['id'],t['image'],t['palette'],t['format'],t['size'],t['width'],t['height']) for t in textures})
report['mixed_alpha_model_owners']=len({(t['bank'],t['id']) for t in textures})
args.output.write_text(json.dumps(report,indent=2)+'\n')
print('Counts:',dict(counts),'mixed alpha textures',len(textures),'billboard',sum(t['billboard'] for t in textures),'edge mismatches',sum(t['edge_rgb_mismatch']>0 for t in textures),'unresolved',len(unknown))
