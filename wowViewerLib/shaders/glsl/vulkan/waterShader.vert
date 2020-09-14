#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

layout(location=0) in vec4 aPositionTransp;
layout(location=1) in vec2 aTexCoord;


layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

layout(std140, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};

//out vec2 vTexCoord;
layout(location=0) out vec3 vPosition;
layout(location=1) out vec2 vTextCoords;

void main() {
    vec4 aPositionVec4 = vec4(aPositionTransp.xyz, 1);
    mat4 cameraMatrix = scene.uLookAtMat * uPlacementMat;

    vec4 cameraPoint = cameraMatrix * aPositionVec4;

    const float posToTextPos = 1.0 / (1600.0/3.0/16.0);
    vTextCoords = aPositionVec4.xy * posToTextPos;

    gl_Position = scene.uPMatrix * cameraPoint;
    //   vTexCoord = aTexCoord;
    vPosition = cameraPoint.xyz;

}
