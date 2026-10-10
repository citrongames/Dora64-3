"""Generate a build-local RT64 raster sampler; pinned dependency stays read-only."""
from pathlib import Path
import hashlib, json, re, sys

ROOT = Path(__file__).resolve().parents[1]
RT64 = ROOT / 'build-deps/lib/rt64'
PINNED = {
    'CMakeLists.txt': '8d9fd515ef4041bc989e4ceb214594d2a6b4969f19d9d6421a188a4c559b7be6',
    'src/shaders/RasterPS.hlsl': 'dfa02fcecdfe881a21e4f8a5c4d11b07184619625eb3a08a1ae7af9a0ce259e5',
    'src/shaders/TextureSampler.hlsli': '3eafa0ea5a55dece3c19a2a07451d3c7977d670c4aad7082811b7b5e2d7da642',
    'src/shaders/FbWriteColorCS.hlsl': 'c589dfa305f3aaa791e05966b16cb9aba3a4661988a7a122ae8dd9806ebfa317',
    'src/shaders/FbWriteDepthCS.hlsl': '2a170828ab61d215848e1a4cd8999d6cc037beba22dde7c78e283b510d6469ef',
    'src/shaders/FbWritePackedCS.hlsl': 'e5b8a52eac01d752f5e15cde78e6c3f7c9545ece9293ccfd8df3bbb94afb8c24',
}

def replace_once(text, old, new):
    if text.count(old) != 1:
        raise SystemExit('RT64 sprite adaptation anchor changed: ' + old[:90])
    return text.replace(old, new)

