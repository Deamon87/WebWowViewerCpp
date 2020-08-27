#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexcoord0;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec4 vColor;
layout(location = 2) out vec2 vTexcoord0;

void main() {
    vec4 aPositionVec4 = vec4(aPosition, 1);

    vColor = aColor;
    vTexcoord0 = aTexcoord0;
    vec4 vertexViewSpace = (scene.uLookAtMat * aPositionVec4);

    vPosition = vertexViewSpace.xyz;
    gl_Position = scene.uPMatrix * vertexViewSpace;

}
