#version 450

precision highp float;
in vec4 vColor;
in vec2 vTexcoord0;

uniform sampler2D uTexture;

out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;

    outputColor = vec4((vColor.rgb*tex.rgb), vColor.a );
}
