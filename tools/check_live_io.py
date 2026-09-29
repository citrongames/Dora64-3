from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[1]
runtime=root/'build-deps/lib/N64ModernRuntime'
flags=subprocess.check_output(['pkg-config','--cflags','--libs','sdl2'],text=True).split()
flags+=['-I'+str(runtime/'thirdparty/concurrentqueue'),'-I'+str(runtime/'thirdparty')]
cmd=['g++','-std=c++20','-O0','-g','-pthread','-I'+str(root/'src'),'-I'+str(runtime/'librecomp/include'),'-I'+str(runtime/'ultramodern/include'),'-I'+str(runtime/'N64Recomp/include'),str(root/'research/live-io-check.cpp'),str(root/'src/live_io.cpp'),str(root/'src/port_settings.cpp'),str(root/'src/port_input.cpp'),'-o',str(root/'build-local/live-io-check')]+flags
for name in ['SDL_GetKeyboardState','SDL_GetWindowFlags','SDL_QueueAudio','SDL_GetQueuedAudioSize']:cmd+=['-Wl,--wrap='+name]
subprocess.run(cmd,check=True,cwd=root)
subprocess.run([str(root/'build-local/live-io-check')],check=True,cwd=root)
