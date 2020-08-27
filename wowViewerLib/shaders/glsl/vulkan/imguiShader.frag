#version 450

precision highp float;

layout(location=0) in vec2 Frag_UV;
layout(location=1) in vec4 Frag_Color;

layout(set=1,binding=5) uniform sampler2D Texture;

layout (location = 0) out vec4 Out_Color;

void main()
{
    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
}