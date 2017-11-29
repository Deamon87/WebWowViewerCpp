//https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
//For drawbuffers in glsl of webgl you need to use GL_EXT_draw_buffers instead of WEBGL_draw_buffers

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
attribute vec2 aTexCoord;
attribute vec2 aTexCoord2;
attribute vec2 aTexCoord3;
attribute vec4 aColor;
attribute vec4 aColor2;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
uniform int uVertexShader;

uniform int uUseLitColor;

uniform mat4 uPlacementMat;
uniform vec4 uAmbientLight;

varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec2 vTexCoord3;
varying vec4 vColor;
varying vec4 vColor2;
varying vec3 vPosition;


#ifdef drawBuffersIsSupported
vec3 vNormal;
varying float fs_Depth;
#endif

void main() {
    vec4 worldPoint = uPlacementMat * vec4(aPosition, 1);

    vec4 cameraPoint = uLookAtMat * worldPoint;

#ifndef drawBuffersIsSupported
    vec3 vNormal;
    gl_Position = uPMatrix * cameraPoint;
    vPosition = cameraPoint.xyz;
    vNormal = normalize((uPlacementMat * vec4(aNormal, 0)).xyz);
#else
    gl_Position = uPMatrix * cameraPoint;
    fs_Depth = gl_Position.z / gl_Position.w;

    vNormal = normalize((uPlacementMat * vec4(aNormal, 0)).xyz);
    vPosition = cameraPoint.xyz;
#endif //drawBuffersIsSupported

    if (uUseLitColor == 0) {
        vColor.rgba = vec4(vec3(0.5, 0.499989986, 0.5), 1.0);
    } else {
        vColor.rgb = clamp(aColor.bgr + uAmbientLight.bgr,  vec3(0.0), vec3(1.0)) ;
        vColor.a = aColor.a;
//        vColor.rgba = vec4(vec3(0.5, 0.499989986, 0.5), 1.0);
    }
    vColor2 = aColor2.bgra;

    if ( uVertexShader == -1 ) {
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2;
        vTexCoord3 = aTexCoord3;
    } else if (uVertexShader == 0) { //MapObjDiffuse_T1
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2; //not used
        vTexCoord3 = aTexCoord3; //not used

        vColor2 = vec4((aColor.bgr * 2.0), aColor2.a);
    } else if (uVertexShader == 1) { //MapObjDiffuse_T1_Refl
        vTexCoord = aTexCoord;
        vTexCoord2 = reflect(normalize(cameraPoint.xyz), vNormal).xy;
        vTexCoord3 = aTexCoord3; //not used

        vColor2 = vec4((aColor.bgr * 2.0), aColor2.a);
    } else if (uVertexShader == 2) { //MapObjDiffuse_T1_Env_T2
        vTexCoord = aTexCoord;

        vec3 normPos = -(normalize(vPosition));
        vec3 temp1 = (normPos - (vNormal * (2.0 * dot(normPos, vNormal))));
        vec3 temp2 = vec3(temp1.x, temp1.y, (temp1.z + 1.0));

        vTexCoord2 = ((normalize(temp2).xy * 0.5) + vec2(0.5));
        vTexCoord3 = aTexCoord3;
    } else if (uVertexShader == 3) { //MapObjSpecular_T1
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2; //not used
        vTexCoord3 = aTexCoord3; //not used
    } else if (uVertexShader == 4) { //MapObjDiffuse_Comp
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2; //not used
        vTexCoord3 = aTexCoord3; //not used

        vColor2 = vec4((aColor.bgr * 2.0), aColor2.a);
    } else if (uVertexShader == 5) { //MapObjDiffuse_Comp_Refl
        vTexCoord = aTexCoord;
        vTexCoord2 = aTexCoord2;
        vTexCoord3 = reflect(normalize(cameraPoint.xyz), vNormal).xy;
    } else if (uVertexShader == 6) { //MapObjDiffuse_Comp_Terrain
        vTexCoord = aTexCoord;
        vTexCoord2 = vPosition.xy * -0.239999995;
        vTexCoord3 = aTexCoord3; //not used
    }


}
#endif //COMPILING_VS

#ifdef COMPILING_FS

precision lowp float;
varying vec3 vNormal;
varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec2 vTexCoord3;
varying vec4 vColor;
varying vec4 vColor2;
varying vec3 vPosition;

//uniform vec4  uGlobalLighting;
uniform float uAlphaTest;
uniform vec4 uMeshColor1;
uniform vec4 uMeshColor2;
uniform vec4 uAmbientLight;
uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;

uniform int uEnableAlpha;

uniform vec3 uFogColor;

uniform float uFogStart;
uniform float uFogEnd;

uniform int uPixelShader;

#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

vec3 makeDiffTerm(vec3 matDiffuse) {
//    vec3 lDiffuse = uAmbientLight.rgb;
//    float mag = length(lDiffuse);
//    if ((mag > 1.0))
//    {
//        lDiffuse = ((lDiffuse * (1.0 + log(mag))) / vec3(mag));
//    }


    return matDiffuse ;
}

