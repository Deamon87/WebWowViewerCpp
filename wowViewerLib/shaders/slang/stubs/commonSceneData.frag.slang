#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonUboSceneData.slang"

/*
-----------------------------------------------------------------------------------
  This is stub shader. It's only purpose is to provide source for Descriptor Layout
-----------------------------------------------------------------------------------
*/

layout(location = 0) out vec4 outColor;

[shader("pixel")]
void main() {
    outColor = vec4(
        texture(lightBuffer, vec2(0))+
        texture(aoBuffer, vec2(0))+
        texture(liquidDepth, vec2(0))+
        vec4(scene.uLookAtMat[0].xy, 0, 0)
    );
}