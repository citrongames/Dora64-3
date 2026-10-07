// Dora64-3 world cutouts: transparent RGB must not tint the visible border.
// RDP texture rectangles are the game's 2D bitmap/menu path. Perspective
// triangles with alpha-derived coverage are the GIMO TEX_EDGE materials.
bool dora3WorldSpriteCutout(OtherMode otherMode, RenderFlags flags) {
    return !renderFlagRect(flags) && (otherMode.textPersp() == G_TP_PERSP) &&
        otherMode.cvgXAlpha() && otherMode.alphaCvgSel() && !otherMode.forceBlend();
}

float4 dora3SpriteFilterResult(float4 color, bool alphaWeighted) {
    if (alphaWeighted) {
        // Samples entered the filter with RGB multiplied by alpha. The
        // combiner expects straight RGB; preserve the original filtered alpha.
        color.rgb = (color.a > 0.0f) ? color.rgb / max(color.a, 1.0e-8f) : 0.0f;
    }
    return color;
}
