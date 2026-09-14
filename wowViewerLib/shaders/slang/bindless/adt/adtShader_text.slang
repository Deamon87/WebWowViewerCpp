import glsl;

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"

#include "../../common/commonADTMaterial.slang"
#include "../../common/commonUboSceneData.slang"
#include "../../common/commonAdtIndirectDescriptorSet.slang"


layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vPosition;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec3 vVertexLighting;
layout(location = 5) in vec2 vAlphaCoords;
layout(location = 6) in flat int meshIndex;

#include "../../common/commonUboSceneData.slang"

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"
#else
#include "../deferred_excerpt.slang"
#endif

const InteriorLightParam intLight = {
    vec4(0,0,0,1.0f),
    vec4(0,0,0,0),
    vec4(0,0,0,0),
    vec4(0,0,0,1),
    vec4(0,0,0,0)
};

//ADT is always opaque
[earlydepthstencil]
[shader("pixel")]
void main() {
    AdtInstanceData adtInstanceData = adtInstanceDatas[nonuniformEXT(meshIndex)];
    AdtMeshWideVSPS adtMeshWideVSPS = adtMeshWideVSPSes[nonuniformEXT(adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd_AlphaTextureInd2.x)];
    AdtMeshWidePS adtMeshWidePS = adtMeshWidePSes[nonuniformEXT(adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd_AlphaTextureInd2.y)];

    const float threshold = 1.5;

    ivec4 animation_rotationPerLayer[2] = adtMeshWidePS.animation_rotationPerLayer;
    ivec4 animation_speedPerLayer[2] = adtMeshWidePS.animation_speedPerLayer;
    vec4 scaleFactorPerLayer[2] = adtMeshWidePS.scaleFactorPerLayer;
    float sceneTime = scene.uViewUpSceneTime.w;

    vec2 tcLayer[8];
    for (int i = 0; i < 4; i++) {
        tcLayer[i] = transformADTUV(vTexCoord, i, sceneTime, animation_rotationPerLayer[0], animation_speedPerLayer[0], scaleFactorPerLayer[0]);
    }
    for (int i = 4; i < 8; i++) {
        tcLayer[i] = transformADTUV(vTexCoord, i-4, sceneTime, animation_rotationPerLayer[1], animation_speedPerLayer[1], scaleFactorPerLayer[1]);
    }

    vec4 uHeightScale[2] = adtMeshWideVSPS.uHeightScale;
    vec4 uHeightOffset[2] = adtMeshWideVSPS.uHeightOffset;

    float heightOffset[8] = {
        uHeightOffset[0].x, uHeightOffset[0].y, uHeightOffset[0].z, uHeightOffset[0].w,
        uHeightOffset[1].x, uHeightOffset[1].y, uHeightOffset[1].z, uHeightOffset[1].w
    };
    float heightScale[8] = {
        uHeightScale[0].x, uHeightScale[0].y, uHeightScale[0].z, uHeightScale[0].w,
        uHeightScale[1].x, uHeightScale[1].y, uHeightScale[1].z, uHeightScale[1].w
    };

    ivec4 layerHeightIndex0 = adtInstanceData.LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind;
    ivec4 layerHeightIndex1 = adtInstanceData.LayerHeight4Ind_LayerHeight5Ind_LayerHeight6Ind_LayerHeight7Ind;

    ivec4 txLayerIndex0 = adtInstanceData.Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind;
    ivec4 txLayerIndex1 = adtInstanceData.Layer4Ind_Layer5Ind_Layer6Ind_Layer7Ind;

    int txLayerIdxs[8] = {
        txLayerIndex0.x, txLayerIndex0.y, txLayerIndex0.z, txLayerIndex0.w,
        txLayerIndex1.x, txLayerIndex1.y, txLayerIndex1.z, txLayerIndex1.w
    };

    int txLayerHeightIdxs[8] = {
        layerHeightIndex0.x, layerHeightIndex0.y, layerHeightIndex0.z, layerHeightIndex0.w,
        layerHeightIndex1.x, layerHeightIndex1.y, layerHeightIndex1.z, layerHeightIndex1.w
    };

    float4 layer[8];
    for (int i = 0; i < 8; i++) {
        layer[i] = s_LayerTextures[nonuniformEXT(txLayerIdxs[i])].Sample(tcLayer[i]);
    }

    float4 alpha[2];

    int alphaTexInd[2] = {
        adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd_AlphaTextureInd2.z,
        adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd_AlphaTextureInd2.w
    };

    alpha[0] = s_AlphaTextures[nonuniformEXT(alphaTexInd[0])].Sample(vAlphaCoords);
    alpha[1] = s_AlphaTextures[nonuniformEXT(alphaTexInd[1])].Sample(vAlphaCoords);

    vec4 final;
    const bool useHeightMixFormula = adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend_maxLayersCount.r > 0;
    const bool useWeightedBlend = adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend_maxLayersCount.g > 0;
    int adtMaxLayers = adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend_maxLayersCount.b;
    if (useHeightMixFormula > 0) {
        float4 layerHeight[8];
        for (int i = 0; i < 8; i++) {
            layerHeight[i] = s_LayerHeightTextures[nonuniformEXT(txLayerHeightIdxs[i])].Sample(tcLayer[i]);
        }

        if (adtMaxLayers > 4) {
            calcADTHeightFragMaterial<8>(layer, layerHeight, alpha, heightOffset, heightScale, final);
        } else {
            calcADTHeightFragMaterial<4>(layer, layerHeight, alpha, heightOffset, heightScale, final);
        }
    } else {
        if (adtMaxLayers > 4) {
            calcADTOrigFragMaterial<8>(layer, alpha, useWeightedBlend, final);
        } else {
            calcADTOrigFragMaterial<4>(layer, alpha, useWeightedBlend, final);
        }
    }

    vec3 matDiffuse = final.rgb * 2.0 * vColor.rgb;

    vec4 finalColor = vec4(matDiffuse, 1.0);

#ifndef DEFERRED

    vec3 accumLight = vec3(0.0);//vVertexLighting.rgb;
    float ao = 0.0;
    {
        vec2 screenUV = (gl_FragCoord.xy / scene.uSceneSize_DisableLightBuffer.xy);
        if (scene.uSceneSize_DisableLightBuffer.z == 0.0) {
            accumLight = texture(lightBuffer, screenUV).xyz;
        }
        ao = texture(aoBuffer, screenUV).x;
    }
//    matDiffuse.rbg = vec3(0, 0, 1);


    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            vPosition.xyz,
            true,
            scene,
            intLight,
            accumLight, /* accumLight */
            vec3(0.0), /*precomputedLight*/
            vec3(0.0), /* specular */
            vec3(0.0), /* emissive */
            ao /* ambient occlusion */
        ),
        1.0
    );
#endif

    //Spec part
    float specBlend = final.a;
    vec3 halfVec = -(normalize((scene.extLight.uExteriorDirectColorDir.xyz + normalize(vPosition))));
    vec3 lSpecular = ((scene.extLight.uExteriorDirectColor.xyz * pow(max(0.0, dot(halfVec, normalize(vNormal))), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular) * scene.extLight.adtSpecMult_fogCount.x;
#ifndef DEFERRED
    finalColor.rgb += specTerm;
#endif

    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz,
                          vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, 0);


    finalColor.a = 1.0;


#ifndef DEFERRED
    outColor = finalColor;
    writeObjectId(0u);
#else
    writeGBuffer(matDiffuse.xyz, normalize(vNormal), specTerm.rgb, vPosition.xyz);
#endif
}