def main():
    out = Path(sys.argv[1]).resolve()
    sources = {}
    for name, digest in PINNED.items():
        data = (RT64 / name).read_bytes()
        if hashlib.sha256(data).hexdigest() != digest:
            raise SystemExit('RT64 source changed; review world sprite adaptation: ' + name)
        sources[name] = data.decode()
    sampler = sources['src/shaders/TextureSampler.hlsli']
    sampler = replace_once(sampler, '#define SIMULATE_LOW_PRECISION 1',
        '#include "world_sprite_edges.hlsli"\n\n#define SIMULATE_LOW_PRECISION 1')
    sampler = replace_once(sampler, 'uint mipLevel, bool usesHDR) {',
        'uint mipLevel, bool usesHDR, bool alphaWeighted) {')
    sampler = replace_once(sampler, '    float4 sample00 = samples[0];', '''    // Weight RGB before either bilinear, average or N64 three-point filtering.
    // Fully transparent texels then contribute no hidden palette color.
    if (filtering && alphaWeighted) {
        [unroll]
        for (int i = 0; i < 4; i++) {
            samples[i].rgb *= samples[i].a;
        }
    }
    float4 sample00 = samples[0];''')
    for expression in [
        'lerp(lerp(sample00, sample10, fracPart.x), lerp(sample01, sample11, fracPart.x), fracPart.y)',
        '(sample00 + sample01 + sample10 + sample11) / 4.0f',
        'lerp(tri0, tri1, step(1.0f, fracPart.x + fracPart.y))',
    ]:
        sampler = replace_once(sampler, 'return ' + expression + ';',
            'return dora3SpriteFilterResult(' + expression + ', alphaWeighted);')
    sampler = replace_once(sampler,
        '    const bool flagHasMipmaps = !forceNearestFiltering && gpuTileFlagHasMipmaps(gpuTile.flags);',
        '''    const bool flagHasMipmaps = !forceNearestFiltering && gpuTileFlagHasMipmaps(gpuTile.flags);
    // Original ROM cutouts have no replacement mipmaps. Framebuffer alpha is
    // coverage, not opacity, and must retain its separate decoder semantics.
    const bool alphaWeighted = dora3WorldSpriteCutout(otherMode, renderFlags) &&
        !gpuTileFlagHasMipmaps(gpuTile.flags) && !gpuTileFlagAlphaIsCvg(gpuTile.flags);''')
    old = 'RDPMipLevels[i], usesHDR);'
    sampler = replace_once(sampler, old, 'RDPMipLevels[i], usesHDR, alphaWeighted);')
    for index in (0, 1):
        sampler = replace_once(sampler, f'RDPMipLevels[{index}], usesHDR);',
            f'RDPMipLevels[{index}], usesHDR, alphaWeighted);')

    cmake = sources['CMakeLists.txt']
    # The adapter owns only this CMake file and raster copies. All remaining
    # sources/tools/headers still resolve to the original pinned checkout.
    cmake = replace_once(cmake, 'project(rt64 LANGUAGES C CXX)',
        'project(rt64 LANGUAGES C CXX)\nset(PROJECT_SOURCE_DIR "' + RT64.as_posix() + '")')
    cmake = re.sub(r'add_subdirectory\((src/[^)\s]+)\)',
        r'add_subdirectory("${PROJECT_SOURCE_DIR}/\1" "\1")', cmake)
    cmake = replace_once(cmake, 'set (DXC_COMMON_OPTS "-I${PROJECT_SOURCE_DIR}/src")',
        'set (DXC_COMMON_OPTS "-I${PROJECT_SOURCE_DIR}/src" "-I${PROJECT_SOURCE_DIR}/src/shaders")')
    needle = '    set(FILENAME "${PROJECT_SOURCE_DIR}/${SHADERNAME}")'
    if cmake.count(needle) != 5:
        raise SystemExit('RT64 shader builder layout changed')
    cmake = cmake.replace(needle, needle + '''
    if(SHADERNAME STREQUAL "src/shaders/RasterPS.hlsl" OR
       SHADERNAME STREQUAL "src/shaders/FbWriteColorCS.hlsl" OR
       SHADERNAME STREQUAL "src/shaders/FbWriteDepthCS.hlsl" OR
       SHADERNAME STREQUAL "src/shaders/FbWritePackedCS.hlsl")
        set(FILENAME "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/${SHADERNAME}")
    endif()''')
    dependency = '        list(APPEND SHADER_INCLUDE_DEPS "${PROJECT_SOURCE_DIR}/src/shaders/TextureSampler.hlsli")'
    if cmake.count(dependency) != 3:
        raise SystemExit('RT64 shader dependency layout changed')
    cmake = cmake.replace(dependency, '''        list(APPEND SHADER_INCLUDE_DEPS
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/shaders/TextureSampler.hlsli"
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/shaders/world_sprite_edges.hlsli")''')
    # Preserve ordinary eager offset/pixel addressing. Only the optional
    # deferred-copy flag selects native pixel centers in a scaled GPU target.
    fb_color=sources['src/shaders/FbWriteColorCS.hlsl']
    fb_depth=sources['src/shaders/FbWriteDepthCS.hlsl']
    fb_packed=sources['src/shaders/FbWritePackedCS.hlsl']
    old='        uint2 offsetCoord = gConstants.offset + coord;'
    new="""        const bool deferred = (gConstants.offset.x & 0x80000000u) != 0;
        uint2 offsetCoord = uint2(deferred ? 0u : gConstants.offset.x, (deferred ? (gConstants.offset.y & 0xffffu) : gConstants.offset.y)) + coord;
        uint2 sourceCoord = deferred ? uint2((float2(offsetCoord) + 0.5f) *
            (float(gConstants.offset.x & 0xffffu) / 256.0f) +
            float2(float((gConstants.offset.x >> 16) & 0x7fffu) + float(gConstants.offset.y >> 16) / 65536.0f, 0.0f)) : offsetCoord;"""
    fb_color=replace_once(fb_color,old,new)
    fb_depth=replace_once(fb_depth,old,new)
    fb_color=replace_once(fb_color,'gInput.Load(uint3(offsetCoord, 0))','gInput.Load(uint3(sourceCoord, 0))')
    fb_depth=replace_once(fb_depth,'gInput.Load(offsetCoord, 0)','gInput.Load(sourceCoord, 0)')
    fb_depth=replace_once(fb_depth,'gInput.Load(uint3(offsetCoord, 0))','gInput.Load(uint3(sourceCoord, 0))')
    old='    const uint2 pixelCoord = uint2(pixelIndex % gConstants.resolution.x, pixelIndex / gConstants.resolution.x);'
    new=old+"""
    const bool deferred = (gConstants.offset.x & 0x80000000u) != 0;
    const uint2 sourceCoord = deferred ? uint2((float2(pixelCoord) + 0.5f) *
        (float(gConstants.offset.x & 0xffffu) / 256.0f) +
        float2(float((gConstants.offset.x >> 16) & 0x7fffu) + float(gConstants.offset.y >> 16) / 65536.0f, 0.0f)) : pixelCoord;"""
    fb_packed=replace_once(fb_packed,old,new)
    fb_packed=replace_once(fb_packed,'gInput.Load(pixelCoord, 0)','gInput.Load(sourceCoord, 0)')
    fb_packed=fb_packed.replace('gInput.Load(uint3(pixelCoord, 0))','gInput.Load(uint3(sourceCoord, 0))')
    outputs = {
        'CMakeLists.txt': cmake,
        'src/shaders/FbWriteColorCS.hlsl': fb_color,
        'src/shaders/FbWriteDepthCS.hlsl': fb_depth,
        'src/shaders/FbWritePackedCS.hlsl': fb_packed,
        'src/shaders/RasterPS.hlsl': sources['src/shaders/RasterPS.hlsl'],
        'src/shaders/TextureSampler.hlsli': sampler,
        'src/shaders/world_sprite_edges.hlsli': (ROOT / 'src/shaders/world_sprite_edges.hlsli').read_text(),
    }
    hashes = {}
    for name, text in outputs.items():
        destination = out / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        data = text.encode()
        if not destination.exists() or destination.read_bytes() != data:
            destination.write_bytes(data)
        hashes[name] = hashlib.sha256(data).hexdigest()
    receipt = {
        'inputs': PINNED,
        'helper_sha256': hashlib.sha256((ROOT / 'src/shaders/world_sprite_edges.hlsli').read_bytes()).hexdigest(),
        'generated': hashes,
        'scope': 'Perspective triangle TEX_EDGE cutouts only, original unmipped opacity textures; three-point/bilinear/average RGB weighted by alpha, filtered alpha/coverage/blender preserved. 2D rectangles, force-blended effects, framebuffer coverage and replacement mipmaps unchanged.',
    }
    (out / 'generation-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print('Prepared isolated RT64 world sprite edge filtering')

if __name__ == '__main__':
    main()
