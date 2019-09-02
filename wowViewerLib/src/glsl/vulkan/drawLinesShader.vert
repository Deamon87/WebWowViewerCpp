#version 450

/* vertex shader code */
layout(location = 0) in vec2 aPosition;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

void main() {
    gl_Position = uPMatrix * uLookAtMat * vec4(aPosition.xy, 0, 1);
}