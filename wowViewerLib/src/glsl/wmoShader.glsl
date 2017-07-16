//https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
//For drawbuffers in glsl of webgl you need to use GL_EXT_draw_buffers instead of WEBGL_draw_buffers

#ifdef ENABLE_DEFERRED
#ifdef GL_EXT_draw_buffers
    #extension GL_EXT_draw_buffers: require
    #extension OES_texture_float_linear : enable
    #define drawBuffersIsSupported 1
#endif
#endif

/*
//0
MapObjDiffuse {
    VertexShader(MapObjDiffuse_T1);
	PixelShader(MapObjDiffuse);
}

//1
MapObjSpecular {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjSpecular);
}

//2
MapObjMetal {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjMetal);
}

//3
MapObjEnv {
	VertexShader(MapObjDiffuse_T1_Refl);
	PixelShader(MapObjEnv);
}

//4
MapObjOpaque {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjOpaque);
}

//5
Effect(MapObjEnvMetal {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnvMetal);
}

//6
Effect(MapObjComposite)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjComposite);
}

pixel:
0 - MapObjDiffuse
1 - MapObjSpecular
2 - MapObjMetal
3 - MapObjEnv
4 - MapObjOpaque
5 - MapObjEnvMetal
6 - MapObjComposite

vertex:
0 - MapObjDiffuse_T1
1 - MapObjSpecular_T1
2 - MapObjDiffuse_T1_Refl
3 - MapObjDiffuse_T1_Refl
4 - MapObjDiffuse_Comp
*/


#ifdef COMPILING_VS
/* vertex shader code */
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTexCoord;
attribute vec2 aTexCoord2;
attribute vec4 aColor;
attribute vec4 aColor2;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
uniform int uVertexShader;

uniform mat4 uPlacementMat;

varying vec2 vTexCoord;
varying vec2 vTexCoord2;
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
varying vec4 vColor;
varying vec4 vColor2;
varying vec3 vPosition;

//uniform vec4  uGlobalLighting;
uniform float uAlphaTest;
uniform vec4 uMeshColor1;
uniform vec4 uMeshColor2;
uniform sampler2D uTexture;
uniform sampler2D uTexture2;

uniform vec3 uFogColor;

uniform float uFogStart;
uniform float uFogEnd;

uniform int uPixelShader;

#ifdef drawBuffersIsSupported
varying float fs_Depth;
#endif

void main() {
    vec4 tex = texture2D(uTexture, vTexCoord).rgba * uMeshColor1;
    vec4 tex2 = texture2D(uTexture2, vTexCoord2).rgba * uMeshColor2*0.00001;

    vec4 finalColor = vec4(tex.rgb * vColor.bgr + tex2.rgb*vColor2.bgr, tex.a);

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
