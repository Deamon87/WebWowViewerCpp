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

uniform mat4 uPlacementMat;

varying vec2 vTexCoord;
varying vec2 vTexCoord2;
varying vec2 vTexCoord3;
varying vec4 vColor;
varying vec4 vColor2;
varying vec3 vPosition;

#ifdef drawBuffersIsSupported
varying vec3 vNormal;
varying float fs_Depth;
#endif

void main() {
    vec4 worldPoint = uPlacementMat * vec4(aPosition, 1);

    vec4 cameraPoint = uLookAtMat * worldPoint;

    vTexCoord = aTexCoord;
    vTexCoord2 = aTexCoord2;
    vColor = aColor;
    vColor2 = aColor2;

#ifndef drawBuffersIsSupported
    gl_Position = uPMatrix * cameraPoint;
    vPosition = cameraPoint.xyz;
#else
    gl_Position = uPMatrix * cameraPoint;
    fs_Depth = gl_Position.z / gl_Position.w;

    vNormal = normalize((uPlacementMat * vec4(aNormal, 0)).xyz);
    vPosition = cameraPoint.xyz;
#endif //drawBuffersIsSupported

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
uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;

uniform vec3 uFogColor;

uniform float uFogStart;
uniform float uFogEnd;

uniform int uPixelShader;

#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

void main() {
    vec4 tex = texture2D(uTexture, vTexCoord).rgba ;
    vec4 tex2 = texture2D(uTexture2, vTexCoord2).rgba;
    vec4 tex3 = texture2D(uTexture3, vTexCoord3).rgba;

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    if ( uPixelShader == -1 ) {
        finalColor = vec4(tex.rgb * vColor.bgr + tex2.rgb*vColor2.bgr, tex.a);
    } else if (uPixelShader == 0) { //MapObjDiffuse

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(diffuse, vColor.a);

    } else if (uPixelShader == 1) { //MapObjSpecular

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(diffuse, vColor.a);

    } else if (uPixelShader == 2) { //MapObjMetal

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(diffuse, vColor.a);

    } else if (uPixelShader == 3) { //MapObjEnv

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = tex2.rgb * tex.a;

        finalColor.rgba = vec4(matDiffuse+env, vColor.a);

    } else if (uPixelShader == 4) { //MapObjOpaque

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        finalColor.rgba = vec4(matDiffuse, vColor.a);

    } else if (uPixelShader == 5) { //MapObjEnvMetal

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = (tex.rgb * tex.a) * tex2.rgb;

        finalColor.rgba = vec4(matDiffuse+env, vColor.a);

    } else if (uPixelShader == 6) { //MapObjTwoLayerDiffuse

        vec3 layer1 = tex1;
        vec3 layer2 = mix(layer1, tex2.xyz, tex2.a);
        finalColor.rgb = ((vColor.xyz * 2.0) * mix(layer2, layer1, vColor2.a));
        finalColor.a = 1.0;

    } else if (uPixelShader == 7) { //MapObjTwoLayerEnvMetal

        vec4 colorMix = mix(tex2, tex1, vColor2.a);
        vec3 env = (colorMix.rgb * colorMix.a) * tex3.rgb;
        vec3 matDiffuse = colorMix.rgb * (2.0 * vColor.rgb);

        finalColor.rgba = vec4(matDiffuse+env, vColor.a);

    } else if (uPixelShader == 8) { //MapObjTwoLayerTerrain

        vec3 layer1 = tex1;
        vec3 layer2 = tex2.xyz;

        vec3 matDiffuse = ((vColor.xyz * 2.0) * mix(layer2, layer1, vColor2.a));
        finalColor.rgba = vec4(matDiffuse, vColor.a);

    } else if (uPixelShader == 9) { //MapObjDiffuseEmissive

        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env = tex2.rgb * tex2.a * vColor2.a;

        finalColor.rgba = vec4(matDiffuse+env, vColor.a);

    } else if (uPixelShader == 10) { //MapObjMaskedEnvMetal

        float mixFactor = clamp((tex3.a * vColor2.a), 0, 1);
        vec3 matDiffuse =
            (vColor.xyz * 2.0) *
            mix(mix(((tex.rgb * tex2.rgb) * 2.0), tex3.rgb, mixFactor), tex.rgb, tex.a);

        finalColor.rgba = vec4(matDiffuse, vColor.a);

    } else if (uPixelShader == 11) { //MapObjEnvMetalEmissive
        vec3 matDiffuse = tex.rgb * (2.0 * vColor.rgb);
        vec3 env =
            (
                ((tex.rgb * tex.a) * tex2.rgb) +
                ((tex3.rgb * tex3.a) * in_col1.w)
            );

        finalColor.rgba = vec4(matDiffuse+env, vColor.a);
    } else if (uPixelShader == 12) { //MapObjTwoLayerDiffuseOpaque
        vec3 matDiffuse =
            (vColor.rgb * 2.0) *
            mix(tex2.rgb, tex.rgb, vColor2.a);

        finalColor.rgba = vec4(matDiffuse, vColor.a);
    } else if (uPixelShader == 13) { //MapObjTwoLayerDiffuseEmissive
        vec3 t1diffuse = (tex2.rgb * (1.0 - tex2.a));

        vec3 matDiffuse =
            ((vColor.rgb * 2.0) *
            mix(t1diffuse, tex.rgb, vColor2.a));

        finalColor.rgba = vec4(matDiffuse+env, vColor.a);
    };

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


    if(finalColor.a < uAlphaTest)
        discard;

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
