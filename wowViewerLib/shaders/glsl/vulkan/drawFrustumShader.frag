#version 450

precision highp float;
precision highp int;

layout(std140, binding=2) uniform meshWideBlockPS {
    vec3 uColor;
};

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);

    finalColor.a = 1.0; //do I really need it now?
    fragColor = finalColor;
}
