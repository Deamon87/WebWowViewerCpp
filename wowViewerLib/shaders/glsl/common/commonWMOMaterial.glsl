#ifndef COMMON_WMO_MATERIAL
#define COMMON_WMO_MATERIAL

#include "commonLightFunctions.glsl"
#include "commonFunctions.glsl"

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
            vec2 dotResult = (TBN * (normalize(-vertexPosInView.xyz) / cosAlpha)).xy;

            vec4 tex_4 = texture(s_texture4, vTexCoord2 - (dotResult * tex_6.r * 0.25)).rgba;
            vec4 tex_5 = texture(s_texture5, vTexCoord3 - (dotResult * tex_6.r * 0.25)).rgba;
            vec4 tex_3 = texture(s_texture3, vTexCoord2).rgba;

            vec3 mix1 = tex_5.rgb + tex_4.rgb * tex_4.a;
            vec3 mix2 = (tex_3.rgb - mix1) * tex_6.g + mix1;
            vec3 mix3 = tex_3.rgb * tex_6.b + (tex_5.rgb * tex_5.a * (1.0 - tex3.b));

            vec4 tex_2 = texture(s_texture3, vColorSecond.bg).rgba;
            vec3 tex_2_mult = tex_2.rgb * tex_2.a;

            vec3 emissive_component;
            if (vColor2.a> 0.0)
            {
                vec4 tex = texture(s_texture, vTexCoord).rgba;
                matDiffuse = (tex.rgb - mix2 ) * vColor2.a + mix2;
                emissive_component = ((tex.rgb * tex.a) - tex_2_mult.rgb) * vColor2.a + tex_2_mult.rgb;
            } else {
                emissive_component = tex_2_mult;
                matDiffuse = mix2;
            }

            emissive = (mix3 - (mix3 * vColor2.a)) + (emissive_component * tex_2.rgb);
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
            break;
        }
    }
}

void calcWMOVertMat(in int vertexShader,
                    in vec3 vertexPosInView,
                    in vec3 normalInView,
                    in vec2 aTexCoord, in vec2 aTexCoord2, in vec2 aTexCoord3,
                    out vec2 vTexCoord, out vec2 vTexCoord2, out vec2 vTexCoord3) {
    if ( vertexShader == -1 ) {
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2;
        vTexCoord3 = aTexCoord3;
    } else if (vertexShader == 0) { //MapObjDiffuse_T1
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 1) { //MapObjDiffuse_T1_Refl
        vTexCoord = aTexCoord;
        vTexCoord2 = reflect(normalize(vertexPosInView.xyz), normalInView).xy;
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 2) { //MapObjDiffuse_T1_Env_T2
        vTexCoord = aTexCoord;
        vTexCoord2 = posToTexCoord(vertexPosInView.xyz, normalInView);
        vTexCoord3 = aTexCoord3;
    } else if (vertexShader == 3) { //MapObjSpecular_T1
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 4) { //MapObjDiffuse_Comp
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 5) { //MapObjDiffuse_Comp_Refl
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2;
        vTexCoord3 = reflect(normalize(vertexPosInView.xyz), normalInView).xy;
    } else if (vertexShader == 6) { //MapObjDiffuse_Comp_Terrain
        vTexCoord = aTexCoord;
        vTexCoord2 = vertexPosInView.xy * 0.239999995;
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 7) { //MapObjDiffuse_CompAlpha
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2;
        vTexCoord3 = aTexCoord3; //not used
    } else if (vertexShader == 8) { //MapObjParallax
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2;
        vTexCoord3 = aTexCoord3;
    }
}
#endif