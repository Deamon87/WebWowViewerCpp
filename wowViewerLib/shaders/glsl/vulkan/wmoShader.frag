#version 450
#extension GL_GOOGLE_include_directive: require

precision highp float;

#include "../common/commonLightFunctions.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord3;
layout(location=3) in vec4 vColor;
layout(location=4) in vec4 vColor2;
layout(location=5) in vec4 vPosition;
layout(location=6) in vec3 vNormal;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
};

layout(std140, set=0, binding=3) uniform modelWideBlockVS {
    mat4 uPlacementMat;
    InteriorLightParam intLight;
};

layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDir_FogStart;
    vec4 uSunColor_uFogEnd;
    vec4 uAmbientLight;
    vec4 uAmbientLight2AndIsBatchA;
    ivec4 UseLitColor_EnableAlpha_PixelShader;
    vec4 FogColor_AlphaTest;
};




layout(set=1, binding=5) uniform sampler2D uTexture;
layout(set=1, binding=6) uniform sampler2D uTexture2;
layout(set=1, binding=7) uniform sampler2D uTexture3;

layout (location = 0) out vec4 outputColor;

//vec3 makeDiffTerm(vec3 matDiffuse) {
//    vec3 currColor;
//    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
//    if (UseLitColor_EnableAlpha_PixelShader.x == 1) {
//        //vec3 viewUp = normalize(vec3(0, 0.9, 0.1));
//        vec3 normalizedN = normalize(vNormal);
//        float nDotL = dot(normalizedN, -(uSunDir_FogStart.xyz));
//        float nDotUp = dot(normalizedN, uViewUp.xyz);
//
//        vec3 precomputed = vColor2.rgb;
//
//        vec3 ambientColor = uAmbientLight.rgb;
//        vec3 directColor = uSunColor_uFogEnd.xyz;
//        if (uAmbientLight2AndIsBatchA.w > 0.0) {
//            ambientColor = mix(uAmbientLight2AndIsBatchA.rgb, uAmbientLight.rgb, vec3(vPosition.w));
//            directColor = mix(vec3(0), directColor.rgb, vec3(vPosition.w));
//        }
//
//        vec3 adjAmbient = (ambientColor.rgb + precomputed);
//        vec3 adjHorizAmbient = (ambientColor.rgb + precomputed);
//        vec3 adjGroundAmbient = (ambientColor.rgb + precomputed);
//
//        if ((nDotUp >= 0.0))
//        {
//            currColor = mix(adjHorizAmbient, adjAmbient, vec3(nDotUp));
//        }
//        else
//        {
//            currColor= mix(adjHorizAmbient, adjGroundAmbient, vec3(-(nDotUp)));
//        }
//
//        vec3 skyColor = (currColor * 1.10000002);
//        vec3 groundColor = (currColor* 0.699999988);
//        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
//        lDiffuse = (directColor * clamp(nDotL, 0.0, 1.0));
//    } else {
//        currColor = vec3 (1.0, 1.0, 1.0) * uAmbientLight.rgb;
//    }
//
//    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
//    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * vec3(0.0);
//    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;
//
////    return matDiffuse * currColor.rgb ;
//}

