#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.slang"
#include "../common/commonFogFunctions.slang"
#include "../common/commonUboSceneData.slang"

layout(std140, set=1, binding=0) uniform planetDataBlockVS {
    vec4 uWorldPosAndScale;
    vec4 uColorAndAlpha;
    vec4 uCamPos;
};

layout(location = 0) in vec4 aPosition;   // xy = quad corner, zw = uv

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

[shader("vertex")]
void main() {
    // Camera-facing billboard: camera right/up in world space are the first two
    // columns of the inverse view matrix
    vec3 right = normalize(scene.uInvLookAtMat[0].xyz);
    vec3 up    = normalize(scene.uInvLookAtMat[1].xyz);

    vec3 worldPos = uWorldPosAndScale.xyz + (right * aPosition.x + up * aPosition.y) * uWorldPosAndScale.w;

    // Horizon fade — the disc fades
    // out as it sets/rises through the camera's horizontal plane and is invisible below it
    float horizonFade = clamp((worldPos.z - uCamPos.z) * 2.5, 0.0, 1.0);

    vTexCoord = aPosition.zw;
    vColor = vec4(uColorAndAlpha.xyz, uColorAndAlpha.w * horizonFade);

    gl_Position = scene.uPMatrix * scene.uLookAtMat * vec4(worldPos, 1.0);
}
