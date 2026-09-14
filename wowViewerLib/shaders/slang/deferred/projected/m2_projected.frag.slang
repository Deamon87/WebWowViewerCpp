#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

#define PROJECTIVE_MATERIAL

//Set 0
#include "../../common/commonUboSceneData.slang"
//Set 1
#include "../../common/commonM2DescriptorSet.slang"
#define GBUFFER_SET_INDEX 3
#include "../../common/commonGBufferData.slang"

#include "../../common/commonProjectiveConstants.slang"
#include "../../common/commonFunctions.slang"
#include "../../common/commonProjectedMaterial.slang"

layout(location = 0) in flat int vMeshIndex;

layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"


[shader("pixel")]
void main() {
    mat4 invPlaceMat4 = placementMats.invPlacementMat;

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

    textMat[0] = textMatIndex1 < 0 ? ident4() : textureMatrix[textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? ident4() : textureMatrix[textMatIndex2];


    vec3 uTexSampleAlpha = vec3(
        meshWide.textureWeightIndexes.x < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4],
        meshWide.textureWeightIndexes.y < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.y / 4][meshWide.textureWeightIndexes.y % 4],
        meshWide.textureWeightIndexes.z < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.z / 4][meshWide.textureWeightIndexes.z % 4]
    );

    vec4 vMeshColorAlpha = vec4(
        meshWide.colorIndex_applyWeight_txac1_txac2.x < 0 ?
        vec4(1.0,1.0,1.0,1.0) :
        colors[meshWide.colorIndex_applyWeight_txac1_txac2.x]
    );
    if (meshWide.colorIndex_applyWeight_txac1_txac2.y > 0)
        vMeshColorAlpha.a *=
            meshWide.textureWeightIndexes.x < 0 ?
            1.0 :
            textureWeight[meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4];

    vec3 matDiffuse;
    float finalOpacity = 0.0;
    int uPixelShader = meshWide.PixelShader_UnFogged_blendMode.x;

    calcProjectedMaterial(uPixelShader,
        uTexture, uTexture2, uTexture3,
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