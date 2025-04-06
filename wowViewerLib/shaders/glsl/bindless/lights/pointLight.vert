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

    //Hack. If camera is inside Point's light sphere -> use full screen quad
    if (length(lightPosView.xyz) < pointLightRadius)
    {
        gl_Position = vec4(position.xy, 0.1, 1.0);
    } else {
        //Hack to get square bounding the circle of point light
        //The quads (-1.0, 1.0) or other is multiplied by radius end of point light
        vec2 quadPosView = (position.xy * pointLightRadius);
        vec3 quadCorner = vec3(lightPosView.xy + quadPosView.xy, lightPosView.z);

        vec4 quadCornerClip = scene.uPMatrix * vec4(quadCorner.xyz, 1.0);

        quadCornerClip.z = quadCornerClip.w - (sign(quadCornerClip.w) * 0.0001);
        //clamp it against screen bounderies
        quadCornerClip.xy = clamp(quadCornerClip.xy, -vec2(abs(quadCornerClip.w)), vec2(abs(quadCornerClip.w)));

        gl_Position = quadCornerClip;
    }

    lightIndex = gl_InstanceIndex;
}