void main() {
    vec4 tex = texture(uTexture, vTexCoord).rgba ;
    vec4 tex2 = texture(uTexture2, vTexCoord2).rgba;
    vec4 tex3 = texture(uTexture3, vTexCoord3).rgba;


    if (UseLitColor_EnableAlpha_PixelShader.y == 1) {
        if ((tex.a - 0.501960814) < 0.0) {
            discard;
        }
    }

    int uPixelShader = UseLitColor_EnableAlpha_PixelShader.z;
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 matDiffuse = vec3(0.0);
    vec3 env = vec3(0.0);
    float finalOpacity = 0.0;

    if ( uPixelShader == -1 ) {
        matDiffuse = tex.rgb * vColor.rgb + tex2.rgb*vColor2.bgr;
        finalOpacity = tex.a;
    } else if (uPixelShader == 0) { //MapObjDiffuse

        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;

    } else if (uPixelShader == 1) { //MapObjSpecular

        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;

    } else if (uPixelShader == 2) { //MapObjMetal

        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;

    } else if (uPixelShader == 3) { //MapObjEnv

        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        env = tex2.rgb * tex.a;
        finalOpacity = vColor.a;

    } else if (uPixelShader == 4) { //MapObjOpaque

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;

    } else if (uPixelShader == 5) { //MapObjEnvMetal

        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        env = (tex.rgb * tex.a) * tex2.rgb;

        finalOpacity = vColor.a;

    } else if (uPixelShader == 6) { //MapObjTwoLayerDiffuse

        vec3 layer1 = tex.rgb;
        vec3 layer2 = mix(layer1, tex2.rgb, tex2.a);
        matDiffuse = (vColor.rgb * 2.0) * mix(layer2, layer1, vColor2.a);

        finalOpacity = vColor.a;
    } else if (uPixelShader == 7) { //MapObjTwoLayerEnvMetal

        vec4 colorMix = mix(tex2, tex, vColor2.a);

        matDiffuse = colorMix.rgb * (2.0 * vColor.rgb);
        env = (colorMix.rgb * colorMix.a) * tex3.rgb;

        finalOpacity = vColor.a;

    } else if (uPixelShader == 8) { //MapObjTwoLayerTerrain

        vec3 layer1 = tex.rgb;
        vec3 layer2 = tex2.rgb;

        matDiffuse = ((vColor.rgb * 2.0) * mix(layer2, layer1, vColor2.a));
        finalOpacity = vColor.a;

    } else if (uPixelShader == 9) { //MapObjDiffuseEmissive

        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        env = tex2.rgb * tex2.a * vColor2.a;

        finalOpacity = vColor.a;

    } else if (uPixelShader == 10) { //MapObjMaskedEnvMetal

        float mixFactor = clamp((tex3.a * vColor2.a), 0.0, 1.0);
        matDiffuse =
            (vColor.rgb * 2.0) *
            mix(mix(((tex.rgb * tex2.rgb) * 2.0), tex3.rgb, mixFactor), tex.rgb, tex.a);

        finalOpacity = vColor.a;

    } else if (uPixelShader == 11) { //MapObjEnvMetalEmissive
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        env =
            (
                ((tex.rgb * tex.a) * tex2.rgb) +
                ((tex3.rgb * tex3.a) * vColor2.a)
            );
        finalOpacity = vColor.a;

    } else if (uPixelShader == 12) { //MapObjTwoLayerDiffuseOpaque
        matDiffuse =
            (vColor.rgb * 2.0) *
            mix(tex2.rgb, tex.rgb, vColor2.a);

        finalOpacity = vColor.a;
    } else if (uPixelShader == 13) { //MapObjTwoLayerDiffuseEmissive
        vec3 t1diffuse = (tex2.rgb * (1.0 - tex2.a));

        matDiffuse =
            ((vColor.rgb * 2.0) *
            mix(t1diffuse, tex.rgb, vColor2.a));

        env = ((tex2.rgb * tex2.a) * (1.0 - vColor2.a));
        finalOpacity = vColor.a;
    } else if (uPixelShader == 13) { //MapObjTwoLayerDiffuseEmissive
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
    } else if (uPixelShader == 14) { //MapObjAdditiveMaskedEnvMetal
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
    } else if (uPixelShader == 15) { //MapObjTwoLayerDiffuseMod2x
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
    } if (uPixelShader == 16) { //MapObjTwoLayerDiffuseMod2xNA
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
    } if (uPixelShader == 17) { //MapObjTwoLayerDiffuseAlpha
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
    } if (uPixelShader == 18) { //MapObjLod
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
    } if (uPixelShader == 19) { //MapObjParallax
        matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalOpacity = vColor.a;
     }

    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            true,
            scene,
            intLight,
            vec3(0.0) /*accumLight*/,
            vColor2.rgb
        ),
        finalOpacity
    );

    if(finalColor.a < FogColor_AlphaTest.w)
        discard;

//    vec3 fogColor = FogColor_AlphaTest.xyz;
//    float fog_start = uSunDir_FogStart.w;
//    float fog_end = uSunColor_uFogEnd.w;
//    float fog_rate = 1.5;
//    float fog_bias = 0.01;
//
//    //vec4 fogHeightPlane = pc_fog.heightPlane;
//    //float heightRate = pc_fog.color_and_heightRate.w;
//
//    float distanceToCamera = length(vPosition.xyz);
//    float z_depth = (distanceToCamera - fog_bias);
//    float expFog = 1.0 / (exp((max(0.0, (z_depth - fog_start)) * fog_rate)));
//    //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
//    //float heightFog = clamp((height * heightRate), 0, 1);
//    float heightFog = 1.0;
//    expFog = (expFog + heightFog);
//    float endFadeFog = clamp(((fog_end - distanceToCamera) / (0.699999988 * fog_end)), 0.0, 1.0);
//
//    finalColor.rgb = mix(fogColor.rgb, finalColor.rgb, vec3(min(expFog, endFadeFog)));

    finalColor.a = 1.0; //do I really need it now?

    outputColor = finalColor;
}
