"""Overlap independent geometry GPU upload with CPU TMEM processing.
The same uploaders, worker, fence and completion waits remain in fullSync.
"""
import hashlib


def adapt(root, out, receipt):
    pins = {
        'hle/rt64_state.cpp': '3a695dfdaa6defa114b15a748b384a3b1f7d41f7b28393c7dbcf61664339f7d1',
        'hle/rt64_workload.cpp': '1182be7cb648251b5225e494900777ed2f8f0058148bb02a1fefeffd808d1166',
        'hle/rt64_rdp.cpp': '87abb3e0bd7f7323ec6fb40318ec01617fca43f01b019461efbebc41b4430a7b',
        'hle/rt64_rdp_tmem.cpp': 'bcf1d32e6f78894901c87a63ed28419d44e27675916756c565dc3baa9335bc4a',
        'render/rt64_buffer_uploader.cpp': 'd1add014be5f6bf1df77b6f3342c6772bb5235e834b1cb1b7a4e83de167acb40',
    }
    for name, digest in pins.items():
        if hashlib.sha256((root/'build-deps/lib/rt64/src'/name).read_bytes()).hexdigest() != digest:
            raise SystemExit('Review upload/TMEM independence for changed ' + name)
    p = out/'rt64_state.cpp'
    text = p.read_text()
    anchor = '            DORA64_FS_TRACE("non-renderToRDRAM branch");'
    begin = text.index(anchor)
    end = text.index('        // Evict from the texture cache', begin)
    body = text[begin:end]
    tiles = """            // Process all tiles.
            for (uint32_t f = 0; f < workload.fbPairCount; f++) {
                fullSyncFramebufferPairTiles(workload, workload.fbPairs[f], loadOpCursor, rdpTileCursor);
            }

"""
    if body.count(tiles) != 1:
        raise SystemExit('Non-RAM tile processing layout changed')
    body = body.replace(tiles, '')
    wait = '            DORA64_FS_TRACE("before upload GPU wait");'
    if body.count(wait) != 1:
        raise SystemExit('Non-RAM upload completion layout changed')
    overlap = """            // Geometry/default buffers were finalized before fullSync tile processing.
            // TMEM work changes callTiles/cache metadata, not these upload sources.
            // Let the submitted GPU copy run during that independent CPU work.
            try {
                for (uint32_t f = 0; f < workload.fbPairCount; f++) {
                    fullSyncFramebufferPairTiles(workload, workload.fbPairs[f], loadOpCursor, rdpTileCursor);
                }
            }
            catch (...) {
                // Preserve buffer/command-list lifetime even if texture allocation fails.
                ext.framebufferGraphicsWorker->wait();
                throw;
            }

"""
    body = body.replace(wait, overlap + wait)
    text = text[:begin] + body + text[end:]
    p.write_text(text)
    receipt['upload_overlap_inputs'] = pins
    receipt['upload_overlap_scope'] = 'Non-RAM fullSync GPU geometry copy overlaps CPU tiles; same upload readiness and final GPU/texture waits; eager rendering unchanged'
    receipt['state_generated_sha256'] = hashlib.sha256(p.read_bytes()).hexdigest()
