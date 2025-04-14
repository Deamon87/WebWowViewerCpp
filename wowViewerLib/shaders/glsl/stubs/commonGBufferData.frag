#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#define GBUFFER_SET_INDEX 0

#include "../common/commonGBufferData.glsl"

/*
-----------------------------------------------------------------------------------
  This is stub shader. It's only purpose is to provide source for Descriptor Layout
-----------------------------------------------------------------------------------
*/

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(
        texture(depthTex, vec2(0))+
        texture(normalTex, vec2(0))+
        vec4(screenSize.xy, 0, 0)
    );
}