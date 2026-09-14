#define FORWARD_RENDERING

import glsl;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"

precision highp float;
precision highp int;

layout(location=0) in vec4 vPositionDepth;
layout(location=1) in vec3 vWorldPosition;
layout(location=2) in vec2 vTextCoords;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec2 vCellUv;

layout(location = 0) out vec4 outColor;


#include "../../common/commonUboSceneData.slang"
#include "../../common/commonLiquidStructs.slang"
// gbuffer (depthTex/screenData/liquidBackBuffer) is bound at set=3 for liquid draws
#define GBUFFER_SET_INDEX 3
#include "../../common/commonLiquidDescriptorSet.slang"




const InteriorLightParam intLight = {
    vec4(0,0,0,1.0f),
    vec4(0,0,0,0),
    vec4(0,0,0,0),
    vec4(0,0,0,1),
    vec4(0,0,0,0)
};

#define M_PI 3.1415926538
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0);
}

mat4 rotationZ(float angle) {
    return mat4(
        cos(angle), -sin(angle), 0.0,  0.0 ,
        sin(angle), cos(angle),  0.0,  0.0 ,
        0.0,        0.0,         1.0,  0.0 ,
        0.0,        0.0,         0.0,  1.0
    );
}

[shader("pixel")]
void main() {
    vec3 matDiffuse;
    float matAlpha = 1.0;

    #ifdef WATER_LIQUID
    // Water material computation
        vec4 waterResult = calcWaterLiquidMat(
            liquidInstanceData,
            waterData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position,
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            uTexture2,
            uTexture3

        );
        matDiffuse = waterResult.rgb;
        matAlpha = waterResult.a;
    #endif

    #ifdef MAGMA_LIQUID
    // Magma material computation
        vec4 magmaResult = calcMagmaLiquidMat(
            liquidInstanceData,
            magmaData,
            scene,
            fogData,
            vPositionDepth.xyz, //position (fog)
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            uTexture1,
            uTexture2,
            uTexture3,
            uTexture6

        );
        matDiffuse = magmaResult.rgb;
        matAlpha = magmaResult.a;
    #endif

    #ifdef MERCURY_LIQUID
    // Mercury material computation
        vec4 mercuryResult = calcMercuryLiquidMat(
            liquidInstanceData,
            mercuryData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            uTexture2

        );
        matDiffuse = mercuryResult.rgb;
        matAlpha = mercuryResult.a;
    #endif

    #ifdef FOG_LIQUID
    // Fog material computation
        vec4 fogResult = calcLiquidFogMat(
            liquidInstanceData,
            liquidFogData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            uTexture0

        );
        matDiffuse = fogResult.rgb;
        matAlpha = fogResult.a;
    #endif

    #ifdef LEYLINE_LIQUID
    // Ley line material computation
        vec4 leyLineResult = calcLeyLineLiquidMat(
            liquidInstanceData,
            leyLineData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            uTexture0,
            uTexture1,
            uTexture3,
            uTexture4

        );
        matDiffuse = leyLineResult.rgb;
        matAlpha = leyLineResult.a;
    #endif

    #ifdef FEL_LIQUID
    // Fel material computation
        vec4 felResult = calcFelLiquidMat(
            liquidInstanceData,
            felData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position (fog/attenuation)
            gl_FragCoord.xy,
            vTextCoords, //uv
            vPositionDepth.w,

            uTexture0,
            uTexture1,
            uTexture2,
            uTexture3,
            uTexture4,
            uTexture5,
            uTexture6

        );
        matDiffuse = felResult.rgb;
        matAlpha = felResult.a;
    #endif

    #ifdef SWAMP_LIQUID
    // Swamp material computation
        vec4 swampResult = calcSwampLiquidMat(
            liquidInstanceData,
            swampData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vCellUv,
            vPositionDepth.w,

            uTexture0,
            uTexture1,
            uTexture2,
            uTexture3,
            uTexture4,
            uTexture6

        );
        matDiffuse = swampResult.rgb;
        matAlpha = swampResult.a;
    #endif

    #ifdef AZERITHE_LIQUID
    // Azerithe material computation
        vec4 azeritheResult = calcAzeritheLiquidMat(
            liquidInstanceData,
            azeritheData,
            scene,
            fogData,
            vWorldPosition.xyz,
            vPositionDepth.xyz, //position
            gl_FragCoord.xy,
            vTextCoords, //uv
            vPositionDepth.w,

            uTexture0,
            uTexture1,
            uTexture2,
            uTexture4,
            uTexture5,
            uTexture6,
            uTexture7, // env map (fileDataId 1797551)
            uTexture8  // foam (fileDataId 1844666)

        );
        matDiffuse = azeritheResult.rgb;
        matAlpha = azeritheResult.a;
    #endif

    // Create final color with alpha
    vec4 finalColor = vec4(matDiffuse, matAlpha);

    // Output the final color
    outColor = finalColor;
} 