#version 450

/* vertex shader code */
attribute vec3 aPosition;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;

uniform mat4 uInverseViewProjection;

void main() {

    vec4 c_world = uInverseViewProjection * vec4(aPosition, 1);
    c_world = c_world*1.0/c_world.w;

    gl_Position = uPMatrix * uLookAtMat * vec4(c_world.xyz, 1);
}
