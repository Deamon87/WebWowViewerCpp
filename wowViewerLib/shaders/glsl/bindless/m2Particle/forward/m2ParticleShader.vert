#version 450
#extension GL_GOOGLE_include_directive: require


precision highp float;
precision highp int;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexcoord0;
layout(location = 3) in vec2 aTexcoord1;
layout(location = 4) in vec2 aTexcoord2;
layout(location = 5) in float aAlphaCutoff;


layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec4 vColor;
layout(location = 2) out vec2 vTexcoord0;
layout(location = 3) out vec2 vTexcoord1;
layout(location = 4) out vec2 vTexcoord2;
layout(location = 5) out float vAlphaCutoff;

#include "../../../common/commonLightFunctions.glsl"
#include "../../../common/commonFogFunctions.glsl"

#include "../../../common/commonUboSceneData.glsl"


void main() {
    vec4 aPositionVec4 = vec4(aPosition, 1);

    vColor = aColor;
    vTexcoord0 = aTexcoord0;
    vTexcoord1 = aTexcoord1;
    vTexcoord2 = aTexcoord2;

    vec4 vertexViewSpace = (scene.uLookAtMat * aPositionVec4);

    vPosition = vertexViewSpace.xyz;
    vAlphaCutoff = aAlphaCutoff;

    gl_Position = scene.uPMatrix * vertexViewSpace;
}