#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

#include "../common/commonFunctions.glsl"
#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord2_animated;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec3 vPosition;

layout(set=1,binding=5) uniform sampler2D uMask;
layout(set=1,binding=6) uniform sampler2D uWhiteWater;
layout(set=1,binding=7) uniform sampler2D uNoise;
layout(set=1,binding=9) uniform sampler2D uNormalTex;

layout(location=0) out vec4 outputColor;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

layout(std140, set=0, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    vec4 values0;
    vec4 values1;
    vec4 values2;
    vec4 values3;
    vec4 values4;
    vec4 baseColor;
};

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

void main() {

    vec2 normalTex_val = texture(uNormalTex, vTexCoord2_animated).xy;
    vec2 normalTex_val_traformed = (normalTex_val*2.0f - 1.0f) * (values3.x * 100);

    vec3 worldSpaceDerivX = dFdx(vPosition);
    vec3 worldSpaceDerivY = dFdy(vPosition);

    vec3 vNormalNormalized = normalize(vNormal);

    vec3 derivNormalCross = cross(worldSpaceDerivY, vNormalNormalized);
    float derivNormVal = dot(worldSpaceDerivX, derivNormalCross);

    vec2 texCoordDeriv = vec2(
        dot(dFdx(vTexCoord2_animated).xy, normalTex_val_traformed),
        dot(dFdy(vTexCoord2_animated).xy, normalTex_val_traformed)
    );

    int cond0 = ((derivNormVal > 0) ? 0 : 1) - ((derivNormVal < 0) ? 0 : 1);

    vec3 normalVecDerivCross = (abs(derivNormVal) * vNormalNormalized) - (texCoordDeriv.y * cross(vNormalNormalized, worldSpaceDerivX) + texCoordDeriv.x * derivNormalCross) * cond0;
    vec3 normalVecDerivCrossNormalized = normalize(normalVecDerivCross);

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
        normalVecDerivCrossNormalized,
        true,
        0.0,
        scene,
        intLight,
        vec3(0.0), /* accumLight */
        vec3(0.0), /*precomputedLight*/
        vec3(0.0) /* specular */
    );



    float w_clamped = clamp((1.0f - mask_val_0.w) * values1.w, 0.0f, 1.0f);
    float w_alpha_combined = clamp(w_clamped + mix_alpha, 0.0f, 1.0f);

    vec4 finalColor = vec4(
        mix(colorAfterLight.rgb, whiteWater_val_baseColor_mix.rgb, values3.w),
        w_alpha_combined
//        whiteWater_val.a+0.2
    );

    outputColor = finalColor;
}
