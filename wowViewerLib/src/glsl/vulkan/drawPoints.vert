#version 450

/* vertex shader code */

attribute vec3 aPosition;

varying vec4 vPos;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
uniform mat4 uPlacementMat;

void main() {
    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * vec4(aPosition.xyz, 1.0);
    gl_PointSize = 10.0;
}