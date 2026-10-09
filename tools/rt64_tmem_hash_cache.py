"""Exact-input memoization of the measured pinned RT64 palette hash path."""
import hashlib

def adapt(root,out,receipt):
    hasher=root/'build-deps/lib/rt64/src/common/rt64_tmem_hasher.h'
    expected='3267cd4a85c61e1a960df175eb64641a75dceccb8d38e680bb0fcf85912d15c5'
    if hashlib.sha256(hasher.read_bytes()).hexdigest()!=expected:
        raise SystemExit('TMEMHasher changed: review all memoization key fields')
    source=root/'src/rt64_tmem_hash_cache.hpp'
    (out/source.name).write_bytes(source.read_bytes())
    p=out/'rt64_rdp_tmem.cpp';s=p.read_text()
    include='#include "rt64_state.h"'
    call='TMEMHasher::hash(TMEM, loadTile, width, height, tlut, TMEMHasher::CurrentHashVersion)'
    if s.count(include)!=1 or s.count(call)!=1:
        raise SystemExit('TextureManager hash call changed: review memoization integration')
    s=s.replace(include,include+'\n#include "rt64_tmem_hash_cache.hpp"')
    s=s.replace(call,'dora3_detail::cachedTMEMHash(TMEM, loadTile, width, height, tlut, TMEMHasher::CurrentHashVersion)')
    p.write_text(s)
    receipt['rt64_rdp_tmem.cpp_generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    receipt['tmem_hasher_source_sha256']=expected
    receipt['tmem_hash_cache_sha256']=hashlib.sha256(source.read_bytes()).hexdigest()
    receipt['tmem_hash_cache_scope']='v5 TLUT hash only; all4096 input bytes and nine parameters compared; GPU cache lookup/eviction/upload unchanged; bounded2048entries'
