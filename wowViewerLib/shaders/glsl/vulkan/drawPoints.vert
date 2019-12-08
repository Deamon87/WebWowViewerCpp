#version 450

/* vertex shader code */

layout(location = 0) in vec3 aPosition;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};


layout(std140, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};

layout(location = 0) out vec4 vPos;

void main() {
    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * vec4(aPosition.xyz, 1.0);
    gl_PointSize = 10.0;
}