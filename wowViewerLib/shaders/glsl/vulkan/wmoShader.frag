#version 450
#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#define FRAGMENT_SHADER

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonFunctions.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord3;
layout(location=3) in vec2 vTexCoord4;
layout(location=4) in vec4 vColor;
layout(location=5) in vec4 vColor2;
layout(location=6) in vec4 vColorSecond;
layout(location=7) in vec4 vPosition;
layout(location=8) in vec3 vNormal;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

layout(std140, set=0, binding=3) uniform modelWideBlockPS {
    InteriorLightParam intLight;

};

layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    ivec4 UseLitColor_EnableAlpha_PixelShader_BlendMode;
    vec4 FogColor_AlphaTest;
};

vec3 Slerp(vec3 p0, vec3 p1, float t)
{
    float dotp = dot(normalize(p0), normalize(p1));
    if ((dotp > 0.9999) || (dotp<-0.9999))
    {
        if (t<=0.5)
        return p0;
        return p1;
    }
    float theta = acos(dotp);
    vec3 P = ((p0*sin((1-t)*theta) + p1*sin(t*theta)) / sin(theta));

    return P;
}

vec3 calcSpec(float texAlpha) {
    vec3 normal = normalize(vNormal);

    vec3 sunDir = vec3(0);
    vec3 sunColor = vec3(0);

    if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
        sunDir = -(scene.extLight.uExteriorDirectColorDir.xyz);
        sunColor = scene.extLight.uExteriorDirectColor.xyz;
    }

    if (intLight.uInteriorAmbientColorAndApplyInteriorLight.w > 0) {
        sunDir = -(scene.uInteriorSunDir.xyz);
        sunColor = intLight.uInteriorDirectColorAndApplyExteriorLight.xyz;

        if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
            sunDir = Slerp(sunDir, -(scene.extLight.uExteriorDirectColorDir.xyz), vColor.a);
            sunColor = mix(sunColor, scene.extLight.uExteriorDirectColor.xyz, vColor.a);
        }
    }

    vec3 t849 = normalize((sunDir + normalize(-(vPosition.xyz))));
    float dirAtten_956 = clamp(dot(normal, sunDir), 0, 1);
    float spec = (1.25 * pow(clamp(dot(normal, t849), 0, 1), 8.0));
    vec3 specTerm = ((((vec3(mix(pow((1.0 - clamp(dot(sunDir, t849), 0, 1)), 5.0), 1.0, texAlpha)) * spec) * sunColor) * dirAtten_956));
//    float distFade = clamp(((vPosition * pc_miscParams.z) + pc_miscParams.w), 0, 1);
    float distFade = 1.0;
    specTerm = (specTerm * distFade);
    return specTerm;
}

layout(set=1, binding=5) uniform sampler2D uTexture;
layout(set=1, binding=6) uniform sampler2D uTexture2;
layout(set=1, binding=7) uniform sampler2D uTexture3;
layout(set=1, binding=8) uniform sampler2D uTexture4;
layout(set=1, binding=9) uniform sampler2D uTexture5;
layout(set=1, binding=10) uniform sampler2D uTexture6;
layout(set=1, binding=11) uniform sampler2D uTexture7;
layout(set=1, binding=12) uniform sampler2D uTexture8;
layout(set=1, binding=13) uniform sampler2D uTexture9;

