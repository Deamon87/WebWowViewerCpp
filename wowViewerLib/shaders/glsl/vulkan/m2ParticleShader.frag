#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vTexcoord0;
layout(location = 3) in vec2 vTexcoord1;
layout(location = 4) in vec2 vTexcoord2;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

//Individual meshes
layout(std140, binding=4) uniform meshWideBlockPS {
    vec4 uAlphaTestv;
    ivec4 uPixelShaderBlendModev;
};

layout(set=1,binding=5) uniform sampler2D uTexture;
layout(set=1,binding=6) uniform sampler2D uTexture2;
layout(set=1,binding=7) uniform sampler2D uTexture3;

layout(location = 0) out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;
    vec4 tex2 = texture(uTexture2, vTexcoord1).rgba;
    vec4 tex3 = texture(uTexture3, vTexcoord2).rgba;

    float uAlphaTest = uAlphaTestv.x;

    if(tex.a < uAlphaTest)
        discard;

    vec4 finalColor = vec4((tex * vColor ).rgb, tex.a*vColor.a );
    int uNonOptPixelShader = uPixelShaderBlendModev.x;
    if (uNonOptPixelShader == 0) { //particle_mod
        vec3 matDiffuse = vColor.xyz * tex.rgb;

        finalColor = vec4(matDiffuse.rgb, tex.a*vColor.a);
    } else if (uNonOptPixelShader == 1) {//particle_2colortex_3alphatex
        vec4 textureMod = tex*tex2;
        float texAlpha = (textureMod.w * tex3.w);
        float opacity = texAlpha*vColor.a;


        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uNonOptPixelShader == 2) { //particle_3colortex_3alphatex
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;


        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uNonOptPixelShader == 3) { //Particle_3ColorTex_3AlphaTex_UV
        //TODO: incorrect implementation, because the original shader is too complicated
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;

        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
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
        finalColor = vec4(height_995, 0.0, 0.0, alpha_997);
    }

    if(finalColor.a < uAlphaTest)
        discard;

//    vec3 sunDir =
//        mix(
//            scene.uInteriorSunDir,
//            scene.extLight.uExteriorDirectColorDir,
//            interiorExteriorBlend.x
//        )
//        .xyz;
    vec3 sunDir =scene.extLight.uExteriorDirectColorDir.xyz;

    finalColor = makeFog(fogData, finalColor, vPosition.xyz, sunDir.xyz, uPixelShaderBlendModev.y);

    outputColor.rgba = finalColor ;
}
