#version 450

#extension GL_GOOGLE_include_directive: require

layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = vec4(
        0.5, 0.5, 0.5, 0.5
    );

}