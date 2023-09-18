#version 450
#extension GL_GOOGLE_include_directive: require

#define FRAGMENT_SHADER 1

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonFunctions.glsl"
#include "../common/commonWMOMaterial.glsl"
#include "../common/commonUboSceneData.glsl"



layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord3;
layout(location=3) in vec2 vTexCoord4;
layout(location=4) in vec4 vColor;
layout(location=5) in vec4 vColor2;
layout(location=6) in vec4 vColorSecond;
layout(location=7) in vec4 vPosition;
layout(location=8) in vec3 vNormal;
layout(location=9) in vec4 vWmoAmbient;



layout(std140, set=1, binding=4) uniform meshWideBlockPS {
    ivec4 UseLitColor_EnableAlpha_PixelShader_BlendMode;
    vec4 FogColor_AlphaTest;
};

layout(set=2, binding=5) uniform sampler2D uTexture;
layout(set=2, binding=6) uniform sampler2D uTexture2;
layout(set=2, binding=7) uniform sampler2D uTexture3;
layout(set=2, binding=8) uniform sampler2D uTexture4;
layout(set=2, binding=9) uniform sampler2D uTexture5;
layout(set=2, binding=10) uniform sampler2D uTexture6;
layout(set=2, binding=11) uniform sampler2D uTexture7;
layout(set=2, binding=12) uniform sampler2D uTexture8;
layout(set=2, binding=13) uniform sampler2D uTexture9;

layout (location = 0) out vec4 outputColor;

void main() {
    int uPixelShader = UseLitColor_EnableAlpha_PixelShader_BlendMode.z;

    vec3 matDiffuse = vec3(0.0);
    vec3 spec = vec3(0.0);
    vec3 emissive = vec3(0.0);
    float finalOpacity = 0.0;
    bool doDiscard;

    caclWMOFragMat(uPixelShader, UseLitColor_EnableAlpha_PixelShader_BlendMode.y == 1,
        uTexture, uTexture2, uTexture3, uTexture4,
        uTexture5, uTexture6, uTexture7, uTexture8, uTexture9,
        vPosition.xyz, vNormal, vTexCoord, vTexCoord2, vTexCoord3, vTexCoord4,
        vColor2,
        vColorSecond,
        matDiffuse, spec, emissive, finalOpacity,
        doDiscard
    );

    if (doDiscard)
        discard;

    InteriorLightParam intLight;
    intLight.uInteriorAmbientColorAndApplyInteriorLight = vWmoAmbient;
    intLight.uInteriorDirectColorAndApplyExteriorLight = vec4(0, 0, 0, 1.0f);

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            UseLitColor_EnableAlpha_PixelShader_BlendMode.x == 1,
            vColor.w,
            scene,
            intLight,
            vec3(0.0) /*accumLight*/,
            vColor.rgb,
            spec, /* specular */
            emissive
        ),
        finalOpacity
    );

    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz,
        vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, UseLitColor_EnableAlpha_PixelShader_BlendMode.w);
    finalColor.a = 1.0;

    outputColor = finalColor;
}
