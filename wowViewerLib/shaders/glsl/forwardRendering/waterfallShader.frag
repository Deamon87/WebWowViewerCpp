#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonFunctions.glsl"
#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord2_animated;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec3 vPosition;

layout(set=3,binding=6) uniform sampler2D uMask;
layout(set=3,binding=7) uniform sampler2D uWhiteWater;
layout(set=3,binding=8) uniform sampler2D uNoise;
layout(set=3,binding=10) uniform sampler2D uNormalTex;

layout(location=0) out vec4 outputColor;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

//Whole model
#include "../common/commonM2DescriptorSet.glsl"

layout(std140, set=2, binding=5) uniform meshWideBlockPS {
    vec4 values0;
    vec4 values1;
    vec4 values2;
    vec4 values3;
    vec4 values4;
    vec4 baseColor;
};

const InteriorLightParam intLightWaterfall = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

// For references:
// http://mmikkelsen3d.blogspot.com/2011/07/derivative-maps.html
// http://www.thetenthplanet.de/archives/1180
// https://mmikk.github.io/papers3d/mm_sfgrad_bump.pdf

vec3 PerturbNormal ( vec3 surf_pos, vec3 surf_norm )
{
    vec2 dBdUV = (texture(uNormalTex, vTexCoord2_animated).xy*2.0f - 1.0f) * (values3.x * 100);

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

void main() {
    vec3 perturbedNormal = PerturbNormal(-vPosition, normalize(vNormal));

    vec2 vTexCoordNorm = vTexCoord / values1.x;

    float noise0 = texture(uNoise, vec2(vTexCoordNorm.x - values1.z, vTexCoordNorm.y-values1.z - values2.z)).x;
    float noise1 = texture(uNoise, vec2(vTexCoordNorm.x - values1.z + 0.418f, vTexCoordNorm.y + 0.355f + values1.z - values2.z)).x;
    float noise2 = texture(uNoise, vec2(vTexCoordNorm.x + values1.z + 0.865f, vTexCoordNorm.y + 0.148f - values1.z - values2.z)).x;
    float noise3 = texture(uNoise, vec2(vTexCoordNorm.x + values1.z + 0.651, vTexCoordNorm.y + 0.752f + values1.z - values2.z)).x;

    float noise_avr = abs(noise0 + noise1 + noise2 + noise3) * 0.25f;
    float noiseFinal = clamp(exp(values0.x * log2(noise_avr) * 2.2f) * values0.y, 0.0f, 1.0f);

    vec4 whiteWater_val = texture(uWhiteWater, vTexCoord2_animated);
    vec4 mask_val_0 = texture(uMask, vTexCoord);
    vec4 mask_val_1 = texture(uMask, vec2(vTexCoord.x, vTexCoord.y +values3.z));

    float mix_alpha = clamp(
        ((whiteWater_val.w * noiseFinal - mask_val_1.y * mask_val_0.x) * 2.0f + values0.z) *
        (values0.w * 2.0f + 1.0f) -
        values0.w, 0.0f, 1.0f);

    vec4 whiteWater_val_baseColor_mix = mix(baseColor, whiteWater_val, mix_alpha);

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

    float w_clamped = clamp((1.0f - mask_val_0.w) * values1.w, 0.0f, 1.0f);
    float w_alpha_combined = clamp(w_clamped + mix_alpha, 0.0f, 1.0f);

    vec4 finalColor = vec4(
        mix(colorAfterLight.rgb, whiteWater_val_baseColor_mix.rgb, values3.w),
        w_alpha_combined
//        whiteWater_val.a+0.2
    );

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;
    finalColor = makeFog(fogData, finalColor, vPosition.xyz, sunDir.xyz, 0);


    outputColor = finalColor;
}
