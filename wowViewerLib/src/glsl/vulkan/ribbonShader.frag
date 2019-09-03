#version 450

precision highp float;
layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexcoord0;

layout(binding=3) uniform sampler2D uTexture;

layout(location = 0) out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;

    outputColor = vec4((vColor.rgb*tex.rgb), vColor.a );
}
