"""Verify canonical source, C preprocessing and compilation of semantic names.

No game, renderer or GPU. The preprocessed C token stream must match exactly
for every generated translation unit, retaining symbol and register identity.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import re
import subprocess
from decomp_annotations import read_decompiled_source

ROOT=Path(__file__).resolve().parents[1]
TOKEN=re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|\w+|>>=|<<=|\.\.\.|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[+*/%&|^!-]=|[^\s]')
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--generated',type=Path,required=True)
    a=p.parse_args();g=a.generated.resolve()
    receipt=json.loads((g/'generation-receipt.json').read_text())
    for base,key in [(ROOT,'inputs'),(g,'generated')]:
        for name,expected in receipt[key].items():
            if sha(base/name)!=expected: raise ValueError('Stale generation: '+name)
    report=json.loads((g/'RecompiledFuncs/decomp-annotations.json').read_text())
    out=ROOT/'build-local/decomp-names-check'
    original=out/'canonical';original.mkdir(parents=True,exist_ok=True)
    flags=['-std=c17','-fno-strict-aliasing',*['-I'+str(p) for p in
           [g,g/'RecompiledFuncs',ROOT/'build-deps/lib/N64ModernRuntime/N64Recomp/include']]]
    def check(path):
        legacy=original/path.name
        legacy.write_text(read_decompiled_source(path))
        # The canonical temporary file has another physical path. Compare it
        # as the same translation unit, preserving __FILE__ diagnostic strings.
        path_map=f'-ffile-prefix-map={original}={g/"RecompiledFuncs"}'
        before=subprocess.check_output(['gcc',*flags,path_map,'-E','-P',str(legacy)],text=True)
        after=subprocess.check_output(['gcc',*flags,'-E','-P',str(path)],text=True)
        before_tokens=TOKEN.findall(before);after_tokens=TOKEN.findall(after)
        if before_tokens!=after_tokens: raise ValueError('Compiler token stream changed: '+path.name)
        subprocess.run(['gcc',*flags,'-fsyntax-only',str(path)],check=True)
        return dict(file=path.name,canonical_sha256=sha(legacy),named_sha256=sha(path),tokens=len(after_tokens))
    with ThreadPoolExecutor(max_workers=2) as pool:
        checked=list(pool.map(check, sorted((g/'RecompiledFuncs').glob('*.c'))))
    result=f'PASS: {report["annotated_functions"]} descriptions; {report["renamed_functions"]} semantic function names; {report["register_role_scopes"]} register scopes; {len(checked)} C units with identical preprocessed tokens and successful C17 syntax compilation; unchanged ABI/register storage; no game/GPU.'
    output=dict(result=result,generation_receipt_sha256=sha(g/'generation-receipt.json'),
         files=checked,source_sha256={n:sha(ROOT/n) for n in
         ['tools/decomp_annotations.py','tools/check_decomp_names.py','recomp/decomp-symbols.json']})
    (ROOT/'research/decomp-names-check.json').write_text(json.dumps(output,indent=2)+'\n')
    print(result)

if __name__=='__main__': main()
