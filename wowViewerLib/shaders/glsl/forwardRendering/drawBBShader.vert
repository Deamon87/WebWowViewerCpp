#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonUboSceneData.glsl"

/* vertex shader code */
layout(location = 0) in vec3 aPosition;



// Whole model
layout(std140, set=0, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;

    vec4 uBBScale;
    vec4 uBBCenter;
    vec4 uColor;
};

void main() {
    vec4 worldPoint = vec4(
        aPosition.x*uBBScale.x + uBBCenter.x,
        aPosition.y*uBBScale.y + uBBCenter.y,
        aPosition.z*uBBScale.z + uBBCenter.z,
    1);

    gl_Position = scene.uPMatrix * scene.uLookAtMat * uPlacementMat * worldPoint;
}