void main() {
    vec4 tex = texture2D(uTexture, vTexCoord).rgba ;
    vec4 tex2 = texture2D(uTexture2, vTexCoord2).rgba;
    vec4 tex3 = texture2D(uTexture3, vTexCoord3).rgba;

    if (uEnableAlpha == 1) {
        if ((tex.a - 0.501960814) < 0.0) {
            discard;
        }
    }

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    if ( uPixelShader == -1 ) {
        finalColor = vec4(tex.rgb * vColor.rgb + tex2.rgb*vColor2.bgr, tex.a);
    } else if (uPixelShader == 0) { //MapObjDiffuse

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);

    } else if (uPixelShader == 1) { //MapObjSpecular

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);

    } else if (uPixelShader == 2) { //MapObjMetal

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);

    } else if (uPixelShader == 3) { //MapObjEnv

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = tex2.rgb * tex.a;

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);

    } else if (uPixelShader == 4) { //MapObjOpaque

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);

    } else if (uPixelShader == 5) { //MapObjEnvMetal

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = (tex.rgb * tex.a) * tex2.rgb;

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);

    } else if (uPixelShader == 6) { //MapObjTwoLayerDiffuse

        vec3 layer1 = tex.rgb;
        vec3 layer2 = mix(layer1, tex2.rgb, tex2.a);
        vec3 matDiffuse = (vColor.rgb * 2.0) * mix(layer2, layer1, vColor2.a);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), 1.0);
    } else if (uPixelShader == 7) { //MapObjTwoLayerEnvMetal

        vec4 colorMix = mix(tex2, tex, vColor2.a);
        vec3 env = (colorMix.rgb * colorMix.a) * tex3.rgb;
        vec3 matDiffuse = colorMix.rgb * (2.0 * vColor.rgb);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);

    } else if (uPixelShader == 8) { //MapObjTwoLayerTerrain

        vec3 layer1 = tex.rgb;
        vec3 layer2 = tex2.rgb;

        vec3 matDiffuse = ((vColor.rgb * 2.0) * mix(layer2, layer1, vColor2.a));
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);

    } else if (uPixelShader == 9) { //MapObjDiffuseEmissive

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = tex2.rgb * tex2.a * vColor2.a;

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);

    } else if (uPixelShader == 10) { //MapObjMaskedEnvMetal

        float mixFactor = clamp((tex3.a * vColor2.a), 0, 1);
        vec3 matDiffuse =
            (vColor.rgb * 2.0) *
            mix(mix(((tex.rgb * tex2.rgb) * 2.0), tex3.rgb, mixFactor), tex.rgb, tex.a);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);

    } else if (uPixelShader == 11) { //MapObjEnvMetalEmissive
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env =
            (
                ((tex.rgb * tex.a) * tex2.rgb) +
                ((tex3.rgb * tex3.a) * vColor2.a)
            );

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    } else if (uPixelShader == 12) { //MapObjTwoLayerDiffuseOpaque
        vec3 matDiffuse =
            (vColor.rgb * 2.0) *
            mix(tex2.rgb, tex.rgb, vColor2.a);

        finalColor.rgba = vec4(makeDiffTerm(matDiffuse), vColor.a);
    } else if (uPixelShader == 13) { //MapObjTwoLayerDiffuseEmissive
        vec3 t1diffuse = (tex2.rgb * (1.0 - tex2.a));

        vec3 matDiffuse =
            ((vColor.rgb * 2.0) *
            mix(t1diffuse, tex.rgb, vColor2.a));

        //TODO: there is env missing here
        vec3 env = ((tex2.rgb * tex2.a) * (1.0 - vColor2.a));
        finalColor.rgba = vec4(makeDiffTerm(matDiffuse)+env, vColor.a);
    };

    //finalColor.rgb *= 4.0;

    if(finalColor.a < uAlphaTest)
        discard;

    vec3 fogColor = uFogColor;
    float fog_start = uFogStart;
    float fog_end = uFogEnd;
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


    //Apply global lighting
/*
    finalColor = vec4(
        (finalColor.r + uGlobalLighting.r) ,
        (finalColor.g + uGlobalLighting.g) ,
        (finalColor.b + uGlobalLighting.b) ,
        finalColor.a);
  */
    finalColor.a = 1.0; //do I really need it now?

//#ifndef drawBuffersIsSupported
    //Forward rendering without lights
    gl_FragColor = finalColor;
//#else
    //Deferred rendering
    //gl_FragColor = finalColor;
//    gl_FragData[0] = vec4(vec3(fs_Depth), 1.0);
//    gl_FragData[1] = vec4(vPosition.xyz,0);
//    gl_FragData[2] = vec4(vNormal.xyz,0);
//    gl_FragData[3] = finalColor;
//#endif //drawBuffersIsSupported
}

#endif //COMPILING_FS
