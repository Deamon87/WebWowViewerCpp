precision highp float;
precision highp int;

#ifdef TRUE_OPAQUE
layout(early_fragment_tests) in;
#endif

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec2 vTexcoord0;
layout (location = 3) in vec2 vTexcoord1;
layout (location = 4) in vec2 vTexcoord2;
layout (location = 5) in float alphaCutoff;

#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"
#include "../../common/commonUboSceneData.glsl"

//Individual meshes
layout (std140, set = 1, binding = 0) uniform meshWideBlockPS {
    vec4 uAlphaTest_alphaMult_colorMult;
    ivec4 uPixelShaderBlendModev;
};

layout (set = 2, binding = 0) uniform sampler2D uTexture;
layout (set = 2, binding = 1) uniform sampler2D uTexture2;
layout (set = 2, binding = 2) uniform sampler2D uTexture3;

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#else
layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outSpecular;
layout(location = 2) out vec4 outNormal;
#endif


void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;
    vec4 tex2 = texture(uTexture2, vTexcoord1).rgba;
    vec4 tex3 = texture(uTexture3, vTexcoord2).rgba;

    float uAlphaTest = uAlphaTest_alphaMult_colorMult.x;
    float alphaMult = uAlphaTest_alphaMult_colorMult.y;
    float colorMult = uAlphaTest_alphaMult_colorMult.z;

    if (tex.a < uAlphaTest)
        discard;

    vec3 matDiffuse = (tex * vColor).rgb;
    float opacity = tex.a * vColor.a;

    int uNonOptPixelShader = uPixelShaderBlendModev.x;
    if (uNonOptPixelShader == 0) { //particle_mod
        matDiffuse.rgb = vColor.xyz * tex.rgb;
        opacity = tex.a * vColor.a;

    } else if (uNonOptPixelShader == 1) {//particle_2colortex_3alphatex
        vec4 textureMod = tex * tex2;
        float texAlpha = (textureMod.w * tex3.w);
        opacity = texAlpha * vColor.a;

        matDiffuse = vColor.xyz * textureMod.rgb;
    } else if (uNonOptPixelShader == 2) { //particle_3colortex_3alphatex
        vec4 textureMod = tex * tex2 * tex3;
        float texAlpha = (textureMod.w);

        matDiffuse = vColor.xyz * textureMod.rgb;
        opacity = texAlpha * vColor.a;

    } else if (uNonOptPixelShader == 3) { //Particle_3ColorTex_3AlphaTex_UV
        //TODO: incorrect implementation, because the original shader is too complicated
        vec4 textureMod = tex * tex2 * tex3;
        float texAlpha = (textureMod.w);

        matDiffuse = vColor.xyz * textureMod.rgb;
        opacity = texAlpha * vColor.a;
    } else if (uNonOptPixelShader == 4) { //Refraction
        discard;
        float t0_973 = tex.x;
        float t1_978 = tex2.y;
        float t2_983 = tex3.z;
        float textureMod_986 = (((t0_973 * t1_978) * t2_983) * 4.0);
        float depthScale_991 = (1.0 - clamp((vPosition.z * 0.00999999978), 0, 1));
        float textureMod_992 = (textureMod_986 * depthScale_991);
        float height_995 = (textureMod_992 * vColor.x);
        float alpha_997 = (textureMod_992 * vColor.w);

        matDiffuse = vec3(height_995, 0.0, 0.0);
        opacity = alpha_997;
    }

    matDiffuse = matDiffuse.rgb * colorMult;
    opacity = opacity * alphaMult;

    if (opacity < uAlphaTest)
        discard;

    if (opacity < alphaCutoff)
        discard;

    //    vec3 sunDir =
    //        mix(
    //            scene.uInteriorSunDir,
    //            scene.extLight.uExteriorDirectColorDir,
    //            interiorExteriorBlend.x
    //        )
    //        .xyz;
    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;

#ifndef DEFERRED
    vec4 finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, vec4(matDiffuse, opacity), scene.uViewUpSceneTime.xyz, vPosition.xyz, sunDir.xyz, uPixelShaderBlendModev.y);
#endif

#ifndef DEFERRED
    outColor = finalColor;
#else
    outAlbedo = vec4(matDiffuse.xyz, 0.0);
    outNormal = vec4(0,0,1.0, 0.0);
    outSpecular = vec4(0.0);
#endif
}