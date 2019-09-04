#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;

//Whole scene
layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
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

        vDiffuseColor = vec4(combinedColorHalved.r, combinedColorHalved.g, combinedColorHalved.b, combinedColor.a);
        vTexCoord = (uTextMat[0] * vec4(aTexCoord, 0.000000, 1.000000)).xy;



    gl_Position = uPMatrix * cameraPoint;
    gl_Position.y *= -1;
    vNormal = normal;
    vPosition = cameraPoint.xyz;
}


