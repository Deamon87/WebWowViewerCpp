#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

layout(location = 0) out vec4 outColor;

[shader("vertex")]
void main() {
    gl_Position = vec4(
        0.5, 0.5, 0.5, 0.5
    );

}