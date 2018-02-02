//https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
//For drawbuffers in glsl of webgl you need to use GL_EXT_draw_buffers instead of WEBGL_draw_buffers

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 59 //Max 59 for ANGLE implementation and max 120? bones in Wotlk client
#endif

#ifdef ENABLE_DEFERRED
#ifdef GL_EXT_draw_buffers
    #extension GL_EXT_draw_buffers: require
    #extension OES_texture_float_linear : enable
    #define drawBuffersIsSupported 1
#endif
#endif

#ifdef COMPILING_VS
/* vertex shader code */
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec4 bones;
attribute vec4 boneWeights;
attribute vec2 aTexCoord;
attribute vec2 aTexCoord2;

uniform mediump mat4 uLookAtMat;
uniform mat4 uPMatrix;
uniform mat4 uBoneMatrixes[MAX_MATRIX_NUM];
uniform int uVertexShader;

uniform lowp int uUseDiffuseColor;
uniform vec4 uColor;
uniform float uTransparency;

#ifdef INSTANCED
attribute vec4 aDiffuseColor;
attribute mat4 aPlacementMat;

#else
uniform mat4 uPlacementMat;
uniform vec4 uDiffuseColor;
#endif

varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec2 vTexCoord3;
varying vec3 vNormal;
varying vec3 vPosition;
varying vec4 vDiffuseColor;

uniform mat4 uTextMat1;
uniform mat4 uTextMat2;


#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

vec2 normalPosToTexCoord(vec3 normPos, vec3 normal){
    vec3 reflection = reflect(normPos, normal);
    return (normalize(vec3(reflection.r, reflection.g, reflection.b + 1.0)).rg * 0.5) + vec2(0.5);
}

float edgeScan(vec3 position, vec3 normal){
    float dotProductClamped = clamp(dot(-normalize(position),normal), 0.000000, 1.000000);
    return clamp(2.700000 * dotProductClamped * dotProductClamped - 0.400000, 0.000000, 1.000000);
}

