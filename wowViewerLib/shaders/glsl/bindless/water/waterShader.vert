#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier: require

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"
#include "../../common/commonUboSceneData.glsl"

layout(location=0) in vec4 aPositionTransp;
layout(location=1) in vec2 aTexCoord;

#include "../../common/commonWaterIndirect.glsl"

//out vec2 vTexCoord;
layout(location=0) out vec3 vPosition;
layout(location=1) out vec2 vTextCoords;
layout(location=2) out vec3 vNormal;
layout(location=3) out flat int meshInd;

void main() {
    WaterBindless waterBindless = waterBindlesses[gl_InstanceIndex];

    vec4 aPositionVec4 = vec4(aPositionTransp.xyz, 1);
    mat4 viewModelMat = scene.uLookAtMat * uPlacementMats[waterBindless.waterDataInd_placementMatInd_textureInd.y];

    vec4 cameraPoint = viewModelMat * aPositionVec4;

    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    //const float posToTextPos = 1.0 / (1600.0/3.0/16.0);
    vTextCoords = aTexCoord;//aPositionVec4.xy * posToTextPos;

    gl_Position = scene.uPMatrix * cameraPoint;
    //   vTexCoord = aTexCoord;
    vPosition = cameraPoint.xyz;
    vNormal = normalize(viewModelMatForNormal * vec4(vec3(0,0,1.0), 0.0)).xyz;

    meshInd = gl_InstanceIndex;
}
