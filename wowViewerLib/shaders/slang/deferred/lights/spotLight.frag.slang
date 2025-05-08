#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
//#extension GL_EXT_debug_printf : enable

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.slang"
#include "../../common/commonGBufferData.slang"
#include "../../common/commonLightFunctions.slang"

layout(location = 0) in flat int lightIndex;

layout(std430, set=2, binding=0) StructuredBuffer<SpotLight> spotLightBuffer;

layout(location = 0) out vec4 outColor;

[shader("pixel")]
void main() {
    SpotLight lightRec = spotLightBuffer[lightIndex];
    
    vec3 viewPos = gbuff_getViewPos(gl_FragCoord.xy, scene.uInvPMatrix);
    vec3 viewNormal = gbuff_readNormal(gl_FragCoord.xy);


//    vec3 viewRay = vec3((in_viewPos.xy / vec2(in_viewPos.z)), 1.0);
//    vec3 viewPos = (viewRay * sceneDepth);
    
    vec3 lightDir = normalize((transpose(inverse(scene.uLookAtMat * lightRec.lightModelMat)) * vec4(0,0,1, 0.0)).xyz);
    vec3 lightAtten = lightRec.attenuationAndCosOuterAngle.xyz;

    vec3 innerColor = lightRec.innerColorAndBlendStart.xyz;
    vec3 outerColor = lightRec.outerColorAndBlendEnd.xyz;

    float blendStart = lightRec.innerColorAndBlendStart.w;
    float blendEnd = lightRec.outerColorAndBlendEnd.w;

    float cosInnerAngle = lightRec.positionAndCosInnerAngle.w;
    float cosOuterAngle = lightRec.attenuationAndCosOuterAngle.w;

    vec3 lightPosWorld = lightRec.positionAndCosInnerAngle.xyz;
    float falloff = lightRec.interiorAndSpotLightLenAndFallOff.z;

    vec3 lightPos = (scene.uLookAtMat * vec4(lightPosWorld, 1.0)).xyz;

    float cosAngleDiffInv = lightRec.directionAndCosAngleDiff.w;

    vec3 vectorToLight = -(viewPos.xyz - lightPos.xyz);

    float distanceToLightSqr = dot(vectorToLight, vectorToLight);
    float distanceToLightInv = inversesqrt(distanceToLightSqr);
    float diffuseTerm = max((dot(vectorToLight, viewNormal.xyz) * distanceToLightInv), 0.0);

    float colorLerp = smoothstep(blendEnd, blendStart, distanceToLightSqr * distanceToLightInv);
	vec3 color = mix(outerColor, innerColor, colorLerp);
/*
//Ld is distance from spot light center
//ld = lightDir * normalize(-(vectorToLight)) * length(vectorToLight) =
//     cos(angle_between_lightDir_and_vectorToLight) * length(vectorToLight) = dist_r
//
//                |\
//                | \
//                |  \
//                |   \
//                |    \
//  lightDir ->   |     \   <- vectorToLight
//                |      \
//                |       \
//                |        \
//                |         \
//                |__________\
//                     ^
//                   dist_r
*/

    float ld = max(dot(lightDir, (-vectorToLight)), 0.0);
    float attenuation = (1.0 - clamp(((ld - lightAtten.x) * lightAtten.z), 0.0, 1.0));
    ld = max(dot(lightDir, ((-vectorToLight) * distanceToLightInv)), 0.0);
    
    float spotLightEffect;
    if ((ld < cosOuterAngle))
    {
        spotLightEffect = 0.0;
    }
    else if ((ld <= cosInnerAngle))
    {
        spotLightEffect = pow(abs(((ld - cosOuterAngle) * cosAngleDiffInv)), falloff);
    } else {
        spotLightEffect = 1.0;
    }

    vec3 finalColor = ((((color * attenuation) * (color * attenuation)) * diffuseTerm) * spotLightEffect);
//    if ((lightRec.interior.x != 0.0))
//    {
//        vec3 finalColor = (t597 * t668.www);
//        color_688 = color_644;
//    }

    outColor = vec4(finalColor, 1.0);
}