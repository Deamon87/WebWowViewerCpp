#ifndef COMMON_SCENE_DATA_SLANG
#define COMMON_SCENE_DATA_SLANG

import glsl;

#include "commonFogFunctions.slang"
#include "commonLightFunctions.slang"

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
    // Underwater fog (Light.db2 LightParams slot 1), used by liquid above shaders
    vec4 underWaterFog;
    vec4 underWaterClassicFogParams;
    vec4 underWaterFogColor;
};
layout(set=0, binding=1) uniform sampler2D lightBuffer;
layout(set=0, binding=2) uniform sampler2D aoBuffer;
layout(set=0, binding=3) uniform sampler2D liquidDepth;
layout(std430, set=0, binding=4) readonly buffer LiquidRenderModeBlock {
    uint liquidRenderMode;  // 0=below liquid, 1=above liquid, 2=all (no filtering)
};

#endif