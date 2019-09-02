#version 450

precision highp float;

layout(std140) uniform meshWideBlockPS {
    uniform vec3 uColor;
};

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);

    finalColor.a = 1.0; //do I really need it now?
    fragColor = finalColor;
}
