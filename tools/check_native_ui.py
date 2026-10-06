"""Focused connected UI check: actual Original math/draw bodies; no game/GPU."""
from pathlib import Path
import argparse,hashlib,json,os,re,subprocess
from decomp_annotations import read_decompiled_source
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def extract(g):return {re.search(r'void (\w+)\(',b)[1]:b for f in (g/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(f),re.S)}
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for n in ['generated','reference','rom']:ap.add_argument('--'+n,type=Path,required=True)
 a=ap.parse_args();g=a.generated.resolve();ref=a.reference.resolve();rom=a.rom.read_bytes()
 assert hashlib.sha256(rom).hexdigest()==json.loads((ROOT/'recomp/rom.lock.json').read_text())['sha256']
 receipt=json.loads((g/'generation-receipt.json').read_text());old=json.loads((ref/'generation-receipt.json').read_text())
 for base,group in [(ROOT,'inputs'),(g,'generated')]:
  for n,v in receipt[group].items():assert sha(base/n)==v,('Stale UI generation',n)
 assert old['rom_sha256']==receipt['rom_sha256']
 for n,v in old['generated'].items():assert sha(ref/n)==v,n
 roots=['dora3_s2_menu_801BA0CC','dora3_s2_menu_801BA3C8','dora3_s3_801F0CB8','dora3_scaled_texture_rect','dora3_s3_801C9E50','dora3_s3_801C9EBC','dora3_s0_8002DA98','dora3_s3_801F1598','dora3_s3_801F4948','dora3_s3_801F6870']+['dora3_s2_menu_'+f'{x:08X}' for x in [0x801B9F10,0x801BA660,0x801B981C,0x801B96F8,0x801BA858,0x801BBCEC,0x801BBEB0,0x801BC17C,0x801BC4C0,0x801C0528,0x801C14B8,0x801C1A90,0x801C1CA0,0x801C22C4,0x801BDEFC,0x801BFC44]]+['dora3_lib_80057A80','dora3_s1_801B5B04','dora3_s1_801B6E1C','dora3_s1_801B7DCC','dora3_s1_801B7BB4','dora3_s1_801B63B8','dora3_s3_801BE5F0','dora3_s3_801BE72C','dora3_s3_801BE7EC']
 allowed={'dora3_color_buffer_setup','dora3_depth_buffer_clear','dora3_s0_800444C0','dora3_texture_info','dora3_common_text_draw','dora3_common_glyph_draw','dora3_s0_menu_8002841C','dora3_texture_rect_state'}
 # Character input/scene/resource and model/sprite commands are explicit
 # boundaries; the actual carousel/spin/matrix arithmetic executes below.
 command_pcs={0x801C9B9C,0x801C9BE0,0x801F6DC4,0x801B8F9C,0x801EFE2C,0x8002A1B8,0x80028C74}
 body_map=extract(g)
 allowed.update(n for n,b in body_map.items() if int(re.search(r'// 0x([0-9A-Fa-f]+):',b)[1],16) in command_pcs)
 def closure(bodies):
  out={};todo=roots.copy()
  while todo:
   n=todo.pop()
   if n in out or n in allowed:continue
   if n not in bodies:raise ValueError('Unexpected UI external call '+n)
   out[n]=bodies[n];todo.extend(x for x in re.findall(r'\b(\w+)\(rdram, ctx\)',bodies[n]) if x!='dora3_trace' and not x.startswith('dora3_native_'))
  return out
 patched=closure(extract(g));original=closure(extract(ref));assert patched.keys()==original.keys()
 assert all('dora3_native_' not in b for b in original.values())
 assert [n for n,b in patched.items() if 'LOOKUP_FUNC' in b]==['dora3_s2_menu_801BDEFC']
 out=ROOT/'build-local/native-ui-check';out.mkdir(exist_ok=True)
 names=sorted(set(patched)|allowed,key=len,reverse=True);source='\n'.join(original.values())
 for n in names:source=re.sub(r'\b'+n+r'\b','reference_'+n,source)
 defs=['RECOMP_FUNC void '+pre+n+'(uint8_t* rdram,recomp_context* ctx){ui_fixture_boundary(rdram,ctx,"'+n+'");}' for n in sorted(allowed) for pre in ['', 'reference_']]
 declarations='\n'.join('RECOMP_FUNC void reference_'+n+'(uint8_t*,recomp_context*);' for n in names)
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\nvoid ui_fixture_boundary(uint8_t*,recomp_context*,const char*);\n'+declarations+'\n'+'\n'.join(patched.values())+'\n'+source+'\n'+'\n'.join(defs))
 meta=json.loads((ROOT/'recomp/startup.json').read_text())
 for tag,scene in [('game','.scene1'),('menu','.scene34')]:
  seed=bytearray(0x400000)
  for s in [dict(rom=0x1000,vram=0x80025C00,size=0x8007FA00-0x80025C00),next(s for s in meta['overlays'] if s['name']=='.scene_common'),next(s for s in meta['overlays'] if s['name']==scene)]:
   data=rom[s['rom']:s['rom']+s['size']];base=s['vram']&0xFFFFFF
   for i in range(0,len(data),4):seed[base+i:base+i+4]=data[i:i+4][::-1]
  (out/f'{tag}.bin').write_bytes(seed)
 runtime=ROOT/'build-deps/lib/N64ModernRuntime';flags=['-DDORA3_UI_CHARGE_FRACTION_EXTERNAL','-O1','-g','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [ROOT/'src',g,g/'RecompiledFuncs',runtime/'N64Recomp/include']]]
 stamp=hashlib.sha256((sha(out/'functions.c')+sha(g/'startup_hooks.h')+sha(g/'RecompiledFuncs/funcs.h')+repr(flags)).encode()).hexdigest();cached=out/'functions.sha256'
 if not (out/'functions.o').exists() or not cached.exists() or cached.read_text()!=stamp:
  subprocess.run(['gcc','-std=c17','-fsanitize=address',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True);cached.write_text(stamp)
 sources=[ROOT/'src/native_simulation_time.cpp',ROOT/'src/native_effects.cpp',ROOT/'src/native_ui.cpp',ROOT/'src/native_scene_policy.cpp',ROOT/'research/native-ui-check.cpp']
 sources.append(ROOT/'research/native-inactive-enemy-fixture.cpp')
 sources.append(ROOT/'research/native-inactive-minigame-fixture.cpp')
 sources.extend([ROOT/'src/native_model_animation.cpp',ROOT/'src/native_frame_time.cpp',ROOT/'src/native_texture_animation.cpp',ROOT/'research/native-ui-model-inactive-fixture.cpp'])
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check'),str(out/'game.bin'),str(out/'menu.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')).strip()
 tracked=sources+[ROOT/'src/native_ui.hpp',ROOT/'src/native_scene_policy.cpp',ROOT/'src/native_game_loop.cpp',ROOT/'recomp/native-timing-rules.json',ROOT/'tools/native_timing_patches.py',ROOT/'tools/generate_startup.py',Path(__file__).resolve()]
 report=dict(scope='Connected UI panels/rectangle/title/intro/selection equations, actual widgets/dialogue/files/arrow/final-panel Source draw cadence and Original commands',result=result,actual_functions=sorted(patched),allowed_counted_boundaries=sorted(allowed),source_sha256={str(x.relative_to(ROOT)):sha(x) for x in tracked},generation_receipt_sha256=sha(g/'generation-receipt.json'),reference_receipt_sha256=sha(ref/'generation-receipt.json'),limitations=['Synthetic UI/controller/font/texture inputs with Original ROM math and Original UI bodies; no game/renderer/GPU','Asset texture-info, textured glyph/rectangle/string draw and sound request are counted fixture boundaries; raw panel/HUD/arrow display-list writes are actual Original commands','Resident dispatcher Source begin/end, draw epilogues and menu policy are statically pinned to exact ROM sites; unrelated world/player/NPC callbacks not rerun','Real menu appearance, glyph assets, gameplay dialogue and save devices require user visual test; full variable FPS still requires blocks1/2/4/6'])
 (ROOT/'research/native-ui-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(result)
if __name__=='__main__':main()
