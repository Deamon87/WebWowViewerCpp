precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"

#include "../../common/commonADTMaterial.glsl"
#include "../../common/commonUboSceneData.glsl"
#include "../../common/commonAdtIndirectDescriptorSet.glsl"

//ADT is always opaque
layout(early_fragment_tests) in;

layout(location = 0) in vec2 vChunkCoords;
layout(location = 1) in vec3 vPosition;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec3 vVertexLighting;
layout(location = 5) in vec2 vAlphaCoords;
layout(location = 6) in flat int meshIndex;

layout (set = 2, binding = 0) uniform sampler2D s_LayerTextures[];
layout (set = 3, binding = 0) uniform sampler2D s_AlphaTextures[];
layout (set = 4, binding = 0) uniform sampler2D s_LayerHeightTextures[];

#include "../../../common/commonUboSceneData.glsl"

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#else
#include "../deferred_excerpt.glsl"
#endif

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

void main() {
    AdtInstanceData adtInstanceData = adtInstanceDatas[nonuniformEXT(meshIndex)];
    AdtMeshWideVSPS adtMeshWideVSPS = adtMeshWideVSPSes[nonuniformEXT(adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd.x)];
    AdtMeshWidePS adtMeshWidePS = adtMeshWidePSes[nonuniformEXT(adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd.y)];


    vec2 vTexCoord = vChunkCoords;
    const float threshold = 1.5;

    ivec4 animation_rotationPerLayer = adtMeshWidePS.animation_rotationPerLayer;
    ivec4 animation_speedPerLayer = adtMeshWidePS.animation_speedPerLayer;
    vec4 scaleFactorPerLayer = adtMeshWidePS.scaleFactorPerLayer;
    float sceneTime = scene.uViewUpSceneTime.w;

    vec2 tcLayer0 = transformADTUV(vTexCoord, 0, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);
    vec2 tcLayer1 = transformADTUV(vTexCoord, 1, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);
    vec2 tcLayer2 = transformADTUV(vTexCoord, 2, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);
    vec2 tcLayer3 = transformADTUV(vTexCoord, 3, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);

    int layerHeight0 = adtInstanceData.LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind.x;
    int layerHeight1 = adtInstanceData.LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind.y;
    int layerHeight2 = adtInstanceData.LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind.z;
    int layerHeight3 = adtInstanceData.LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind.w;

    int txLayer0 = adtInstanceData.Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind.x;
    int txLayer1 = adtInstanceData.Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind.y;
    int txLayer2 = adtInstanceData.Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind.z;
    int txLayer3 = adtInstanceData.Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind.w;

    vec4 heightScale = adtMeshWideVSPS.uHeightScale;
    vec4 heightOffset = adtMeshWideVSPS.uHeightOffset;

    vec4 final;
    if (adtMeshWideVSPS.uUseHeightMixFormula.r > 0) {
        calcADTHeightFragMaterial(
            tcLayer0, tcLayer1, tcLayer2, tcLayer3,
            s_LayerTextures[nonuniformEXT(txLayer0)], s_LayerTextures[nonuniformEXT(txLayer1)],
            s_LayerTextures[nonuniformEXT(txLayer2)], s_LayerTextures[nonuniformEXT(txLayer3)],
            s_LayerHeightTextures[nonuniformEXT(layerHeight0)], s_LayerHeightTextures[nonuniformEXT(layerHeight1)],
            s_LayerHeightTextures[nonuniformEXT(layerHeight2)], s_LayerHeightTextures[nonuniformEXT(layerHeight3)],
            vAlphaCoords,
            s_AlphaTextures[nonuniformEXT(adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd.z)],
            heightOffset, heightScale,
            final
        );
    } else {
        calcADTOrigFragMaterial(
            tcLayer0, tcLayer1, tcLayer2, tcLayer3,
            s_LayerTextures[nonuniformEXT(txLayer0)], s_LayerTextures[nonuniformEXT(txLayer1)],
            s_LayerTextures[nonuniformEXT(txLayer2)], s_LayerTextures[nonuniformEXT(txLayer3)],
            vAlphaCoords,
            s_AlphaTextures[nonuniformEXT(adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd.z)],
            final
        );
    }

    vec3 matDiffuse = final.rgb * 2.0 * vColor.rgb;

    vec4 finalColor = vec4(matDiffuse, 1.0);

#ifndef DEFERRED

    vec3 accumLight = vVertexLighting.rgb;
    float ao = 0.0;
    {
        vec2 screenUV = (gl_FragCoord.xy / scene.uSceneSize_DisableLightBuffer.xy);
        if (scene.uSceneSize_DisableLightBuffer.z == 0.0) {
            accumLight = texture(lightBuffer, screenUV).xyz;
        }
        ao = texture(aoBuffer, screenUV).x;
    }

    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            true,
            0.0,
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
#else
    writeGBuffer(matDiffuse.xyz, normalize(vNormal), specTerm.rgb, vPosition.xyz);
#endif
}
