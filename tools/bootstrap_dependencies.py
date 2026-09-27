"""Materialize exact Dora64 dependency commits in a separate ignored directory."""
import argparse,json,subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]


def git(repo,*args,capture=False):
    return subprocess.run(['git','-C',str(repo),*args],check=True,text=True,
                          stdout=subprocess.PIPE if capture else None).stdout


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--local-donor',type=Path,help='Optional read-only source of Git objects; never copies its working files')
    a=p.parse_args();lock=json.loads((ROOT/'dependencies.lock.json').read_text())
    for entry in lock['dependencies']:
        target=ROOT/'build-deps'/entry['path']
        if not (target/'.git').exists():
            if target.exists() and any(target.iterdir()):raise RuntimeError(f'Refusing occupied {target}')
            origin=entry['repository']
            if a.local_donor:
                donor=a.local_donor/entry['path']
                git(donor,'cat-file','-e',entry['commit']+'^{commit}')
                origin=str(donor)
            target.parent.mkdir(parents=True,exist_ok=True)
            # Independent objects: the new project remains usable if Dora64 is moved/deleted.
            subprocess.run(['git','clone','--no-hardlinks','--no-checkout',origin,str(target)],check=True)
            git(target,'checkout','--detach',entry['commit'])
            git(target,'remote','set-url','origin',entry['repository'])
        if git(target,'rev-parse','HEAD',capture=True).strip()!=entry['commit']:
            raise RuntimeError(f'Wrong revision in {target}; refusing reset')
        if git(target,'status','--porcelain','--untracked-files=no','--ignore-submodules=all',capture=True).strip():
            raise RuntimeError(f'Local tracked changes in {target}; refusing overwrite')
        print(entry['path'],entry['commit'],flush=True)
    (ROOT/'build-deps/verified-lock.json').write_text(json.dumps(lock,indent=2)+'\n')


if __name__=='__main__':main()
