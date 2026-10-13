#!/usr/bin/env python3
"""Build and optionally package Dora64-3 without launching the application."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]

def run(*args, env=None):
    subprocess.run([str(v) for v in args], cwd=ROOT, env=env, check=True)

def generated(directory, kind, rom):
    receipt = directory/'generation-receipt.json'
    if receipt.is_file():
        data = json.loads(receipt.read_text())
        for group, base in [('inputs', ROOT), ('generated', directory)]:
            for name, expected in data[group].items():
                f = base/name
                if not f.is_file() or hashlib.sha256(f.read_bytes()).hexdigest() != expected:
                    raise SystemExit(f'Stale generation: {f}. Select a new --{kind}-directory.')
        if kind == 'cpu' and data.get('diagnostics', False):
            raise SystemExit('CPU generation contains diagnostic hooks; select a new --cpu-directory.')
        return
    if directory.exists():
        raise SystemExit(f'{directory} exists without a receipt. Select a new generation directory.')
    if not rom:
        raise SystemExit('Provide --rom for the first build.')
    tool = 'generate_startup.py' if kind == 'cpu' else 'generate_audio_rsp.py'
    args = [sys.executable, ROOT/'tools'/tool, '--rom', rom, '--output-dir', directory]
    if kind == 'cpu':
        args.append('--no-diagnostics')
    run(*args)

def windows_crt():
    finder = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)'))/'Microsoft Visual Studio/Installer/vswhere.exe'
    vs = subprocess.check_output([str(finder), '-latest', '-products', '*',
        '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
        '-property', 'installationPath'], text=True).strip()
    folders = list((Path(vs)/'VC/Redist/MSVC').glob('*/x64/Microsoft.VC143.CRT'))
    if not folders:
        raise SystemExit('Visual C++ runtime not found; provide --crt-directory.')
    return max(folders, key=lambda f: tuple(int(v) for v in f.parents[1].name.split('.')))

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--platform', choices=['windows', 'linux', 'android'],
        default='windows' if os.name == 'nt' else 'linux')
    p.add_argument('--rom', type=Path)
    p.add_argument('--cpu-directory', type=Path, default=ROOT/'build-local/startup')
    p.add_argument('--audio-directory', type=Path, default=ROOT/'build-local/audio-rsp')
    p.add_argument('--build-directory', type=Path, default=ROOT/'build-local/linux-release')
    p.add_argument('--jobs', type=int, default=4)
    p.add_argument('--skip-bootstrap', action='store_true')
    p.add_argument('--signing-properties', type=Path)
    p.add_argument('--package-directory', type=Path)
    p.add_argument('--crt-directory', type=Path)
    a = p.parse_args()
    if a.jobs < 1:
        p.error('--jobs must be positive')
    if a.platform == 'windows' and os.name != 'nt':
        p.error('Build Windows with native Windows tools.')
    env = dict(os.environ)
    if a.platform == 'android':
        signing = a.signing_properties or env.get('DORA3_SIGNING_PROPERTIES')
        if not signing or not Path(signing).is_file():
            p.error('Android release requires --signing-properties or DORA3_SIGNING_PROPERTIES.')
        env['DORA3_SIGNING_PROPERTIES'] = str(Path(signing).resolve())
    if not a.skip_bootstrap:
        run(sys.executable, ROOT/'tools/bootstrap_dependencies.py')
        run(sys.executable, ROOT/'tools/bootstrap_n64recomp.py', '--jobs', a.jobs)
    cpu, audio = a.cpu_directory.resolve(), a.audio_directory.resolve()
    generated(cpu, 'cpu', a.rom)
    generated(audio, 'audio', a.rom)
    if a.platform == 'windows':
        shell = shutil.which('pwsh') or shutil.which('powershell')
        if not shell:
            p.error('PowerShell is required.')
        run(shell, '-NoProfile', '-File', ROOT/'tools/build_windows.ps1',
            '-Python', sys.executable, '-CpuDirectory', cpu, '-AudioDirectory', audio,
            '-Jobs', a.jobs)
        binary = ROOT/'build-local/windows/Dora64-3.exe'
        target = 'Windows-x64'
    elif a.platform == 'linux':
        directory = a.build_directory.resolve()
        source = ROOT
        cache = directory/'CMakeCache.txt'
        if cache.exists():
            line = next((v for v in cache.read_text().splitlines()
                if v.startswith('CMAKE_HOME_DIRECTORY:INTERNAL=')), None)
            if line:
                cached = Path(line.split('=', 1)[1])
                if cached not in [ROOT, ROOT/'research/runtime']:
                    p.error('Build directory belongs to another project.')
                source = cached
        run('cmake', '-S', source, '-B', directory, '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Release', '-DDORA3_LIVE_RT64=ON',
            '-DDORA3_DIAGNOSTICS=OFF', '-DDORA3_CRASH_LOG=OFF', '-DDORA3_GRAPHICS_PROFILE=OFF',
            f'-DDORA3_GENERATED_DIR={cpu}', f'-DDORA3_AUDIO_GENERATED_DIR={audio}',
            f'-DPython3_EXECUTABLE={sys.executable}')
        run('cmake', '--build', directory, '--target', 'dora3_startup', '-j', a.jobs)
        binary = directory/'Dora64-3'
        target = 'Linux-x86_64'
    else:
        host = env.get('DORA3_HOST_FILE_TO_C')
        if not host:
            host = str(ROOT/'build-local/host'/('file_to_c.exe' if os.name == 'nt' else 'file_to_c'))
            source = ROOT/'build-deps/lib/rt64/src/contrib/plume/examples/cmake/tools/file_to_c.cpp'
            if not Path(host).is_file() or source.stat().st_mtime_ns > Path(host).stat().st_mtime_ns:
                Path(host).parent.mkdir(parents=True, exist_ok=True)
                if os.name == 'nt':
                    run('cl', '/nologo', '/EHsc', '/std:c++17', '/O2', source, '/Fe:'+host,
                        '/Fo:'+str(Path(host).with_suffix('.obj')))
                else:
                    run(shutil.which('c++') or 'g++', '-std=c++17', '-O2', source, '-o', host)
            env['DORA3_HOST_FILE_TO_C'] = host
        env['DORA3_HOST_PYTHON'] = sys.executable
        wrapper = ROOT/'android'/('gradlew.bat' if os.name == 'nt' else 'gradlew')
        run(wrapper, '-p', ROOT/'android', f'--max-workers={a.jobs}', ':app:assembleRelease',
            f'-Pdora3CpuDir={cpu}', f'-Pdora3AudioDir={audio}', env=env)
        binary = ROOT/'android/app/build/outputs/apk/release/app-release.apk'
        if not binary.is_file():
            raise SystemExit('Signed release APK was not produced.')
        target = 'Android-arm64'
    print(f'Built {binary}')
    if a.package_directory:
        args = [sys.executable, ROOT/'tools/package_release.py', '--platform', target,
            '--binary', binary, '--destination', a.package_directory.resolve()]
        if a.platform == 'windows':
            args += ['--crt-directory', a.crt_directory or windows_crt()]
        run(*args)

if __name__ == '__main__':
    main()
