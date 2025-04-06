#version 450

precision highp float;
precision highp int;

//Individual mesh
layout(std140, set=1, binding=0) uniform meshWideBlockPS {
    vec4 uColor;
};

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 finalColor = uColor ;

    fragColor = finalColor;
}
