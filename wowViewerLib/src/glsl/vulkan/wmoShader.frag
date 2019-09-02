#version 450

precision highp float;
in vec2 vTexCoord;
in vec2 vTexCoord2;
in vec2 vTexCoord3;
in vec4 vColor;
in vec4 vColor2;
in vec4 vPosition;
in vec3 vNormal;

layout(std140) uniform meshWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDir_FogStart;
    vec4 uSunColor_uFogEnd;
    vec4 uAmbientLight;
    vec4 uAmbientLight2AndIsBatchA;
    ivec4 UseLitColor_EnableAlpha_PixelShader;
    vec4 FogColor_AlphaTest;
};

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;

layout (location = 0) out vec4 outputColor;

vec3 makeDiffTerm(vec3 matDiffuse) {
    vec3 currColor;
    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    if (UseLitColor_EnableAlpha_PixelShader.x == 1) {
        //vec3 viewUp = normalize(vec3(0, 0.9, 0.1));
        vec3 normalizedN = normalize(vNormal);
        float nDotL = dot(normalizedN, -(uSunDir_FogStart.xyz));
        float nDotUp = dot(normalizedN, uViewUp.xyz);

        vec3 precomputed = vColor2.rgb;

        vec3 ambientColor = uAmbientLight.rgb;
        if (uAmbientLight2AndIsBatchA.w > 0.0) {
            ambientColor = mix(uAmbientLight.rgb, uAmbientLight2AndIsBatchA.rgb, vec3(vPosition.w));
        }

        vec3 adjAmbient = (ambientColor.rgb + precomputed);
        vec3 adjHorizAmbient = (ambientColor.rgb + precomputed);
        vec3 adjGroundAmbient = (ambientColor.rgb + precomputed);

        if ((nDotUp >= 0.0))
        {
            currColor = mix(adjHorizAmbient, adjAmbient, vec3(nDotUp));
        }
        else
        {
            currColor= mix(adjHorizAmbient, adjGroundAmbient, vec3(-(nDotUp)));
        }

        vec3 skyColor = (currColor * 1.10000002);
        vec3 groundColor = (currColor* 0.699999988);
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
        lDiffuse = (uSunColor_uFogEnd.xyz * clamp(nDotL, 0.0, 1.0));
    } else {
        currColor = vec3 (1.0, 1.0, 1.0) * uAmbientLight.rgb;
    }

    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * vec3(0.0);
    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;

//    return matDiffuse * currColor.rgb ;
}

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
    #if(FRAGMENTSHADER==-1)
        finalColor = vec4(makeDiffTerm(tex.rgb * vColor.rgb + tex2.rgb*vColor2.bgr), tex.a);
    #endif
    #if(FRAGMENTSHADER==0) //MapObjDiffuse
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==1) //MapObjSpecular
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==2) //MapObjMetal
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==3) //MapObjEnv
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = tex2.rgb * tex.a;

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    #endif
    #if(FRAGMENTSHADER==4) //MapObjOpaque
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==5) //MapObjEnvMetal
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = (tex.rgb * tex.a) * tex2.rgb;

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    #endif
    #if(FRAGMENTSHADER==6) //MapObjTwoLayerDiffuse
        vec3 layer1 = tex.rgb;
        vec3 layer2 = mix(layer1, tex2.rgb, tex2.a);
        vec3 matDiffuse = (vColor.rgb * 2.0) * mix(layer2, layer1, vColor2.a);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), 1.0);
    #endif
    #if(FRAGMENTSHADER==7) //MapObjTwoLayerEnvMetal
        vec4 colorMix = mix(tex2, tex, vColor2.a);
        vec3 env = (colorMix.rgb * colorMix.a) * tex3.rgb;
        vec3 matDiffuse = colorMix.rgb * (2.0 * vColor.rgb);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    #endif
    #if(FRAGMENTSHADER==8) //MapObjTwoLayerTerrain
        vec3 layer1 = tex.rgb;
        vec3 layer2 = tex2.rgb;

        vec3 matDiffuse = ((vColor.rgb * 2.0) * mix(layer2, layer1, vColor2.a));
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==9) //MapObjDiffuseEmissive
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = tex2.rgb * tex2.a * vColor2.a;

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    #endif
    #if(FRAGMENTSHADER==10) //MapObjMaskedEnvMetal
        float mixFactor = clamp((tex3.a * vColor2.a), 0.0, 1.0);
        vec3 matDiffuse =
            (vColor.rgb * 2.0) *
            mix(mix(((tex.rgb * tex2.rgb) * 2.0), tex3.rgb, mixFactor), tex.rgb, tex.a);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==11) //MapObjEnvMetalEmissive
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env =
            (
                ((tex.rgb * tex.a) * tex2.rgb) +
                ((tex3.rgb * tex3.a) * vColor2.a)
            );

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    #endif
    #if(FRAGMENTSHADER==12) //MapObjTwoLayerDiffuseOpaque
        vec3 matDiffuse =
            (vColor.rgb * 2.0) *
            mix(tex2.rgb, tex.rgb, vColor2.a);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==13) //MapObjTwoLayerDiffuseEmissive
        vec3 t1diffuse = (tex2.rgb * (1.0 - tex2.a));

        vec3 matDiffuse =
            ((vColor.rgb * 2.0) *
            mix(t1diffuse, tex.rgb, vColor2.a));

        //TODO: there is env missing here
        vec3 env = ((tex2.rgb * tex2.a) * (1.0 - vColor2.a));
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    #endif
    #if(FRAGMENTSHADER==14) //MapObjAdditiveMaskedEnvMetal
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==15) //MapObjTwoLayerDiffuseMod2x
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==16) //MapObjTwoLayerDiffuseMod2xNA
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==17) //MapObjTwoLayerDiffuseAlpha
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==18) //MapObjLod
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif
    #if(FRAGMENTSHADER==19) //MapObjParallax
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    #endif

    //finalColor.rgb *= 4.0;

    if(finalColor.a < FogColor_AlphaTest.w)
        discard;

    vec3 fogColor = FogColor_AlphaTest.xyz;
    float fog_start = uSunDir_FogStart.w;
    float fog_end = uSunColor_uFogEnd.w;
    float fog_rate = 1.5;
    float fog_bias = 0.01;

    //vec4 fogHeightPlane = pc_fog.heightPlane;
    //float heightRate = pc_fog.color_and_heightRate.w;

    float distanceToCamera = length(vPosition.xyz);
    float z_depth = (distanceToCamera - fog_bias);
    float expFog = 1.0 / (exp((max(0.0, (z_depth - fog_start)) * fog_rate)));
    //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
    //float heightFog = clamp((height * heightRate), 0, 1);
    float heightFog = 1.0;
    expFog = (expFog + heightFog);
    float endFadeFog = clamp(((fog_end - distanceToCamera) / (0.699999988 * fog_end)), 0.0, 1.0);

    finalColor.rgb = mix(fogColor.rgb, finalColor.rgb, vec3(min(expFog, endFadeFog)));

    finalColor.a = 1.0; //do I really need it now?

    outputColor = finalColor;
}
