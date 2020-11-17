#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTextCoords;

layout(location=0) out vec4 outputColor;

layout(set=1,binding=5) uniform sampler2D uTexture;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

//Individual meshes
layout(std140, binding=4) uniform meshWideBlockPS {
//ivec4 waterTypeV;
    vec4 color;
};

void main() {
    vec3 finalColor = color.rgb+texture(uTexture, vTextCoords).rgb;

    vec3 sunDir =scene.extLight.uExteriorDirectColorDir.xyz;

    //BlendMode is always GxBlend_Alpha
    finalColor = makeFog(fogData, finalColor.rgb, vPosition.xyz, sunDir.xyz, 2);
    outputColor = vec4(finalColor, 0.7);
}
