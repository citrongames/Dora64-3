"""Recover verified fixed-load wrappers and inspect the game's 16-byte LZSS records."""
import argparse,json,struct,hashlib
from pathlib import Path
from inspect_rom import ROOT,read_rom


def decode_record(rom,offset):
    if offset<0 or offset+16>len(rom):raise ValueError('Header outside ROM')
    flag,reserved,packed,unpacked=struct.unpack_from('>4I',rom,offset)
    if flag not in (0,1) or reserved!=0 or not 0<unpacked<=0x400000 or not 0<packed<=len(rom)-offset-16:
        raise ValueError('Unsupported or out-of-range record')
    source=rom[offset+16:offset+16+packed]
    if flag==0:
        if packed!=unpacked:raise ValueError('Raw size mismatch')
        return source
    ring=bytearray(1024)
    write=0x3be
    out=bytearray()
    pos=0
    while pos<len(source):
        flags=source[pos];pos+=1
        for bit in range(8):
            if pos==len(source):break
            if flags&(1<<bit):
                values=[source[pos]];pos+=1
                out.extend(values);ring[write]=values[0];write=(write+1)&1023
            else:
                if pos+2>len(source):
                    # ROM routine returns at EOF even between the two reference bytes.
                    # Accept only if the declared output is already complete.
                    if len(out)==unpacked:pos=len(source);break
                    raise ValueError(f'Truncated back-reference at {pos}; decoded {len(out)}/{unpacked}')
                low,high=source[pos:pos+2];pos+=2
                index=low|((high&0xc0)<<2)
                count=(high&0x3f)+3
                for i in range(count):
                    value=ring[(index+i)&1023]
                    out.append(value);ring[write]=value;write=(write+1)&1023
            if len(out)>unpacked:raise ValueError('Output exceeds declared length')
    if len(out)!=unpacked:raise ValueError('Decoded length mismatch')
    return bytes(out)


def wrappers(rom):
    result=[]
    def pair(words,index,reg):
        hi,lo=words[index:index+2]
        if hi>>16 != 0x3c00|reg or lo>>16 != 0x2400|(reg<<5)|reg:
            raise ValueError('Wrapper instruction pattern changed')
        return ((hi&65535)<<16)+struct.unpack('>h',struct.pack('>H',lo&65535))[0]
    for i in range(14):
        address=0x800430b8+i*0x34
        off=address-0x80024c00
        words=struct.unpack_from('>13I',rom,off)
        if words[8]!=0x0c009cc3 or words[9]!=0x00c43023 or words[11]!=0x03e00008:
            raise ValueError('Unexpected DMA call or delay-slot argument calculation')
        start,end,dest=pair(words,2,4),pair(words,4,6),pair(words,6,5)
        if not (0<=start<end<=len(rom) and 0x80000400<=dest<dest+end-start<=0x80400000):
            raise ValueError('Segment exceeds locked ROM/RDRAM bounds')
        result.append({'loader':f'0x{address:08X}','rom_start':f'0x{start:08X}',
                       'rom_end':f'0x{end:08X}','ram_start':f'0x{dest:08X}',
                       'ram_end':f'0x{dest+end-start:08X}','size':end-start,
                       'status':'static-verified raw DMA range; individual function boundaries pending'})
    return result


def survey_records(rom):
    offset=0x1c5de0
    records=[]
    # Stop at the first unknown structure; never reinterpret arbitrary ROM tail as assets.
    while offset+16<=len(rom):
        try:decoded=decode_record(rom,offset)
        except ValueError as error:
            return {'records':records,'stop_offset':f'0x{offset:08X}','stop_reason':str(error)}
        flag,reserved,packed,unpacked=struct.unpack_from('>4I',rom,offset)
        records.append({'offset':f'0x{offset:08X}','compressed':bool(flag),'packed_size':packed,
                        'decoded_size':unpacked,'decoded_sha256':hashlib.sha256(decoded).hexdigest()})
        offset=(offset+16+packed+15)&~15
    return {'records':records,'stop_offset':f'0x{offset:08X}','stop_reason':'End of ROM'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();rom=read_rom(a.rom)
    report={'fixed_loads':wrappers(rom),'record_survey':survey_records(rom)}
    with a.output.open('x',encoding='utf-8') as f:json.dump(report,f,ensure_ascii=False,indent=2);f.write('\n')
    print(f"Fixed loaders: {len(report['fixed_loads'])}; consecutive decoded records: {len(report['record_survey']['records'])}")
    print('Stop:', report['record_survey']['stop_offset'],report['record_survey']['stop_reason'])


if __name__=='__main__':main()
