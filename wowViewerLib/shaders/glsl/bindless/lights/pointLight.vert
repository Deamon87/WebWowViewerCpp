#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_shader_draw_parameters: require

precision highp float;
precision highp int;

#include "../../common/commonUboSceneData.glsl"

layout (location = 0) in vec2 position;

layout(std430, set=1, binding=0) buffer readonly pointLightBuffer {
    LocalLight lights[];
};

layout(location = 0) out flat int lightIndex;

void main() {

    LocalLight lightRec = lights[gl_InstanceIndex];

    vec4 lightPosView = scene.uLookAtMat * vec4(lightRec.position.xyz, 1.0);

    float attenStart = lightRec.attenuation.x;
    float attenEnd = lightRec.attenuation.z;

    float pointLightRadius = attenEnd;

    //Hack. If the point light is behind the camera
    //With current first person camera positive Z is behind the camera. Should fix?
    {
        float distToLight = abs(lightPosView.z);
        if (lightPosView.z > 0.0 && distToLight < attenEnd) {
            pointLightRadius *= ((attenEnd - distToLight) / attenEnd);
        }
    }


    //Hack to get square bounding the circle of point light
    //The quads (-1.0, 1.0) or other is multiplied by radius end of point light
    vec2 quadPosView = (position.xy * pointLightRadius);

    lightPosView.xy += quadPosView;
    //Bring the projection on sphere to the front

    //Now it's safe to multiply it by perspective matrix
    vec4 pointLightClip = scene.uPMatrix * vec4(lightPosView.xyz, 1.0);
    pointLightClip.z = pointLightClip.w - (sign(pointLightClip.w) * 0.0001);
    //clamp it against screen bounderies

    pointLightClip.xy = clamp(pointLightClip.xy, -vec2(abs(pointLightClip.w)), vec2(abs(pointLightClip.w)));

    gl_Position = pointLightClip;
    lightIndex = gl_InstanceIndex;
}

