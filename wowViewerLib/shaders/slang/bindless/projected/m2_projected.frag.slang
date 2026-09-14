#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

//Set 0
#include "../../common/commonUboSceneData.slang"
//Set 1
#include "../../common/commonM2IndirectDescriptorSet.slang"
#define GBUFFER_SET_INDEX 3
#include "../../common/commonGBufferData.slang"

#include "../../common/commonProjectiveConstants.slang"
#include "../../common/commonFunctions.slang"
#include "../../common/commonProjectedMaterial.slang"


layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

layout(location = 0) in flat int vMeshIndex;

layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"


[shader("pixel")]
void main() {
    meshWideBlockVSPSBindless meshWideBindless = meshWideBindleses[vMeshIndex];
    meshWideBlockVSPS meshWide = meshWides[nonuniformEXT(meshWideBindless.instanceIndex_meshIndex.y)];

    ProjectiveData projectiveData = projectiveData[nonuniformEXT(meshWideBindless.instanceIndex_meshIndex.z)];

    int instanceIndex = meshWideBindless.instanceIndex_meshIndex.x;

    M2InstanceRecordBindless m2Instance = instances[nonuniformEXT(instanceIndex)];

    modelWideBlockPSStruct modelWide = modelWides[m2Instance.textureMatricesInd_modelFragmentDatasInd.y];

    int placementMatrixInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    int textureWeightsInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.w;
    int m2ColorsInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.z;
    int textureMatricesInd = m2Instance.textureMatricesInd_modelFragmentDatasInd.x;

    int placementIndex = instances[nonuniformEXT(instanceIndex)].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    mat4 invPlaceMat4 = placementMats[nonuniformEXT(placementIndex)].invPlacementMat;

    // --------------------------------
    // Read from GBuffer
    // --------------------------------

    vec3 viewPos = gbuff_getViewPos(gl_FragCoord.xy, scene.uInvPMatrix);
    vec4 worldPos = scene.uInvLookAtMat * vec4(viewPos, 1.0);

    vec4 localPoint = invPlaceMat4 * worldPos;
    if (
        any(lessThan(localPoint.xyz, projectiveData.localMin.xyz - vec3(0,0,PROJECTIVE_BOUNDING_BOX_BOTTOM))) ||
        any(greaterThan(localPoint.xyz, projectiveData.localMax.xyz + vec3(0,0,PROJECTIVE_BOUNDING_BOX_UP)))
    ) {
        discard;
    }

    vec2 texCoordinates = (projectiveData.localToUVMat * localPoint).xy;

    if (texCoordinates.x < 0 || texCoordinates.x > 1 ||
        texCoordinates.y < 0 || texCoordinates.y > 1) discard;
//
    //Calculate material based on pixel shader
    mat4 textMat[2];
    int textMatIndex1 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.z;
    int textMatIndex2 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.w;

    textMat[0] = textMatIndex1 < 0 ? ident4() : textureMatrix[textureMatricesInd + textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? ident4() : textureMatrix[textureMatricesInd + textMatIndex2];


    vec3 uTexSampleAlpha = vec3(
        meshWide.textureWeightIndexes.x < 0 ? 1.0 : textureWeight[textureWeightsInd+meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4],
        meshWide.textureWeightIndexes.y < 0 ? 1.0 : textureWeight[textureWeightsInd+meshWide.textureWeightIndexes.y / 4][meshWide.textureWeightIndexes.y % 4],
        meshWide.textureWeightIndexes.z < 0 ? 1.0 : textureWeight[textureWeightsInd+meshWide.textureWeightIndexes.z / 4][meshWide.textureWeightIndexes.z % 4]
    );

    vec4 vMeshColorAlpha = vec4(
        meshWide.colorIndex_applyWeight_txac1_txac2.x < 0 ?
        vec4(1.0,1.0,1.0,1.0) :
        colors[m2ColorsInd + meshWide.colorIndex_applyWeight_txac1_txac2.x]
    );
    if (meshWide.colorIndex_applyWeight_txac1_txac2.y > 0)
        vMeshColorAlpha.a *=
            meshWide.textureWeightIndexes.x < 0 ?
            1.0 :
            textureWeight[textureWeightsInd + meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4];

    vec3 matDiffuse;
    float finalOpacity = 0.0;
    int uPixelShader = meshWide.PixelShader_UnFogged_blendMode.x;

    calcProjectedMaterial(uPixelShader,
        s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.x)],
        s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.y)],
        s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.z)],
        texCoordinates,
        textMat[0], textMat[1],
        viewPos.xy,
        matDiffuse, finalOpacity
    );

//        matDiffuse *= vMeshColorAlpha.rgb;
    finalOpacity *= vMeshColorAlpha.a;

    //Calculate lighting
//    vec3 l_Normal = vNormal;



    outColor = vec4(matDiffuse.rgb, finalOpacity);
    writeObjectId(0x80000000u | modelWide.objectId_pad.x);
}