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

    float attentuationEnd = lightRec.attenuation.z;

    //Hack to get square bounding the circle of point light
    //The quads (-1.0, 1.0) or other is multiplied by radius end of point light
    vec2 quadPosView = (position.xy * attentuationEnd);

    lightPosView.xy += quadPosView;
    //Bring the projection on sphere to the front
    lightPosView.z = min(-0.001, lightPosView.z + attentuationEnd);



    //Now it's safe to multiply it by perspective matrix
    gl_Position = scene.uPMatrix * vec4(lightPosView.xyz, 1.0);
    lightIndex = gl_InstanceIndex;
}

