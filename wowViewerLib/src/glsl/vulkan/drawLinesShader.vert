#version 450

/* vertex shader code */
attribute vec2 aPosition;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;

uniform mat4 uInverseViewProjection;

void main() {
    gl_Position = uPMatrix * uLookAtMat * vec4(aPosition.xy, 0, 1);
}