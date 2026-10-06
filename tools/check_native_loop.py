"""Original common game loop + real native coordinator on synthetic RAM, no renderer."""
from decomp_annotations import read_decompiled_source
import argparse,hashlib,json,os,re,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',required=True,type=Path);p.add_argument('--rom',required=True,type=Path)
 a=p.parse_args();r=ROOT;g=a.generated.resolve()
 receipt=json.loads((g/'generation-receipt.json').read_text())
 for base,group in [(r,'inputs'),(g,'generated')]:
  for n,v in receipt[group].items():assert sha(base/n)==v,('Stale',n)
 rom=a.rom.read_bytes();assert hashlib.sha256(rom).hexdigest()==json.loads((r/'recomp/rom.lock.json').read_text())['sha256']
 bodies={re.search(r'void (\w+)\(',b)[1]:b for p in (g/'RecompiledFuncs').glob('*.c') for b in re.findall(r'RECOMP_FUNC void \w+\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)',read_decompiled_source(p),re.S)}
 stop={'dora3_controller_read','dora3_controller_accumulate','dora3_controller_latch','dora3_controller_reset_accumulator',
  'dora3_rumble_tick','dora3_controller_pak_tick','dora3_sound_events_tick','dora3_common_progress_reset','dora3_scene34_controller_message',
  'dora3_object_lifetime_tick','dora3_deferred_free_tick','dora3_process_pair_install','dora3_cpu_framebuffer_copy'}
 # Reviewed generated names of timestamp and pause helpers resolved by address.
 for n,b in bodies.items():
  pc=re.search(r'// 0x([0-9A-F]+):',b)
  if pc and int(pc[1],16) in {0x800331F8,0x8003321C,0x8003324C,0x80047EF4,0x800271B0,0x8003CD48,0x8003D39C,0x8004281C,0x80038B4C}:stop.add(n)
 todo=['dora3_s0_800393C4','dora3_game_message_loop','dora3_scene34_draw','dora3_scene34_update'];selected={};external=set()
 while todo:
  n=todo.pop()
  if n in selected:continue
  if n in stop or n not in bodies:external.add(n);continue
  selected[n]=bodies[n]
  todo.extend(re.findall(r'\b(\w+)\(rdram, ctx\)',bodies[n]))
 # Image-loop fixture never activates the gameplay/projectile adapter. Keep
 # its new direct phase-command dependencies explicit fail-fast boundaries;
 # the joint gameplay fixture executes their real generated bodies.
 phase_boundaries={'dora3_s0_menu_8002A3B0','dora3_s0_8002EA4C','dora3_s0_8002A2E8','dora3_s0_8002A328','dora3_s3_801C05F0','dora3_s3_core_801CFA94','dora3_s3_core_801D0088','dora3_s0_8002A1B8','dora3_s0_80028C74'}
 phase_boundaries.update({'dora3_s3_801C0624', 'dora3_s3_801DD8FC', 'dora3_s3_801DCC1C', 'dora3_s3_801DCC50', 'dora3_s3_801B922C', 'dora3_s3_801DC880', 'dora3_s3_801DCDF0'})
 assert not phase_boundaries.intersection(selected)
 external.update(phase_boundaries)
 external.add('dora3_s3_core_801C120C')
 external.add('dora3_s3_801E810C')
 external.add('dora3_s3_core_801EBCC0')
 external.add('dora3_s3_core_801EBAD4')
 external.update({'dora3_s3_801E3768','dora3_s3_801E0760','dora3_s3_801BBDDC','dora3_s0_8002F064','dora3_s2_menu_801BBCEC','dora3_s2_menu_801BBEB0','dora3_s2_menu_801BC17C','dora3_s2_menu_801BC4C0','dora3_s2_menu_801C14B8','dora3_lib_80057A80'})
 external.add('dora3_s3_801E89CC')
 external.add('dora3_lib_80055BD0')
 external.update({'dora3_s0_80040104','dora3_s0_800327A4','dora3_s0_8002DA98'})
 external.add('dora3_s33_remaining_8021FED8')
 out=r/'build-local/native-loop-check';out.mkdir(exist_ok=True)
 (out/'functions.c').write_text('#include "startup_hooks.h"\n#include "funcs.h"\n'+'\n'.join(selected.values()))
 (out/'loop-functions.h').write_text('extern "C" {\n'+'\n'.join(f'void {n}(uint8_t*,recomp_context*);' for n in selected)+'\n}\n')

 (out/'seed.bin').write_bytes(struct.pack('<32I',*struct.unpack_from('>32I',rom,0x1000+0x8007CD80-0x80025C00)))
 runtime=r/'build-deps/lib/N64ModernRuntime'
 flags=['-O1','-g','-pthread','-fsanitize=address','-fno-omit-frame-pointer','-fno-strict-aliasing',*['-I'+str(x) for x in [r/'src',g,g/'RecompiledFuncs',out,runtime/'N64Recomp/include']]]
 subprocess.run(['gcc','-std=c17',*flags,'-c',str(out/'functions.c'),'-o',str(out/'functions.o')],check=True)
 sources=[r/'src'/n for n in ['native_cutscene.cpp','native_ui.cpp','native_texture_animation.cpp','native_effects.cpp','native_process_time.cpp','native_simulation_time.cpp','native_camera_timeline.cpp','native_model_animation.cpp','native_game_loop.cpp','native_minigames.cpp','native_minigame_math.cpp','native_gameplay_preview.cpp','native_enemy.cpp','native_input_timeline.cpp','native_scene_policy.cpp','native_frame_time.cpp','native_frame_scheduler.cpp','native_frame_bridge.cpp','native_present.cpp','native_present_hooks.cpp','native_graphics_pump.cpp']]+[r/'research/native-loop-check.cpp']
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',*flags,*map(str,sources),str(out/'functions.o'),'-o',str(out/'check')],check=True)
 result=subprocess.check_output([str(out/'check'),str(out/'seed.bin')],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1')).strip()
 tracked=sources+[ROOT/'src/recomp_context_copy.hpp',r/'src/native_game_loop.hpp',r/'src/native_scene_policy.hpp',r/'src/native_frame_session.hpp',r/'src/frame_lights.hpp',r/'tools/generate_startup.py',Path(__file__).resolve()]
 (r/'research/native-loop-check.json').write_text(json.dumps({'scope':'Original game loop/update/dispatcher/full graphics build/draw/submit with real common coordinator; synthetic scheduler/VI/input/sound/present endpoints; no renderer',
  'result':result,'sanitizers':'ASan/LSan','source_sha256':{str(p.relative_to(r)):sha(p) for p in tracked},'generation_receipt_sha256':sha(g/'generation-receipt.json'),
  'original_functions':sorted(selected),'stub_boundaries':sorted(external)},indent=2)+'\n')
 print(result)
if __name__=='__main__':main()
