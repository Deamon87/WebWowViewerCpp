#include "commonFunctions.glsl"

void calcM2FragMaterial(const in int uPixelShader,
                     in sampler2D texSampler1,
                     in sampler2D texSampler2,
                     in sampler2D texSampler3,
                     in sampler2D texSampler4,
                    const in vec2 inUv1,
                    const in vec2 inUv2,
                    const in vec2 inUv3,
                    const in vec3 meshColor,
                    const in float meshOpacity,
                    const in vec3 texSampleAlpha,
                    const in int blendMode,

                    out vec3 matDiffuse, out vec3 specular,
                    out float finalOpacity, out bool discardThisFragment)
{
    vec4 genericParams[3];
    genericParams[0] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[1] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[2] = vec4( 1.0, 1.0, 1.0, 1.0 );

    vec2 uv1 = inUv1;
    vec2 uv2 = inUv2;
    vec2 uv3 = inUv3;

    if (uPixelShader == 26 || uPixelShader == 27 || uPixelShader == 28) {
        uv2 = uv1;
        uv3 = uv1;
    }

    vec4 tex = texture(texSampler1, uv1).rgba;
    vec4 tex2 = texture(texSampler2, uv2).rgba;
    vec4 tex3 = texture(texSampler3, uv3).rgba;

    float discardAlpha = 1.0;
    bool canDiscard = false;
    matDiffuse = vec3(0.0, 0.0, 0.0);
    specular = vec3(0.0, 0.0, 0.0);

    switch (uPixelShader) {
        case (0): { //Combiners_Opaque
            matDiffuse = meshColor * tex.rgb;
            break;
        }
        case (1): { //Combiners_Mod
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (2): { //Combiners_Opaque_Mod
            matDiffuse = meshColor * tex.rgb * tex2.rgb;
            discardAlpha = tex2.a;
            canDiscard = true;
            break;
        }
        case (3): { //Combiners_Opaque_Mod2x
            matDiffuse = meshColor * tex.rgb * tex2.rgb * 2.0;
            discardAlpha = tex2.a * 2.0;
            canDiscard = true;
            break;
        }
        case (4): { //Combiners_Opaque_Mod2xNA
            matDiffuse = meshColor * tex.rgb * tex2.rgb * 2.0;
            break;
        }
        case (5): { //Combiners_Opaque_Opaque
            matDiffuse = meshColor * tex.rgb * tex2.rgb;
            break;
        }
        case (6): { //Combiners_Mod_Mod
            matDiffuse = meshColor * tex.rgb * tex2.rgb;
            discardAlpha = tex.a * tex2.a;
            canDiscard = true;
            break;
        }
        case (7): { //Combiners_Mod_Mod2x
            matDiffuse = meshColor * tex.rgb * tex2.rgb * 2.0;
            discardAlpha = tex.a * tex2.a * 2.0;
            canDiscard = true;
            break;
        }
        case (8): { //Combiners_Mod_Add
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a + tex2.a;
            canDiscard = true;
            specular = tex2.rgb;
            break;
        }
        case (9): { //Combiners_Mod_Mod2xNA
            matDiffuse = meshColor * tex.rgb * tex2.rgb * 2.0;
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (10): { //Combiners_Mod_AddNA
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a;
            canDiscard = true;
            specular = tex2.rgb;
            break;
        }
        case (11): { //Combiners_Mod_Opaque
            matDiffuse = meshColor * tex.rgb * tex2.rgb;
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (12): { //Combiners_Opaque_Mod2xNA_Alpha
            matDiffuse = meshColor * mix(tex.rgb * tex2.rgb * 2.0, tex.rgb, vec3(tex.a));
            break;
        }
        case (13): { //Combiners_Opaque_AddAlpha
            matDiffuse = meshColor * tex.rgb;
            specular = tex2.rgb * tex2.a;
            break;
        }
        case (14): { //Combiners_Opaque_AddAlpha_Alpha
            matDiffuse = meshColor * tex.rgb;
            specular = tex2.rgb * tex2.a * (1.0 - tex.a);
            break;
        }
        case (15): { //Combiners_Opaque_Mod2xNA_Alpha_Add
            matDiffuse = meshColor * mix(tex.rgb * tex2.rgb * 2.0, tex.rgb, vec3(tex.a));
            specular = tex3.rgb * tex3.a * texSampleAlpha.b;
            break;
        }
        case (16): { //Combiners_Mod_AddAlpha
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a;
            canDiscard = true;
            specular = tex2.rgb * tex2.a;
            break;
        }
        case (17): { //Combiners_Mod_AddAlpha_Alpha
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a + tex2.a * (0.3 * tex2.r + 0.59 * tex2.g + 0.11 * tex2.b);
            canDiscard = true;
            specular = tex2.rgb * tex2.a * (1.0 - tex.a);
            break;
        }
        case (18): { //Combiners_Opaque_Alpha_Alpha
            matDiffuse = meshColor * mix(mix(tex.rgb, tex2.rgb, vec3(tex2.a)), tex.rgb, vec3(tex.a));
            break;
        }
        case (19): { //Combiners_Opaque_Mod2xNA_Alpha_3s
            matDiffuse = meshColor * mix(tex.rgb * tex2.rgb * 2.0, tex3.rgb, vec3(tex3.a));
            break;
        }
        case (20): { //Combiners_Opaque_AddAlpha_Wgt
            matDiffuse = meshColor * tex.rgb;
            specular = tex2.rgb * tex2.a * texSampleAlpha.g;
            break;
        }
        case (21): { //Combiners_Mod_Add_Alpha
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a + tex2.a;
            canDiscard = true;
            specular = tex2.rgb * (1.0 - tex.a);
            break;
        }
        case (22): { //Combiners_Opaque_ModNA_Alpha
            matDiffuse = meshColor * mix(tex.rgb * tex2.rgb, tex.rgb, vec3(tex.a));
            break;
        }
        case (23): { //Combiners_Mod_AddAlpha_Wgt
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a;
            canDiscard = true;
            specular = tex2.rgb * tex2.a * texSampleAlpha.g;
            break;
        }
        case (24): { //Combiners_Opaque_Mod_Add_Wgt
            matDiffuse = meshColor * mix(tex.rgb, tex2.rgb, vec3(tex2.a));
            specular = tex.rgb * tex.a * texSampleAlpha.r;
            break;
        }
        case (25): { //Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha
            float glowOpacity = clamp(tex3.a * texSampleAlpha.b, 0.0, 1.0);
            matDiffuse = meshColor * mix(tex.rgb * tex2.rgb * 2.0, tex.rgb, vec3(tex.a)) * (1.0 - glowOpacity);
            specular = tex3.rgb * glowOpacity;
            break;
        }
        case (26): { //Combiners_Mod_Dual_Crossfade
            matDiffuse = meshColor * mix(mix(tex, tex2, vec4(clamp(texSampleAlpha.g, 0.0, 1.0))), tex3, vec4(clamp(texSampleAlpha.b, 0.0, 1.0))).rgb;
            discardAlpha = mix(mix(tex, tex2, vec4(clamp(texSampleAlpha.g, 0.0, 1.0))), tex3, vec4(clamp(texSampleAlpha.b, 0.0, 1.0))).a;
            canDiscard = true;
            break;
        }
        case (27): { //Combiners_Opaque_Mod2xNA_Alpha_Alpha
            matDiffuse = meshColor * mix(mix(tex.rgb * tex2.rgb * 2.0, tex3.rgb, vec3(tex3.a)), tex.rgb, vec3(tex.a));
            break;
        }
        case (28): { //Combiners_Mod_Masked_Dual_Crossfade
            vec4 tex4 = texture(texSampler4, uv2).rgba;
            matDiffuse = meshColor * mix(mix(tex, tex2, vec4(clamp(texSampleAlpha.g, 0.0, 1.0))), tex3, vec4(clamp(texSampleAlpha.b, 0.0, 1.0))).rgb;
            discardAlpha = mix(mix(tex, tex2, vec4(clamp(texSampleAlpha.g, 0.0, 1.0))), tex3, vec4(clamp(texSampleAlpha.b, 0.0, 1.0))).a * tex4.a;
            canDiscard = true;
            break;
        }
        case (29): { //Combiners_Opaque_Alpha
            matDiffuse = meshColor * mix(tex.rgb, tex2.rgb, vec3(tex2.a));
            break;
        }
        case (30): { //Guild
            matDiffuse = meshColor * mix(tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a)), tex3.rgb * genericParams[2].rgb, vec3(tex3.a));
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (31): { //Guild_NoBorder
            matDiffuse = meshColor * tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a));
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (32): { //Guild_Opaque
            matDiffuse = meshColor * mix(tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a)), tex3.rgb * genericParams[2].rgb, vec3(tex3.a));
            break;
        }
        case (33): { //Combiners_Mod_Depth
            matDiffuse = meshColor * tex.rgb;
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (34): { //Illum
            discardAlpha = tex.a;
            canDiscard = true;
            break;
        }
        case (35): { //Combiners_Mod_Mod_Mod_Const
            matDiffuse = meshColor * (tex * tex2 * tex3 * genericParams[0]).rgb;
            discardAlpha = (tex * tex2 * tex3 * genericParams[0]).a;
            canDiscard = true;
            break;
        }
        case (36): { //unk shader combiner
            matDiffuse = meshColor * tex.rgb * tex2.rgb;
            discardAlpha = tex.a * tex2.a;
            canDiscard = true;
            break;
        }
    }

    bool doDiscard = false;

    if (blendMode == 13) {
        finalOpacity = discardAlpha * meshOpacity;
    } else if (blendMode == 1) {
        finalOpacity = meshOpacity;
        if (canDiscard && discardAlpha < 0.501960814)
            doDiscard = true;
    } else if (blendMode == 0) {
        finalOpacity = meshOpacity;
    } else {
        finalOpacity = discardAlpha * meshOpacity;
    }

    discardThisFragment = doDiscard;
}

