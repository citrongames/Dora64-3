"""Dora64-3 explicit display-list roles; isolate donor renderer rules."""
import hashlib
PINNED = {'rt64_framebuffer_renderer.cpp': '3c9d222843e6075fdbfb828a655503c7f1faf0e56fae46c9e55e73d38f313af2', 'rt64_projection_processor.cpp': 'cf04cb4da1bc39ab60cb24c92c0668385632ab7c6894157e6857c93252a39de9'}
def once(t,a,b):
    if t.count(a)!=1:raise ValueError('Widescreen adapter boundary changed: '+a[:100])
    return t.replace(a,b)
def cut(t,start,end,replacement=''):
    a=t.index(start);b=t.index(end,a);return t[:a]+replacement+t[b:]
def adapt(root,out,receipt):
    for name,digest in PINNED.items():
        raw=(root/'build-deps/lib/rt64/src/render'/name).read_bytes()
        if hashlib.sha256(raw).hexdigest()!=digest:raise ValueError('Unreviewed widescreen RT64 source '+name)
        t=raw.decode()
        if name=='rt64_projection_processor.cpp':
            t=once(t,'(viewportOrigin == G_EX_ORIGIN_NONE) && coversWholeWidth && horizontalRatio;',
                '(viewportOrigin == G_EX_ORIGIN_NONE) && ((proj.type == Projection::Type::Perspective) || (coversWholeWidth && horizontalRatio));')
        else:
            t=once(t, 'const float aspectRatioScale = adjustRatio ?', 'const float aspectRatioScale = (p.fbWidth == 320 || adjustRatio) ?')
            t=cut(t,'        constexpr uint64_t CameraHudTextureHash','        for (uint32_t pr =')
            t=cut(t,'            bool precedingThreeDimensionalProjection','#       if RT_ENABLED')
            t=once(t,'(viewportOrigin == G_EX_ORIGIN_NONE) && coversWholeWidth && horizontalRatio;',
                '(viewportOrigin == G_EX_ORIGIN_NONE) && ((proj.type == Projection::Type::Perspective) || (coversWholeWidth && horizontalRatio));')
            t=cut(t,'                if (wideThreeDimensionalScissor) {','            }\n\n            for (uint32_t d')
            t=cut(t,'                const auto menuTile =','                renderIndices.instanceIndex', '''                const bool repeatMenuTile = widescreenRequested &&
                    (call.callDesc.rectLeftOrigin == 0x801 || call.callDesc.rectLeftOrigin == 0x802);
                const bool irisMask = widescreenRequested && (call.callDesc.rectLeftOrigin == 0x803);
                const float repeatWidth = call.callDesc.rectLeftOrigin == 0x802 ? 162.0f : 160.0f;
''')
            t=cut(t,'                    const bool wideIntroBackdropFill =','                    // A rect that spans', '                    const bool wideIntroBackdropFill = false;\n\n')
            t=cut(t,'                        const bool regularRectangleOrigins =','                        switch (proj.type)', '''                        const bool panoramaRectangle = false;
                        const bool wideOverlayRectangle = false; // STRETCH maps partial effect bounds proportionally below.
                        const bool fitGameplayHudRectangle = false;
                        const int gameplayHudHorizontalDirection = 0;
''')
            t=cut(t,'                            if (wideBackgroundProjection &&','                            if (wideOverlayRectangle)')
            t=once(t, '                            RenderViewport viewportRect = convertViewportRect(call.callDesc.rect, p.resolutionScale, p.fbWidth, invRatioScale, extOriginPercentage, horizontalMisalignment, call.callDesc.rectLeftOrigin, call.callDesc.rectRightOrigin);', """                            const bool viewCentered = call.callDesc.rectLeftOrigin == 0x804 || call.callDesc.rectLeftOrigin == 0x805;
                            const uint16_t viewCenter = call.callDesc.rectLeftOrigin == 0x804 ? 0x100 : 0x300;
                            RenderViewport viewportRect = convertViewportRect(call.callDesc.rect, p.resolutionScale, p.fbWidth, invRatioScale,
                                viewCentered ? 1.0f : extOriginPercentage, horizontalMisalignment,
                                viewCentered ? viewCenter : call.callDesc.rectLeftOrigin,
                                viewCentered ? viewCenter : call.callDesc.rectRightOrigin);""")
            t=once(t, '                        triangles.scissor = convertFixedRect(call.callDesc.scissorRect, p.resolutionScale, p.fbWidth, invRatioScale, extOriginPercentage, int32_t(horizontalMisalignment), call.callDesc.scissorLeftOrigin, call.callDesc.scissorRightOrigin);', """                        triangles.scissor = convertFixedRect(call.callDesc.scissorRect, p.resolutionScale, p.fbWidth, invRatioScale, extOriginPercentage, int32_t(horizontalMisalignment), call.callDesc.scissorLeftOrigin, call.callDesc.scissorRightOrigin);
                        if (proj.type == Projection::Type::Rectangle &&
                            (call.callDesc.rectLeftOrigin == 0x804 || call.callDesc.rectLeftOrigin == 0x805)) {
                            const float shift = call.callDesc.rectLeftOrigin == 0x804 ? -middleViewport * 0.5f : middleViewport * 0.5f;
                            triangles.scissor.left += lround(shift);
                            triangles.scissor.right += lround(shift);
                        }""")
            t=once(t,'MenuBackground::RepeatWidth * scale, wideWidth);','repeatWidth * scale, wideWidth);')
            t=once(t,'triangles.horizontalRepeatLast = menuRepeatSource ? repeats.last : repeats.first - 1;','triangles.horizontalRepeatLast = repeats.last;')
            t=once(t,'triangles.horizontalRepeatStep = float(MenuBackground::RepeatWidth);','triangles.horizontalRepeatStep = repeatWidth;')
            t=cut(t,'                        if (wideThreeDimensionalScissor) {','                        if (repeatMenuTile ||')
            t=once(t,'                        if (repeatMenuTile ||','                        if (irisMask || repeatMenuTile ||')
            t=once(t,'                        if (fitGameplayHudRectangle) {\n                            triangles.scissor.left = 0;\n                            triangles.scissor.right = lround(wideWidth);\n                        }', '''                        // Explicit HUD origins shift their own clipping region by
                        // the same displacement. Centered dialogue/map stay clipped.
                        if ((proj.type == Projection::Type::Rectangle) &&
                            (call.callDesc.rectLeftOrigin == call.callDesc.rectRightOrigin) &&
                            (call.callDesc.rectLeftOrigin < G_EX_ORIGIN_NONE)) {
                            const float shift = (float(call.callDesc.rectLeftOrigin) / G_EX_ORIGIN_CENTER - 1.0f) * middleViewport * extOriginPercentage;
                            triangles.scissor.left += lround(shift);
                            triangles.scissor.right += lround(shift);
                        }''')
        (out/name).write_text(t)
        receipt.setdefault('widescreen',{})[name]={'source':digest,'generated':hashlib.sha256(t.encode()).hexdigest()}

    # ADJUST is our default centered-UI role. Preserve original RDP one-pixel
    # edge repair for fills/UI; only periodic strips require exact motif edges.
    p=out/'rt64_rdp.cpp';t=p.read_text()
    t=once(t, '(extended.global.rectAspect != G_EX_ASPECT_ADJUST);',
        '(extended.global.rect.leftOrigin != 0x801) && (extended.global.rect.leftOrigin != 0x802);')
    t=once(t, '        const FixedRect drawRect(movedFromOrigin(ulx, extAlignment.leftOrigin), uly, movedFromOrigin(lrx, extAlignment.rightOrigin), lry);', """        // Scene1's ROM-qualified iris scope uses a clamped32x32 I4 mask.
        // Extend its quad, preserving dS/dX and the circular opening. The
        // opaque edge texels cover the added sides; surrounding fills extend
        // only their exterior edges. Coordinates remain signed internally.
        const int32_t irisOriginalUlx = ulx;
        const bool irisMask = (extAlignment.leftOrigin == 0x803);
        if (irisMask) {
            const bool textureMask = dsdx != 0 && dtdy != 0;
            if (textureMask || ulx <= 0) ulx = std::min(ulx, -4096);
            if (textureMask || lrx >= 1276) lrx = std::max(lrx, 5376);
        }
        const FixedRect drawRect(movedFromOrigin(ulx, extAlignment.leftOrigin), uly, movedFromOrigin(lrx, extAlignment.rightOrigin), lry);""")
    t=once(t, '        float u2 = lrs / 32.0f;', """        float u2 = lrs / 32.0f;
        if (irisMask && !flip) {
            const float delta = float(ulx - irisOriginalUlx) * float(dsdx) / 4096.0f;
            u1 += delta;
            u2 += delta;
        }""")
    p.write_text(t)
    receipt['widescreen']['rdp_edge_repair']=hashlib.sha256(t.encode()).hexdigest()

    # Dora3 changes layer after the frame's common viewport was already loaded.
    # Rebind its role immediately; offsets also cancel the origin for any later
    # viewport load. Current viewport geometry itself stays unchanged.
    p=out/'rt64_rsp.cpp';t=p.read_text()
    t=once(t, '        extended.global.viewportOffsetY = offy;', """        extended.global.viewportOffsetY = offy;
        if (extended.viewportOriginStack[viewportStackSize - 1] != ori) {
            extended.viewportOriginStack[viewportStackSize - 1] = ori;
            viewportChanged = true;
        }""")
    p.write_text(t)
    receipt['widescreen']['viewport_role_binding']=hashlib.sha256(t.encode()).hexdigest()
