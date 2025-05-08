#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../../../common/commonLightFunctions.slang"
#include "../../../common/commonFogFunctions.slang"
#include "../../../common/commonUboSceneData.slang"
#include "../../../common/commonLiquidDepthTest.slang"

precision highp float;
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vTexcoord0;

struct M2TextureMatrices {
    mat4 textureMatrix[64];
};

struct M2MeshWideBlockPS {
   ivec4 uPixelShader_BlendMode_TextureTransformIndex;
   ivec4 ribbonGpuBind; // unused by the fragment shader; part of the CPU-side struct
};

layout(std140, set=1, binding=0) StructuredBuffer<M2TextureMatrices> textureMatrices;

layout(std140, set=1, binding=1) ConstantBuffer<M2MeshWideBlockPS> meshWideBlockPS;

layout(set=2, binding=0) uniform sampler2D uTexture;

layout(location = 0) out vec4 outColor;
#include "../../../common/commonObjectId.slang"

[shader("pixel")]
void main() {
    int textureTransformIndex = meshWideBlockPS.uPixelShader_BlendMode_TextureTransformIndex.z;

    vec2 texcoord = vTexcoord0;
    if (textureTransformIndex >= 0) {
        mat4 textMat = textureMatrices[0].textureMatrix[textureTransformIndex];
        vec2 textCoordScale = vec2(length(textMat[0].xyz), length(textMat[2].xyz));
        vec2 textureTranslate = textMat[3].xy;
        texcoord = (vTexcoord0 * textCoordScale) + textureTranslate.xy;
    }

    vec4 tex = texture(uTexture, texcoord).rgba;

    vec4 finalColor = vec4((vColor.rgb*tex.rgb), tex.a * vColor.a);

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;

    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/,
                          finalColor, scene.uViewUpSceneTime.xyz,
                          vPosition.xyz, sunDir.xyz,
                          meshWideBlockPS.uPixelShader_BlendMode_TextureTransformIndex.y);

    liquidDepthClip();
    outColor = finalColor;
    // .w of the mesh-wide block carries the owning M2's object id (Ribbon::meshRibbonWideBlockPS::objectId)
    writeObjectId(0x80000000u | uint(meshWideBlockPS.uPixelShader_BlendMode_TextureTransformIndex.w));
}
