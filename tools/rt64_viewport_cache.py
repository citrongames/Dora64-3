"""Memoize the pure viewport clipping rectangle with an exact, complete key."""
import hashlib


def adapt(root, out, receipt):
    pins = {
        'hle/rt64_rsp.cpp': '848b9b35f3e76ad94fe2dbacc3a8dc25c689f870c41fd2d190c62af1511d34bc',
        'shared/rt64_rsp_viewport.h': '454c82ebf28cc91350273e19d1aea0de528ecf3ff1ec9ca4db89375a89b572d7',
    }
    for name, digest in pins.items():
        if hashlib.sha256((root/'build-deps/lib/rt64/src'/name).read_bytes()).hexdigest() != digest:
            raise SystemExit('Review viewport rectangle dependencies for changed ' + name)
    p = out/'rt64_rsp.cpp'
    text = p.read_text()
    anchor = 'namespace RT64 {'
    helper = r"""
    // Only rect() inputs enter the key: no vector padding, pointers, or ownership.
    // Comparing every call also covers reset, viewport-stack pops and clip changes.
    static FixedRect dora3ViewportClipRect(const interop::RSPViewport &viewport,
                                         const int16_t *clipRatios) {
        struct Cache {
            uint32_t key[6]{};
            FixedRect value;
            bool valid = false;
        };
        static thread_local Cache cache;
        const float xy[4] = { viewport.scale.x, viewport.scale.y,
                              viewport.translate.x, viewport.translate.y };
        uint32_t key[6];
        static_assert(sizeof(xy) == 16 && sizeof(int16_t) == 2);
        std::memcpy(key, xy, sizeof(xy));
        std::memcpy(key + 4, clipRatios, 4 * sizeof(int16_t));
        if (cache.valid && std::memcmp(key, cache.key, sizeof(key)) == 0) {
            return cache.value;
        }
        // Preserve upstream arithmetic and lround semantics exactly on a miss.
        const FixedRect value = viewport.rect(clipRatios);
        std::memcpy(cache.key, key, sizeof(key));
        cache.value = value;
        cache.valid = true;
        return value;
    }
"""
    needle = 'const FixedRect viewportClipRect = viewport.rect(clipRatios.data());'
    if text.count(anchor) != 1 or text.count(needle) != 1:
        raise SystemExit('Viewport clipping call layout changed')
    text = '#include <cstring>\n' + text.replace(anchor, anchor + helper)
    text = text.replace(needle, 'const FixedRect viewportClipRect = dora3ViewportClipRect(viewport, clipRatios.data());')
    p.write_text(text)
    receipt['viewport_clip_inputs'] = pins
    receipt['viewport_clip_scope'] = 'Thread-local last exact 24-byte input key; unchanged upstream rect on miss; no triangle, scissor or geometry omissions'
    receipt['rsp_generated_sha256'] = hashlib.sha256(p.read_bytes()).hexdigest()
