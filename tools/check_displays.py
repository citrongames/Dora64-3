from pathlib import Path
import subprocess,tempfile,json,hashlib,argparse
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--sanitizer',choices=['address','thread'],default='address');args=parser.parse_args()
sources=[root/'research/display-check.cpp',root/'src/port_display.cpp',root/'src/port_settings.cpp']
flags=subprocess.check_output(['pkg-config','--cflags','--libs','sdl2'],text=True).split()
out=root/'build-local'/('display-check-'+args.sanitizer)
cmd=['g++','-std=c++20','-O1','-g','-pthread','-fsanitize='+args.sanitizer,'-fno-omit-frame-pointer','-I'+str(root/'src'),'-I'+str(root/'build-deps/lib/N64ModernRuntime/thirdparty'),*map(str,sources),'-o',str(out),*flags]
for name in ['GetNumVideoDisplays','GetDisplayName','GetDesktopDisplayMode','GetNumDisplayModes','GetDisplayMode','GetWindowFlags','SetWindowFullscreen','SetWindowDisplayMode','SetWindowPosition','RestoreWindow','SetWindowSize','GetDisplayUsableBounds','GetWindowBordersSize']:cmd+=['-Wl,--wrap=SDL_'+name]
subprocess.run(cmd,check=True)
with tempfile.TemporaryDirectory(prefix='dora3-displays-') as tmp:result=subprocess.run([str(out),tmp],capture_output=True,text=True)
print(result.stdout,end='');print(result.stderr,end='');result.check_returncode()
# Test the actual switch extracted from the adapted RT64 source, not only UI target text.
adapted=root/'build-local/rt64-framerate-check'
subprocess.run(['python3',str(root/'tools/prepare_rt64_framerate.py'),str(adapted)],check=True)
text=(adapted/'rt64_workload_queue.cpp').read_text();a=text.index('        const auto refreshRate =');b=text.index('        // Store the rate',a)
cpp='#include <cassert>\nstruct UserConfiguration {enum class RefreshRate {Original,Display,Manual};};\n'
cpp+='unsigned rate(UserConfiguration::RefreshRate mode,unsigned display,unsigned manual){struct U {UserConfiguration::RefreshRate refreshRate;unsigned refreshRateTarget;};struct R {U userConfig;unsigned swapChainRate;} r{{mode,manual},display};struct E {R* sharedResources;} ext{&r};struct W {unsigned targetRate;} workloadConfig;'+text[a:b]+'return workloadConfig.targetRate;}\n'
cpp+='int main(){using R=UserConfiguration::RefreshRate;assert(rate(R::Manual,60,240)==240);assert(rate(R::Manual,165,360)==360);assert(rate(R::Display,165,240)==165);assert(rate(R::Original,165,240)==0);}'
present=(adapted/'rt64_present_queue.cpp').read_text();a=present.index('                bool manualAboveDisplay;');b=present.index('                // End Dora64-3 Manual VSync policy.',a)
cpp=cpp.replace('#include <cassert>', '#include <cassert>\n#include <mutex>')
cpp=cpp.replace('int main(){','void test_vsync();\nint main(){test_vsync();')
cpp+='bool vsync(UserConfiguration::RefreshRate mode,unsigned hz,unsigned fps){struct U {UserConfiguration::RefreshRate refreshRate;unsigned refreshRateTarget;};struct R {U userConfig;unsigned swapChainRate;std::mutex configurationMutex;} r{{mode,fps},hz};struct S {bool enabled=true;void setVsyncEnabled(bool v){enabled=v;}} swap;struct E {R* sharedResources;S* swapChain;} ext{&r,&swap};'+present[a:b]+'return swap.enabled;}\n'
cpp+='void test_vsync(){using R=UserConfiguration::RefreshRate;assert(!vsync(R::Manual,60,240));assert(vsync(R::Manual,165,120));assert(vsync(R::Display,60,240));assert(vsync(R::Original,60,240));}'
test=adapted/'rate-check.cpp';test.write_text(cpp);subprocess.run(['g++','-std=c++20',str(test),'-o',str(adapted/'rate-check')],check=True);subprocess.run([str(adapted/'rate-check')],check=True)
print('PASS: actual RT64 switch: Manual240@60Hz/360@165Hz, Display165, Original0; actual VSync policy above/below Hz and restoration')
report={'result':'pass','scope':'Wrapped SDL (no real window/GPU); actual adapted RT64 refresh-rate switch','sanitizer':args.sanitizer,'source_sha256':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources+[root/'tools/prepare_rt64_framerate.py',root/'tools/check_displays.py',root/'src/port_display.hpp']}}
(root/'research'/('display-'+args.sanitizer+'-check.json')).write_text(json.dumps(report,indent=2)+'\n')
