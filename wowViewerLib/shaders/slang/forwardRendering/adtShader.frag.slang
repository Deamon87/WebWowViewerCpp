#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.slang"
#include "../common/commonFogFunctions.slang"
#include "../common/commonADTMaterial.slang"

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in vec3 vPosition;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec3 vVertexLighting;
layout(location = 5) in vec2 vAlphaCoords;

#include "../common/commonUboSceneData.slang"
#include "../common/commonAdtDescriptorSet.slang"

layout(location = 0) out vec4 outColor;

const InteriorLightParam intLight = {
    vec4(0,0,0,1.0),
    vec4(0,0,0,1),
    vec4(0,0,0,1),
    vec4(0,0,0,1),
    vec4(0,0,0,0)
};

[shader("pixel")]
void main() {
    const float threshold = 1.5;

    float sceneTime = scene.uViewUpSceneTime.w;

    vec2 tcLayer[8];
    for (int i = 0; i < 4; i++) {
        tcLayer[i] = transformADTUV(vTexCoords, i, sceneTime, adtMeshWidePS.animation_rotationPerLayer[0], adtMeshWidePS.animation_speedPerLayer[0], adtMeshWidePS.scaleFactorPerLayer[0]);
    }
    for (int i = 4; i < 8; i++) {
        tcLayer[i] = transformADTUV(vTexCoords, i-4, sceneTime, adtMeshWidePS.animation_rotationPerLayer[1], adtMeshWidePS.animation_speedPerLayer[1], adtMeshWidePS.scaleFactorPerLayer[1]);
    }

    float heightOffset[8] = {
        adtMeshWideVSPS.uHeightOffset[0].x, adtMeshWideVSPS.uHeightOffset[0].y, adtMeshWideVSPS.uHeightOffset[0].z, adtMeshWideVSPS.uHeightOffset[0].w,
        adtMeshWideVSPS.uHeightOffset[1].x, adtMeshWideVSPS.uHeightOffset[1].y, adtMeshWideVSPS.uHeightOffset[1].z, adtMeshWideVSPS.uHeightOffset[1].w
    };
    float heightScale[8] = {
        adtMeshWideVSPS.uHeightScale[0].x, adtMeshWideVSPS.uHeightScale[0].y, adtMeshWideVSPS.uHeightScale[0].z, adtMeshWideVSPS.uHeightScale[0].w,
        adtMeshWideVSPS.uHeightScale[1].x, adtMeshWideVSPS.uHeightScale[1].y, adtMeshWideVSPS.uHeightScale[1].z, adtMeshWideVSPS.uHeightScale[1].w
    };

    float4 layer[8];
    layer[0] = uLayer0.Sample(tcLayer[0]);
    layer[1] = uLayer1.Sample(tcLayer[1]);
    layer[2] = uLayer2.Sample(tcLayer[2]);
    layer[3] = uLayer3.Sample(tcLayer[3]);
    layer[4] = uLayer4.Sample(tcLayer[4]);
    layer[5] = uLayer5.Sample(tcLayer[5]);
    layer[6] = uLayer6.Sample(tcLayer[6]);
    layer[7] = uLayer7.Sample(tcLayer[7]);

    float4 alpha[2];
    alpha[0] = uAlphaTexture.Sample(vAlphaCoords);
    alpha[1] = uAlphaTexture2.Sample(vAlphaCoords);


    vec4 final;
    if (adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend.r > 0) {
        float4 layerHeight[8];
        layerHeight[0] = uLayerHeight0.Sample(tcLayer[0]);
        layerHeight[1] = uLayerHeight1.Sample(tcLayer[1]);
        layerHeight[2] = uLayerHeight2.Sample(tcLayer[2]);
        layerHeight[3] = uLayerHeight3.Sample(tcLayer[3]);
        layerHeight[4] = uLayerHeight4.Sample(tcLayer[4]);
        layerHeight[5] = uLayerHeight5.Sample(tcLayer[5]);
        layerHeight[6] = uLayerHeight6.Sample(tcLayer[6]);
        layerHeight[7] = uLayerHeight7.Sample(tcLayer[7]);


        calcADTHeightFragMaterial<8>(layer, layerHeight, alpha, heightOffset, heightScale, final);
    } else {
        calcADTOrigFragMaterial<8>(
            layer, alpha,
            adtMeshWideVSPS.uUseHeightMixFormula_useWeightedBlend.g > 0,
            final
        );
    }

    vec3 matDiffuse = final.rgb * 2.0 * vColor.rgb;

    vec4 finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            vPosition.xyz,
            true,
            scene,
            intLight,
            vVertexLighting.rgb, /* accumLight */
            vec3(0.0), /*precomputedLight*/
            vec3(0.0), /* specular */
            vec3(0.0), /* emissive */
            1.0 /* ao */
        ),
        1.0
    );

    //Spec part
    float specBlend = final.a;
    vec3 halfVec = -(normalize((scene.extLight.uExteriorDirectColorDir.xyz + normalize(vPosition))));
    vec3 lSpecular = ((scene.extLight.uExteriorDirectColor.xyz * pow(max(0.0, dot(halfVec, normalize(vNormal))), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular) * scene.extLight.adtSpecMult_fogCount.x;
    finalColor.rgb += specTerm;

    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz,
        vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, 0);

    finalColor.a = 1.0;
    outColor = finalColor;
}
