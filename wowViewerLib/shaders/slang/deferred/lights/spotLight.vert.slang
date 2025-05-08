#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_shader_draw_parameters: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.slang"
#include "../../common/commonGBufferData.slang"

layout (location = 0) in vec2 position;

layout(std430, set=2, binding=0) StructuredBuffer<SpotLight> spotLightBuffer;

layout(location = 0) out flat int lightIndex;

vec3 quat_transform( vec4 q, vec3 v )
{
    return v + 2.*cross( q.xyz, cross( q.xyz, v ) + q.w*v );
}

[shader("vertex")]
void main() {
    SpotLight lightRec = spotLightBuffer[gl_InstanceIndex];

    vec3 lightAtten = lightRec.attenuationAndCosOuterAngle.xyz;
    vec4 vertPos = vec4(position, gl_VertexIndex > 0 ? 1.0 : 0.0, 1.0);

    float spotLightLen = lightRec.interiorAndSpotLightLenAndFallOff.y;

    vertPos = vertPos * vec4(
        spotLightLen,
        spotLightLen,
        lightRec.attenuationAndCosOuterAngle.y,
        1.0
    );

    //Do rotation of cone and positioning
    vertPos.xyz = (lightRec.lightModelMat * vec4(vertPos.xyz, 1.0)).xyz;

    //And world to view transform
    vec4 viewPos = scene.uLookAtMat * vertPos;

     vec4 spotPointClip = scene.uPMatrix * viewPos;

//    float tol = 0.000f;
//
//    spotPointClip.x = clamp(spotPointClip.x, -abs(spotPointClip.w)+tol, abs(spotPointClip.w)-tol);
//    spotPointClip.y = clamp(spotPointClip.y, -abs(spotPointClip.w)+tol, abs(spotPointClip.w)-tol);
////
//
//    float least = min(0, spotPointClip.w);
//    float most = max(0, spotPointClip.w);
//    spotPointClip.z = clamp(spotPointClip.z, least+tol, most-tol);

    gl_Position = spotPointClip;
    lightIndex = gl_InstanceIndex;
}