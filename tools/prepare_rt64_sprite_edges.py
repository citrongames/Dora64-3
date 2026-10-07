"""Generate a build-local RT64 raster sampler; pinned dependency stays read-only."""
from pathlib import Path
import hashlib, json, re, sys

ROOT = Path(__file__).resolve().parents[1]
RT64 = ROOT / 'build-deps/lib/rt64'
PINNED = {
    'CMakeLists.txt': '8d9fd515ef4041bc989e4ceb214594d2a6b4969f19d9d6421a188a4c559b7be6',
    'src/shaders/RasterPS.hlsl': 'dfa02fcecdfe881a21e4f8a5c4d11b07184619625eb3a08a1ae7af9a0ce259e5',
    'src/shaders/TextureSampler.hlsli': '3eafa0ea5a55dece3c19a2a07451d3c7977d670c4aad7082811b7b5e2d7da642',
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
    if(SHADERNAME STREQUAL "src/shaders/RasterPS.hlsl")
        set(FILENAME "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/shaders/RasterPS.hlsl")
    endif()''')
    dependency = '        list(APPEND SHADER_INCLUDE_DEPS "${PROJECT_SOURCE_DIR}/src/shaders/TextureSampler.hlsli")'
    if cmake.count(dependency) != 3:
        raise SystemExit('RT64 shader dependency layout changed')
    cmake = cmake.replace(dependency, '''        list(APPEND SHADER_INCLUDE_DEPS
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/shaders/TextureSampler.hlsli"
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/shaders/world_sprite_edges.hlsli")''')
    outputs = {
        'CMakeLists.txt': cmake,
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
