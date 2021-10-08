#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;

//Whole scene
layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

// Whole model
layout(std140, set=0, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

//Individual meshes
layout(std140, set=0, binding=2) uniform meshWideBlockVS {
    ivec4 vertexShader_IsAffectedByLight;
    vec4 color_Transparency;
    mat4 uTextMat[2];
};

//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord3;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec3 vPosition;
layout(location=5) out vec4 vDiffuseColor;

#include "../common/commonFunctions.glsl"

void main() {
    vec4 modelPoint = vec4(0,0,0,0);

    vec4 aPositionVec4 = vec4(aPosition, 1);
    mat4 boneTransformMat =  mat4(0.0);

//
    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[bones.x];
    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[bones.y];
    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[bones.z];
    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[bones.w];


    mat4 placementMat;
    placementMat = uPlacementMat;

    vec4 lDiffuseColor = color_Transparency;

    mat4 cameraMatrix = scene.uLookAtMat * placementMat  * boneTransformMat ;
    vec4 cameraPoint = cameraMatrix * aPositionVec4;
    mat3 viewModelMatTransposed =
        blizzTranspose(scene.uLookAtMat) *
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

    if ( uVertexShader == 0 ) {//Diffuse_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 1 ) {//Diffuse_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
    } else if ( uVertexShader == 2 ) {//Diffuse_T1_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 3 ) {//Diffuse_T1_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = envCoord;
    } else if ( uVertexShader == 4 ) {//Diffuse_Env_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
        vTexCoord2 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 5 ) {//Diffuse_Env_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = envCoord;
        vTexCoord2 = envCoord;
    } else if ( uVertexShader == 6 ) {//Diffuse_T1_Env_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000) ).xy;
        vTexCoord2 = envCoord;
        vTexCoord3 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 7 ) {//Diffuse_T1_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 8 ) {//Diffuse_T1_T1_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord3 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 9 ) {//Diffuse_EdgeFade_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = ((uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy).xy;
    } else if ( uVertexShader == 10 ) {//Diffuse_T2

        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 11 ) {//Diffuse_T1_Env_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = envCoord;
        vTexCoord3 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 12 ) {//Diffuse_EdgeFade_T1_T2
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 13 ) {//Diffuse_EdgeFade_Env
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a * edgeScanVal);
        vTexCoord = envCoord;
    } else if ( uVertexShader == 14 ) {//Diffuse_T1_T2_T1
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
        vTexCoord3 = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 15 ) {//Diffuse_T1_T2_T3
        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
        vTexCoord2 = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
        vTexCoord3 = vTexCoord3;
    } else if ( uVertexShader == 16 ) {//Color_T1_T2_T3
        vec4 in_col0 = vec4(1.0, 1.0, 1.0, 1.0);
        vDiffuseColor = vec4((in_col0.rgb * 0.500000).r, (in_col0.rgb * 0.500000).g, (in_col0.rgb * 0.500000).b, in_col0.a);
        vTexCoord = (uTextMat[1] * vec4(aTexCoord2, 0.000000, 1.000000)).xy;
        vTexCoord2 = vec2(0.000000, 0.000000);
        vTexCoord3 = vTexCoord3;
    } else if ( uVertexShader == 17 ) {//BW_Diffuse_T1
        vDiffuseColor = vec4(combinedColor.rgb * 0.500000, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    } else if ( uVertexShader == 18 ) {//BW_Diffuse_T1_T2
        vDiffuseColor = vec4(combinedColor.rgb * 0.500000, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;
    }


    gl_Position = scene.uPMatrix * cameraPoint;
    vNormal = normal;
    vPosition = cameraPoint.xyz;


}


