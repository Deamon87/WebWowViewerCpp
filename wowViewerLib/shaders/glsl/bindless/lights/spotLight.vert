#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_shader_draw_parameters: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"

layout (location = 0) in vec2 position;

layout(std430, set=1, binding=0) buffer readonly spotLightBuffer {
    SpotLight lights[];
};

layout(location = 0) out flat int lightIndex;

vec3 quat_transform( vec4 q, vec3 v )
{
    return v + 2.*cross( q.xyz, cross( q.xyz, v ) + q.w*v );
}

void main() {
    SpotLight lightRec = lights[gl_InstanceIndex];

    vec3 lightAtten = lightRec.attenuationAndcosOuterAngle.xyz;
    vec4 vertPos = vec4(position * lightAtten.y, 0.0, 1.0);

    if (gl_VertexIndex > 0) {
        vertPos.z += lightRec.spotLightLen.x;
    }

    //Do rotation of cone
//    vertPos.xyz = quat_transform(lightRec.rotQuaternion, vertPos.xyz);
    vertPos.xyz = (lightRec.rotMat * vec4(vertPos.xyz, 0.0)).xyz;
    vertPos.xyz = vertPos.xyz + lightRec.positionAndcosInnerAngle.xyz;

    //And world to view transform
    vec4 viewPos = scene.uLookAtMat * vertPos;

     vec4 spotPointClip = scene.uPMatrix * viewPos;

//    spotPointClip.xy = clamp(spotPointClip.xy, -vec2(abs(spotPointClip.w)), vec2(abs(spotPointClip.w)));

    gl_Position = spotPointClip;
    lightIndex = gl_InstanceIndex;
}