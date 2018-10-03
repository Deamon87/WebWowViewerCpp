//https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
//For drawbuffers in glsl of webgl you need to use GL_EXT_draw_buffers instead of WEBGL_draw_buffers

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

#ifdef ENABLE_DEFERRED
#ifdef GL_EXT_draw_buffers
    #extension GL_EXT_draw_buffers: require
    #extension OES_texture_float_linear : enable
    #define drawBuffersIsSupported 1
#endif
#endif

#ifdef COMPILING_VS
precision highp FLOATDEC
/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;

//Whole scene
layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

// Whole model
#ifndef INSTANCED
layout(std140) uniform modelWideBlockVS {
    mat4 uPlacementMat;
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};
#else
attribute mat4 aPlacementMat;
#endif

//Individual meshes
layout(std140) uniform meshWideBlockVS {
    ivec4 vertexShader_IsAffectedByLight;
    vec4 color_Transparency;
    mat4 uTextMat[2];
};

//Shader output
out vec2 vTexCoord;
out vec2 vTexCoord2;
out vec2 vTexCoord3;
out vec3 vNormal;
out vec3 vPosition;
out vec4 vDiffuseColor;

#ifdef drawBuffersIsSupported
in float fs_Depth;
#endif

vec2 posToTexCoord(vec3 cameraPoint, vec3 normal){
//    vec3 normPos = -normalize(cameraPoint.xyz);
    vec3 normPos = cameraPoint.xyz;
    vec3 reflection = reflect(normPos, normal);
    return (normalize(vec3(reflection.r, reflection.g, reflection.b + 1.0)).rg * 0.5) + vec2(0.5);

//    vec3 normPos_495 = normPos;
//    vec3 temp_500 = (normPos_495 - (normal * (2.0 * dot(normPos_495, normal))));
//    vec3 temp_657 = vec3(temp_500.x, temp_500.y, (temp_500.z + 1.0));
//
//    return ((normalize(temp_657).xy * 0.5) + vec2(0.5));
}

float edgeScan(vec3 position, vec3 normal){
    float dotProductClamped = clamp(dot(-normalize(position),normal), 0.000000, 1.000000);
    return clamp(2.700000 * dotProductClamped * dotProductClamped - 0.400000, 0.000000, 1.000000);
}

mat3 blizzTranspose(mat4 value) {
    return mat3(
        value[0].xyz,
        value[1].xyz,
        value[2].xyz
    );
}

