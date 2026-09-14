#version 450

import glsl;

precision highp float;
precision highp int;

//Individual mesh
layout(std140, set=1, binding=0) uniform meshWideBlockPS {
    vec4 uColor;
};

layout(location = 0) out vec4 fragColor;
#include "../common/commonObjectId.slang"

[shader("pixel")]
void main() {
    vec4 finalColor = uColor ;

    fragColor = finalColor;
    writeObjectId(0u);
}
