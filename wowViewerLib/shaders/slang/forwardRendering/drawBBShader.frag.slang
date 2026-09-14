#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.slang"
#include "../common/commonFogFunctions.slang"


layout(std140, set=0, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;

    vec4 uBBScale;
    vec4 uBBCenter;
    vec4 uColor;
};

layout(location = 0) out vec4 outColor;

[shader("pixel")]
void main() {
    vec4 finalColor = uColor;
    outColor = finalColor;
}
