#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../../../common/commonLightFunctions.glsl"
#include "../../../common/commonFogFunctions.glsl"
#include "../../../common/commonFunctions.glsl"
#include "../../../common/commonWMOMaterial.glsl"

/* vertex shader code */
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec2 aTexCoord2;
layout (location = 4) in vec2 aTexCoord3;
layout (location = 5) in vec2 aTexCoord4;
layout (location = 6) in vec4 aColor;
layout (location = 7) in vec4 aColor2;
layout (location = 8) in vec4 aColorSecond;

#include "../../../common/commonUboSceneData.glsl"
#include "../../../common/commonWMOIndirectDescriptorSet.glsl"

layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord3;
layout(location=3) out vec2 vTexCoord4;
layout(location=4) out vec4 vColor;
layout(location=5) out vec4 vColor2;
layout(location=6) out vec4 vColorSecond;
layout(location=7) out vec4 vPosition;
layout(location=8) out vec3 vNormal;
layout(location=9) out flat int vMeshIndex;


void main() {
    WMOPerMeshData perMeshData = perMeshDatas[gl_InstanceIndex];
    WmoMeshWideBindless meshWideBindless = wmoMeshWideBindlesses[perMeshData.meshWideBindlessIndex_groupNum.x];
    int placementMatInd = meshWideBindless.placementMat_meshWideIndex_blockVSIndex_texture9.x;
    int blockVSIndex = meshWideBindless.placementMat_meshWideIndex_blockVSIndex_texture9.y;

    mat4 placementMat = uPlacementMats[placementMatInd];
    vec4 worldPoint = placementMat * vec4(aPosition, 1);

    vec4 cameraPoint = scene.uLookAtMat * worldPoint;

    mat4 viewModelMat = scene.uLookAtMat * placementMat;
    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    gl_Position = scene.uPMatrix * cameraPoint;
    vPosition = vec4(cameraPoint.xyz, 0);
    vNormal = normalize((viewModelMatForNormal * vec4(aNormal, 0.0)).xyz);

    vColor = aColor.bgra;
    vColor2 = aColor2;
    vColorSecond = aColorSecond;
    vTexCoord4 = aTexCoord4;

    WmoVertMeshWide meshWide = wmoVertMeshWide[blockVSIndex];

    int uVertexShader = meshWide.VertexShader_UseLitColors.x;

    calcWMOVertMat(uVertexShader,
        vPosition.xyz, vNormal,
        aTexCoord, aTexCoord2, aTexCoord3,
        scene.uViewUpSceneTime.w,
        meshWide.translationSpeedXY,
        vTexCoord, vTexCoord2, vTexCoord3
    );

    vMeshIndex = gl_InstanceIndex;
}
