#!/usr/bin/env python3
"""Offline Dora3 text extraction and translation linting; never executes guest code.

Original Japanese is written only to ignored *_source.json files. Public indices
contain offsets/commands/layout, and translations contain no ROM source text.
"""
from pathlib import Path
import argparse,hashlib,json,re,struct

ROOT=Path(__file__).resolve().parents[1]
COMMANDS=dict(zip('終完続白赤黄青次飛答真偽円',
 ['end','close','next','white','red','yellow','blue','wait','jump','choice','yes','no','cursor']))
PREFIXES=set('共度田静尾剛見')
ROM_HASH='410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602'

def save(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def read(path):return json.loads(path.read_text(encoding='utf-8'))
def decode(data):
    out=[];i=0
    while i+1<len(data):
        pair=data[i:i+2];i+=2
        if pair==b'\0\0':break
        if pair==b'  ':out.append('　');continue
        c=pair.decode('shift_jis')
        if c in COMMANDS:
            token=COMMANDS[c]
            if c in '飛真偽':
                start=i
                while i<len(data) and 48<=data[i]<=57:i+=1
                token+=':'+data[start:i].decode('ascii')
            out.append('{'+token+'}')
        else:out.append(c)
    return ''.join(out)

def extract(rom,assets):
    output=[]
    for name in ['dialogue','game_text']:
        for e in read(assets/(name+'_index.json'))['entries']:
            raw=rom[e['rom']:e['rom']+e['length']]
            output.append(dict(**e,source=decode(raw),sha256=hashlib.sha256(raw).hexdigest()))
    save(assets/'extracted_source.json',output)
    return output

def lint(assets,source):
    glyphs=read(assets/'dialogue_font.json')['glyphs'];index={r['id']:r for r in source}
    errors=[];counts={}
    for language in read(assets/'languages.json')['languages']:
        code=language['code']
        if code=='ja':continue
        seen=set();filled=0;values={}
        for kind in ['dialogue','game_text']:
            for row in read(assets/code/(kind+'.json'))['entries']:
                key=row['id'];value=row['translation'];e=index.get(key);values[key]=value
                if key in seen:errors.append([code,key,'duplicate']);continue
                seen.add(key)
                if e is None:errors.append([code,key,'unknown ID']);continue
                if row.get('original'):
                    if set(re.sub(r'\{[^}]+\}','',e['source']))<=set('×☆○↓！？'):continue
                    errors.append([code,key,'unexpected original-only entry']);continue
                if not value:errors.append([code,key,'empty']);continue
                expected=re.findall(r'\{([^}]+)\}',e['source'])
                if expected!=e['controls']:errors.append([code,key,'index/ROM commands differ'])
                if re.findall(r'\{([^}]+)\}',value)!=expected:errors.append([code,key,'translation commands differ'])
                runs=re.split(r'\{[^}]+\}',value)
                cells=sum((len(run)+1)//2 for run in runs)
                missing=set(''.join(runs))-glyphs.keys()
                if missing:errors.append([code,key,'missing glyphs: '+repr(sorted(missing))])
                if cells>e['max_cells']:errors.append([code,key,f'{cells} cells > {e["max_cells"]}'])
                filled+=1
        for key in index.keys()-seen:errors.append([code,key,'missing record'])
        maximum=0
        for page in read(assets/'dialogue_pages.json')['pages']:
            tokens=0
            for key in page['entries']:
                e=index[key];value=values.get(key)
                if value:
                    tokens+=sum((len(run)+1)//2 for run in re.split(r'\{[^}]+\}',value))+len(e['controls'])
                else:
                    tokens+=sum(len(run) for run in re.split(r'\{[^}]+\}',e['source']))+len(e['controls'])
            maximum=max(maximum,tokens)
            if tokens>=100:errors.append([code,page['entries'][0],f'page: {tokens} tokens >=100'])
        counts[code]=dict(filled=filled,total=len(index),maximum_page_tokens=maximum,errors=sum(e[0]==code for e in errors))
    return dict(counts=counts,errors=errors)

def lint_art(rom,assets,report):
    from PIL import Image
    from analyze_layout import decode_record
    errors=report['errors'];art={}
    for lang in ['en','ru']:
        count=0
        for e in read(assets/'texture_index.json')['entries']:
            path=assets/lang/'textures'/e['file']
            if not path.exists():
                if lang=='en' and e['role'] in ['item-frame','weapon-frame']:continue
                errors.append([lang,e['file'],'missing PNG']);continue
            b=decode_record(rom,e['record']);w,h=struct.unpack_from('>II',b,8)
            if b[:3]!=b'BMP' or b[3] not in [5,6] or (w,h)!=(e['width'],e['height']):errors.append([lang,e['file'],'ROM identity/dimensions'])
            im=Image.open(path).convert('RGBA')
            if im.size!=(w,h):errors.append([lang,e['file'],'PNG dimensions'])
            colors={(r>>3,g>>3,b>>3,1) if a>=128 else (0,0,0,0) for r,g,b,a in im.getdata()}
            if len(colors|{(0,0,0,0)})>256:errors.append([lang,e['file'],'more than256 native palette colours'])
            count+=1
        art[lang]=count
    report['texture_files']=art


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom',required=True,type=Path);p.add_argument('--assets',type=Path,default=ROOT/'assets/localization')
    p.add_argument('--report',type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if hashlib.sha256(rom).hexdigest()!=ROM_HASH:raise ValueError('Unsupported ROM revision')
    source=extract(rom,a.assets);report=lint(a.assets,source);lint_art(rom,a.assets,report)
    if a.report:save(a.report,report)
    print(json.dumps(report['counts'],ensure_ascii=False))
    print('Texture files:',report['texture_files']);print('Static catalog diagnostics:',len(report['errors']))
    if report['errors']:raise SystemExit(1)

if __name__=='__main__':main()
