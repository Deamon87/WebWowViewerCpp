#version 450

import glsl;

precision highp float;
precision highp int;

layout(location=0) in vec2 Frag_UV;
layout(location=1) in vec4 Frag_Color;

layout(set=1,binding=0) uniform sampler2D Texture;

layout (location = 0) out vec4 Out_Color;

[shader("pixel")]
void main()
{
    Out_Color = Frag_Color * vec4(texture(Texture, Frag_UV).rgb, 1.0);
}