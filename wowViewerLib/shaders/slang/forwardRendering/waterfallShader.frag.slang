#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#define M2_WATERFALLSHADER

#include "../common/commonFunctions.slang"
#include "../common/commonLightFunctions.slang"
#include "../common/commonFogFunctions.slang"
#include "../common/commonUboSceneData.slang"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord2_animated;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec3 vPosition;

layout(location=0) out vec4 outputColor;

#include "../common/commonM2DescriptorSet.slang"
#include "../common/commonM2WaterfallDescriptorSet.slang"

const InteriorLightParam intLightWaterfall = {
    vec4(0,0,0,1.0f),
    vec4(0,0,0,0),
    vec4(0,0,0,0),
    vec4(0,0,0,1),
    vec4(0,0,0,0)
};

// For references:
// http://mmikkelsen3d.blogspot.com/2011/07/derivative-maps.html
// http://www.thetenthplanet.de/archives/1180
// https://mmikk.github.io/papers3d/mm_sfgrad_bump.pdf

vec3 PerturbNormal ( vec3 surf_pos, vec3 surf_norm )
{
    vec2 dBdUV = (texture(uNormalTex, vTexCoord2_animated).xy*2.0f - 1.0f) * (waterfallCommon.values3.x * 100);

    vec2 duv1 = dFdx( vTexCoord2_animated ).xy;
    vec2 duv2 = dFdy( vTexCoord2_animated ).xy;

    vec3 vSigmaS = dFdx ( surf_pos );
    vec3 vSigmaT = dFdy ( surf_pos );
    vec3 vN = surf_norm ; // normalized
    vec3 vR1 = cross ( vSigmaT , vN );
    vec3 vR2 = cross (vN , vSigmaS );
    float fDet = dot ( vSigmaS , vR1 );
    float dBs = dBdUV.x * duv1.x + dBdUV.y * duv1.y;
    float dBt = dBdUV.x * duv2.x + dBdUV.y * duv2.y;
    vec3 vSurfGrad = sign ( fDet ) * ( dBs * vR1 + dBt * vR2 );
    return normalize ( abs ( fDet )*vN - vSurfGrad );
}

[shader("pixel")]
void main() {
    vec3 perturbedNormal = PerturbNormal(-vPosition, normalize(vNormal));

    vec2 vTexCoordNorm = vTexCoord / waterfallCommon.values1.x;

    float noise0 = texture(uNoise, vec2(vTexCoordNorm.x - waterfallCommon.values1.z, vTexCoordNorm.y-waterfallCommon.values1.z - waterfallCommon.values2.z)).x;
    float noise1 = texture(uNoise, vec2(vTexCoordNorm.x - waterfallCommon.values1.z + 0.418f, vTexCoordNorm.y + 0.355f + waterfallCommon.values1.z - waterfallCommon.values2.z)).x;
    float noise2 = texture(uNoise, vec2(vTexCoordNorm.x + waterfallCommon.values1.z + 0.865f, vTexCoordNorm.y + 0.148f - waterfallCommon.values1.z - waterfallCommon.values2.z)).x;
    float noise3 = texture(uNoise, vec2(vTexCoordNorm.x + waterfallCommon.values1.z + 0.651, vTexCoordNorm.y + 0.752f + waterfallCommon.values1.z - waterfallCommon.values2.z)).x;

    float noise_avr = abs(noise0 + noise1 + noise2 + noise3) * 0.25f;
    float noiseFinal = clamp(exp(waterfallCommon.values0.x * log2(noise_avr) * 2.2f) * waterfallCommon.values0.y, 0.0f, 1.0f);

    vec4 whiteWater_val = texture(uWhiteWater, vTexCoord2_animated);
    vec4 mask_val_0 = texture(uMask, vTexCoord);
    vec4 mask_val_1 = texture(uMask, vec2(vTexCoord.x, vTexCoord.y +waterfallCommon.values3.z));

    float mix_alpha = clamp(
        ((whiteWater_val.w * noiseFinal - mask_val_1.y * mask_val_0.x) * 2.0f + waterfallCommon.values0.z) *
        (waterfallCommon.values0.w * 2.0f + 1.0f) -
        waterfallCommon.values0.w, 0.0f, 1.0f);

    vec4 whiteWater_val_baseColor_mix = mix(waterfallCommon.baseColor, whiteWater_val, mix_alpha);

    vec3 colorAfterLight = calcLight(
        whiteWater_val_baseColor_mix.rgb,
        perturbedNormal,
        vPosition.xyz,
        true,
        scene,
        intLightWaterfall,
        vec3(0.0), /* accumLight */
        vec3(0.0), /*precomputedLight*/
        vec3(0.0), /* specular */
        vec3(0.0), /* emissive */
        1.0 /* ao */
    );

    float w_clamped = clamp((1.0f - mask_val_0.w) * waterfallCommon.values1.w, 0.0f, 1.0f);
    float w_alpha_combined = clamp(w_clamped + mix_alpha, 0.0f, 1.0f);

    vec4 finalColor = vec4(
        mix(colorAfterLight.rgb, whiteWater_val_baseColor_mix.rgb, waterfallCommon.values3.w),
        w_alpha_combined
//        whiteWater_val.a+0.2
    );

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;
    finalColor = makeFog2(fogData,/*int(scene.extLight.adtSpecMult_fogCount.y),*/ finalColor, scene.uViewUpSceneTime.xyz, vPosition.xyz, sunDir.xyz, 0);


    outputColor = finalColor;
}
