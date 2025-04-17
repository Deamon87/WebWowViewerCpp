#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

//Set 0
#include "../../common/commonUboSceneData.glsl"
//Set 1
#include "../../common/commonM2IndirectDescriptorSet.glsl"
#define GBUFFER_SET_INDEX 3
#include "../../common/commonGBufferData.glsl"

#include "../../common/commonProjectiveConstants.glsl"


layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

layout(location = 0) in flat int vMeshIndex;

layout(location = 0) out vec4 outColor;


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

    textMat[0] = textMatIndex1 < 0 ? mat4(1.0) : textureMatrix[textureMatricesInd + textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? mat4(1.0) : textureMatrix[textureMatricesInd + textMatIndex2];


    vec3 uTexSampleAlpha = vec3(
        meshWide.textureWeightIndexes.x < 0 ? 1.0 : textureWeight[textureWeightsInd+meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4],
        meshWide.textureWeightIndexes.y < 0 ? 1.0 : textureWeight[textureWeightsInd+meshWide.textureWeightIndexes.y / 4][meshWide.textureWeightIndexes.y % 4],
        meshWide.textureWeightIndexes.z < 0 ? 1.0 : textureWeight[textureWeightsInd+meshWide.textureWeightIndexes.z / 4][meshWide.textureWeightIndexes.z % 4]
    );

    vec4 vMeshColorAlpha = vec4(
        meshWide.colorIndex_applyWeight.x < 0 ?
        vec4(1.0,1.0,1.0,1.0) :
        colors[m2ColorsInd + meshWide.colorIndex_applyWeight.x]
    );
    if (meshWide.colorIndex_applyWeight.y > 0)
        vMeshColorAlpha.a *=
            meshWide.textureWeightIndexes.x < 0 ?
            1.0 :
            textureWeight[textureWeightsInd + meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4];

    vec3 matDiffuse;
    float finalOpacity = 0.0;
    int uPixelShader = meshWide.PixelShader_UnFogged_blendMode.x;

    {
        switch(uPixelShader) {
            case 0 : //Opaque_Single_Texture
            {
                vec4 tex0 = texture(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.x)], texCoordinates);
//                float lowerHeightFade_1005 = (6.0 * in_tc1.x);
//                float upperHeightFade_1006 = (6.0 - lowerHeightFade_1005);
//                float heightFade_1008 = clamp(min(lowerHeightFade_1005, upperHeightFade_1006), 0, 1);
                vec3 matDiffuse = tex0.xyz;
                finalOpacity = tex0.a;
            }
            break;
            case 1 : //Two_Texture
            {
                vec2 tc0 = (textMat[0] * vec4(texCoordinates.xy, 0, 1)).xy;
                vec2 tc1 = (textMat[1] * vec4(texCoordinates.xy, 0, 1)).xy;

                vec4 tex0 = textureGrad(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.x)], tc0, dFdx(viewPos.xy), dFdy(viewPos.xy));
                vec4 tex1 = textureGrad(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.y)], tc1, dFdx(viewPos.xy), dFdy(viewPos.xy));
//                float lowerHeightFade_1005 = (6.0 * in_tc1.x);
//                float upperHeightFade_1006 = (6.0 - lowerHeightFade_1005);
//                float heightFade_1008 = clamp(min(lowerHeightFade_1005, upperHeightFade_1006), 0, 1);
                vec4 textureMod = tex0 * tex1;
                matDiffuse = 2 * textureMod.xyz;
                finalOpacity = 2 * textureMod.a;
            }
            break;
            case 2 : //Three_Texture
                        {
                vec2 tc0 = (textMat[0] * vec4(texCoordinates.xy, 0, 1)).xy;
                vec2 tc1 = (textMat[1] * vec4(texCoordinates.xy, 0, 1)).xy;
                vec2 tc2 = texCoordinates.xy;

                vec4 tex0 = textureGrad(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.x)], tc0, dFdx(viewPos.xy), dFdy(viewPos.xy));
                vec4 tex1 = textureGrad(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.y)], tc1, dFdx(viewPos.xy), dFdy(viewPos.xy));
                vec4 tex2 = texture(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.z)], tc2);
//                float lowerHeightFade_1005 = (6.0 * in_tc1.x);
//                float upperHeightFade_1006 = (6.0 - lowerHeightFade_1005);
//                float heightFade_1008 = clamp(min(lowerHeightFade_1005, upperHeightFade_1006), 0, 1);
                vec4 textureMod = tex0 * tex1 * tex2;
                matDiffuse = 4 * textureMod.xyz;
                finalOpacity = 4 * textureMod.a;
            }
            break;
            case 4 : //Add_Single_Texture
            {
                vec4 tex0 = texture(s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.x)], texCoordinates);
//                float lowerHeightFade_1005 = (6.0 * in_tc1.x);
//                float upperHeightFade_1006 = (6.0 - lowerHeightFade_1005);
//                float heightFade_1008 = clamp(min(lowerHeightFade_1005, upperHeightFade_1006), 0, 1);
                matDiffuse = tex0.xyz;
                finalOpacity = tex0.a;
            }
            break;
        }

//        matDiffuse *= vMeshColorAlpha.rgb;
        finalOpacity *= vMeshColorAlpha.a;
    }

    //Calculate lighting
//    vec3 l_Normal = vNormal;



    outColor = vec4(matDiffuse.rgb, finalOpacity);
}