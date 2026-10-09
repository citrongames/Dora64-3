"""Retain current workload textures before eviction on the deferred render path.
Only generated State is adapted; the pinned RT64 checkout stays untouched.
"""
import hashlib


def adapt(root, out, receipt):
    pins = {'hle/rt64_state.cpp': '3a695dfdaa6defa114b15a748b384a3b1f7d41f7b28393c7dbcf61664339f7d1', 'hle/rt64_workload_queue.cpp': '7fe61533ff8d1ffa475eb90a49310028e10dd59552c0f2a47a3a662cbfd7757a', 'hle/rt64_rdp_tmem.cpp': 'bcf1d32e6f78894901c87a63ed28419d44e27675916756c565dc3baa9335bc4a', 'render/rt64_texture_cache.cpp': 'a58099fefa98d16d2f09b543a2c0acb37f58a3de23ca112013c74c1429b7a23e', 'render/rt64_texture_cache.h': '65ed2d35cefc0595106c5cde5aa741f78bff0252fbfbe183bca53cee7d191374', 'render/rt64_framebuffer_renderer.cpp': '3c9d222843e6075fdbfb828a655503c7f1faf0e56fae46c9e55e73d38f313af2'}
    for name, digest in pins.items():
        if hashlib.sha256((root/'build-deps/lib/rt64/src'/name).read_bytes()).hexdigest() != digest:
            raise SystemExit('Review deferred texture lifetime for changed ' + name)
    p = out/'rt64_state.cpp'
    text = p.read_text()
    needle = '        if (ext.textureCache->evict(workloadCounter, evictedTextureHashes)) {'
    replacement = """        const bool texturesEvicted = [&]() {
            if (renderToRDRAM) {
                return ext.textureCache->evict(workloadCounter, evictedTextureHashes);
            }

            // Deferred rendering has not called createGPUTiles/useTexture yet.
            // A hash hit in TextureManager skips upload but does not refresh age:
            // eviction here could remove a texture needed by this very workload.
            // Refresh all sampled hashes and evict under the same cache lock.
            std::unique_lock lock(ext.textureCache->textureMapMutex);
            auto &map = ext.textureCache->textureMap;
            for (const auto &tile : workload.drawData.callTiles) {
                if (!tile.valid || tile.tileCopyUsed) {
                    continue;
                }
                const auto entry = map.hashMap.find(tile.tmemHashOrID);
                if (entry == map.hashMap.end()) {
                    continue;
                }
                auto access = map.listIterators[entry->second];
                assert(access != map.accessList.end());
                if ((access != map.accessList.end()) && (access->second < workload.submissionFrame)) {
                    // Preserve iterator validity and avoid allocation per tile.
                    // Duplicate hashes and any newer access need no update.
                    access->second = workload.submissionFrame;
                    map.accessList.splice(map.accessList.begin(), map.accessList, access);
                }
            }
            // Keep the original bounded age policy, replacement references and
            // GPU-deletion locks. Framebuffer tile copies use another owner.
            return map.evict(workloadCounter, evictedTextureHashes);
        }();
        if (texturesEvicted) {"""
    if text.count(needle) != 1:
        raise SystemExit('Deferred texture eviction boundary changed')
    text = text.replace(needle, replacement)
    p.write_text(text)
    receipt['deferred_texture_lifetime_inputs'] = pins
    receipt['deferred_texture_lifetime_scope'] = 'Refresh current valid non-copy callTile hashes atomically with eviction after texture uploads; eager path and original bounded eviction policy unchanged'
    receipt['state_generated_sha256'] = hashlib.sha256(p.read_bytes()).hexdigest()
