#version 450

#define FORWARD_RENDERING

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier: require

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"

layout(location=0) in vec4 vPositionDepth;
layout(location=1) in vec3 vWorldPosition;
layout(location=2) in vec2 vTextCoords;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec2 vCellUv;
layout(location=5) in flat int meshInd;

layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"

#include "../../common/commonUboSceneData.slang"
// gbuffer (depthTex/screenData/liquidBackBuffer) is bound at set=3 for liquid draws
#define GBUFFER_SET_INDEX 3
#include "../../common/commonLiquidIndirectDescriptorSet.slang"

// All per-type material functions (unique names, include-guarded)
#include "../../common/liquid/liquidWaterMat.slang"
#include "../../common/liquid/liquidMagmaMat.slang"
#include "../../common/liquid/liquidMercuryMat.slang"
#include "../../common/liquid/liquidFogMat.slang"
#include "../../common/liquid/liquidLeyLineMat.slang"
#include "../../common/liquid/liquidFelMat.slang"
#include "../../common/liquid/liquidSwampMat.slang"
#include "../../common/liquid/liquidAzeriteMat.slang"

// Bindless counterpart of deferred/liquids/liquidShader_text.slang: instead of per-type
// #define dispatch against UBOs and fixed samplers, everything is fetched per instance
// from the SSBO arrays + s_Textures using the LiquidBindless record.
[shader("pixel")]
void main() {
    LiquidBindless liquidBindless = liquidBindlesses[meshInd];
    LiquidInstanceData liquidInstanceData = liquidInstanceDatas[meshInd];
    int dataInd = liquidBindless.liquidDataInd_placementMatInd_liquidMaterialType.x;

    vec3 matDiffuse = vec3(1.0, 0.0, 1.0);
    float matAlpha = 1.0;

    switch (liquidBindless.liquidDataInd_placementMatInd_liquidMaterialType.z) {
    case LIQUID_MAT_WATER: {
        vec4 waterResult = calcWaterLiquidMat(
            liquidInstanceData,
            waterDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 2))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 3))]
        );
        matDiffuse = waterResult.rgb;
        matAlpha = waterResult.a;
        break;
    }
    case LIQUID_MAT_MAGMA: {
        vec4 magmaResult = calcMagmaLiquidMat(
            liquidInstanceData,
            magmaDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vPositionDepth.xyz, //position (fog)
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 1))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 2))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 3))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 6))]
        );
        matDiffuse = magmaResult.rgb;
        matAlpha = magmaResult.a;
        break;
    }
    case LIQUID_MAT_MERCURY: {
        vec4 mercuryResult = calcMercuryLiquidMat(
            liquidInstanceData,
            mercuryDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 2))]
        );
        matDiffuse = mercuryResult.rgb;
        matAlpha = mercuryResult.a;
        break;
    }
    case LIQUID_MAT_FOG: {
        vec4 fogResult = calcLiquidFogMat(
            liquidInstanceData,
            fogDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 0))]
        );
        matDiffuse = fogResult.rgb;
        matAlpha = fogResult.a;
        break;
    }
    case LIQUID_MAT_LEYLINE: {
        vec4 leyLineResult = calcLeyLineLiquidMat(
            liquidInstanceData,
            leyLineDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 0))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 1))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 3))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 4))]
        );
        matDiffuse = leyLineResult.rgb;
        matAlpha = leyLineResult.a;
        break;
    }
    case LIQUID_MAT_FEL: {
        vec4 felResult = calcFelLiquidMat(
            liquidInstanceData,
            felDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position (fog/attenuation)
            gl_FragCoord.xy,
            vTextCoords, //uv
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 0))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 1))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 2))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 3))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 4))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 5))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 6))]
        );
        matDiffuse = felResult.rgb;
        matAlpha = felResult.a;
        break;
    }
    case LIQUID_MAT_SWAMP: {
        vec4 swampResult = calcSwampLiquidMat(
            liquidInstanceData,
            swampDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 0))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 1))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 2))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 3))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 4))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 6))]
        );
        matDiffuse = swampResult.rgb;
        matAlpha = swampResult.a;
        break;
    }
    case LIQUID_MAT_AZERITHE: {
        vec4 azeritheResult = calcAzeritheLiquidMat(
            liquidInstanceData,
            azeritheDatas[nonuniformEXT(dataInd)],
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vPositionDepth.w,

            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 0))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 1))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 2))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 4))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 5))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 6))],
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 7))], // env map
            s_Textures[nonuniformEXT(getLiquidTextureIndex(liquidBindless, 8))]  // foam
        );
        matDiffuse = azeritheResult.rgb;
        matAlpha = azeritheResult.a;
        break;
    }
    default:
        break;
    }

    // Create final color with alpha
    vec4 finalColor = vec4(matDiffuse, matAlpha);

    // Output the final color
    outColor = finalColor;
    writeObjectId(0u);
}
