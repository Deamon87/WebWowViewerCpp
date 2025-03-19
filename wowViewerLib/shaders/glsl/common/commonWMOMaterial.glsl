#ifndef COMMON_WMO_MATERIAL
#define COMMON_WMO_MATERIAL

#include "commonLightFunctions.glsl"
#include "commonFunctions.glsl"

#ifndef MAX_WMO_GROUPS
#define MAX_WMO_GROUPS 512
#endif

struct WmoVertMeshWide {
    ivec4 VertexShader_UseLitColors;
    vec4 translationSpeedXY;
};

struct WmoFragMeshWide {
    ivec4 UseLitColor_EnableAlpha_PixelShader_BlendMode;
    vec4 FogColor_AlphaTest;
};

struct WmoInteriorBlockData {
    vec4 uAmbientColorAndIsExteriorLit;
    vec4 uHorizontAmbientColor;
    vec4 uGroundAmbientColor;
};

void caclWMOFragMat(in int pixelShader, bool enableAlpha,
    in sampler2D s_texture, in sampler2D s_texture2, in sampler2D s_texture3, in sampler2D s_texture4, in sampler2D s_texture5,
    in sampler2D s_texture6, in sampler2D s_texture7, in sampler2D s_texture8, in sampler2D s_texture9,
    in vec3 vertexPosInView,
    in vec3 normalInView,
    in vec2 vTexCoord,
    in vec2 vTexCoord2,
    in vec2 vTexCoord3,
    in vec2 vTexCoord4,
    in vec4 vColor2,

    in vec4 vColorSecond,
    out vec3 matDiffuse, out vec3 specular, out vec3 emissive, out float finalOpacity, out bool doDiscard)
{
    matDiffuse = vec3(0.0);
    specular = vec3(0.0);
    emissive = vec3(0.0);
    finalOpacity = 1.0;

    vec4 tex = texture(s_texture, vTexCoord).rgba ;
    vec4 tex2 = texture(s_texture2, vTexCoord2).rgba;
    vec4 tex3 = texture(s_texture3, vTexCoord3).rgba;

    doDiscard = false;
    if (enableAlpha) {
        if ((tex.a - 0.501960814) < 0.0) {
            doDiscard = true;
            return;
        }
    }
    //        float distFade_1070 = clamp(((in_vpos.z * pc_miscParams.z) + pc_miscParams.w), 0, 1);
    float distFade = 1.0;

    switch (pixelShader) {
        case (-1) : {
            matDiffuse = tex.rgb * tex2.rgb;
            finalOpacity = tex.a;
            break;
        }
        case (0): { //MapObjDiffuse
            matDiffuse = tex.rgb;
            finalOpacity = tex.a;
            break;
        }
        case (1) : { //MapObjSpecular
            matDiffuse = tex.rgb;
            specular = calcSpec(tex.a);

            finalOpacity = tex.a;
            break;
        }
        case (2) : { //MapObjMetal
            matDiffuse = tex.rgb ;
            specular = calcSpec(((tex * 4.0) * tex.a).x);

            finalOpacity = tex.a;
            break;
        }
        case (3): { //MapObjEnv
            matDiffuse = tex.rgb ;

            emissive = tex2.rgb * tex.a * distFade;
            finalOpacity = 1.0;
            break;
        }
        case (4) : { //MapObjOpaque

            matDiffuse = tex.rgb ;
            finalOpacity = 1.0;
            break;
        }
        case (5) : { //MapObjEnvMetal
            matDiffuse = tex.rgb ;
            emissive = (((tex.rgb * tex.a) * tex2.rgb) * distFade);

            finalOpacity = 1.0;
            break;
        }
        case (6): { //MapObjTwoLayerDiffuse
            vec3 layer1 = tex.rgb;
            vec3 layer2 = mix(layer1, tex2.rgb, tex2.a);
            matDiffuse = mix(layer2, layer1, vColor2.a);

            finalOpacity = tex.a;
            break;
        }
        case (7): { //MapObjTwoLayerEnvMetal
            vec4 colorMix = mix(tex, tex2, 1.0 - vColor2.a);

            matDiffuse = colorMix.rgb ;
            emissive = (colorMix.rgb * colorMix.a) * tex3.rgb * distFade;

            finalOpacity = tex.a;
            break;
        }
        case (8): { //MapObjTwoLayerTerrain
            vec3 layer1 = tex.rgb;
            vec3 layer2 = tex2.rgb;

            matDiffuse = mix(layer2, layer1, vColor2.a);
            specular = calcSpec(tex2.a * (1.0 - vColor2.a));

            finalOpacity = tex.a;
            break;
        }
        case (9): { //MapObjDiffuseEmissive
            matDiffuse = tex.rgb ;
            emissive = tex2.rgb * tex2.a * vColor2.a;

            finalOpacity = tex.a;
            break;
        }
        case (10): { //MapObjMaskedEnvMetal
            float mixFactor = clamp((tex3.a * vColor2.a), 0.0, 1.0);
            matDiffuse =
                mix(mix(((tex.rgb * tex2.rgb) * 2.0), tex3.rgb, mixFactor), tex.rgb, tex.a);

            finalOpacity = tex.a;
            break;
        }
        case (11): { //MapObjEnvMetalEmissive
            matDiffuse = tex.rgb ;
            emissive =
                (
                    ((tex.rgb * tex.a) * tex2.rgb) +
                    ((tex3.rgb * tex3.a) * vColor2.a)
                );
            finalOpacity = tex.a;
            break;
        }
        case (12): { //MapObjTwoLayerDiffuseOpaque
            matDiffuse = mix(tex2.rgb, tex.rgb, vColor2.a);

            finalOpacity = 1.0;
            break;
        }
        case (13): { //MapObjTwoLayerDiffuseEmissive
            vec3 t1diffuse = (tex2.rgb * (1.0 - tex2.a));

            matDiffuse = mix(t1diffuse, tex.rgb, vColor2.a);

            emissive = (tex2.rgb * tex2.a) * (1.0 - vColor2.a);

            finalOpacity = tex.a;
            break;
        }
        case (14): { //MapObjAdditiveMaskedEnvMetal
            matDiffuse = mix(
                (tex.rgb * tex2.rgb * 2.0) + (tex3.rgb * clamp(tex3.a * vColor2.a, 0.0, 1.0)),
                tex.rgb,
                vec3(tex.a)
            );
            finalOpacity = 1.0;
            break;
        }
        case (15): { //MapObjTwoLayerDiffuseMod2x
            vec3 layer1 = tex.rgb;
            vec3 layer2 = mix(layer1, tex2.rgb, vec3(tex2.a));
            vec3 layer3 = mix(layer2, layer1, vec3(vColor2.a));

            matDiffuse = layer3 * tex3.rgb * 2.0;
            finalOpacity = tex.a;
            break;
        }
        case(16): { //MapObjTwoLayerDiffuseMod2xNA
            vec3 layer1 = ((tex.rgb * tex2.rgb) * 2.0);

            matDiffuse = mix(tex.rgb, layer1, vec3(vColor2.a)) ;
            finalOpacity = tex.a;
            break;
        }
        case(17): { //MapObjTwoLayerDiffuseAlpha
            vec3 layer1 = tex.rgb;
            vec3 layer2 = mix(layer1, tex2.rgb, vec3(tex2.a));
            vec3 layer3 = mix(layer2, layer1, vec3(tex3.a));

            matDiffuse = ((layer3 * tex3.rgb) * 2.0);
            finalOpacity = tex.a;
            break;
        }
        case (18): { //MapObjLod
            matDiffuse = tex.rgb ;
            finalOpacity = tex.a;
            break;
        }
        case (19): { //MapObjParallax
            vec4 tex_6 = texture(s_texture6, vTexCoord2).rgba;

            mat3 TBN = contangent_frame(normalInView, -vertexPosInView, vTexCoord2);

            float cosAlpha = dot(normalize(vertexPosInView.xyz), normalInView);
            vec3 viewTS = TBN * normalize(-vertexPosInView.xyz);
            vec2 viewTSNorm = (viewTS.xy / viewTS.z);

            float bumpOffset_backLayer = tex_6.r;
            float bumpOffset_middleLayer = 0.5f * bumpOffset_backLayer;
            vec4 tex_4 = texture(s_texture4, vTexCoord2 - (viewTSNorm * bumpOffset_middleLayer * 0.5)).rgba;
            vec4 tex_5 = texture(s_texture5, vTexCoord3 - (viewTSNorm * bumpOffset_middleLayer * 0.5)).rgba;
            vec4 tex_3 = texture(s_texture3, vTexCoord2).rgba;

            vec3 diffuseResult = tex_5.rgb + tex_4.rgb * tex_4.a;
            vec3 diffuseResult2 = (tex_3.rgb - diffuseResult) * tex_6.g + diffuseResult;
            vec3 mix3 = tex_3.rgb * tex_6.b + (tex_5.rgb * tex_5.a * (1.0 - tex3.b));

            vec4 tex_2 = texture(s_texture3, vColorSecond.bg).rgba;
            vec3 tex_2_mult = tex_2.rgb * tex_2.a;

            vec3 fakeSpecularResult;
            if (vColor2.a> 0.0)
            {
                vec4 tex = texture(s_texture, vTexCoord).rgba;
                matDiffuse = mix(diffuseResult2, tex.rgb, vColor2.a);
                fakeSpecularResult = ((tex.rgb * tex.a) - tex_2_mult.rgb) * vColor2.a + tex_2_mult.rgb;
            } else {
                fakeSpecularResult = tex_2_mult;
                matDiffuse = diffuseResult2;
            }

            vec3 envResult = fakeSpecularResult * tex_2.rgb;
            emissive = (mix3 - (mix3 * vColor2.a)) + (envResult);
            break;
        }
        case (20): { //MapObjUnkShader
            vec4 tex_1 = texture(s_texture, posToTexCoord(vertexPosInView.xyz, normalInView.xyz));
            vec4 tex_2 = texture(s_texture2, vTexCoord);
            vec4 tex_3 = texture(s_texture3, vTexCoord2);
            vec4 tex_4 = texture(s_texture4, vTexCoord3);
            vec4 tex_5 = texture(s_texture5, vTexCoord4);

            vec4 tex_6 = texture(s_texture6, vTexCoord).rgba;
            vec4 tex_7 = texture(s_texture7, vTexCoord2).rgba;
            vec4 tex_8 = texture(s_texture8, vTexCoord3).rgba;
            vec4 tex_9 = texture(s_texture9, vTexCoord4).rgba;

            float secondColorSum = dot(vColorSecond.bgr, vec3(1.0));
            vec4 weights = vec4(vColorSecond.bgr, 1.0 - clamp(secondColorSum, 0.0, 1.0));
            vec4 heights = max(vec4(tex_6.a, tex_7.a, tex_8.a, tex_9.a), 0.004);
            vec4 alphaVec =  weights * heights;
            float weightsMax = max(alphaVec.r, max(alphaVec.g, max(alphaVec.r, alphaVec.a)));
            vec4 alphaVec2 = (1.0 - clamp(vec4(weightsMax) - alphaVec, 0.0, 1.0));
            alphaVec2 = alphaVec2 * alphaVec;

            vec4 alphaVec2Normalized = alphaVec2 * (1.0 / dot(alphaVec2, vec4(1.0)));

            vec4 texMixed = tex_2 * alphaVec2Normalized.r +
                            tex_3 * alphaVec2Normalized.g +
                            tex_4 * alphaVec2Normalized.b +
                            tex_5 * alphaVec2Normalized.a;

            emissive = (texMixed.w * tex_1.rgb) * texMixed.rgb;
            vec3 ambientOcclusionColor = vec3(0,0,0); // TODO: when ambient occlusion is ready, add it here
            matDiffuse = mix(texMixed.rgb, ambientOcclusionColor,  vColorSecond.a);
            break;
        }
    }
}

