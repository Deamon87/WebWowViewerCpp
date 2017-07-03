#version 120
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
uniform int isEnviroment;
uniform lowp int isTransparent;

uniform lowp int uUseDiffuseColor;

#ifdef INSTANCED
attribute vec4 aDiffuseColor;
attribute mat4 aPlacementMat;

#else
uniform mat4 uPlacementMat;
uniform vec4 uDiffuseColor;
#endif

varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec3 vNormal;
varying vec3 vPosition;
varying vec4 vDiffuseColor;




#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

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

    mat4 cameraMatrix = uLookAtMat * placementMat  * boneTransformMat ;
    vec4 cameraPoint = cameraMatrix * aPositionVec4;

    vec3 normal = mat3(cameraMatrix) * aNormal;

    vec2 texCoord;
    if (isEnviroment == 1) {

        vec3 normPos = -(normalize(cameraPoint.xyz));
        vec3 temp = reflect(normPos, normal);
        temp.z += 1.0;

        texCoord = ((normalize(temp).xy * 0.5) + vec2(0.5));

    } else {
        /* Animation support */
        texCoord = vec4(aTexCoord, 0, 1).xy;
    }

    vTexCoord = texCoord;
    vTexCoord2 = aTexCoord2;

    if ((uUseDiffuseColor == 1)) {

#ifdef INSTANCED
    vDiffuseColor = aDiffuseColor;
#else
    vDiffuseColor = uDiffuseColor;
#endif
    } else {
        vDiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
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

precision mediump float;
varying vec3 vNormal;
varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec3 vPosition;

varying vec4 vDiffuseColor;

uniform lowp int isTransparent;

uniform int uBlendMode;
uniform int uPixelShader;
uniform int uUnFogged;

uniform vec4 uColor;
uniform vec3 uFogColor;

//uniform vec4  uGlobalLighting;
uniform float uAlphaTest;
uniform float uTransparency;
uniform sampler2D uTexture;
uniform sampler2D uTexture2;

uniform mat4 uTextMat1;
uniform mat4 uTextMat2;

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

void main() {
    /* Animation support */
    vec2 texCoord = (uTextMat1 * vec4(vTexCoord, 0, 1)).xy;
    vec2 texCoord2 = (uTextMat2 * vec4(vTexCoord2, 0, 1)).xy;

    /* Get color from texture */
    vec4 tex = texture2D(uTexture, texCoord).rgba;
    vec4 tex2 = texture2D(uTexture2, texCoord2).rgba;

    vec4 meshColor = uColor;
    /*if (uBlendMode == 6) {
       meshColor.rbg *= vec3(0.65);
    }*/

    vec4 finalColor = vec4(0);
    //finalColor = vec4((tex.rgb * tex2.rgb), 1.0);
    //finalColor.rgb = finalColor.rgb * meshColor.rgb * vDiffuseColor.bgr;
    //finalColor.a = tex.a * tex2.a * uColor.a* uTransparency;
    //vec4 meshResColor = vec4(meshColor.rgb * vDiffuseColor.bgr, uColor.a* uTransparency);
    vec4 meshResColor = vec4(meshColor.rgb * vDiffuseColor.bgr, uColor.a* uTransparency);

    if(meshResColor.a < uAlphaTest)
        discard;

    if ((uUseDiffuseColor == 1)) {
        vec3 vPos3 = vPosition.xyz;
        vec3 vNormal3 = normalize(vNormal.xyz);
        int count = int(pc_lights[0].attenuation.w);
        vec3 lightColor = vec3(0.0);

        for (int index = 0;index < 4;index++)
        {
            if ( index >= uLightCount) break;
            LocalLight lightRecord = pc_lights[index];
            vec3 vectorToLight = ((lightRecord.position).xyz - vPos3);
            float distanceToLightSqr = dot(vectorToLight, vectorToLight);
            float distanceToLightInv = inversesqrt(distanceToLightSqr);
            float distanceToLight = (distanceToLightSqr * distanceToLightInv);
            float diffuseTerm = max((dot(vectorToLight, vNormal3) * distanceToLightInv), 0.0);
            vec4 attenuationRec = lightRecord.attenuation;
            //float attenuationDiv = (1.0 ) / (( attenuationRec.z -  attenuationRec.x));
            float atten1 = distanceToLightSqr * 0.03 + distanceToLight*0.7;
            float atten2 = 1.0 / atten1;
            float lightAtten = diffuseTerm * atten2;

            //float attenuation = (1.0 - clamp(((distanceToLight - attenuationRec.x) * attenuationDiv), 0.0, 1.0));
            //vec3 light_atten = ((lightRecord.color.xyz ) * attenuation * diffuseTerm)   ;
            lightColor = (lightColor + (lightRecord.color.xyz * attenuationRec.y * lightAtten));
            //lightColor = (lightColor + vec3(light_atten * light_atten * diffuseTerm ));
        }

        //vec3 gammaDiffuse = finalColor.rgb;
        //vec3 gammaDiffuse = uPcColor.rgb;
        //vec3 linearDiffuse =finalColor.rgb * finalColor.rgb * lightColor.rgb;

        vec3 sunDir = vec3(0.294422, -0.11700600000000004, 0.948486);
        vec3 sunLight = vec3(0.392941, 0.268235, 0.308235);
        lightColor  =  (lightColor + (clamp(dot(-sunDir, vNormal3), 0.0, 1.0) * sunLight) + vDiffuseColor.bgr) * meshColor.rgb;

        meshResColor.rgb = clamp(lightColor , 0.0, 1.0); //lightColor *  meshResColor.rgb;
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

    if (uPixelShader == 0) { //Combiners_Opaque
        finalColor.rgb = tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == 1) { // Combiners_Decal
        finalColor.rgb = (meshResColor.rgb - tex.rgb) * meshResColor.a + tex.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == 2) { // Combiners_Add
        finalColor.rgba = tex.rgba + meshResColor.rgba;
    } else if (uPixelShader == 3) { // Combiners_Mod2x
        finalColor.rgb = tex.rgb * meshResColor.rgb * vec3(2.0);
        finalColor.a = tex.a * meshResColor.a * 2.0;
    } else if (uPixelShader == 4) { // Combiners_Fade
        finalColor.rgb = (tex.rgb - meshResColor.rgb) * meshResColor.a + meshResColor.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == 5) { // Combiners_Mod
        finalColor.rgba = tex.rgba * meshResColor.rgba;
    } else if (uPixelShader == 6) { // Combiners_Opaque_Opaque
        finalColor.rgb = tex.rgb * tex2.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == 7) { // Combiners_Opaque_Add
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a + tex.a;
    } else if (uPixelShader == 8) { // Combiners_Opaque_Mod2x
        finalColor.rgb = tex.rgb * meshResColor.rgb * tex2.rgb * vec3(2.0);
        finalColor.a  = tex2.a * meshResColor.a * 2.0;
    } else if (uPixelShader == 9) { // Combiners_Opaque_Mod2xNA
        finalColor.rgb = tex.rgb * meshResColor.rgb * tex2.rgb * vec3(2.0);
        finalColor.a  = meshResColor.a;
    } else if (uPixelShader == 10) { // Combiners_Opaque_AddNA
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = meshResColor.a;
    } else if (uPixelShader == 11) { // Combiners_Opaque_Mod
        finalColor.rgb = tex.rgb * tex2.rgb * meshResColor.rgb;
        finalColor.a = tex2.a * meshResColor.a;
    } else if (uPixelShader == 12) { // Combiners_Mod_Opaque
        finalColor.rgb = tex.rgb * tex2.rgb * meshResColor.rgb;
        finalColor.a = tex.a;
    } else if (uPixelShader == 13) { // Combiners_Mod_Add
        finalColor.rgba = tex2.rgba + tex.rgba * meshResColor.rgba;
    } else if (uPixelShader == 14) { // Combiners_Mod_Mod2x
        finalColor.rgba = tex.rgba * tex2.rgba * meshResColor.rgba * vec4(2.0);
    } else if (uPixelShader == 15) { // Combiners_Mod_Mod2xNA
        finalColor.rgb = tex.rgb * tex2.rgb * meshResColor.rgb * vec3(2.0);
        finalColor.a = tex.a * meshResColor.a;
    } else if (uPixelShader == 16) { // Combiners_Mod_AddNA
        finalColor.rgb = tex2.rgb + tex.rgb * meshResColor.rgb;
        finalColor.a = tex.a * meshResColor.a;
    } else if (uPixelShader == 17) { // Combiners_Mod_Mod
        finalColor.rgba = tex.rgba * tex2.rgba * meshResColor.rgba;
    } else if (uPixelShader == 18) { // Combiners_Add_Mod
        finalColor.rgb = (tex.rgb + meshResColor.rgb) * tex2.a;
        finalColor.a = (tex.a + meshResColor.a) * tex2.a;
    } else if (uPixelShader == 19) { // Combiners_Mod2x_Mod2x
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
    finalColor.rgb = finalColor.rgb;

#ifndef drawBuffersIsSupported
    //Forward rendering without lights
    gl_FragColor = finalColor;
#else
    //Deferred rendering
    //gl_FragColor = finalColor;
    gl_FragData[0] = vec4(vec3(fs_Depth), 1.0);
    gl_FragData[1] = vec4(vPosition.xyz,0);
    gl_FragData[2] = vec4(vNormal.xyz,0);
    gl_FragData[3] = finalColor;
#endif //drawBuffersIsSupported
}

#endif //COMPILING_FS

