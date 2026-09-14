#ifndef COMMON_LIQUID_FUNCTIONS_SLANG
#define COMMON_LIQUID_FUNCTIONS_SLANG

import glsl;

#include "../commonLightFunctions.slang"
#include "../commonGBufferData.slang"

// ---------------------------------------------------------------------------
// Shared liquid shading helpers (ports of LiquidUtil.h / DirLight helpers)
// ---------------------------------------------------------------------------

// Scene depth at window coords, reconstructed into view space (gbuffer depth)
float sceneViewDepth(vec2 fragCoords, in SceneWideParams scene) {
    return gbuff_getViewPos(fragCoords, scene.uInvPMatrix).z;
}

#endif //COMMON_LIQUID_FUNCTIONS