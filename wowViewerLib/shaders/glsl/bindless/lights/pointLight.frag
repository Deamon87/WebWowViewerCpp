#version 450

#extension GL_GOOGLE_include_directive: require
//#extension GL_EXT_debug_printf : enable

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"
#include "../../common/commonLightFunctions.glsl"

layout(location = 0) in flat int lightIndex;

layout(std430, set=1, binding=0) buffer readonly pointLightBuffer {
    LocalLight lights[];
};
layout(set=1, binding=1) uniform sampler2D depthTex;
layout(set=1, binding=2) uniform sampler2D normalTex;
layout(std140, set=1, binding=3) uniform pointLightBuffer {
    vec4 screenSize;
};

layout(location = 0) out vec4 outColor;


float unpack_depth(const in vec4 rgba_depth)
{
    const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth, bit_shift);
    return depth;
}

void main() {
    LocalLight lightRec = lights[lightIndex];

    vec2 uv = gl_FragCoord.xy / screenSize.xy;

    vec3 normal = texture(normalTex, uv).rgb * 2.0 - vec3(1.0);
    float depth = texture(depthTex, uv).r;

    float z = (depth - 0.06f) / (1.0f - 0.06f) ;

    vec4 viewPos = inverse(scene.uPMatrix) * vec4(uv.xy * 2.0 - 1.0, z, 1.0);
    viewPos.xyz = viewPos.xyz / viewPos.w;

    vec4 worldPos = inverse(scene.uLookAtMat) * vec4(viewPos.xyz, 1.0);
    worldPos.xyz = worldPos.xyz / worldPos.w;


    //    debugPrintfEXT("My worldPos is %f %f %f", worldPos.x, worldPos.y, worldPos.z);

    //Calc color
    vec3 lightViewPos = (scene.uLookAtMat * vec4(lightRec.position.xyz, 1.0)).xyz;
    vec3 vectorToLight = (lightViewPos.xyz - viewPos.xyz);
    float distanceToLightSqr = dot(vectorToLight, vectorToLight);
    float distanceToLightInv = inversesqrt(distanceToLightSqr);
    float distanceToLight = (distanceToLightSqr * distanceToLightInv);
    float diffuseTerm1 = max((dot(vectorToLight, normal) * distanceToLightInv), 0.0);
    vec4 attenuationRec = lightRec.attenuation;

    float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1.0 / (attenuationRec.z - attenuationRec.x)), 0.0, 1.0));

    vec3 attenuatedColor = attenuation * lightRec.innerColor.xyz;
    vec3 lightColor = vec3(attenuatedColor * attenuatedColor * diffuseTerm1 );

    outColor = vec4(vec3(lightColor), 1.0);
}

//void test() {
//t391 = cb_local_light_data.cb_localLightData.c_lights[index_388];
//float3 vectorToLight_394 = (t391.position.xyz - t376);
//float distanceToLightSqr_395 = dot(vectorToLight_394, vectorToLight_394);
//float distanceToLightInv_396 = rsqrt(distanceToLightSqr_395);
//float distanceToLight_397 = (distanceToLightSqr_395 * distanceToLightInv_396);
//float dot_398 = dot(vectorToLight_394, normalize_366);
//float diffuseTerm_400 = fmax((dot_398 * distanceToLightInv_396), 0.0f);
//float4 t401 = t391.attenuation;
//float saturate_406 = saturate(((distanceToLight_397 - t401.x) * t401.z));
//float attenuation_407 = (1.0f - saturate_406);
//float4 t408 = t391.blendParams;
//float colorLerp_411 = smoothstep(t408.y, t408.x, distanceToLight_397);
//float3 lerp_417 = mix(t391.outerColor.xyz, t391.innerColor.xyz, float3(colorLerp_411));
//float3 color_421 = (color_389 + (((lerp_417 * attenuation_407) * (lerp_417 * attenuation_407)) * diffuseTerm_400));
//);
//
//
//1.0f - (x - attStart) * (1.)