float makeWmoUVAnim(float sceneTimeInSec, float value) {
    if (value == 0.0f) return 0.0f;

    float animPeriod = 1000.0f / value;
    if (animPeriod > 0.0f) {
        return (int(sceneTimeInSec) % int(animPeriod)) / animPeriod;
    } else {
        return 1.0f - (int(sceneTimeInSec) % int(-animPeriod)) / -animPeriod;
    }
}

vec2 makeWmoUVAnimVec(float sceneTimeInSec, vec2 value){
    return vec2(
        makeWmoUVAnim(sceneTimeInSec,value.x),
        makeWmoUVAnim(sceneTimeInSec,value.y)
    );
}

void calcWMOVertMat(in int vertexShader,
                    in vec3 vertexPosInView,
                    in vec3 normalInView,
                    in vec2 aTexCoord, in vec2 aTexCoord2, in vec2 aTexCoord3,
                    float sceneTime,
                    in vec4 uvTransl,
                    out vec2 vTexCoord, out vec2 vTexCoord2, out vec2 vTexCoord3) {

    float sceneTimeInSec = sceneTime;
    vec2 texCoordAnim = aTexCoord + makeWmoUVAnimVec(sceneTimeInSec, uvTransl.xy);
    vec2 texCoordAnim2 = aTexCoord2 + makeWmoUVAnimVec(sceneTimeInSec, uvTransl.zw);

    if ( vertexShader == -1 ) {
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2;
        vTexCoord3 = aTexCoord3;
    } else if (vertexShader == 0) { //MapObjDiffuse_T1
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 1) { //MapObjDiffuse_T1_Refl
        vTexCoord = texCoordAnim;
        vTexCoord2 = reflect(normalize(vertexPosInView.xyz), normalInView).xy;
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 2) { //MapObjDiffuse_T1_Env_T2
        vTexCoord = texCoordAnim;
        vTexCoord2 = posToTexCoord(vertexPosInView.xyz, normalInView);
        vTexCoord3 = texCoordAnim2;
    } else if (vertexShader == 3) { //MapObjSpecular_T1
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 4) { //MapObjDiffuse_Comp
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 5) { //MapObjDiffuse_Comp_Refl
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2;
        vTexCoord3 = reflect(normalize(vertexPosInView.xyz), normalInView).xy;
    } else if (vertexShader == 6) { //MapObjDiffuse_Comp_Terrain
        vTexCoord = texCoordAnim;
        vTexCoord2 = vertexPosInView.xy * 0.239999995;
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 7) { //MapObjDiffuse_CompAlpha
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2;
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 8) { //MapObjParallax
        vTexCoord = texCoordAnim;
        vTexCoord2 = texCoordAnim2;
        vTexCoord3 = aTexCoord3;
    }
}
#endif