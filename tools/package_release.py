#!/usr/bin/env python3
"""Package compiled Release outputs only. Never launch a game or publish files."""
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import tarfile
import zipfile
from pathlib import Path

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--platform', choices=['Windows-x64', 'Linux-x86_64', 'Android-arm64'], required=True)
p.add_argument('--binary', type=Path, required=True)
p.add_argument('--destination', type=Path, required=True)
p.add_argument('--version', required=True)
p.add_argument('--crt-directory', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[1]
if not re.fullmatch(r'[A-Za-z0-9-]+', a.version):
    p.error('Invalid version')
name = f'Dora64-3-{a.version}-{a.platform}'
stage = root / 'build-local/packages' / name
archive = a.destination / (name + ('.tar.gz' if a.platform.startswith('Linux') else '.zip'))
if stage.exists() or archive.exists():
    raise SystemExit('Package already exists: use a new version/destination; old packages are preserved')
stage.mkdir(parents=True)
a.destination.mkdir(parents=True, exist_ok=True)

def copy(source, relative):
    target = stage / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target)

def git(*args, cwd=root):
    return subprocess.check_output(['git', *args], cwd=cwd).decode().strip()

lock = json.loads((root / 'dependencies.lock.json').read_text())
for dep in lock['dependencies']:
    checkout = root / 'build-deps' / dep['path']
    for relative in git('ls-files', '-z', cwd=checkout).split('\0'):
        if relative and re.match(r'^(LICENSE|COPYING|NOTICE)', Path(relative).name, re.I):
            source = checkout / relative
            if source.is_file():
                copy(source, Path('licenses') / dep['path'] / relative)
copy(root / 'resources/licenses/DirectXShaderCompiler-LICENSE.txt', 'licenses/DXC-LICENSE.txt')
copy(root / 'NOTICE.md', 'NOTICE.md')
copy(root / 'dependencies.lock.json', 'licenses/dependencies.lock.json')

common = """Dora64-3 — локальная Release-сборка для проверки.
Игра и тесты агентом не запускались. Не опубликовано на GitHub.
ROM, настройки, сейвы и отладочные файлы в комплект не входят.
Нужен собственный японский Doraemon 3 N3DJ revision0, 16 МиБ, big-endian z64.
SHA256: 410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602
Проверяется содержимое ROM; имя файла любое.
"""
if a.platform == 'Android-arm64':
    copy(a.binary, 'Dora64-3.apk')
    extra = """
Android 7.0/API24+, ARM64, Vulkan1.0. Установите Dora64-3.apk.
При первом запуске выберите ROM через системное окно.
APK compiled Release, debuggable=false, jniDebuggable=false; тестовая подпись
служит только для локальной установки и не включает режим отладки.
Manual60 FPS по умолчанию; шестерёнка открывает меню, Touch — редактор кнопок.
Application ID: com.n64recomp.dora64three. Данные отдельны от Dora64.
"""
else:
    assets = root / 'assets'
    for source in sorted(assets.rglob('*')):
        rel = source.relative_to(assets)
        if not source.is_file() or source.name.endswith('_source.json'):
            continue
        if rel.parts[0] == 'icons' or (rel.parts[0] == 'localization' and
                (len(rel.parts) == 2 and source.suffix in ['.json', '.bmp'] or
                 len(rel.parts) > 2 and rel.parts[1] in ['en', 'ru'])):
            copy(source, Path('assets') / rel)
    extra = """
Распакуйте архив целиком в доступную для записи папку. Положите ROM .z64 рядом
с программой. Assets оставьте рядом; здесь же создадутся настройки и сейвы.
Place your original Japanese .z64 ROM beside the executable; any filename.
Keep assets beside the program. Save files and settings are created here.
"""
    if a.platform == 'Windows-x64':
        if not a.crt_directory:
            p.error('--crt-directory is required for Windows')
        copy(a.binary, 'Dora64-3.exe')
        for n in ['SDL2.dll', 'dxcompiler.dll', 'dxil.dll']:
            copy(a.binary.parent / n, n)
        for n in ['msvcp140.dll', 'msvcp140_atomic_wait.dll', 'vcruntime140.dll', 'vcruntime140_1.dll']:
            copy(a.crt_directory / n, n)
        extra += '\nWindows x64: запустите Dora64-3.exe. Нужен актуальный графический драйвер.\n'
    else:
        copy(a.binary, 'Dora64-3')
        subprocess.run(['strip', '--strip-unneeded', str(stage / 'Dora64-3')], check=True)
        (stage / 'Dora64-3').chmod(0o755)
        extra += """
Linux x86_64: сборка Ubuntu26.04, glibc2.43+, SDL2, GTK3, libstdc++ и Vulkan.
Ubuntu26.04: sudo apt install libsdl2-2.0-0 libgtk-3-0t64 libstdc++6 libvulkan1
Из распакованной папки запустите ./Dora64-3.
Requires a working system Vulkan GPU driver; this is not an AppImage.
"""
(stage / 'README.txt').write_text(common + extra, encoding='utf-8')
(stage / 'VERSION.txt').write_text(f'Dora64-3 {a.version}\nPlatform: {a.platform}\nConfiguration: Release\n'
    f'Base commit: {git("rev-parse", "HEAD")}\nWorking tree changes: awaiting owner verification\n'
    'Diagnostics: OFF; Graphics profile: OFF; runtime not launched\n', encoding='utf-8')
files = sorted(f for f in stage.rglob('*') if f.is_file())
for f in files:
    if f.suffix.lower() in {'.z64', '.v64', '.n64', '.pdb', '.log', '.keystore', '.jks'} or f.name.endswith('_settings.json'):
        raise RuntimeError(f'Forbidden package file: {f}')
(stage / 'SHA256SUMS.txt').write_text(''.join(f'{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.relative_to(stage).as_posix()}\n' for f in files), encoding='utf-8')
if a.platform.startswith('Linux'):
    with tarfile.open(archive, 'w:gz', compresslevel=6) as out:
        out.add(stage, arcname=name)
else:
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as out:
        for f in sorted(stage.rglob('*')):
            if f.is_file():
                out.write(f, f.relative_to(stage.parent).as_posix())
print(json.dumps({'archive': str(archive), 'bytes': archive.stat().st_size,
                  'sha256': hashlib.sha256(archive.read_bytes()).hexdigest()}, ensure_ascii=False))
