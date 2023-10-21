#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonUboSceneData.glsl"

precision highp float;
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vTexcoord0;



layout(std140, set=1, binding=0) uniform textureMatrices {
    mat4 textureMatrix[64];
};
layout(std140, set=1, binding=1) uniform meshWideBlockPS {
    ivec4 uPixelShader_BlendMode_TextureTransformIndex;
};

layout(set=2, binding=0) uniform sampler2D uTexture;

layout(location = 0) out vec4 outputColor;

void main() {
    int textureTransformIndex = uPixelShader_BlendMode_TextureTransformIndex.z;

    vec2 texcoord = vTexcoord0;
    if (textureTransformIndex >= 0) {
        mat4 textMat = textureMatrix[textureTransformIndex];
        vec2 textCoordScale = vec2(length(textMat[0].xyz), length(textMat[2].xyz));
        vec2 textureTranslate = textMat[3].xy;
        texcoord = (vTexcoord0 * textCoordScale) + textureTranslate.xy;
    }

    vec4 tex = texture(uTexture, texcoord).rgba;

    vec4 finalColor = vec4((vColor.rgb*tex.rgb), tex.a * vColor.a);

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;

    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz, vPosition.xyz, sunDir.xyz, uPixelShader_BlendMode_TextureTransformIndex.y);

    outputColor = finalColor;
}
