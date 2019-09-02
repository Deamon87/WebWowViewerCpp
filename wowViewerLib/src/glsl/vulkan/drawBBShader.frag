#version 450

precision highp float;

layout(std140) uniform modelWideBlockVS {
    mat4 uPlacementMat;

    vec4 uBBScale;
    vec4 uBBCenter;
    vec4 uColor;
};

out vec4 outColor;

void main() {
    vec4 finalColor = uColor;
    outColor = finalColor;
}
