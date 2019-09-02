#version 450

/* vertex shader code */
attribute vec3 aPosition;

//Whole scene
uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
//Whole model
uniform mat4 uPlacementMat;

void main() {
    vec4 worldPoint = vec4(aPosition.xyz, 1);

    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * worldPoint;
}