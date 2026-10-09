"""Batch the measured RT64 vertex/triangle bookkeeping without dropping geometry."""
import hashlib


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise SystemExit('RT64 geometry boundary changed: ' + old[:100])
    return text.replace(old, new)


def adapt(root, out, receipt):
    pinned = {
        'rt64_rsp.cpp': '848b9b35f3e76ad94fe2dbacc3a8dc25c689f870c41fd2d190c62af1511d34bc',
        'rt64_rdp.cpp': '87abb3e0bd7f7323ec6fb40318ec01617fca43f01b019461efbebc41b4430a7b',
    }
    for name, expected in pinned.items():
        source = root / 'build-deps/lib/rt64/src/hle' / name
        if hashlib.sha256(source.read_bytes()).hexdigest() != expected:
            raise SystemExit('Review geometry and texcoord semantics for changed ' + name)
    p = out / 'rt64_rsp.cpp'
    text = p.read_text()
    begin = text.index('    void RSP::setVertexCommon(')
    end = text.index('    void RSP::modifyVertex(', begin)
    body = text[begin:end]
    anchor = '        const uint32_t globalIndex = workload.drawData.vertexCount();'
    # Each vector grows once per command. Use its own old size; the existing
    # global vertex index remains unchanged for extended-segment addressing.
    setup = anchor + "\n        const size_t batchCount = dstMax - dstIndex;\n"
    for name, count, value in [
        ('posFloats', 3, None), ('normColBytes', 4, None),
        ('viewProjIndices', 1, 'curViewProjIndex'), ('worldIndices', 1, 'curTransformIndex'),
        ('fogIndices', 1, 'curFogIndex'), ('lightIndices', 1, 'curLightIndex'),
        ('lightCounts', 1, 'curLightCount'), ('lookAtIndices', 1, 'curLookAtIndex'),
        ('posTransformed', 1, None), ('posScreen', 1, None), ('tcFloats', 2, None),
    ]:
        if value is None:
            setup += f'        const size_t {name}Base = {name}.size();\n'
        fill = ', ' + value if value is not None else ''
        setup += f'        {name}.resize({name}.size() + batchCount * {count}{fill});\n'
    setup += """        if constexpr (addEmptyVelocity) {
            velFloats.resize(velFloats.size() + batchCount * 3, 0.0f);
            tcVelFloats.resize(tcVelFloats.size() + batchCount * 2, 0.0f);
        }
"""
    body = replace_once(body, anchor, setup.rstrip())
    for name, values in [('posFloats', ['v.x', 'v.y', 'v.z']),
                         ('normColBytes', ['v.color.r', 'v.color.g', 'v.color.b', 'v.color.a'])]:
        for index, value in enumerate(values):
            body = replace_once(body, f'{name}.emplace_back({value});',
                f'{name}[{name}Base + (i - dstIndex) * {len(values)} + {index}] = {value};')
    for name, value in [('viewProjIndices', 'curViewProjIndex'), ('worldIndices', 'curTransformIndex'),
                        ('fogIndices', 'curFogIndex'), ('lightIndices', 'curLightIndex'),
                        ('lightCounts', 'curLightCount'), ('lookAtIndices', 'curLookAtIndex')]:
        body = replace_once(body, f'            {name}.emplace_back({value});\n', '')
    body = replace_once(body, """
            if constexpr (addEmptyVelocity) {
                velFloats.emplace_back(0.0f);
                velFloats.emplace_back(0.0f);
                velFloats.emplace_back(0.0f);
                tcVelFloats.emplace_back(0.0f);
                tcVelFloats.emplace_back(0.0f);
            }
""", '')
    body = replace_once(body, 'posTransformed.emplace_back(tfPos);',
                        'posTransformed[posTransformedBase + (i - dstIndex)] = tfPos;')
    body = replace_once(body,
        'posScreen.emplace_back((tfPos.xyz / hlslpp::float3(tfPos.w, -tfPos.w, tfPos.w)) * viewport.scale + viewport.translate);',
        'posScreen[posScreenBase + (i - dstIndex)] = (tfPos.xyz / hlslpp::float3(tfPos.w, -tfPos.w, tfPos.w)) * viewport.scale + viewport.translate;')
    for index, axis in enumerate(['s', 't']):
        scale = 'TextureSc' if axis == 's' else 'TextureTc'
        body = replace_once(body, f'tcFloats.emplace_back({scale});',
            f'tcFloats[tcFloatsBase + (i - dstIndex) * 2 + {index}] = {scale};')
        expr = f'(float)((double)((vertices[i].{axis}) * {scale}) / Divisor)'
        body = replace_once(body, f'tcFloats.emplace_back({expr});',
            f'tcFloats[tcFloatsBase + (i - dstIndex) * 2 + {index}] = {expr};')
    text = text[:begin] + body + text[end:]
    # RDP::updateCallTexcoords only reduces integer bounds over callTiles.
    # Reduce all three vertices first, preserving truncation/ceil conversion,
    # then update each tile once. No tile means no conversion, as before.
    old = """        for (int i = 0; i < 3; i++) {
            // TODO: Figure out how to handle texcoord tracking on TEXGEN cases.
            const uint32_t globalIndex = globalIndices[i];
            state->rdp->updateCallTexcoords(tcFloats[globalIndex * 2 + 0], tcFloats[globalIndex * 2 + 1]);
            faceIndices.push_back(globalIndices[i]);
            minMatrix = std::min(minMatrix, worldIndices[globalIndex]);
            maxMatrix = std::max(maxMatrix, worldIndices[globalIndex]);
        }"""
    new = """        // Same integer bounds as three updateCallTexcoords calls, one tile pass.
        if (drawCall.tileCount != 0) {
            int minU = 0, minV = 0, maxU = 0, maxV = 0;
            for (int i = 0; i < 3; i++) {
                const float u = tcFloats[globalIndices[i] * 2 + 0];
                const float v = tcFloats[globalIndices[i] * 2 + 1];
                const int loU = int(u), loV = int(v);
                const int hiU = int(ceilf(u)), hiV = int(ceilf(v));
                minU = (i == 0) ? loU : std::min(minU, loU);
                minV = (i == 0) ? loV : std::min(minV, loV);
                maxU = (i == 0) ? hiU : std::max(maxU, hiU);
                maxV = (i == 0) ? hiV : std::max(maxV, hiV);
            }
            for (uint32_t t = 0; t < drawCall.tileCount; t++) {
                DrawCallTile &callTile = workload.drawData.callTiles[drawCall.tileIndex + t];
                callTile.minTexcoord.x = std::min(callTile.minTexcoord.x, minU);
                callTile.minTexcoord.y = std::min(callTile.minTexcoord.y, minV);
                callTile.maxTexcoord.x = std::max(callTile.maxTexcoord.x, maxU);
                callTile.maxTexcoord.y = std::max(callTile.maxTexcoord.y, maxV);
            }
        }
        faceIndices.insert(faceIndices.end(), globalIndices, globalIndices + 3);
        for (int i = 0; i < 3; i++) {
            const uint32_t globalIndex = globalIndices[i];
            minMatrix = std::min(minMatrix, worldIndices[globalIndex]);
            maxMatrix = std::max(maxMatrix, worldIndices[globalIndex]);
        }"""
    text = replace_once(text, old, new)
    text = replace_once(text, '#include <cassert>', '#include <cassert>\n#include <cmath>')
    p.write_text(text)
    receipt['geometry_batch_inputs'] = pinned
    receipt['geometry_batch_scope'] = 'Per-command vector resize/fill, indexed vertex writes; same integer texture bounds reduced per triangle; same topology, transforms, clipping, draw state and order'
    receipt['rt64_rsp.cpp_generated_sha256'] = hashlib.sha256(p.read_bytes()).hexdigest()
