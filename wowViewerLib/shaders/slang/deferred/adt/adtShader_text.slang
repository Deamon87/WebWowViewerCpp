import glsl;

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"

#include "../../common/commonADTMaterial.slang"
#include "../../common/commonUboSceneData.slang"
#include "../../common/commonAdtDescriptorSet.slang"

layout(constant_id = 0) const int adtMaxLayers = 4;

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

    float4 layer[8];
    layer[0] = uLayer0.Sample(tcLayer[0]);
    layer[1] = uLayer1.Sample(tcLayer[1]);
    layer[2] = uLayer2.Sample(tcLayer[2]);
    layer[3] = uLayer3.Sample(tcLayer[3]);
    if (adtMaxLayers > 4) {
        layer[4] = uLayer4.Sample(tcLayer[4]);
        layer[5] = uLayer5.Sample(tcLayer[5]);
        layer[6] = uLayer6.Sample(tcLayer[6]);
        layer[7] = uLayer7.Sample(tcLayer[7]);
    }

    float4 alpha[2];
    alpha[0] = uAlphaTexture.Sample(vAlphaCoords);
    if (adtMaxLayers > 4) {
        alpha[1] = uAlphaTexture2.Sample(vAlphaCoords);
    }

    vec4 final;
    const bool useHeightMixFormula = adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend.r > 0;
    const bool useWeightedBlend = adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend.g > 0;
    if (useHeightMixFormula) {

        float4 layerHeight[8];
        layerHeight[0] = uLayerHeight0.Sample(tcLayer[0]);
        layerHeight[1] = uLayerHeight1.Sample(tcLayer[1]);
        layerHeight[2] = uLayerHeight2.Sample(tcLayer[2]);
        layerHeight[3] = uLayerHeight3.Sample(tcLayer[3]);
        if (adtMaxLayers > 4) {
            layerHeight[4] = uLayerHeight4.Sample(tcLayer[4]);
            layerHeight[5] = uLayerHeight5.Sample(tcLayer[5]);
            layerHeight[6] = uLayerHeight6.Sample(tcLayer[6]);
            layerHeight[7] = uLayerHeight7.Sample(tcLayer[7]);
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
