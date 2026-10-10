"""Typeset new UI labels from the project's reusable bitmap font.
All geometry is authored on the native pixel grid. No ROM image is edited.
"""
from pathlib import Path
import json,sys,math,shutil
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1]; A=R/'assets/localization'; W=R/'build-local/localization-preview'; W.mkdir(parents=True,exist_ok=True)
rows=[dict(id=int(r['resource'].split(':')[1],16),record=hex(r['record']),width=r['width'],height=r['height']) for r in json.loads((A/'texture_index.json').read_text())['entries']];lookup={r['id']:r for r in rows}
font=Image.open(A/'dialogue_font.bmp').convert('RGB');glyphs=json.loads((A/'dialogue_font.json').read_text())['glyphs']
# Explicitly authored palette/style profiles, including the original normal and
# highlighted button colours. Font shapes remain the shared hand-drawn atlas.
styles={
 'white':('#ffffff','#b5b5bd','#080810',1,False),
 'gold':('#ffff94','#ffb529','#181008',1,True),
 'orange':('#fff773','#ff8418','#211008',1,True),
 'green':('#adff63','#18de10','#001800',1,False),
 'blue':('#bdefff','#315ade','#080829',1,False),
 'pink':('#fff7d6','#ff52bd','#ef1863',2,True),
 'cyan':('#63ffff','#10d6ff','#1031d6',2,True),
 'finish':('#ffb5e7','#ff42a5','#ffffc6',2,True),
 'red':('#ffce84','#e73921','#631010',1,True),
 'yellow':('#ffff94','#e7bd00','#524200',1,True),
 'foodgreen':('#b5ff42','#10ad08','#185200',1,True),
 'dimred':('#c65a52','#8c2918','#4a0808',1,True),
 'dimyellow':('#bda529','#8c7300','#423900',1,True),
 'dimgreen':('#63a521','#318c00','#184200',1,True),
}
def shape(text,bold=False,italic=False):
 points=set();advance=0
 for ch in text:
  if ch==' ':advance+=4;continue
  code=glyphs[ch];pts={(x,y) for y in range(12) for x in range(12) if sum(font.getpixel((code%16*12+x,code//16*12+y)))>=576}
  if not pts:advance+=4;continue
  left=min(x for x,y in pts);right=max(x for x,y in pts)
  for x,y in pts:
   px=advance+x-left+((9-y)//4 if italic else 0)
   points.add((px,y));
   if bold:points.add((px+1,y))
  advance+=right-left+2+int(bold)
 if not points:return {(0,0)},1,1
 l=min(x for x,y in points);t=min(y for x,y in points)
 points={(x-l,y-t) for x,y in points};return points,max(x for x,y in points)+1,max(y for x,y in points)+1
def label(canvas,text,box,style='white',big=False):
 x,y,w,h=box;top,bottom,edge,stroke,italic=styles[style]
 if h<=10:stroke=0
 p,pw,ph=shape(text,big,italic)
 # Large words grow by integer pixel cells. Narrow labels use the thin original
 # face; never squash glyph widths to force a long translation into its slot.
 if pw+stroke*2+1>w and big:p,pw,ph=shape(text,False,italic)
 if big:
  q,qw,qh=shape(text,False,italic)
  if min((w-stroke*2-1)//qw,(h-stroke*2-1)//qh)>min((w-stroke*2-1)//pw,(h-stroke*2-1)//ph):p,pw,ph=q,qw,qh
 factor=max(1,min((w-stroke*2-1)//pw,(h-stroke*2-1)//ph))
 if pw*factor+stroke*2+1>w or ph*factor+stroke*2+1>h:
  raise ValueError(f'{text!r} needs {pw+stroke*2+1}x{ph+stroke*2+1}, slot {w}x{h}')
 sw=pw*factor;sh=min(h-stroke*2-1,ph*factor+(max(0,h//5) if h>=14 else 0))
 p={(xx*factor+dx,dy) for xx,yy in p for dx in range(factor) for dy in range(yy*sh//ph,(yy+1)*sh//ph)}
 x+=max(stroke,(w-sw)//2);y+=max(stroke,(h-sh)//2-1)
 d=ImageDraw.Draw(canvas)
 for xx,yy in p:
  for dy in range(-stroke,stroke+1):
   for dx in range(-stroke,stroke+1):
    if abs(dx)+abs(dy)<=stroke+1:d.point((x+xx+dx+1,y+yy+dy+1),fill=edge)
 from PIL.ImageColor import getrgb
 a=getrgb(top);b=getrgb(bottom)
 for xx,yy in p:
  k=yy/max(1,sh-1);d.point((x+xx,y+yy),fill=tuple(round(a[i]*(1-k)+b[i]*k) for i in range(3))+(255,))
def button(im,hue,active):
 palettes={'blue':('#9ce7ff','#428cbd','#215273'),'red':('#ff94ad','#d64263','#7b1029'),'yellow':('#ffff73','#c6ce10','#7b8410')}
 light,base,dark=palettes[hue];d=ImageDraw.Draw(im);w,h=im.size
 d.rounded_rectangle((0,0,w-1,h-1),radius=5,fill=dark)
 d.rounded_rectangle((1,0,w-2,h-3),radius=4,fill=light if active else base)
 d.rounded_rectangle((3,3,w-4,h-4),radius=3,fill=base if active else dark)
 d.line((6,2,w-7,2),fill=light,width=1)
def panel(im):
 d=ImageDraw.Draw(im);w,h=im.size;d.rounded_rectangle((0,0,w-1,h-1),radius=4,fill='#292963');d.point((1,1),fill='#ffff84');d.point((w-2,h-2),fill='#ffff84')

# ID, semantic filename, English, Russian, style. All dimensions are ROM-locked.
spec=[
 (0xA6A,'status','STATUS','СТАТУС','orange'),(0xA6D,'resume','RESUME','ПРОДОЛЖИТЬ','white'),(0xA6E,'quit','QUIT','ВЫЙТИ','white'),
 (0xA75,'pause','PAUSE','ПАУЗА','gold'),(0xA76,'item','ITEM','ВЕЩИ','white'),(0xA77,'weapon','WEAPON','ОРУЖИЕ','white'),(0xA78,'stats','STATUS','СТАТУС','white'),(0xA79,'yes','YES','ДА','white'),(0xA7A,'no','NO','НЕТ','white'),
 (0xA87,'ready','READY?','ГОТОВЫ?','pink'),(0xA88,'start','START!','СТАРТ!','cyan'),(0xA89,'last-ten','10 SECONDS','10 СЕКУНД','gold'),(0xA8A,'finish','FINISH!','ФИНИШ!','finish'),(0xA8B,'new-record','NEW RECORD','РЕКОРД!','orange'),(0xA8C,'draw','DRAW','НИЧЬЯ','gold'),
 (0xA8D,'doraemon','DORAEMON','ДОРАЭМОН','white'),(0xA8E,'nobita','NOBITA','НОБИТА','white'),(0xA8F,'shizuka','SHIZUKA','СИДЗУКА','white'),(0xA90,'suneo','SUNEO','СУНЭО','white'),(0xA91,'gian','GIAN','ДЖАЙАН','white'),(0xA92,'dorami','DORAMI','ДОРАМИ','white'),(0xA93,'wins','WINS!','ПОБЕДА!','gold'),
 (0xAE1,'best-lap','BEST LAP','ЛУЧШ. КРУГ','gold'),(0xAE2,'best-time','BEST TIME','ЛУЧШ. ВРЕМЯ','gold'),(0xAE3,'final-lap','FINAL','ФИНАЛ','gold'),(0xAE4,'lap','LAP','КРУГ','gold'),(0xAE5,'last-lap','LAST','ПОСЛ.','gold'),(0xAEA,'lap-alt','LAP','КРУГ','gold'),(0xAEB,'total','TOTAL','ВСЕГО','gold'),
 (0xB1C,'dorayaki-dim','DORAYAKI','ДОРАЯКИ','dimred'),(0xB1D,'dorayaki','DORAYAKI','ДОРАЯКИ','red'),(0xB1E,'dango-dim','DANGO','ДАНГО','dimyellow'),(0xB1F,'dango','DANGO','ДАНГО','yellow'),(0xB20,'melon-dim','MELON','ДЫНЯ','dimgreen'),(0xB21,'melon','MELON','ДЫНЯ','foodgreen'),
 (0xB2B,'apologize','APOLOGIZE','ИЗВИНИТЬСЯ','gold'),(0xB2C,'cheer','CHEER','ПОДБОДРИТЬ','green'),(0xB97,'penalty','PENALTY','ШТРАФ','orange'),
 (0xB9E,'press-start','PRESS START','НАЖМИ START','white'),(0xBA1,'save','SAVE','СОХР.','white'),
 (0xBCF,'file-select','FILE|SELECT','ВЫБОР|ФАЙЛА','gold'),(0xBD4,'select-dim','SELECT','ВЫБОР','white'),(0xBD5,'select','SELECT','ВЫБОР','white'),(0xBD7,'erase-dim','ERASE','СТЕРЕТЬ','white'),(0xBD8,'erase','ERASE','СТЕРЕТЬ','white'),(0xBD9,'copy-dim','COPY','КОПИЯ','white'),(0xBDA,'copy','COPY','КОПИЯ','white'),
 (0xBDB,'area','AREA','ЗОНА','white'),(0xBE9,'no-data','NO|DATA','НЕТ|ДАННЫХ','gold'),(0xBF4,'continue','CONTINUE?','ПРОДОЛЖИТЬ?','gold'),(0xBF5,'continue-yes','YES','ДА','green'),(0xBF6,'continue-no','NO','НЕТ','green'),(0xBF7,'credits','CREDITS','АВТОРЫ','green'),
 (0xC11,'human','HUMAN','ИГРОК','gold'),(0xC12,'computer','CPU','КОМП.','blue'),(0xC2B,'minigame-start','START','СТАРТ','white'),
]
entries=[];previews={code:[] for code in ['en','ru']}
for ident,name,en,ru,style in spec:
 row=lookup[ident];w,h=row['width'],row['height'];filename=f'{ident:04x}-{name}.png'
 entries.append(dict(file=filename,record=int(row['record'],16),resource=f'0:{ident:04x}',width=w,height=h,role=name))
 for code,text in [('en',en),('ru',ru)]:
  im=Image.new('RGBA',(w,h));box=(0,0,w,h)
  if ident in [0xBD4,0xBD5,0xBD7,0xBD8,0xBD9,0xBDA]:
   button(im,'blue' if ident<0xBD7 else 'red' if ident<0xBD9 else 'yellow',ident in [0xBD5,0xBD8,0xBDA]);box=(3,3,w-6,h-6)
  if ident in [0xB2B,0xB2C]:
   d=ImageDraw.Draw(im);d.rounded_rectangle((0,0,w-1,h-1),radius=6,fill='#c6ef18' if ident==0xB2B else '#de7b08');d.rounded_rectangle((2,2,w-3,h-3),radius=4,fill='#631873' if ident==0xB2B else '#ffde42');box=(3,3,w-6,h-6)
  if ident==0xBA1:
   d=ImageDraw.Draw(im);d.rounded_rectangle((0,0,13,9),radius=2,fill='#9c9ca5');d.text((4,-2),'L',fill='#ffffff');box=(16,0,w-16,h)
  if ident==0xC2B:
   d=ImageDraw.Draw(im);d.ellipse((0,1,10,11),fill='#bd2110');d.text((3,0),'S',fill='#ff9442');box=(12,0,w-12,h)
  lines=text.split('|');x,y,bw,bh=box
  for n,line in enumerate(lines):label(im,line,(x,y+bh*n//len(lines),bw,bh//len(lines)),style,ident in [0xA75,0xA87,0xA88,0xA8A,0xBCF])
  dest=A/code/'textures';dest.mkdir(exist_ok=True);im.save(dest/filename);previews[code].append((filename,im))
# Multi-row item commands: same baselines/line count as the original image.
for ident,en,ru in [(0xA61,['USE','REVIVE','INFO'],['ПРИМЕН.','ОЖИВИТЬ','ОПИСАНИЕ']),(0xA62,['EQUIP','REVIVE','INFO'],['ВЫБРАТЬ','ОЖИВИТЬ','ОПИСАНИЕ']),(0xC18,['HUMAN','CPU'],['ИГРОК','КОМП.'])]:
 row=lookup[ident];w,h=row['width'],row['height'];filename=f'{ident:04x}-commands.png';entries.append(dict(file=filename,record=int(row['record'],16),resource=f'0:{ident:04x}',width=w,height=h,role='commands'))
 for code,lines in [('en',en),('ru',ru)]:
  im=Image.new('RGBA',(w,h))
  if ident==0xC18:panel(im)
  for n,t in enumerate(lines):label(im,t,(0,(0 if n==0 else 52) if ident==0xC18 else n*17,w,16),'gold' if ident==0xC18 and n==0 else 'green' if ident==0xC18 else 'white')
  im.save(A/code/'textures'/filename);previews[code].append((filename,im))
(A/'texture_index.json').write_text(json.dumps({'format_version':1,'identity':'ROM palette and texels, before original scaled sprite clipping','entries':entries},indent=2)+'\n')
for code,images in previews.items():
 sheet=Image.new('RGB',(1100,math.ceil(len(images)/5)*90),'#303039');d=ImageDraw.Draw(sheet)
 for i,(n,im) in enumerate(images):
  x=i%5*220;y=i//5*90;d.text((x+4,y+2),n,fill='white');scale=min(3,210/im.width,64/im.height);im=im.resize((int(im.width*scale),int(im.height*scale)),Image.Resampling.NEAREST);sheet.paste(im,(x+4,y+20),im)
 sheet.save(W/f'localized-ui-{code}.png')
print(len(entries),'UI textures per language')

index=json.loads((A/'texture_index.json').read_text())
def add(ident,name,**extra):
 r=lookup[ident];f=f'{ident:04x}-{name}.png';index['entries'].append(dict(file=f,record=int(r['record'],16),resource=f'0:{ident:04x}',width=r['width'],height=r['height'],role=name,**extra));return f,r['width'],r['height']
# New label textures composed by the loader into ROM panels. The controller
# picture and inventory frames remain the original ROM pixels outside regions.
for ident,name,region,en,ru,bg in [
 (0xC10,'voice-controller',[1,1,62,11],'1P+VOICE','1P+ГОЛОС','#ffffe7'),
 (0xA5A,'item-frame',[118,3,55,10],'ITEM','ВЕЩИ','#bd5208'),
 (0xA5E,'weapon-frame',[104,3,78,10],'WEAPON','ОРУЖИЕ','#397b94')]:
 f,w,h=add(ident,name,regions=[region])
 for code,text in [('en',en),('ru',ru)]:
  if ident!=0xC10 and code=='en':continue # English is already on the ROM frame.
  im=Image.new('RGBA',(w,h));d=ImageDraw.Draw(im);x,y,bw,bh=region;d.rectangle((x,y,x+bw-1,y+bh-1),fill=bg)
  old=styles['white'];styles['white']=('#181810','#181810','#ffffe7',0,False) if ident==0xC10 else (('#b5efff','#94deef','#397b94',0,False) if ident==0xA5E else ('#ffbd63','#ffa54a','#bd5208',0,False))
  label(im,text,region);styles['white']=old;im.save(A/code/'textures'/f)
for ident in [0x96B,0x96C]:
 f,w,h=add(ident,'gian-recital',material=True)
 for code,text in [('en','GIAN LIVE!'),('ru','КОНЦЕРТ ДЖАЙАНА')]:
  im=Image.new('RGBA',(w,h));d=ImageDraw.Draw(im)
  d.polygon([(4,12),(22,5),(65,1),(106,5),(124,12),(113,26),(15,26)],fill='#ffffc6')
  for n in range(28):
   a=math.pi*2*n/28;x=64+58*math.cos(a);y=15+12*math.sin(a)
   d.ellipse((round(x)-3,round(y)-3,round(x)+3,round(y)+3),fill='#e7b521',outline='#ad8c31')
   d.rectangle((round(x)-1,round(y)-2,round(x)+1,round(y)),fill='#ffff94' if (n+ident)%2 else '#ffffff')
  styles['sign']=('#ff94de','#ff31b5','#ffffe7',0,True);label(im,text,(12,7,104,15),'sign');im.save(A/code/'textures'/f)
# The main Doraemon wordmark is the already-approved localized artwork from
# game1. Only a nearest-grid size conversion is needed for Dora3's original slot.
f,w,h=add(0xB9C,'doraemon-logo')
for code in ['en','ru']:
 im=Image.open(R/'resources/artwork/localization'/f'doraemon-logo-{code}.png').convert('RGBA');im.resize((w,h),Image.Resampling.NEAREST).save(A/code/'textures'/f)
f,w,h=add(0xB9B,'town-subtitle')
# High-resolution imagegen sources remain editable artwork; native PNG exports
# are copied separately after transparent variants are available.
(A/'texture_index.json').write_text(json.dumps(index,indent=2)+'\n')

for code in ['en','ru']:
 Image.open(R/'resources/artwork/localization'/f'town-subtitle-{code}.png').convert('RGBA').resize((184,56),Image.Resampling.NEAREST).save(A/code/'textures/0b9b-town-subtitle.png')
print('Final localized texture records:',len(index['entries']))

for png in A.glob('*/textures/*.png'):
 im=Image.open(png).convert('RGBA')
 im.quantize(colors=240,method=Image.Quantize.FASTOCTREE,dither=Image.Dither.NONE).save(png)
