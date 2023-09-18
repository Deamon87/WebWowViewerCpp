#ifndef COMMON_SCENE_DATA
#define COMMON_SCENE_DATA

#include "commonFogFunctions.glsl"
#include "commonLightFunctions.glsl"

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

#endif