void main() {
    vec4 modelPoint = vec4(0,0,0,0);

    vec4 aPositionVec4 = vec4(aPosition, 1);
    mat4 boneTransformMat = mat4(1.0);

#if BONEINFLUENCES>0
    boneTransformMat = mat4(0.0);
    const float inttofloat = (1.0/255.0);
    boneTransformMat += (boneWeights.x * inttofloat) * uBoneMatrixes[int(bones.x)];
#endif
#if BONEINFLUENCES>1
    boneTransformMat += (boneWeights.y * inttofloat) * uBoneMatrixes[int(bones.y)];
#endif
#if BONEINFLUENCES>2
    boneTransformMat += (boneWeights.z * inttofloat) * uBoneMatrixes[int(bones.z)];
#endif
#if BONEINFLUENCES>3
    boneTransformMat += (boneWeights.w * inttofloat) * uBoneMatrixes[int(bones.w)];
#endif




    mat4 placementMat;
#ifdef INSTANCED
    placementMat = aPlacementMat;
#else
    placementMat = uPlacementMat;
#endif
    vec4 lDiffuseColor = color_Transparency;

    mat4 cameraMatrix = uLookAtMat * placementMat  * boneTransformMat ;
    vec4 cameraPoint = cameraMatrix * aPositionVec4;
    mat3 viewModelMatTransposed =
        blizzTranspose(uLookAtMat) *
        blizzTranspose(placementMat) *
        blizzTranspose(boneTransformMat);

//    vec3 normal = normalize(mat3(cameraMatrix) * aNormal);
    vec3 normal = normalize(viewModelMatTransposed * aNormal);
    vec4 combinedColor = clamp(lDiffuseColor /*+ vc_matEmissive*/, 0.000000, 1.000000);
    vec4 combinedColorHalved = combinedColor * 0.5;
    
    vec2 envCoord = posToTexCoord(cameraPoint.xyz, normal);
    float edgeScanVal = edgeScan(cameraPoint.xyz, normal);

    vTexCoord2 = vec2(0.0);
    vTexCoord3 = vec2(0.0);

    int uVertexShader = vertexShader_IsAffectedByLight.x;

    //Diffuse_T1
    #if VERTEXSHADER== 0 
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    //Diffuse_Env
    #if VERTEXSHADER== 1 
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
    #endif
    //Diffuse_T1_T2
    #if VERTEXSHADER== 2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==3 //Diffuse_T1_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = envCoord;
    #endif
    #if VERTEXSHADER==4 //Diffuse_Env_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
        vTexCoord2 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==5 //Diffuse_Env_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
        vTexCoord2 = envCoord;
    #endif
    #if VERTEXSHADER==6 //Diffuse_T1_Env_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000) ).xy;
        vTexCoord2 = envCoord;
        vTexCoord3 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==7 //Diffuse_T1_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==8 //Diffuse_T1_T1_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord3 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==9 //Diffuse_EdgeFade_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = ((uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy).xy;
    #endif
    #if VERTEXSHADER==10 //Diffuse_T2

        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==11 //Diffuse_T1_Env_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = envCoord;
        vTexCoord3 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==12 //Diffuse_EdgeFade_T1_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==13 //Diffuse_EdgeFade_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = envCoord;
    #endif
    #if VERTEXSHADER==14 //Diffuse_T1_T2_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
        vTexCoord3 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==15 //Diffuse_T1_T2_T3
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
        vTexCoord3 = vTexCoord3;
    #endif
    #if VERTEXSHADER==16 //Color_T1_T2_T3
        vec4 in_col0 = vec4(1.0, 1.0, 1.0, 1.0);
        vDiffuseColor = vec4((in_col0.rgb * 0.500000).r, (in_col0.rgb * 0.500000).g, (in_col0.rgb * 0.500000).b, in_col0.a);
        vTexCoord = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
        vTexCoord2 = vec2(0.000000, 0.000000);
        vTexCoord3 = vTexCoord3;
    #endif
    #if VERTEXSHADER==17 //BW_Diffuse_T1
        vDiffuseColor = vec4(combinedColor.rgb * 0.500000, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif
    #if VERTEXSHADER==18 //BW_Diffuse_T1_T2
        vDiffuseColor = vec4(combinedColor.rgb * 0.500000, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    #endif

#ifndef drawBuffersIsSupported
    gl_Position = uPMatrix * cameraPoint;
    vNormal = normal;
    vPosition = cameraPoint.xyz;
#else
    gl_Position = uPMatrix * cameraPoint;
    fs_Depth = gl_Position.z / gl_Position.w;

    vNormal = normal;
    vPosition = cameraPoint.xyz;
#endif //drawBuffersIsSupported

}
#endif //COMPILING_VS

#ifdef COMPILING_FS
//precision mediump float;

precision highp float;

struct LocalLight
{
    vec4 color;
    vec4 position;
    vec4 attenuation;
};

in vec3 vNormal;
in vec2 vTexCoord;
in vec2 vTexCoord2;
in vec2 vTexCoord3;
in vec3 vPosition;
in vec4 vDiffuseColor;

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;

out vec4 outputColor;

layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

//Whole model
layout(std140) uniform modelWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDirAndFogStart;
    vec4 uSunColorAndFogEnd;
    vec4 uAmbientLight;
};

//Individual meshes
layout(std140) uniform meshWideBlockPS {
    ivec4 PixelShader_UnFogged_IsAffectedByLight_LightCount;
    vec4 uFogColorAndAlphaTest;
    LocalLight pc_lights[4];
    vec4 uPcColor;
};

#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

vec3 makeDiffTerm(vec3 matDiffuse, vec3 accumLight) {
//    return matDiffuse;
    vec3 currColor;
    float mult = 1.0;
    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    if (PixelShader_UnFogged_IsAffectedByLight_LightCount.z == 1) {
        vec3 normalizedN = normalize(vNormal);
        float nDotL = clamp(dot(normalizedN, -(uSunDirAndFogStart.xyz)), 0.0, 1.0);
        float nDotUp = dot(normalizedN, uViewUp.xyz);

        vec4 AmbientLight = uAmbientLight;

        vec3 adjAmbient = (AmbientLight.rgb );
        vec3 adjHorizAmbient = (AmbientLight.rgb );
        vec3 adjGroundAmbient = (AmbientLight.rgb );

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


        lDiffuse = (uSunColorAndFogEnd.xyz * nDotL);
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
//

    } else {
        currColor = vec3 (1.0, 1.0, 1.0) ;
        accumLight = vec3(0,0,0);
        mult = 1.0;
    }


//    return currColor.rgb * matDiffuse;
//    return sqrt((matDiffuse*matDiffuse)*0.5 + currColor.rgb*(matDiffuse*matDiffuse));
    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * accumLight;
//    return sqrt((matDiffuse*matDiffuse)*mult + currColor.rgb*(matDiffuse*matDiffuse)) ;
    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;
}

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord3.xy;

    /* Get color from texture */
    vec4 tex = texture(uTexture, texCoord).rgba;
    vec4 tex2 = texture(uTexture2, texCoord2).rgba;
    vec4 tex3 = texture(uTexture3, texCoord3).rgba;

    vec4 tex2WithTextCoord1 = texture(uTexture2,texCoord);
    vec4 tex3WithTextCoord1 = texture(uTexture3,texCoord);
    vec4 tex4WithTextCoord2 = texture(uTexture4,texCoord2);

    vec4 finalColor = vec4(0);
    vec4 meshResColor = vDiffuseColor;

//    if(meshResColor.a < uAlphaTest)
//        discard;
    vec3 accumLight;
    if ((PixelShader_UnFogged_IsAffectedByLight_LightCount.z == 1)) {
        vec3 vPos3 = vPosition.xyz;
        vec3 vNormal3 = normalize(vNormal.xyz);
        vec3 lightColor = vec3(0.0);
        int count = int(pc_lights[0].attenuation.w);
        int index = 0;
        for (;;)
        {
            if ( index >= PixelShader_UnFogged_IsAffectedByLight_LightCount.w) break;
            LocalLight lightRecord = pc_lights[index];
            vec3 vectorToLight = ((lightRecord.position).xyz - vPos3);
            float distanceToLightSqr = dot(vectorToLight, vectorToLight);
            float distanceToLightInv = inversesqrt(distanceToLightSqr);
            float distanceToLight = (distanceToLightSqr * distanceToLightInv);
            float diffuseTerm1 = max((dot(vectorToLight, vNormal3) * distanceToLightInv), 0.0);
            vec4 attenuationRec = lightRecord.attenuation;

            float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1.0 / (attenuationRec.z - attenuationRec.x)), 0.0, 1.0));

            vec3 attenuatedColor = attenuation * lightRecord.color.xyz * attenuationRec.y;
            lightColor = (lightColor + vec3(attenuatedColor * attenuatedColor * diffuseTerm1 ));
            index++;
        }
        meshResColor.rgb = clamp(lightColor , 0.0, 1.0);
        accumLight = meshResColor.rgb;
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

    float opacity;
    float finalOpacity = 0.0;
    vec3 matDiffuse;
    vec3 specular = vec3(0.0, 0.0, 0.0);
    vec3 visParams = vec3(1.0, 1.0, 1.0);
    vec4 genericParams[3];
    genericParams[0] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[1] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[2] = vec4( 1.0, 1.0, 1.0, 1.0 );

    int uPixelShader = PixelShader_UnFogged_IsAffectedByLight_LightCount.x;

    #if(FRAGMENTSHADER==0) //Combiners_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==1) //Combiners_Mod
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==2) //Combiners_Opaque_Mod
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = tex2.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==3) //Combiners_Opaque_Mod2x
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = tex2.a * 2.000000 * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==4) //Combiners_Opaque_Mod2xNA
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==5) //Combiners_Opaque_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==6) //Combiners_Mod_Mod
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = tex.a * tex2.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==7) //Combiners_Mod_Mod2x
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = tex.a * tex2.a * 2.000000 * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==8) //Combiners_Mod_Add
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = (tex.a + tex2.a) * vDiffuseColor.a;
        specular = tex2.rgb;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==9) //Combiners_Mod_Mod2xNA
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = tex.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if(FRAGMENTSHADER==10) //Combiners_Mod_AddNA
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        specular = tex2.rgb;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==11 //Combiners_Mod_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = tex.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==12 //Combiners_Opaque_Mod2xNA_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex.rgb, vec3(tex.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==13 //Combiners_Opaque_AddAlpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        specular = tex2.rgb * tex2.a;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==14 //Combiners_Opaque_AddAlpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        specular = tex2.rgb * tex2.a * (1.000000 - tex.a);
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==15 //Combiners_Opaque_Mod2xNA_Alpha_Add
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex.rgb, vec3(tex.a));
        specular = tex3.rgb * tex3.a * genericParams[0].b;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==16 //Combiners_Mod_AddAlpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        specular = tex2.rgb * tex2.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==17 //Combiners_Mod_AddAlpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = (tex.a + tex2.a * (0.300000 * tex2.r + 0.590000 * tex2.g + 0.110000 * tex2.b)) * vDiffuseColor.a;
        specular = tex2.rgb * tex2.a * (1.000000 - tex.a);
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==18 //Combiners_Opaque_Alpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex.rgb, tex2.rgb, vec3(tex2.a)), tex.rgb, vec3(tex.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==19 //Combiners_Opaque_Mod2xNA_Alpha_3s
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex3.rgb, vec3(tex3.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==20 //Combiners_Opaque_AddAlpha_Wgt
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        specular = tex2.rgb * tex2.a * genericParams[0].g;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==21 //Combiners_Mod_Add_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = (tex.a + tex2.a) * vDiffuseColor.a;
        specular = tex2.rgb * (1.000000 - tex.a);
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==22 //Combiners_Opaque_ModNA_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb, tex.rgb, vec3(tex.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==23 //Combiners_Mod_AddAlpha_Wgt
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        specular = tex2.rgb * tex2.a * genericParams[0].g;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==24 //Combiners_Opaque_Mod_Add_Wgt
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb, tex2.rgb, vec3(tex2.a));
        specular = tex.rgb * tex.a * genericParams[0].r;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==25 //Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha
        float glowOpacity = clamp((tex3.a * genericParams[0].z), 0.0, 1.0);
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex.rgb, vec3(tex.a)) * (1.000000 - glowOpacity);
        specular = tex3.rgb * glowOpacity;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==26 //Combiners_Mod_Dual_Crossfade
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex, tex2WithTextCoord1, vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), tex3WithTextCoord1, vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).rgb;
        opacity = mix(mix(tex, tex2WithTextCoord1, vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), tex3WithTextCoord1, vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==27 //Combiners_Opaque_Mod2xNA_Alpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex.rgb * tex2.rgb * 2.000000, tex3.rgb, vec3(tex3.a)), tex.rgb, vec3(tex.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==28 //Combiners_Mod_Masked_Dual_Crossfade
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex, tex2WithTextCoord1, vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), tex3WithTextCoord1, vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).rgb;
        opacity = mix(mix(tex, tex2WithTextCoord1, vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), tex3WithTextCoord1, vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).a * tex4WithTextCoord2.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==29 //Combiners_Opaque_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb, tex2.rgb, vec3(tex2.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==30 //Guild
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a)), tex3.rgb * genericParams[2].rgb, vec3(tex3.a));
        opacity = tex.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==31 //Guild_NoBorder
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a));
        opacity = tex.a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==32 //Guild_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a)), tex3.rgb * genericParams[2].rgb, vec3(tex3.a));
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==33 //Combiners_Mod_Depth
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a * visParams.r;
        finalOpacity = opacity * visParams.r;
    #endif
    #if FRAGMENTSHADER==34  //Illum
        finalColor = vec4(1.0,1.0,1.0, 1.0);

        //Unusued
    #endif
    #if FRAGMENTSHADER==35 //Combiners_Mod_Mod_Mod_Const
        matDiffuse = vDiffuseColor.rgb * 2.000000 * (tex * tex2 * tex3 * genericParams[0]).rgb;
        opacity = (tex * tex2 * tex3 * genericParams[0]).a * vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;
    #endif
        /*
            WOTLK DEPRECATED SHADERS!
        */
    #if FRAGMENTSHADER==-1 // Combiners_Decal
        finalColor.rgb = (meshResColor.rgb - tex.rgb) * meshResColor.a + tex.rgb;
        finalColor.a = meshResColor.a;
    #endif
    #if FRAGMENTSHADER==-2 // Combiners_Add
        finalColor.rgba = tex.rgba + meshResColor.rgba;
    #endif
    #if FRAGMENTSHADER==-3 // Combiners_Mod2x
        finalColor.rgb = tex.rgb * meshResColor.rgb * vec3(2.0);
        finalColor.a = tex.a * meshResColor.a * 2.0;
    #endif
    #if FRAGMENTSHADER==-4 // Combiners_Fade
        finalColor.rgb = (tex.rgb - meshResColor.rgb) * meshResColor.a + meshResColor.rgb;
        finalColor.a = meshResColor.a;
    #endif
    #if FRAGMENTSHADER==-5 // Combiners_Opaque_Add
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a + tex.a;
    #endif
    #if FRAGMENTSHADER==-6 // Combiners_Opaque_AddNA
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a;
    #endif
    #if FRAGMENTSHADER==-7 // Combiners_Add_Mod
        finalColor.rgb = (tex.rgb + meshResColor.rgb) * tex2.a;
        finalColor.a = (tex.a + meshResColor.a) * tex2.a;
    #endif
    #if FRAGMENTSHADER==-8 // Combiners_Mod2x_Mod2x
        finalColor.rgba = tex.rgba * tex2.rgba * meshResColor.rgba * vec4(4.0);
    #endif

    finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + specular, finalOpacity);

    if(finalColor.a < uFogColorAndAlphaTest.w)
        discard;

    /*
    int uUnFogged = PixelShader_UnFogged_IsAffectedByLight_LightCount.y;
    float uFogEnd = uSunColorAndFogEnd.z;
    if (uUnFogged == 0) {
        vec3 fogColor = uFogColorAndAlphaTest.xyz;
        float fog_rate = 1.5;
        float fog_bias = 0.01;
    
        //vec4 fogHeightPlane = pc_fog.heightPlane;
        //float heightRate = pc_fog.color_and_heightRate.w;

        float distanceToCamera = length(vPosition.xyz);
        float z_depth = (distanceToCamera - fog_bias);
        float expFog = 1.0 / (exp((max(0.0, (z_depth - uSunDirAndFogStart.z)) * fog_rate)));
        //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
        //float heightFog = clamp((height * heightRate), 0, 1);
        float heightFog = 1.0;
        expFog = (expFog + heightFog);
        float endFadeFog = clamp(((uFogEnd - distanceToCamera) / (0.699999988 * uFogEnd)), 0.0, 1.0);
        float fog_out = min(expFog, endFadeFog);
        finalColor.rgba = vec4(mix(fogColor.rgb, finalColor.rgb, vec3(fog_out)), finalColor.a);
    }*/
//    finalColor.rgb = finalColor.rgb;


    //Forward rendering without lights
    outputColor = finalColor;

    //Deferred rendering
    //gl_FragColor = finalColor;
//    gl_FragData[0] = vec4(vec3(fs_Depth), 1.0);
//    gl_FragData[1] = vec4(vPosition.xyz,0);
//    gl_FragData[2] = vec4(vNormal.xyz,0);
//    gl_FragData[3] = finalColor;

}

#endif //COMPILING_FS


