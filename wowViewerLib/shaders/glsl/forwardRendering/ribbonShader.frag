#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vTexcoord0;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

//layout(std140, binding=3) uniform textureMatrices {
//    mat4 textureMatrix[64];
//};
layout(std140, binding=4) uniform meshWideBlockPS {
    ivec4 uPixelShader_BlendMode_TextureTransformIndex;
};

layout(set=1, binding=5) uniform sampler2D uTexture;

layout(location = 0) out vec4 outputColor;

void main() {
//    vec2 textCoordScale = uAlphaTestScalev.yz;
//    vec2 texcoord = (vTexcoord0 * textCoordScale) + uTextureTranslate.xy;

    vec4 tex = texture(uTexture, vTexcoord0).rgba;

    vec4 finalColor = vec4((vColor.rgb*tex.rgb), tex.a * vColor.a);


//    vec3 sunDir =
//        mix(
//            scene.uInteriorSunDir,
//            scene.extLight.uExteriorDirectColorDir,
//            interiorExteriorBlend.x
//        )
//        .xyz;
    vec3 sunDir =scene.extLight.uExteriorDirectColorDir.xyz;

    finalColor = makeFog(fogData, finalColor, vPosition.xyz, sunDir.xyz, uPixelShader_BlendMode_TextureTransformIndex.y);

    outputColor = finalColor;
}
