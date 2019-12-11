#version 450

precision highp float;

//Individual mesh
layout(std140, binding=1) uniform modelWideBlockVS {
    uniform vec4 uColor;
};

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 finalColor = uColor ;

    fragColor = finalColor;
}
