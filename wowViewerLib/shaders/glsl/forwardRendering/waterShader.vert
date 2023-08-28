#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

layout(location=0) in vec4 aPositionTransp;
layout(location=1) in vec2 aTexCoord;


layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData[8];
};

layout(std140, set=1, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};

//out vec2 vTexCoord;
layout(location=0) out vec3 vPosition;
layout(location=1) out vec2 vTextCoords;
layout(location=2) out vec3 vNormal;

void main() {
    vec4 aPositionVec4 = vec4(aPositionTransp.xyz, 1);
    mat4 viewModelMat = scene.uLookAtMat * uPlacementMat;

    vec4 cameraPoint = viewModelMat * aPositionVec4;

    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    //const float posToTextPos = 1.0 / (1600.0/3.0/16.0);
    vTextCoords = aTexCoord;//aPositionVec4.xy * posToTextPos;

    gl_Position = scene.uPMatrix * cameraPoint;
    //   vTexCoord = aTexCoord;
    vPosition = cameraPoint.xyz;
    vNormal = normalize(viewModelMatForNormal * vec4(vec3(0,0,1.0), 0.0)).xyz;

}
