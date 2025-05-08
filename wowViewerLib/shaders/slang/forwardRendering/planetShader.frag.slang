#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonUboSceneData.slang"

layout(set=2, binding=0) uniform sampler2D uTexture;

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 outputColor;
#include "../common/commonObjectId.slang"

[shader("pixel")]
void main() {
    vec4 texColor = texture(uTexture, vTexCoord);

    outputColor = vec4(texColor.xyz * vColor.xyz, texColor.w * vColor.w);
    writeObjectId(0u);
}
