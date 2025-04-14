#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"

layout(location = 0) out vec4 outColor;

void main() {
    // localPoint = inverse(modelMat) * worldPoint
    // texCoordinates = projectMat * localPoint;
}