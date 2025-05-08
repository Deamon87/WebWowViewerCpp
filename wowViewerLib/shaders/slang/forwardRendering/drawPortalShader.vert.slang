#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.slang"
#include "../common/commonFogFunctions.slang"

/* vertex shader code */
layout(location = 0) in vec3 aPosition;

//Whole scene
#include "../common/commonUboSceneData.slang"

[shader("vertex")]
void main() {
    vec4 worldPoint = vec4(aPosition.xyz, 1);

    gl_Position = scene.uPMatrix * scene.uLookAtMat * worldPoint;
}