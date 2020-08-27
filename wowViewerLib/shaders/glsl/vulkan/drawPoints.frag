#version 450

precision highp float;

layout(location = 0) in vec4 vPos;

layout(std140, binding=1) uniform modelWideBlockVS {
    vec3 uColor;
};

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(uColor.rgb, 1.0);
}