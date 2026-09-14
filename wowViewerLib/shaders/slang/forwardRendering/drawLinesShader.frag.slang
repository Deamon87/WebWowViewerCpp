#version 450

import glsl;

precision highp float;
precision highp int;

layout(std140, binding=1) uniform modelWideBlockPS {
    vec3 uColor;
};

layout(location = 0) out vec4 fragColor;

[shader("pixel")]
void main() {
    vec4 finalColor = vec4(1.0, 1.0, 0.0, 1.0);

    fragColor = finalColor;
}
