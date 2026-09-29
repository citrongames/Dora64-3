from pathlib import Path
import subprocess,tempfile,json,hashlib,os,argparse
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--sanitizer',choices=['none','address','thread'],default='none');args=parser.parse_args()
runtime=root/'build-deps/lib/N64ModernRuntime';imgui=root/'build-deps/lib/rt64/src/contrib/imgui'
sources=[root/'research/port-menu-check.cpp',*[root/'src'/n for n in ['port_settings.cpp','port_input.cpp','port_menu.cpp']],*[imgui/n for n in ['imgui.cpp','imgui_draw.cpp','imgui_widgets.cpp','imgui_tables.cpp']]]
flags=subprocess.check_output(['pkg-config','--cflags','--libs','sdl2'],text=True).split()
out=root/'build-local'/('port-menu-check-'+args.sanitizer)
cmd=['g++','-std=c++20','-O1','-g','-pthread','-I'+str(root/'src'),'-I'+str(runtime/'thirdparty'),'-I'+str(imgui.parent),*map(str,sources),'-o',str(out),*flags]
if args.sanitizer!='none':cmd+=['-fsanitize='+args.sanitizer,'-fno-omit-frame-pointer']
subprocess.run(cmd,check=True,cwd=root)
with tempfile.TemporaryDirectory(prefix='dora3-port-check-') as temp:
    result=subprocess.run([str(out),temp],check=False,cwd=root,capture_output=True,text=True)
print(result.stdout,end='');print(result.stderr,end='')
result.check_returncode()
report={'result':'pass','scope':'Real settings/input and ImGui CPU draw data only; no ROM/window/GPU','sanitizer':args.sanitizer,'checks':result.stdout.strip(),'source_sha256':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources[:4]}}
(root/'research'/('port-menu-'+args.sanitizer+'-check.json')).write_text(json.dumps(report,indent=2)+'\n')
