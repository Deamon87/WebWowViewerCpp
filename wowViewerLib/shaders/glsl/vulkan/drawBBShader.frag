#version 450

precision highp float;
precision highp int;

layout(std140, binding=0) uniform modelWideBlockVS {
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
