#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.slang"

struct AABBRecord {
    vec4 boxMin;
    vec4 boxMax;
};

layout(std430, set=1, binding=0) StructuredBuffer<AABBRecord> aabbBuffer;

[shader("vertex")]
void main() {
    AABBRecord box = aabbBuffer[gl_InstanceIndex];

    vec3 corner = vec3(
        (gl_VertexIndex & 1) != 0 ? box.boxMax.x : box.boxMin.x,
        (gl_VertexIndex & 2) != 0 ? box.boxMax.y : box.boxMin.y,
        (gl_VertexIndex & 4) != 0 ? box.boxMax.z : box.boxMin.z
    );

    vec4 viewPos = scene.uLookAtMat * vec4(corner, 1.0);
    gl_Position = scene.uPMatrix * viewPos;
}