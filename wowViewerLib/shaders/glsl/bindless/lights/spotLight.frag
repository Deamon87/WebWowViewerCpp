#version 450

#extension GL_GOOGLE_include_directive: require
//#extension GL_EXT_debug_printf : enable

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"
#include "../../common/commonLightFunctions.glsl"

layout(location = 0) in flat int lightIndex;

layout(std430, set=1, binding=0) buffer readonly spotLightBuffer {
    SpotLight lights[];
};

layout(set=1, binding=1) uniform sampler2D depthTex;
layout(set=1, binding=2) uniform sampler2D normalTex;
layout(std140, set=1, binding=3) uniform pointLightBuffer {
    vec4 screenSize;
};

layout(location = 0) out vec4 outColor;

void main() {
    SpotLight lightRec = lights[lightIndex];
    
    vec2 uv = gl_FragCoord.xy / screenSize.xy;

    vec3 viewNormal = texture(normalTex, uv).rgb * 2.0 - vec3(1.0);
    float sceneDepth = texture(depthTex, uv).r;

    float z = (sceneDepth - 0.06f) / (1.0f - 0.06f) ;

    vec4 viewPos = inverse(scene.uPMatrix) * vec4(uv.xy * 2.0 - 1.0, z, 1.0);
    viewPos.xyz = viewPos.xyz / viewPos.w;

//    vec3 viewRay = vec3((in_viewPos.xy / vec2(in_viewPos.z)), 1.0);
//    vec3 viewPos = (viewRay * sceneDepth);
    
    vec3 lightDir = normalize((transpose(inverse(scene.uLookAtMat * lightRec.lightModelMat)) * vec4(0,0,1, 0.0)).xyz);
    vec3 lightAtten = lightRec.attenuationAndcosOuterAngle.xyz;

    vec3 color = lightRec.colorAndFalloff.xyz;
    float cosOuterAngle = lightRec.attenuationAndcosOuterAngle.w;

    vec3 lightPos = (scene.uLookAtMat * vec4(lightRec.positionAndcosInnerAngle.xyz, 1.0)).xyz;
    float cosInnerAngle = lightRec.positionAndcosInnerAngle.w;
    float falloff = lightRec.colorAndFalloff.w;

    float cosAngleDiffInv = lightRec.directionAndcosAngleDiff.w;

    vec3 vectorToLight = -(viewPos.xyz - lightPos.xyz);

    float distanceToLightSqr = dot(vectorToLight, vectorToLight);
    float distanceToLightInv = inversesqrt(distanceToLightSqr);
    float diffuseTerm = max((dot(vectorToLight, viewNormal.xyz) * distanceToLightInv), 0.0);

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