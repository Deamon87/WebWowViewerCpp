#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

layout(std140, set=1, binding=1) uniform AABBDebugColorBlock {
    vec4 uColor;
};

layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"

[shader("pixel")]
void main() {
    outColor = uColor;
    writeObjectId(0u);
}