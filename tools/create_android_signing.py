#!/usr/bin/env python3
"""Create a private Android release signing key without printing passwords."""
import argparse
import os
from pathlib import Path
import secrets
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--keytool', default='keytool')
    p.add_argument('--directory', type=Path, default=ROOT/'build-local/signing')
    a = p.parse_args()
    directory = a.directory.resolve()
    directory.mkdir(parents=True, exist_ok=True, mode=0o700)
    key = directory/'dora64-3-release.keystore'
    properties = directory/'release-signing.properties'
    if key.exists() or properties.exists():
        p.error('Signing files already exist; keep the existing key for updates.')
    password = secrets.token_urlsafe(40)
    env = dict(os.environ, DORA3_KEY_PASSWORD=password)
    subprocess.run([a.keytool, '-genkeypair', '-storetype', 'PKCS12',
        '-keystore', str(key), '-alias', 'dora64three', '-keyalg', 'RSA',
        '-keysize', '4096', '-validity', '10000', '-storepass:env', 'DORA3_KEY_PASSWORD',
        '-keypass:env', 'DORA3_KEY_PASSWORD', '-dname', 'CN=Dora64-3, O=citrongames',
        '-noprompt'], env=env, check=True)
    key.chmod(0o600)
    with properties.open('x', encoding='utf-8') as f:
        f.write(f'storeFile={key.as_posix()}\nstorePassword={password}\n'
            f'keyAlias=dora64three\nkeyPassword={password}\n')
    properties.chmod(0o600)
    print(f'Created {key}\nSigning properties: {properties}\nKeep both files private and backed up.')

if __name__ == '__main__':
    main()
