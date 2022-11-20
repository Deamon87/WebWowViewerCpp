#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonFunctions.glsl"
#include "../common/commonM2Material.glsl"

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
layout(location=5) out vec4 vMeshColorAlpha;


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

    mat4 viewModelMat = scene.uLookAtMat * placementMat  * boneTransformMat ;
    vec4 vertexPosInView = viewModelMat * aPositionVec4;
    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    vec3 normal = normalize(viewModelMatForNormal * vec4(aNormal, 0.0)).xyz;

    int uVertexShader = vertexShader_IsAffectedByLight.x;
    calcM2VertexMat(uVertexShader,
        vertexPosInView.xyz, normal,
        aTexCoord, aTexCoord2,
        color_Transparency, uTextMat,
        vMeshColorAlpha, vTexCoord, vTexCoord2, vTexCoord3);

    gl_Position = scene.uPMatrix * vertexPosInView;
    vNormal = normal;
    vPosition = vertexPosInView.xyz;
}


