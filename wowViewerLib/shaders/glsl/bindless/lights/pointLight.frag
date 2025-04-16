#version 450

#extension GL_GOOGLE_include_directive: require
//#extension GL_EXT_debug_printf : enable

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"
#include "../../common/commonGBufferData.glsl"
#include "../../common/commonLightFunctions.glsl"

layout(location = 0) in flat int lightIndex;

layout(std430, set=2, binding=0) buffer readonly pointLightBuffer {
    LocalLight lights[];
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

    vec3 viewPos = gbuff_getViewPos(gl_FragCoord.xy, scene.uInvPMatrix);
    vec3 viewNormal = gbuff_readNormal(gl_FragCoord.xy);

    vec4 worldPos = scene.uInvLookAtMat * vec4(viewPos.xyz, 1.0);
    worldPos.xyz = worldPos.xyz / worldPos.w;


    //    debugPrintfEXT("My worldPos is %f %f %f", worldPos.x, worldPos.y, worldPos.z);

    //Calc color
    vec3 lightViewPos = (scene.uLookAtMat * vec4(lightRec.position.xyz, 1.0)).xyz;
    vec3 vectorToLight = (lightViewPos.xyz - viewPos.xyz);
    float distanceToLightSqr = dot(vectorToLight, vectorToLight);
    float distanceToLightInv = inversesqrt(distanceToLightSqr);
    float distanceToLight = (distanceToLightSqr * distanceToLightInv);
    float diffuseTerm1 = max((dot(vectorToLight, viewNormal) * distanceToLightInv), 0.0);
    vec4 attenuationRec = lightRec.attenuation;

    float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1.0 / (attenuationRec.z - attenuationRec.x)), 0.0, 1.0));

    vec4 blendParams = lightRec.blendParams;
    float colorLerp = smoothstep(blendParams.y, blendParams.x, distanceToLight);
    vec3 lerp = mix(lightRec.outerColor.xyz, lightRec.innerColor.xyz, vec3(colorLerp));

    vec3 attenuatedColor = attenuation * lerp * attenuationRec.y;
    vec3 lightColor = vec3(attenuatedColor * attenuatedColor * diffuseTerm1 );

    outColor = vec4(vec3(lightColor), 1.0);
}