layout (location = 0) out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexCoord).rgba ;
    vec4 tex2 = texture(uTexture2, vTexCoord2).rgba;
    vec4 tex3 = texture(uTexture3, vTexCoord3).rgba;

    if (UseLitColor_EnableAlpha_PixelShader_BlendMode.y == 1) {
        if ((tex.a - 0.501960814) < 0.0) {
            discard;
        }
    }

    int uPixelShader = UseLitColor_EnableAlpha_PixelShader_BlendMode.z;
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 matDiffuse = vec3(0.0);
    vec3 spec = vec3(0.0);
    vec3 emissive = vec3(0.0);
    float finalOpacity = 0.0;

    //        float distFade_1070 = clamp(((in_vpos.z * pc_miscParams.z) + pc_miscParams.w), 0, 1);
    float distFade = 1.0;

    if ( uPixelShader == -1 ) {
        matDiffuse = tex.rgb * tex2.rgb;
        finalOpacity = tex.a;
    } else if (uPixelShader == 0) { //MapObjDiffuse

        matDiffuse = tex.rgb;
        finalOpacity = tex.a;

    } else if (uPixelShader == 1) { //MapObjSpecular

        matDiffuse = tex.rgb;
        spec = calcSpec(tex.a);

        finalOpacity = tex.a;
    } else if (uPixelShader == 2) { //MapObjMetal

        matDiffuse = tex.rgb ;
        spec = calcSpec(((tex * 4.0) * tex.a).x);

        finalOpacity = tex.a;
    } else if (uPixelShader == 3) { //MapObjEnv
        matDiffuse = tex.rgb ;

        emissive = tex2.rgb * tex.a * distFade;
        finalOpacity = 1.0;

    } else if (uPixelShader == 4) { //MapObjOpaque

        matDiffuse = tex.rgb ;
        finalOpacity = 1.0;

    } else if (uPixelShader == 5) { //MapObjEnvMetal

        matDiffuse = tex.rgb ;
        emissive = (((tex.rgb * tex.a) * tex2.rgb) * distFade);

        finalOpacity = 1.0;

    } else if (uPixelShader == 6) { //MapObjTwoLayerDiffuse

        vec3 layer1 = tex.rgb;
        vec3 layer2 = mix(layer1, tex2.rgb, tex2.a);
        matDiffuse = mix(layer2, layer1, vColor2.a);

        finalOpacity = tex.a;
    } else if (uPixelShader == 7) { //MapObjTwoLayerEnvMetal

        vec4 colorMix = mix(tex, tex2, 1.0 - vColor2.a);

        matDiffuse = colorMix.rgb ;
        emissive = (colorMix.rgb * colorMix.a) * tex3.rgb * distFade;

        finalOpacity = tex.a;

    } else if (uPixelShader == 8) { //MapObjTwoLayerTerrain

        vec3 layer1 = tex.rgb;
        vec3 layer2 = tex2.rgb;

        matDiffuse = mix(layer2, layer1, vColor2.a);
        spec = calcSpec(tex2.a * (1.0 - vColor2.a));

        finalOpacity = tex.a;

    } else if (uPixelShader == 9) { //MapObjDiffuseEmissive

        matDiffuse = tex.rgb ;
        emissive = tex2.rgb * tex2.a * vColor2.a;

        finalOpacity = tex.a;

    } else if (uPixelShader == 10) { //MapObjMaskedEnvMetal

        float mixFactor = clamp((tex3.a * vColor2.a), 0.0, 1.0);
        matDiffuse =
            mix(mix(((tex.rgb * tex2.rgb) * 2.0), tex3.rgb, mixFactor), tex.rgb, tex.a);

        finalOpacity = tex.a;

    } else if (uPixelShader == 11) { //MapObjEnvMetalEmissive
        matDiffuse = tex.rgb ;
        emissive =
            (
                ((tex.rgb * tex.a) * tex2.rgb) +
                ((tex3.rgb * tex3.a) * vColor2.a)
            );
        finalOpacity = tex.a;

    } else if (uPixelShader == 12) { //MapObjTwoLayerDiffuseOpaque
        matDiffuse = mix(tex2.rgb, tex.rgb, vColor2.a);

        finalOpacity = 1.0;
    } else if (uPixelShader == 13) { //MapObjTwoLayerDiffuseEmissive
        vec3 t1diffuse = (tex2.rgb * (1.0 - tex2.a));

        matDiffuse = mix(t1diffuse, tex.rgb, vColor2.a);

        emissive = (tex2.rgb * tex2.a) * (1.0 - vColor2.a);

        finalOpacity = tex.a;
    } else if (uPixelShader == 14) { //MapObjAdditiveMaskedEnvMetal
        matDiffuse = mix(
            (tex.rgb * tex2.rgb * 2.0) + (tex3.rgb * clamp(tex3.a * vColor2.a, 0.0, 1.0)),
            tex.rgb,
            vec3(tex.a)
        );
        finalOpacity = 1.0;
    } else if (uPixelShader == 15) { //MapObjTwoLayerDiffuseMod2x
        vec3 layer1 = tex.rgb;
        vec3 layer2 = mix(layer1, tex2.rgb, vec3(tex2.a));
        vec3 layer3 = mix(layer2, layer1, vec3(vColor2.a));

        matDiffuse = layer3 * tex3.rgb * 2.0;
        finalOpacity = tex.a;
    } if (uPixelShader == 16) { //MapObjTwoLayerDiffuseMod2xNA
        vec3 layer1 = ((tex.rgb * tex2.rgb) * 2.0);

        matDiffuse = mix(tex.rgb, layer1, vec3(vColor2.a)) ;
        finalOpacity = tex.a;
    } if (uPixelShader == 17) { //MapObjTwoLayerDiffuseAlpha
        vec3 layer1 = tex.rgb;
        vec3 layer2 = mix(layer1, tex2.rgb, vec3(tex2.a));
        vec3 layer3 = mix(layer2, layer1, vec3(tex3.a));

        matDiffuse = ((layer3 * tex3.rgb) * 2.0);
        finalOpacity = tex.a;
    } if (uPixelShader == 18) { //MapObjLod
        matDiffuse = tex.rgb ;
        finalOpacity = tex.a;
    } if (uPixelShader == 19) { //MapObjParallax
        vec4 tex_6 = texture(uTexture6, vTexCoord2).rgba;

        mat3 TBN = contangent_frame(vNormal.xyz, -vPosition.xyz, vTexCoord2);

        float cosAlpha = dot(normalize(vPosition.xyz), vNormal.xyz);
        vec2 dotResult = (TBN * (normalize(-vPosition.xyz) / cosAlpha)).xy;

        vec4 tex_4 = texture(uTexture4, vTexCoord2 - (dotResult * tex_6.r * 0.25)).rgba;
        vec4 tex_5 = texture(uTexture5, vTexCoord3 - (dotResult * tex_6.r * 0.25)).rgba;
        vec4 tex_3 = texture(uTexture3, vTexCoord2).rgba;

        vec3 mix1 = tex_5.rgb + tex_4.rgb * tex_4.a;
        vec3 mix2 = (tex_3.rgb - mix1) * tex_6.g + mix1;
        vec3 mix3 = tex_3.rgb * tex_6.b + (tex_5.rgb * tex_5.a * (1.0 - tex3.b));

        vec4 tex_2 = texture(uTexture3, vColorSecond.bg).rgba;
        vec3 tex_2_mult = tex_2.rgb * tex_2.a;

        vec3 emissive_component;
        if (vColor2.a> 0.0)
        {
            vec4 tex = texture(uTexture, vTexCoord).rgba;
            matDiffuse = (tex.rgb - mix2 ) * vColor2.a + mix2;
            emissive_component = ((tex.rgb * tex.a) - tex_2_mult.rgb) * vColor2.a + tex_2_mult.rgb;
        } else {
            emissive_component = tex_2_mult;
            matDiffuse = mix2;
        }

        emissive = (mix3 - (mix3 * vColor2.a)) + (emissive_component * tex_2.rgb);

    } if (uPixelShader == 20) { //MapObjUnkShader
        vec4 tex_1 = texture(uTexture, posToTexCoord(vPosition.xyz, vNormal.xyz));
        vec4 tex_2 = texture(uTexture2, vTexCoord);
        vec4 tex_3 = texture(uTexture3, vTexCoord2);
        vec4 tex_4 = texture(uTexture4, vTexCoord3);
        vec4 tex_5 = texture(uTexture5, vTexCoord4);

        vec4 tex_6 = texture(uTexture6, vTexCoord).rgba;
        vec4 tex_7 = texture(uTexture7, vTexCoord2).rgba;
        vec4 tex_8 = texture(uTexture8, vTexCoord3).rgba;
        vec4 tex_9 = texture(uTexture9, vTexCoord4).rgba;

        float secondColorSum = dot(vColorSecond.bgr, vec3(1.0));
        vec4 alphaVec = max(vec4(tex_6.a, tex_7.a, tex_8.a, tex_9.a), 0.004) * vec4(vColorSecond.bgr, 1.0 - clamp(secondColorSum, 0.0, 1.0));
        float maxAlpha = max(alphaVec.r, max(alphaVec.g, max(alphaVec.r, alphaVec.a)));
        vec4 alphaVec2 = (1.0 - clamp(vec4(maxAlpha) - alphaVec, 0.0, 1.0));
        alphaVec2 = alphaVec2 * alphaVec;

        vec4 alphaVec2Normalized = alphaVec2 * (1.0 / dot(alphaVec2, vec4(1.0)));

        vec4 texMixed = tex_2 * alphaVec2Normalized.r +
                        tex_3 * alphaVec2Normalized.g +
                        tex_4 * alphaVec2Normalized.b +
                        tex_5 * alphaVec2Normalized.a;

        emissive = (texMixed.w * tex_1.rgb) * texMixed.rgb;
        vec3 diffuseColor = vec3(0,0,0); //<= it's unknown where this color comes from. But it's not MOMT chunk
        matDiffuse = (diffuseColor - texMixed.rgb) * vColorSecond.a + texMixed.rgb;
    }

    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            true,
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

    finalColor = makeFog(fogData, finalColor, vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, UseLitColor_EnableAlpha_PixelShader_BlendMode.w);

//    finalColor.a = 1.0; //do I really need it now?

    outputColor = finalColor;
}