void main() {
    vec4 modelPoint = vec4(0,0,0,0);

    vec4 aPositionVec4 = vec4(aPosition, 1);
    mat4 boneTransformMat = mat4(0.0);

    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[int(bones.x)];
    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[int(bones.y)];
    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[int(bones.z)];
    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[int(bones.w)];

    mat4 placementMat;
#ifdef INSTANCED
    placementMat = aPlacementMat;
#else
    placementMat = uPlacementMat;
#endif

    vec4 lDiffuseColor;
    if ((uUseDiffuseColor == 1)) {
#ifdef INSTANCED
        lDiffuseColor = aDiffuseColor;
#else
        lDiffuseColor = uDiffuseColor;
#endif
    } else {
        lDiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

    vec4 meshColor = uColor;
    lDiffuseColor = vec4(meshColor.rgb * lDiffuseColor.bgr, uTransparency);


    mat4 cameraMatrix = uLookAtMat * placementMat  * boneTransformMat ;
    vec4 cameraPoint = cameraMatrix * aPositionVec4;

    vec3 normal = normalize(mat3(cameraMatrix) * aNormal);
    vec4 combinedColor = clamp(lDiffuseColor /*+ vc_matEmissive*/, 0.000000, 1.000000);
    vec4 combinedColorHalved = combinedColor * 0.5;
    
    vec3 normPos = -normalize(cameraPoint.xyz);
    vec2 envCoord = normalPosToTexCoord(normPos, normal);
    float edgeScanVal = edgeScan(cameraPoint.xyz, normal);

    vTexCoord2 = vec2(0.0);
    vTexCoord3 = vec2(0.0);

    if ( uVertexShader == 0 ) {//Diffuse_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 1 ) {//Diffuse_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
    } else if ( uVertexShader == 2 ) {//Diffuse_T1_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
    } else if ( uVertexShader == 3 ) {//Diffuse_T1_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = envCoord;
    } else if ( uVertexShader == 4 ) {//Diffuse_Env_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
        vTexCoord2 = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 5 ) {//Diffuse_Env_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
        vTexCoord2 = envCoord;
    } else if ( uVertexShader == 6 ) {//Diffuse_T1_Env_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = envCoord;
        vTexCoord3 = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 7 ) {//Diffuse_T1_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 8 ) {//Diffuse_T1_T1_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord3 = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 9 ) {//Diffuse_EdgeFade_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = ((vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy).xy;
    } else if ( uVertexShader == 10 ) {//Diffuse_T2

        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
    } else if ( uVertexShader == 11 ) {//Diffuse_T1_Env_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = envCoord;
        vTexCoord3 = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
    } else if ( uVertexShader == 12 ) {//Diffuse_EdgeFade_T1_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
    } else if ( uVertexShader == 13 ) {//Diffuse_EdgeFade_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = envCoord;
    } else if ( uVertexShader == 14 ) {//Diffuse_T1_T2_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
        vTexCoord3 = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 15 ) {//Diffuse_T1_T2_T3
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
        vTexCoord2 = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
        vTexCoord3 = vTexCoord3;
    } else if ( uVertexShader == 16 ) {//Color_T1_T2_T3
        vec4 in_col0 = vec4(1.0, 1.0, 1.0, 1.0);
        vDiffuseColor = vec4((in_col0.rgb * 0.500000).r, (in_col0.rgb * 0.500000).g, (in_col0.rgb * 0.500000).b, in_col0.a);
        vTexCoord = (vec4(aTexCoord2, 0.000000, 1.000000) * transpose(uTextMat2)).xy;
        vTexCoord2 = vec2(0.000000, 0.000000);
        vTexCoord3 = vTexCoord3;
    } else if ( uVertexShader == 17 ) {//BW_Diffuse_T1
        vDiffuseColor = vec4(combinedColor.rgb * 0.500000, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    } else if ( uVertexShader == 18 ) {//BW_Diffuse_T1_T2
        vDiffuseColor = vec4(combinedColor.rgb * 0.500000, combinedColor.a);
        vTexCoord = (vec4(aTexCoord, 0.000000, 1.000000) * transpose(uTextMat1)).xy;
    }

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
varying vec3 vNormal;
varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec2 vTexCoord3;
varying vec3 vPosition;

varying vec4 vDiffuseColor;

uniform lowp int isTransparent;

uniform int uBlendMode;
uniform int uPixelShader;
uniform int uUnFogged;


uniform vec3 uFogColor;

//uniform vec4  uGlobalLighting;
uniform float uAlphaTest;

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;

uniform vec3 uViewUp;
uniform vec3 uSunDir;
uniform vec4 uAmbientLight;

uniform float uFogStart;
uniform float uFogEnd;

uniform mediump mat4 uLookAtMat;

uniform lowp int uUseDiffuseColor;
struct LocalLight
{
    vec4 color;
    vec4 position;
    vec4 attenuation;
};

uniform vec4 uPcColor;

uniform LocalLight pc_lights[3];
uniform lowp int uLightCount;
#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

vec3 makeDiffTerm(vec3 matDiffuse, vec3 accumLight) {
    vec3 currColor;
    float mult = 1.0;
    if (uUseDiffuseColor == 1) {
        vec3 normalizedN = normalize(vNormal);
        float t823 = dot(normalizedN, -(uSunDir.xyz));
        float t846 = dot(normalizedN, uViewUp.xyz);

        vec3 adjAmbient = (uAmbientLight.rgb );
        vec3 adjHorizAmbient = (uAmbientLight.rgb );
        vec3 adjGroundAmbient = (uAmbientLight.rgb );

        if ((t846 >= 0.0))
        {
            currColor = mix(adjHorizAmbient, adjAmbient, vec3(t846));
        }
        else
        {
            currColor= mix(adjHorizAmbient, adjGroundAmbient, vec3(-(t846)));
        }

        vec3 skyColor = (currColor * 1.10000002);
        vec3 groundColor = (currColor* 0.699999988);
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * t823))));
//

    } else {
        currColor = vec3 (1.0, 1.0, 1.0) ;
        accumLight = vec3(0,0,0);
        mult = 1.0;
    }


//    return currColor.rgb * matDiffuse;
//    return sqrt((matDiffuse*matDiffuse)*0.5 + currColor.rgb*(matDiffuse*matDiffuse));
    vec3 gammaDiffTerm = matDiffuse * currColor;
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
    vec4 tex = texture2D(uTexture, texCoord).rgba;
    vec4 tex2 = texture2D(uTexture2, texCoord2).rgba;
    vec4 tex3 = texture2D(uTexture3, texCoord3).rgba;



    vec4 finalColor = vec4(0);
    vec4 meshResColor = vDiffuseColor;

