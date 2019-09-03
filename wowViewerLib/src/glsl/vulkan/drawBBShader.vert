#version 450

/* vertex shader code */
layout(location = 0) in vec3 aPosition;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

// Whole model
layout(std140, binding=1) uniform modelWideBlockVS {
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

    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * worldPoint;
}