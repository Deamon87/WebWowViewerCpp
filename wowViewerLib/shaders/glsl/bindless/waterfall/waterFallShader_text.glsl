precision highp float;
precision highp int;

#include "../../common/commonFunctions.glsl"
#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"
#include "../../common/commonUboSceneData.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord2_animated;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec3 vPosition;
layout(location=5) flat in int meshInd;

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#else
layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outSpecular;
layout(location = 2) out vec4 outNormal;
#endif

//Whole model
#include "../../common/commonM2IndirectDescriptorSet.glsl"
#include "../../common/commonM2WaterfallDescriptorSet.glsl"

const InteriorLightParam intLightWaterfall = {
vec4(0,0,0,0),
vec4(0,0,0,1)
};

// For references:
// http://mmikkelsen3d.blogspot.com/2011/07/derivative-maps.html
// http://www.thetenthplanet.de/archives/1180
// https://mmikk.github.io/papers3d/mm_sfgrad_bump.pdf

vec3 PerturbNormal ( vec3 surf_pos, vec3 surf_norm, sampler2D uNormalTex, vec2 texCoord, float value3_x )
{
    vec2 dBdUV = (texture(uNormalTex, texCoord).xy*2.0f - 1.0f) * (value3_x * 100);

    vec2 duv1 = dFdx( texCoord ).xy;
    vec2 duv2 = dFdy( texCoord ).xy;

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

void main() {
    WaterfallBindless waterfallBindless = waterfallBindlesses[meshInd];
    WaterfallCommon waterfallCommon = waterfallCommons[waterfallBindless.instanceIndex_waterfallInd_bumpTextureInd_maskInd.y];

    int normalTexInd = waterfallBindless.whiteWaterInd_noiseInd_normalTexInd.z;
    vec3 perturbedNormal = PerturbNormal(-vPosition, normalize(vNormal), s_Textures[normalTexInd], vTexCoord2_animated, waterfallCommon.values3.x);

    vec2 vTexCoordNorm = vTexCoord / waterfallCommon.values1.x;

    int noiseInd = waterfallBindless.whiteWaterInd_noiseInd_normalTexInd.y;

    float noise0 = texture(s_Textures[noiseInd], vec2(vTexCoordNorm.x - waterfallCommon.values1.z, vTexCoordNorm.y-waterfallCommon.values1.z - waterfallCommon.values2.z)).x;
    float noise1 = texture(s_Textures[noiseInd], vec2(vTexCoordNorm.x - waterfallCommon.values1.z + 0.418f, vTexCoordNorm.y + 0.355f + waterfallCommon.values1.z - waterfallCommon.values2.z)).x;
    float noise2 = texture(s_Textures[noiseInd], vec2(vTexCoordNorm.x + waterfallCommon.values1.z + 0.865f, vTexCoordNorm.y + 0.148f - waterfallCommon.values1.z - waterfallCommon.values2.z)).x;
    float noise3 = texture(s_Textures[noiseInd], vec2(vTexCoordNorm.x + waterfallCommon.values1.z + 0.651, vTexCoordNorm.y + 0.752f + waterfallCommon.values1.z - waterfallCommon.values2.z)).x;

    float noise_avr = abs(noise0 + noise1 + noise2 + noise3) * 0.25f;
    float noiseFinal = clamp(exp(waterfallCommon.values0.x * log2(noise_avr) * 2.2f) * waterfallCommon.values0.y, 0.0f, 1.0f);

    int whiteWaterInd = waterfallBindless.whiteWaterInd_noiseInd_normalTexInd.x;
    vec4 whiteWater_val = texture(s_Textures[whiteWaterInd], vTexCoord2_animated);

    int maskInd = waterfallBindless.instanceIndex_waterfallInd_bumpTextureInd_maskInd.w;
    vec4 mask_val_0 = texture(s_Textures[maskInd], vTexCoord);
    vec4 mask_val_1 = texture(s_Textures[maskInd], vec2(vTexCoord.x, vTexCoord.y +waterfallCommon.values3.z));

    float mix_alpha = clamp(
        ((whiteWater_val.w * noiseFinal - mask_val_1.y * mask_val_0.x) * 2.0f + waterfallCommon.values0.z) *
        (waterfallCommon.values0.w * 2.0f + 1.0f) -
        waterfallCommon.values0.w, 0.0f, 1.0f);

    vec4 whiteWater_val_baseColor_mix = mix(waterfallCommon.baseColor, whiteWater_val, mix_alpha);

    vec3 colorAfterLight = calcLight(
        whiteWater_val_baseColor_mix.rgb,
        perturbedNormal,
        true,
        0.0,
        scene,
        intLightWaterfall,
        vec3(0.0), /* accumLight */
        vec3(0.0), /*precomputedLight*/
        vec3(0.0), /* specular */
        vec3(0.0) /* emissive */
    );

    float w_clamped = clamp((1.0f - mask_val_0.w) * waterfallCommon.values1.w, 0.0f, 1.0f);
    float w_alpha_combined = clamp(w_clamped + mix_alpha, 0.0f, 1.0f);

    vec4 finalColor = vec4(
        mix(colorAfterLight.rgb, whiteWater_val_baseColor_mix.rgb, waterfallCommon.values3.w),
        w_alpha_combined
    //        whiteWater_val.a+0.2
    );

#ifndef DEFERRED
    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;
    finalColor = makeFog2(fogData,/*int(scene.extLight.adtSpecMult_fogCount.y),*/ finalColor, scene.uViewUpSceneTime.xyz, vPosition.xyz, sunDir.xyz, 0);
#endif

#ifndef DEFERRED
    outColor = finalColor;
#else
    outAlbedo = vec4(whiteWater_val_baseColor_mix.rgb, 0.0);
    outNormal = vec4(perturbedNormal.rgb, 0.0);
    outSpecular = vec4(0.0);
#endif
}
