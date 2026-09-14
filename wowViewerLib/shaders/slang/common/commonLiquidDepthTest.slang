#ifndef COMMON_LIQUID_DEPTH_TEST_SLANG
#define COMMON_LIQUID_DEPTH_TEST_SLANG

// Per-fragment liquid depth test for transparent objects.
// Discards fragments that are on the wrong side of the liquid plane
// based on the current rendering mode (below=0, above=1, all=2).
// Requires: liquidDepth (sampler2D at set=0 binding=3)
//           liquidRenderMode (SSBO uint at set=0 binding=4)

void liquidDepthClip() {
    if (liquidRenderMode < 2u) {
        ivec2 liquidTexSize = textureSize(liquidDepth, 0);
        vec2 screenUV = gl_FragCoord.xy / vec2(liquidTexSize);
        float liquidZ = textureLod(liquidDepth, screenUV, 0.0).x;
        float fragZ = gl_FragCoord.z;

        // This renderer uses regular Z (invertZ = false): smaller depth = nearer to camera.
        // A fragment nearer than the liquid surface is ABOVE the liquid.
        bool fragAboveLiquid = (fragZ < liquidZ);

        if (liquidRenderMode == 0u && fragAboveLiquid) discard;  // below pass: discard above
        if (liquidRenderMode == 1u && !fragAboveLiquid) discard; // above pass: discard below
    }
}

#endif
