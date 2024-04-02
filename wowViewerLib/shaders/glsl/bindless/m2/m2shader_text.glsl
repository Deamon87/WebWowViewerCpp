precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"
#include "../../common/commonM2Material.glsl"

#ifdef TRUE_OPAQUE
layout(early_fragment_tests) in;
#endif
layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec3 vNormal;
layout(location=3) in vec4 vPosition_EdgeFade;
layout(location=4) in flat int vMeshIndex;

#include "../../common/commonUboSceneData.glsl"

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#else
#include "../deferred_excerpt.glsl"
#endif

#include "../../common/commonUboSceneData.glsl"
//Whole model
#include "../../common/commonM2IndirectDescriptorSet.glsl"

layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord2.xy;

    vec4 finalColor = vec4(0);

    meshWideBlockVSPSBindless meshWideBindless = meshWideBindleses[vMeshIndex];
    meshWideBlockVSPS meshWide = meshWides[nonuniformEXT(meshWideBindless.instanceIndex_meshIndex.y)];

    int instanceIndex = meshWideBindless.instanceIndex_meshIndex.x;
    M2InstanceRecordBindless m2Instance = instances[nonuniformEXT(instanceIndex)];

    modelWideBlockPSStruct modelWide = modelWides[m2Instance.textureMatricesInd_modelFragmentDatasInd.y];

    bool calcMaterial = true;
#ifdef DEFERRED
//    {
//        int blendMode = meshWide.PixelShader_UnFogged_blendMode.z;
//        calcMaterial = (blendMode == 1);
//    }
#endif


    int placementMatrixInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    int textureWeightsInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.w;
    int m2ColorsInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.z;
    int textureMatricesInd = m2Instance.textureMatricesInd_modelFragmentDatasInd.x;

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

    vec3 l_Normal = vNormal;

    //Accumulate and apply lighting

    vec3 meshResColor = vMeshColorAlpha.rgb;

    //----------------------
    // Calc Diffuse and Specular
    //---------------------
    int uVertexShader = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.x;

    mat4 textMat[2];
    int textMatIndex1 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.z;
    int textMatIndex2 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.w;

    textMat[0] = textMatIndex1 < 0 ? mat4(1.0) : textureMatrix[textureMatricesInd + textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? mat4(1.0) : textureMatrix[textureMatricesInd + textMatIndex2];
    float edgeFade = 1.0;

    calcM2VertexMat(uVertexShader,
        vPosition_EdgeFade.xyz, l_Normal,
        vTexCoord, vTexCoord2,
        textMat, edgeFade,
        texCoord, texCoord2, texCoord3);

    vMeshColorAlpha *= edgeFade;

    float finalOpacity = 0.0;
    vec3 matDiffuse;
    vec3 specular;

    int uPixelShader = meshWide.PixelShader_UnFogged_blendMode.x;
    int blendMode = meshWide.PixelShader_UnFogged_blendMode.z;

    bool doDiscard = false;

    if (calcMaterial) {
        calcM2FragMaterial(uPixelShader,
            s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.x)], s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.y)],
            s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.z)], s_Textures[nonuniformEXT(meshWideBindless.textureIndicies.w)],
            texCoord, texCoord2, texCoord3,
            vMeshColorAlpha.rgb, vMeshColorAlpha.a,
            uTexSampleAlpha.rgb,
            blendMode,
            matDiffuse, specular, finalOpacity, doDiscard
        );

#ifndef TRUE_OPAQUE
        if (doDiscard)
            discard;
#endif

        // ------------------------------
        // Apply lighting
        // ------------------------------


        specular *= vMeshColorAlpha.rgb;

#ifndef DEFERRED
        vec3 accumLight = vec3(0.0);
        if (scene.uSceneSize_DisableLightBuffer.z == 0.0 && blendMode <= 1) {
            accumLight = texture(lightBuffer, (gl_FragCoord.xy / scene.uSceneSize_DisableLightBuffer.xy)).xyz;
        }

        finalColor = vec4(
            calcLight(
                matDiffuse,
                l_Normal,
                meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y > 0,
                modelWide.interiorExteriorBlend.x,
                scene,
                modelWide.intLight,
                accumLight,
                vec3(0.0),
                specular,
                vec3(0.0)
            ) ,
            finalOpacity
        );
#endif
    }

    // ------------------------------
    // Apply Fog
    // ------------------------------

#ifndef DEFERRED
    int uUnFogged = meshWide.PixelShader_UnFogged_blendMode.y;
    if (uUnFogged == 0) {
        vec3 sunDir =
            mix(
                scene.uInteriorSunDir,
                scene.extLight.uExteriorDirectColorDir,
                modelWide.interiorExteriorBlend.x
            ).xyz;

        finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/,
            finalColor, scene.uViewUpSceneTime.xyz, vPosition_EdgeFade.xyz, sunDir.xyz,
            meshWide.PixelShader_UnFogged_blendMode.z
        );
    }
#endif

    //Forward rendering without lights
#ifndef DEFERRED
    outColor = finalColor;
#else
    writeGBuffer(matDiffuse.xyz, normalize(vNormal), specular.rgb, vPosition_EdgeFade.xyz);
#endif
}