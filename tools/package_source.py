#!/usr/bin/env python3
"""Create the release source archive from project files and pinned dependencies."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tarfile

ROOT=Path(__file__).resolve().parents[1]
EXCLUDED={'.z64','.v64','.n64','.rom','.sav','.eep','.sra','.fla','.keystore','.jks',
    '.pdb','.dmp','.log','.snapshot','.exe','.dll','.so','.a','.lib','.obj','.o','.apk','.aab','.bin'}

def files(cwd, project=False):
    args=['git','ls-files','-z']
    if project:
        args += ['--cached','--others','--exclude-standard']
    names=subprocess.check_output(args,cwd=cwd).decode().split('\0')
    for name in sorted(set(names)):
        f=cwd/name
        if not name or not f.is_file() or f.is_symlink():
            continue
        if f.suffix.lower() in EXCLUDED or '.so.' in f.name or f.name.endswith('signing.properties'):
            continue
        if f.name.endswith('_settings.json') or name.startswith('research/private/'):
            continue
        yield f,Path(name)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--destination',type=Path,required=True)
    a=p.parse_args()
    version=(ROOT/'VERSION').read_text().strip()
    prefix=f'Dora64-3-{version}-Source'
    archive=a.destination/(prefix+'.tar.gz')
    if archive.exists():
        p.error('Source archive already exists; preserve it and select a new version/destination.')
    a.destination.mkdir(parents=True,exist_ok=True)
    entries={}
    for f,relative in files(ROOT,True):
        entries[relative]=f
    lock=json.loads((ROOT/'dependencies.lock.json').read_text())
    for dep in lock['dependencies']:
        checkout=ROOT/'build-deps'/dep['path']
        head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=checkout,text=True).strip()
        if head!=dep['commit']:
            raise SystemExit(f'Dependency revision mismatch: {dep["path"]}')
        for f,relative in files(checkout):
            entries[Path('dependency-sources')/dep['path']/relative]=f
    with tarfile.open(archive,'w:gz',compresslevel=6) as out:
        for relative,f in sorted(entries.items()):
            out.add(f,arcname=(Path(prefix)/relative).as_posix(),recursive=False)
    print(json.dumps(dict(archive=str(archive),files=len(entries),bytes=archive.stat().st_size,
        sha256=hashlib.sha256(archive.read_bytes()).hexdigest())))

if __name__=='__main__':
    main()
