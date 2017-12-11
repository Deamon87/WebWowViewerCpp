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

uniform vec4 uColor;
uniform vec3 uFogColor;

//uniform vec4  uGlobalLighting;
uniform float uAlphaTest;
uniform float uTransparency;
uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;

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
    vec2 texCoord3 = (uTextMat2 * vec4(vTexCoord2, 0, 1)).xy;


    /* Get color from texture */
    vec4 tex = texture2D(uTexture, texCoord).rgba;
    vec4 tex2 = texture2D(uTexture2, texCoord2).rgba;
    vec4 tex3 = texture2D(uTexture3, texCoord3).rgba;

    vec4 meshColor = uColor;

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
        //vec3 linearDiffuse = finalColor.rgb * finalColor.rgb * lightColor.rgb;

        vec3 sunDir = vec3(0.294422, -0.11700600000000004, 0.948486);
        vec3 sunLight = vec3(0.392941, 0.268235, 0.308235);
        lightColor  =  (lightColor + (clamp(dot(-sunDir, vNormal3), 0.0, 1.0) * sunLight) + vDiffuseColor.bgr) * meshColor.rgb;

        meshResColor.rgb = clamp(lightColor , 0.0, 1.0); //lightColor *  meshResColor.rgb;
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

    vec3 diffTerm = vec3(0.0);;
    vec3 specTerm = vec3(0.0);
    vec3 envTerm = vec3(0.0);;
    float finalAlpha;

    if ( uPixelShader == 0 ) {//Combiners_Opaque
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t989 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t989.rgb, t989.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t989.rgb, t989.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t989.rgb, t989.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 1 ) {//Combiners_Mod
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t989 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t989.rgb, t989.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t989.rgb, t989.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t989.rgb, t989.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 2 ) {//Combiners_Opaque_Mod
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb;
    opacity = texture(pt_map1,in_tc1).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 3 ) {//Combiners_Opaque_Mod2x
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000;
    opacity = texture(pt_map1,in_tc1).a * 2.000000 * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 4 ) {//Combiners_Opaque_Mod2xNA
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 5 ) {//Combiners_Opaque_Opaque
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 6 ) {//Combiners_Mod_Mod
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb;
    opacity = texture(pt_map0,in_tc0).a * texture(pt_map1,in_tc1).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 7 ) {//Combiners_Mod_Mod2x
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000;
    opacity = texture(pt_map0,in_tc0).a * texture(pt_map1,in_tc1).a * 2.000000 * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 8 ) {//Combiners_Mod_Add
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = (texture(pt_map0,in_tc0).a + texture(pt_map1,in_tc1).a) * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb, opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 9 ) {//Combiners_Mod_Mod2xNA
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 10 ) {//Combiners_Mod_AddNA
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb, opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 11 ) {//Combiners_Mod_Opaque
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 12 ) {//Combiners_Opaque_Mod2xNA_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000, texture(pt_map0,in_tc0).rgb, vec3(texture(pt_map0,in_tc0).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 13 ) {//Combiners_Opaque_AddAlpha
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * texture(pt_map1,in_tc1).a, in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 14 ) {//Combiners_Opaque_AddAlpha_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * texture(pt_map1,in_tc1).a * (1.000000 - texture(pt_map0,in_tc0).a), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 15 ) {//Combiners_Opaque_Mod2xNA_Alpha_Add
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000, texture(pt_map0,in_tc0).rgb, vec3(texture(pt_map0,in_tc0).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map2,in_tc2).rgb * texture(pt_map2,in_tc2).a * pc_genericParams[0].b, in_col0.a * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 16 ) {//Combiners_Mod_AddAlpha
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * texture(pt_map1,in_tc1).a, opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 17 ) {//Combiners_Mod_AddAlpha_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = (texture(pt_map0,in_tc0).a + texture(pt_map1,in_tc1).a * (0.300000 * texture(pt_map1,in_tc1).r + 0.590000 * texture(pt_map1,in_tc1).g + 0.110000 * texture(pt_map1,in_tc1).b)) * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * texture(pt_map1,in_tc1).a * (1.000000 - texture(pt_map0,in_tc0).a), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 18 ) {//Combiners_Opaque_Alpha_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * mix(mix(texture(pt_map0,in_tc0).rgb, texture(pt_map1,in_tc1).rgb, vec3(texture(pt_map1,in_tc1).a)), texture(pt_map0,in_tc0).rgb, vec3(texture(pt_map0,in_tc0).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 19 ) {//Combiners_Opaque_Mod2xNA_Alpha_3s
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000, texture(pt_map2,in_tc2).rgb, vec3(texture(pt_map2,in_tc2).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 20 ) {//Combiners_Opaque_AddAlpha_Wgt
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * texture(pt_map1,in_tc1).a * pc_genericParams[0].g, in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 21 ) {//Combiners_Mod_Add_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = (texture(pt_map0,in_tc0).a + texture(pt_map1,in_tc1).a) * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * (1.000000 - texture(pt_map0,in_tc0).a), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 22 ) {//Combiners_Opaque_ModNA_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb, texture(pt_map0,in_tc0).rgb, vec3(texture(pt_map0,in_tc0).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 23 ) {//Combiners_Mod_AddAlpha_Wgt
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map1,in_tc1).rgb * texture(pt_map1,in_tc1).a * pc_genericParams[0].g, opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 24 ) {//Combiners_Opaque_Mod_Add_Wgt
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb, texture(pt_map1,in_tc1).rgb, vec3(texture(pt_map1,in_tc1).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map0,in_tc0).rgb * texture(pt_map0,in_tc0).a * pc_genericParams[0].r, in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 25 ) {//Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000, texture(pt_map0,in_tc0).rgb, vec3(texture(pt_map0,in_tc0).a)) * (1.000000 - glowOpacity);
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm) + texture(pt_map2,in_tc2).rgb * glowOpacity, in_col0.a * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 26 ) {//Combiners_Mod_Dual_Crossfade
    matDiffuse = in_col0.rgb * 2.000000 * mix(mix(texture(pt_map0,in_tc0), texture(pt_map1,in_tc0), vec4(clamp(pc_genericParams[0].g, 0.000000, 1.000000))), texture(pt_map2,in_tc0), vec4(clamp(pc_genericParams[0].b, 0.000000, 1.000000))).rgb;
    opacity = mix(mix(texture(pt_map0,in_tc0), texture(pt_map1,in_tc0), vec4(clamp(pc_genericParams[0].g, 0.000000, 1.000000))), texture(pt_map2,in_tc0), vec4(clamp(pc_genericParams[0].b, 0.000000, 1.000000))).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 27 ) {//Combiners_Opaque_Mod2xNA_Alpha_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * mix(mix(texture(pt_map0,in_tc0).rgb * texture(pt_map1,in_tc1).rgb * 2.000000, texture(pt_map2,in_tc2).rgb, vec3(texture(pt_map2,in_tc2).a)), texture(pt_map0,in_tc0).rgb, vec3(texture(pt_map0,in_tc0).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 28 ) {//Combiners_Mod_Masked_Dual_Crossfade
    matDiffuse = in_col0.rgb * 2.000000 * mix(mix(texture(pt_map0,in_tc0), texture(pt_map1,in_tc0), vec4(clamp(pc_genericParams[0].g, 0.000000, 1.000000))), texture(pt_map2,in_tc0), vec4(clamp(pc_genericParams[0].b, 0.000000, 1.000000))).rgb;
    opacity = mix(mix(texture(pt_map0,in_tc0), texture(pt_map1,in_tc0), vec4(clamp(pc_genericParams[0].g, 0.000000, 1.000000))), texture(pt_map2,in_tc0), vec4(clamp(pc_genericParams[0].b, 0.000000, 1.000000))).a * texture(pt_map3,in_tc1).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t995 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t995.rgb, t995.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t995.rgb, t995.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t995.rgb, t995.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 29 ) {//Combiners_Opaque_Alpha
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb, texture(pt_map1,in_tc1).rgb, vec3(texture(pt_map1,in_tc1).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 30 ) {//Guild
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * mix(pc_genericParams[0].rgb, texture(pt_map1,in_tc1).rgb * pc_genericParams[1].rgb, vec3(texture(pt_map1,in_tc1).a)), texture(pt_map2,in_tc2).rgb * pc_genericParams[2].rgb, vec3(texture(pt_map2,in_tc2).a));
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 31 ) {//Guild_NoBorder
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb * mix(pc_genericParams[0].rgb, texture(pt_map1,in_tc1).rgb * pc_genericParams[1].rgb, vec3(texture(pt_map1,in_tc1).a));
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t991 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t991.rgb, t991.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 32 ) {//Guild_Opaque
    matDiffuse = in_col0.rgb * 2.000000 * mix(texture(pt_map0,in_tc0).rgb * mix(pc_genericParams[0].rgb, texture(pt_map1,in_tc1).rgb * pc_genericParams[1].rgb, vec3(texture(pt_map1,in_tc1).a)), texture(pt_map2,in_tc2).rgb * pc_genericParams[2].rgb, vec3(texture(pt_map2,in_tc2).a));
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), in_col0.a * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, in_col0.a * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 33 ) {//Combiners_Mod_Depth
    matDiffuse = in_col0.rgb * 2.000000 * texture(pt_map0,in_tc0).rgb;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a * pc_visParams.r;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t989 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), t989.rgb, vec3(min)), opacity * pc_visParams.r);
    }
    else if ( uPixelShader == 34 ) {//Illum
    specTerm = vec3(mix(0.000000, 1.000000, texture(pt_map1,in_tc0).rgb.r)) * (texture(pt_map1,in_tc0).a * 64.000000 + 2.000000)/8.000000 * 0.000000 * pc_sunColor.rgb * dirAtten * 0.800000 * 1.000000;
    opacity = texture(pt_map0,in_tc0).a * in_col0.a;
    final = vec4(mix(vec4(0.000000, 0.000000, 0.000000 + specTerm, opacity * pc_visParams.r).rgb, vec4(0.000000, 0.000000, 0.000000 + specTerm, opacity * pc_visParams.r).rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(vec4(0.000000, 0.000000, 0.000000 + specTerm, opacity * pc_visParams.r).rgb, vec4(0.000000, 0.000000, 0.000000 + specTerm, opacity * pc_visParams.r).rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(vec4(0.000000, 0.000000, 0.000000 + specTerm, opacity * pc_visParams.r).rgb, vec4(0.000000, 0.000000, 0.000000 + specTerm, opacity * pc_visParams.r).rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);
    }
    else if ( uPixelShader == 35 ) {//Combiners_Mod_Mod_Mod_Const
    matDiffuse = in_col0.rgb * 2.000000 * (texture(pt_map0,in_tc0) * texture(pt_map1,in_tc1) * texture(pt_map2,in_tc2) * pc_genericParams[0]).rgb;
    opacity = (texture(pt_map0,in_tc0) * texture(pt_map1,in_tc1) * texture(pt_map2,in_tc2) * pc_genericParams[0]).a * in_col0.a;
    gammaDiffTerm = matDiffuse * vec3(1.000000);
    t993 = vec4(sqrt(gammaDiffTerm * gammaDiffTerm), opacity * pc_visParams.r);
    final = vec4(mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).r, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).g, mix(t993.rgb, t993.rgb * pc_impactColor.rgb * 3.000000 * sqrt(1.000000 - lum), vec3(pc_impactColor.a * finalDistLength)).b, opacity * pc_visParams.r);
    out_col0 = vec4(mix(mix(mix(color_and_heightRate.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a * length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), mix(heightColor_and_endFogDistance.rgb, heightDensity_and_endColor.gba, vec3(clamp(length/heightColor_and_endFogDistance.a, 0.000000, 1.000000))), vec3(heightFog)), final.rgb, vec3(min)), final.a);

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

    finalColor.rgb = diffTerm + envTerm + specTerm;
    finalColor.a = finalAlpha;

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

