#version 450

/* vertex shader code */
layout(location = 0) in vec3 aPosition;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};


void main() {

    vec4 c_world = inverse(uPMatrix) * vec4(aPosition, 1);
    c_world = c_world*1.0/c_world.w;

    gl_Position = uPMatrix * uLookAtMat * vec4(c_world.xyz, 1);
}