void calcM2VertexMat(in int vertexShader,
                     in vec3 vertexPosInView,
                     in vec3 normal,
                     in vec2 texCoord,
                     in vec2 texCoord2,
                     in vec4 color_Transparency,
                     in mat4 textMat[2],
                     out vec4 vMeshColorAlpha,
                     out vec2 vTexCoord,
                     out vec2 vTexCoord2,
                     out vec2 vTexCoord3) {
    vTexCoord2 = vec2(0.0);
    vTexCoord3 = vec2(0.0);

    vec2 envCoord = posToTexCoord(vertexPosInView, normal);
    float edgeScanVal = edgeScan(vertexPosInView, normal);

    vMeshColorAlpha = color_Transparency;

    switch(vertexShader) {
        case (0): { //Diffuse_T1
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (1): { //Diffuse_Env
            vTexCoord = envCoord;
            break;
        }
        case (2): { //Diffuse_T1_T2
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            break;
        }
        case (3): { //Diffuse_T1_Env
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = envCoord;
            break;
        }
        case (4): { //Diffuse_Env_T1
            vTexCoord = envCoord;
            vTexCoord2 = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (5): { //Diffuse_Env_Env
            vTexCoord = envCoord;
            vTexCoord2 = envCoord;
            break;
        }
        case (6): { //Diffuse_T1_Env_T1
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0) ).xy;
            vTexCoord2 = envCoord;
            vTexCoord3 = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (7): { //Diffuse_T1_T1
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (8): { //Diffuse_T1_T1_T1
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord3 = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (9): { //Diffuse_EdgeFade_T1
            vMeshColorAlpha.a *= edgeScanVal;
            vTexCoord = ((textMat[0] * vec4(texCoord, 0.0, 1.0)).xy).xy;
            break;
        }
        case (10): { //Diffuse_T2
            vTexCoord = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            break;
        }
        case (11): { //Diffuse_T1_Env_T2
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = envCoord;
            vTexCoord3 = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            break;
        }
        case (12): { //Diffuse_EdgeFade_T1_T2
            vMeshColorAlpha.a *= edgeScanVal;
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            break;
        }
        case (13): { //Diffuse_EdgeFade_Env
            vMeshColorAlpha.a *= edgeScanVal;
            vTexCoord = envCoord;
            break;
        }
        case (14): { //Diffuse_T1_T2_T1
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            vTexCoord3 = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (15): { //Diffuse_T1_T2_T3
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            vTexCoord2 = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            vTexCoord3 = vTexCoord3;
            break;
        }
        case (16): { //Color_T1_T2_T3
            vec4 in_col0 = vec4(1.0, 1.0, 1.0, 1.0);
            vMeshColorAlpha = vec4((in_col0.rgb * 0.5).r, (in_col0.rgb * 0.5).g, (in_col0.rgb * 0.5).b, in_col0.a);
            vTexCoord = (textMat[1] * vec4(texCoord2, 0.0, 1.0)).xy;
            vTexCoord2 = vec2(0.0, 0.0);
            vTexCoord3 = vTexCoord3;
            break;
        }
        case (17): { //BW_Diffuse_T1
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
        case (18): { //BW_Diffuse_T1_T2
            vTexCoord = (textMat[0] * vec4(texCoord, 0.0, 1.0)).xy;
            break;
        }
    }
}