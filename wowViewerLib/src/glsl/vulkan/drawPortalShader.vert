#version 450

/* vertex shader code */
layout(location = 0) in vec3 aPosition;

//Whole scene
layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

layout(std140, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};


void main() {
    vec4 worldPoint = vec4(aPosition.xyz, 1);

    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * worldPoint;
}