#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

/* vertex shader code */
layout(location = 0) in vec3 aPosition;

//Whole scene
#include "../common/commonUboSceneData.glsl"

void main() {
    vec4 worldPoint = vec4(aPosition.xyz, 1);

    gl_Position = scene.uPMatrix * scene.uLookAtMat * worldPoint;
}