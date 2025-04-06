#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"

precision highp float;
precision highp int;

layout(location = 0) in vec2 vChunkCoords;
layout(location = 1) in vec3 vPosition;

layout(std140, binding=0) uniform modelWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDir_FogStart;
    vec4 uSunColor_uFogEnd;
    vec4 uAmbientLight;
    vec4 FogColor;

    int uNewFormula;
};

layout(binding=4) uniform sampler2D uDiffuseTexture;
layout(binding=5) uniform sampler2D uNormalTexture;

layout(location = 0) out vec4 fragColor;


void main() {
    vec2 TextureCoords = vec2(vChunkCoords.x, vChunkCoords.y );


    vec3 matDiffuse;
    vec4 finalColor;

    vec4 texDiffuse =  texture(uDiffuseTexture, TextureCoords).rgba;
    matDiffuse.rgb = texDiffuse.rgb;
    vec3 vNormal = 2.0*texture(uNormalTexture, TextureCoords).rgb - 1.0 ;
    vNormal = vec3(-(vNormal.z), -(vNormal.x), vNormal.y);



    finalColor.rgba = vec4(0,0,0, 1.0);

    //Spec part
//    float specBlend = texDiffuse.a;
//    vec3 halfVec = -(normalize((uSunDir_FogStart.xyz + normalize(vPosition))));
//    vec3 lSpecular = ((uSunColor_uFogEnd.xyz * pow(max(0.0, dot(halfVec, vNormal)), 20.0)));
//    vec3 specTerm = (vec3(specBlend) * lSpecular);
//    finalColor.rgb += specTerm;

    // --- Fog start ---
    vec3 fogColor = FogColor.xyz;
    float fog_start = uSunDir_FogStart.w;;
    float fog_end = uSunColor_uFogEnd.w;
    float fog_rate = 1.5;
    float fog_bias = 0.01;

    //vec4 fogHeightPlane = pc_fog.heightPlane;
    //float heightRate = pc_fog.color_and_heightRate.w;

    float distanceToCamera = length(vPosition.xyz);
    float z_depth = (distanceToCamera - fog_bias);
    float expFog = 1.0 / (exp((max(0.0, (z_depth - fog_start)) * fog_rate)));
    //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
    //float heightFog = clamp((height * heightRate), 0, 1);
    float heightFog = 1.0;
    expFog = (expFog + heightFog);
    float endFadeFog = clamp(((fog_end - distanceToCamera) / (0.699999988 * fog_end)), 0.0, 1.0);

    finalColor.rgb = mix(fogColor.rgb, finalColor.rgb, vec3(min(expFog, endFadeFog)));
    // --- Fog end ---

    finalColor.a = 1.0;
    fragColor = finalColor;
}
