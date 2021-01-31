#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

#include "../common/commonFunctions.glsl"
#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;

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
    vec4 bumpScale;
    mat4 uTextMat[2];
};

//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord2_animated;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec3 vPosition;


layout(set=1,binding=8) uniform sampler2D uBumpTexture;

void main() {

    vec2 texCoord2 = (uTextMat[0] * vec4(aTexCoord2, 0, 1)).xy;

    vec4 bumpValue = texture(uBumpTexture, texCoord2);
    vec3 pos = (aNormal * bumpScale.x) * bumpValue.z + aPosition;

    mat4 boneTransformMat =  mat4(0.0);

    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[bones.x];
    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[bones.y];
    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[bones.z];
    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[bones.w];

    mat4 cameraMatrix = scene.uLookAtMat * uPlacementMat  * boneTransformMat ;
    vec4 cameraPoint = cameraMatrix * vec4(pos, 1.0);

    mat3 viewModelMatTransposed =
        blizzTranspose(scene.uLookAtMat) *
        blizzTranspose(uPlacementMat) *
        blizzTranspose(boneTransformMat);

    vNormal = normalize(viewModelMatTransposed * aNormal);
    vPosition = pos;

    vTexCoord = aTexCoord;
    vTexCoord2_animated = texCoord2;
    vTexCoord2 = aTexCoord2;

    gl_Position = scene.uPMatrix * cameraPoint;
}
