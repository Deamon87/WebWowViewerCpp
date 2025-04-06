#version 450

#extension GL_GOOGLE_include_directive: require
//#extension GL_EXT_debug_printf : enable

precision highp float;
precision highp int;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0f,1.0f,1.0f,0.0);
}