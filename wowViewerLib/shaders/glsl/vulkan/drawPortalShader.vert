#version 450

precision highp float;
precision highp int;

/* vertex shader code */
layout(location = 0) in vec3 aPosition;

//Whole scene
layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

void main() {
    vec4 worldPoint = vec4(aPosition.xyz, 1);

    gl_Position = uPMatrix * uLookAtMat * worldPoint;
}