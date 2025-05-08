#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_shader_draw_parameters: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.slang"

layout (location = 0) in vec2 position;

layout(std430, set=2, binding=0) StructuredBuffer<SpotLight> spotLightBuffer;

layout(location = 0) out flat int lightIndex;

[shader("vertex")]
void main() {
    gl_Position = vec4(position.xy, 0.1, 1);
    lightIndex = gl_InstanceIndex;
}