//    if(meshResColor.a < uAlphaTest)
//        discard;
    vec3 accumLight;
    if ((uUseDiffuseColor == 1)) {
        vec3 vPos3 = vPosition.xyz;
        vec3 vNormal3 = normalize(vNormal.xyz);
        int count = int(pc_lights[0].attenuation.w);
        vec3 lightColor = vec3(0.0);
        int index = 0;
        for (;;)
        {
            if ( index >= uLightCount) break;
            LocalLight lightRecord = pc_lights[index];
            vec3 vectorToLight = ((lightRecord.position).xyz - vPos3);
            float distanceToLightSqr = dot(vectorToLight, vectorToLight);
            float distanceToLightInv = inversesqrt(distanceToLightSqr);
            float distanceToLight = (distanceToLightSqr * distanceToLightInv);
            float diffuseTerm1 = max((dot(vectorToLight, vNormal3) * distanceToLightInv), 0.0);
            vec4 attenuationRec = lightRecord.attenuation;

            float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1 / (attenuationRec.z - attenuationRec.x)), 0, 1));

            vec3 attenuatedColor = attenuation * lightRecord.color.xyz;
            lightColor = (lightColor + vec3(attenuatedColor * attenuatedColor * diffuseTerm1 ));
            index++;
        }

        meshResColor.rgb = clamp(lightColor , 0.0, 1.0);
        accumLight = meshResColor.rgb;
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

    float opacity;
    vec3 matDiffuse;
    vec3 visParams = vec3(1.0, 1.0, 1.0);
    vec4 genericParams[3];
    genericParams[0] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[1] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[2] = vec4( 1.0, 1.0, 1.0, 1.0 );

    if ( uPixelShader == 0 ) {//Combiners_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 1 ) {//Combiners_Mod
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 2 ) {//Combiners_Opaque_Mod
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = tex2.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 3 ) {//Combiners_Opaque_Mod2x
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = tex2.a * 2.000000 * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 4 ) {//Combiners_Opaque_Mod2xNA
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 5 ) {//Combiners_Opaque_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 6 ) {//Combiners_Mod_Mod
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = tex.a * tex2.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 7 ) {//Combiners_Mod_Mod2x
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = tex.a * tex2.a * 2.000000 * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 8 ) {//Combiners_Mod_Add
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = (tex.a + tex2.a) * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb, opacity * visParams.r);
    } else if ( uPixelShader == 9 ) {//Combiners_Mod_Mod2xNA
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb * 2.000000;
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 10 ) {//Combiners_Mod_AddNA
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb, opacity * visParams.r);
    } else if ( uPixelShader == 11 ) {//Combiners_Mod_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * tex2.rgb;
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 12 ) {//Combiners_Opaque_Mod2xNA_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex.rgb, vec3(tex.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 13 ) {//Combiners_Opaque_AddAlpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * tex2.a, vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 14 ) {//Combiners_Opaque_AddAlpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * tex2.a * (1.000000 - tex.a), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 15 ) {//Combiners_Opaque_Mod2xNA_Alpha_Add
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex.rgb, vec3(tex.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex3.rgb * tex3.a * genericParams[0].b, vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 16 ) {//Combiners_Mod_AddAlpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * tex2.a, opacity * visParams.r);
    } else if ( uPixelShader == 17 ) {//Combiners_Mod_AddAlpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = (tex.a + tex2.a * (0.300000 * tex2.r + 0.590000 * tex2.g + 0.110000 * tex2.b)) * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * tex2.a * (1.000000 - tex.a), opacity * visParams.r);
    } else if ( uPixelShader == 18 ) {//Combiners_Opaque_Alpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex.rgb, tex2.rgb, vec3(tex2.a)), tex.rgb, vec3(tex.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 19 ) {//Combiners_Opaque_Mod2xNA_Alpha_3s
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex3.rgb, vec3(tex3.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 20 ) {//Combiners_Opaque_AddAlpha_Wgt
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * tex2.a * genericParams[0].g, vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 21 ) {//Combiners_Mod_Add_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = (tex.a + tex2.a) * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * (1.000000 - tex.a), opacity * visParams.r);
    } else if ( uPixelShader == 22 ) {//Combiners_Opaque_ModNA_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb, tex.rgb, vec3(tex.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 23 ) {//Combiners_Mod_AddAlpha_Wgt
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex2.rgb * tex2.a * genericParams[0].g, opacity * visParams.r);
    } else if ( uPixelShader == 24 ) {//Combiners_Opaque_Mod_Add_Wgt
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb, tex2.rgb, vec3(tex2.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex.rgb * tex.a * genericParams[0].r, vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 25 ) {//Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha
        float glowOpacity = clamp((tex3.a * genericParams[0].z), 0, 1);
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * tex2.rgb * 2.000000, tex.rgb, vec3(tex.a)) * (1.000000 - glowOpacity);
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + tex3.rgb * glowOpacity, vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 26 ) {//Combiners_Mod_Dual_Crossfade
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex, texture(uTexture2,texCoord), vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), texture(uTexture3,texCoord), vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).rgb;
        opacity = mix(mix(tex, texture(uTexture2,texCoord), vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), texture(uTexture3,texCoord), vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 27 ) {//Combiners_Opaque_Mod2xNA_Alpha_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex.rgb * tex2.rgb * 2.000000, tex3.rgb, vec3(tex3.a)), tex.rgb, vec3(tex.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 28 ) {//Combiners_Mod_Masked_Dual_Crossfade
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(mix(tex, texture(uTexture2,texCoord), vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), texture(uTexture3,texCoord), vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).rgb;
        opacity = mix(mix(tex, texture(uTexture2,texCoord), vec4(clamp(genericParams[0].g, 0.000000, 1.000000))), texture(uTexture3,texCoord), vec4(clamp(genericParams[0].b, 0.000000, 1.000000))).a * texture(uTexture4,texCoord2).a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 29 ) {//Combiners_Opaque_Alpha
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb, tex2.rgb, vec3(tex2.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 30 ) {//Guild
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a)), tex3.rgb * genericParams[2].rgb, vec3(tex3.a));
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 31 ) {//Guild_NoBorder
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a));
        opacity = tex.a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 32 ) {//Guild_Opaque
        matDiffuse = vDiffuseColor.rgb * 2.000000 * mix(tex.rgb * mix(genericParams[0].rgb, tex2.rgb * genericParams[1].rgb, vec3(tex2.a)), tex3.rgb * genericParams[2].rgb, vec3(tex3.a));
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), vDiffuseColor.a * visParams.r);
    } else if ( uPixelShader == 33 ) {//Combiners_Mod_Depth
        matDiffuse = vDiffuseColor.rgb * 2.000000 * tex.rgb;
        opacity = tex.a * vDiffuseColor.a * visParams.r;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);
    } else if ( uPixelShader == 34 ) { //Illum
        finalColor = vec4(1.0,1.0,1.0, 1.0);

        //Unusued
    } else if ( uPixelShader == 35 ) {//Combiners_Mod_Mod_Mod_Const
        matDiffuse = vDiffuseColor.rgb * 2.000000 * (tex * tex2 * tex3 * genericParams[0]).rgb;
        opacity = (tex * tex2 * tex3 * genericParams[0]).a * vDiffuseColor.a;
        
        finalColor = vec4(makeDiffTerm(matDiffuse, accumLight), opacity * visParams.r);

    /*
        WOTLK DEPRECATED SHADERS!
    */
    } else if (uPixelShader == -1) { // Combiners_Decal
        finalColor.rgb = (meshResColor.rgb - tex.rgb) * meshResColor.a + tex.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == -2) { // Combiners_Add
        finalColor.rgba = tex.rgba + meshResColor.rgba;
    } else if (uPixelShader == -3) { // Combiners_Mod2x
        finalColor.rgb = tex.rgb * meshResColor.rgb * vec3(2.0);
        finalColor.a = tex.a * meshResColor.a * 2.0;
    } else if (uPixelShader == -4) { // Combiners_Fade
        finalColor.rgb = (tex.rgb - meshResColor.rgb) * meshResColor.a + meshResColor.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == -5) { // Combiners_Opaque_Add
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a + tex.a;
    } else if (uPixelShader == -6) { // Combiners_Opaque_AddNA
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == -7) { // Combiners_Add_Mod
        finalColor.rgb = (tex.rgb + meshResColor.rgb) * tex2.a;
        finalColor.a = (tex.a + meshResColor.a) * tex2.a;
    } else if (uPixelShader == -8) { // Combiners_Mod2x_Mod2x
        finalColor.rgba = tex.rgba * tex2.rgba * meshResColor.rgba * vec4(4.0);
    }

    if(finalColor.a < uAlphaTest)
        discard;

    if (uUnFogged == 0) {
        vec3 fogColor = uFogColor;
        float fog_rate = 1.5;
        float fog_bias = 0.01;
    
        //vec4 fogHeightPlane = pc_fog.heightPlane;
        //float heightRate = pc_fog.color_and_heightRate.w;

        float distanceToCamera = length(vPosition.xyz);
        float z_depth = (distanceToCamera - fog_bias);
        float expFog = 1.0 / (exp((max(0.0, (z_depth - uFogStart)) * fog_rate)));
        //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
        //float heightFog = clamp((height * heightRate), 0, 1);
        float heightFog = 1.0;
        expFog = (expFog + heightFog);
        float endFadeFog = clamp(((uFogEnd - distanceToCamera) / (0.699999988 * uFogEnd)), 0.0, 1.0);
        float fog_out = min(expFog, endFadeFog);
        finalColor.rgba = vec4(mix(fogColor.rgb, finalColor.rgb, vec3(fog_out)), finalColor.a);
    }
//    finalColor.rgb = finalColor.rgb;


    //Forward rendering without lights
    gl_FragColor = finalColor;

    //Deferred rendering
    //gl_FragColor = finalColor;
//    gl_FragData[0] = vec4(vec3(fs_Depth), 1.0);
//    gl_FragData[1] = vec4(vPosition.xyz,0);
//    gl_FragData[2] = vec4(vNormal.xyz,0);
//    gl_FragData[3] = finalColor;

}

#endif //COMPILING_FS


