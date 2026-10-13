#!/usr/bin/env python3
"""Package compiled Dora64-3 releases with assets, licenses and checksums."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]

def git(*args, cwd=ROOT):
    return subprocess.check_output(['git', *args], cwd=cwd).decode().strip()

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--platform', choices=['Windows-x64','Linux-x86_64','Android-arm64'], required=True)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--destination', type=Path, required=True)
    p.add_argument('--version', default=(ROOT/'VERSION').read_text().strip())
    p.add_argument('--crt-directory', type=Path)
    a = p.parse_args()
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?', a.version):
        p.error('Version must use major.minor.patch format.')
    if not a.binary.is_file():
        p.error('Binary does not exist.')
    if a.platform == 'Windows-x64' and not a.crt_directory:
        p.error('--crt-directory is required for Windows.')
    name = f'Dora64-3-{a.version}-{a.platform}'
    stage = ROOT/'build-local/packages'/name
    archive = a.destination/(name+('.tar.gz' if a.platform.startswith('Linux') else '.zip'))
    apk = a.destination/(name+'.apk')
    if stage.exists() or archive.exists() or (a.platform.startswith('Android') and apk.exists()):
        p.error('Package exists; select a new version or destination. Existing packages are preserved.')
    stage.mkdir(parents=True)
    a.destination.mkdir(parents=True, exist_ok=True)
    def copy(source, relative):
        target = stage/relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    lock = json.loads((ROOT/'dependencies.lock.json').read_text())
    for dep in lock['dependencies']:
        checkout = ROOT/'build-deps'/dep['path']
        for relative in git('ls-files', '-z', cwd=checkout).split('\0'):
            if relative and re.match(r'^(LICENSE|COPYING|NOTICE)', Path(relative).name, re.I):
                source = checkout/relative
                if source.is_file():
                    copy(source, Path('licenses')/dep['path']/relative)
    copy(ROOT/'resources/licenses/DirectXShaderCompiler-LICENSE.txt','licenses/DXC-LICENSE.txt')
    copy(ROOT/'tools/n64recomp/LICENSE.N64Recomp','licenses/N64Recomp-tool-LICENSE.txt')
    copy(ROOT/'LICENSE','LICENSE')
    copy(ROOT/'NOTICE.md','NOTICE.md')
    copy(ROOT/'dependencies.lock.json','licenses/dependencies.lock.json')
    copy(ROOT/'docs/user-guide.md','USER-GUIDE.md')
    instructions = f'''Dora64-3 {a.version} — {a.platform}

Supply your own Japanese Doraemon 3: Nobita no Machi SOS! ROM.
Required format: N3DJ revision 0, 16 MiB, big-endian .z64.
SHA-256: 410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602
The filename does not matter. The ROM is not included.
'''
    if a.platform == 'Android-arm64':
        copy(a.binary,'Dora64-3.apk')
        instructions += '''
Android 7.0/API 24+, ARM64, Vulkan 1.0.
Install Dora64-3.apk, then select your ROM in the system file picker.
Tap the gear to open settings. Use Touch to edit on-screen controls.
Physical gamepads are supported. Modern uses Manual 60 FPS.
Application ID: com.n64recomp.dora64three.
Keep application data by installing updates over the existing application.
'''
    else:
        for source in sorted((ROOT/'assets').rglob('*')):
            rel = source.relative_to(ROOT/'assets')
            if not source.is_file() or source.name.endswith('_source.json'):
                continue
            if rel.parts[0]=='icons' or (rel.parts[0]=='localization' and
                    ((len(rel.parts)==2 and source.suffix in ['.json','.bmp']) or
                     (len(rel.parts)>2 and rel.parts[1] in ['en','ru']))):
                copy(source,Path('assets')/rel)
        instructions += '''
Extract the complete archive into a writable folder. Place the .z64 ROM beside
the executable. Keep assets and libraries with the program. Settings and saves
are created in this folder; preserve them when upgrading. Press Esc for settings.
'''
        if a.platform == 'Windows-x64':
            copy(a.binary,'Dora64-3.exe')
            for n in ['SDL2.dll','dxcompiler.dll','dxil.dll']:
                copy(a.binary.parent/n,n)
            for n in ['msvcp140.dll','msvcp140_atomic_wait.dll','vcruntime140.dll','vcruntime140_1.dll']:
                copy(a.crt_directory/n,n)
            instructions += '\nWindows 10/11 x64: start Dora64-3.exe. Use a current graphics driver.\n'
        else:
            copy(a.binary,'Dora64-3')
            subprocess.run(['strip','--strip-unneeded',str(stage/'Dora64-3')],check=True)
            (stage/'Dora64-3').chmod(0o755)
            copy(ROOT/'resources/linux/dora64-3.desktop','dora64-3.desktop')
            copy(ROOT/'resources/linux/install-desktop.py','install-desktop.py')
            (stage/'install-desktop.py').chmod(0o755)
            instructions += '''
Linux x86_64: glibc 2.43+, libstdc++, SDL2, GTK3 and a Vulkan GPU driver.
Ubuntu 26.04: sudo apt install libsdl2-2.0-0 libgtk-3-0t64 libstdc++6 libvulkan1
Run ./Dora64-3 from the extracted folder.
Optional menu launcher: python3 install-desktop.py
Run the installer again if you move the folder.
'''
    (stage/'README.txt').write_text(instructions,encoding='utf-8')
    (stage/'VERSION.txt').write_text(f'Dora64-3 {a.version}\nPlatform: {a.platform}\nConfiguration: Release\n',encoding='utf-8')
    (stage/'SOURCE.txt').write_text(f'''Dora64-3 source and build instructions:
https://github.com/citrongames/Dora64-3
Source archive supplied alongside this release: Dora64-3-{a.version}-Source.tar.gz
Pinned dependencies: licenses/dependencies.lock.json
Own code license: GPL-3.0-or-later; see LICENSE and NOTICE.md.
''',encoding='utf-8')
    files = sorted(f for f in stage.rglob('*') if f.is_file())
    for f in files:
        if f.suffix.lower() in {'.z64','.v64','.n64','.rom','.pdb','.log','.dmp','.keystore','.jks','.sav','.eep','.sra','.fla'} or f.name.endswith('_settings.json'):
            raise RuntimeError(f'Unexpected personal/game/debug file: {f}')
    (stage/'SHA256SUMS.txt').write_text(''.join(f'{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.relative_to(stage).as_posix()}\n' for f in files),encoding='utf-8')
    if a.platform.startswith('Linux'):
        with tarfile.open(archive,'w:gz',compresslevel=6) as out:
            out.add(stage,arcname=name)
    else:
        with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as out:
            for f in sorted(stage.rglob('*')):
                if f.is_file():
                    out.write(f,f.relative_to(stage.parent).as_posix())
    if a.platform.startswith('Android'):
        shutil.copy2(a.binary,apk)
    print(json.dumps(dict(archive=str(archive),bytes=archive.stat().st_size,
        sha256=hashlib.sha256(archive.read_bytes()).hexdigest())))

if __name__ == '__main__':
    main()
