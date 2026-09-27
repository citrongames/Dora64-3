"""Reject stale metadata or edited generated code before configuring the probe."""
import hashlib, json, sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=Path(sys.argv[1])
receipt=json.loads((out/'generation-receipt.json').read_text())
for base,group in [(root,'inputs'),(out,'generated')]:
    for relative,expected in receipt[group].items():
        path=base/relative
        if hashlib.sha256(path.read_bytes()).hexdigest()!=expected:
            raise SystemExit(f'Stale or modified {path}; regenerate into a new directory.')
expected=set(receipt['generated'])
actual={p.relative_to(out).as_posix() for p in (out/'RecompiledFuncs').glob('*') if p.is_file()}
if actual != expected:raise SystemExit('Generated file set changed; regenerate.')
print('Generation receipt verified')
