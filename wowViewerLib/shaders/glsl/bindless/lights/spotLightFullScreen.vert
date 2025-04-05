#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_shader_draw_parameters: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"

layout (location = 0) in vec2 position;

layout(std430, set=1, binding=0) buffer readonly spotLightBuffer {
    SpotLight lights[];
};

layout(location = 0) out flat int lightIndex;

void main() {
    gl_Position = vec4(position.xy, 0.1, 1);
    lightIndex = gl_InstanceIndex;
}