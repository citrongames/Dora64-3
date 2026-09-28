"""Test the pinned runtime's saving on a disposable copy, never the source save."""
import argparse
import hashlib
import shutil
import subprocess
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--save', required=True, type=Path)
args = p.parse_args()
before = hashlib.sha256(args.save.read_bytes()).hexdigest()
if args.save.stat().st_size != 2048:
    raise ValueError('Expected Eep16k save of 2048 bytes')
runtime = root / 'build-deps/lib/N64ModernRuntime'
binary = root / 'build-local/save-file-check'
includes = ['librecomp/include', 'librecomp/include/librecomp', 'ultramodern/include', 'N64Recomp/include', 'thirdparty/concurrentqueue', 'thirdparty']
subprocess.run(['g++', '-std=c++20', '-O2', '-pthread', '-ffunction-sections', '-fdata-sections',
                *['-I' + str(runtime / name) for name in includes], str(root / 'research/save-file-check.cpp'),
                str(runtime / 'librecomp/src/files.cpp'), '-Wl,--gc-sections', '-o', str(binary)], check=True)
with tempfile.TemporaryDirectory(prefix='save-check-', dir=root / 'build-local') as temp:
    destination = Path(temp) / 'saves/dora3.n64.jp.bin'
    destination.parent.mkdir()
    shutil.copyfile(args.save, destination)
    subprocess.run([str(binary), temp], check=True)
assert hashlib.sha256(args.save.read_bytes()).hexdigest() == before
print('PASS: source save unchanged; SHA256=' + before)
