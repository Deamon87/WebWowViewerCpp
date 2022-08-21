#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"


layout(std140, set=0, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;

    vec4 uBBScale;
    vec4 uBBCenter;
    vec4 uColor;
};

layout(location = 0) out vec4 outColor;

void main() {
    vec4 finalColor = uColor;
    outColor = finalColor;
}
