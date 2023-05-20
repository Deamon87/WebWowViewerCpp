#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

precision highp float;
precision highp int;

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
};

layout(std140, set=0, binding=2) uniform boneMats {
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

layout(std140, set=0, binding=3) uniform textureMatrices {
    mat4 textureMatrix[64];
};

//Individual meshes
layout(std140, set=0, binding=4) uniform meshWideBlockVS {
    vec4 bumpScaleTextIndexes;
};

//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord2_animated;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec3 vPosition;


layout(set=1,binding=9) uniform sampler2D uBumpTexture;

void main() {
    float bumpScale = bumpScaleTextIndexes.x;
    int textMatIndex1 = floatBitsToInt(bumpScaleTextIndexes.y);
    int textMatIndex2 = floatBitsToInt(bumpScaleTextIndexes.z);

    mat4 textMat[2];
    textMat[0] = textMatIndex1 < 0 ? mat4(1.0) : textureMatrix[textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? mat4(1.0) : textureMatrix[textMatIndex2];

    vec2 texCoord2 = (textMat[0] * vec4(aTexCoord2, 0, 1)).xy;

    vec4 bumpValue = texture(uBumpTexture, texCoord2);
    vec3 pos = (aNormal * bumpScale.x) * bumpValue.z + aPosition;

    mat4 boneTransformMat =  mat4(0.0);

    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[bones.x];
    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[bones.y];
    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[bones.z];
    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[bones.w];

    mat4 viewModelMat = scene.uLookAtMat * uPlacementMat  * boneTransformMat ;
    vec4 cameraPoint = viewModelMat * vec4(pos, 1.0);

    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));
    vec3 normal = normalize(viewModelMatForNormal * vec4(aNormal, 0.0)).xyz;

    vNormal = (scene.uLookAtMat * uPlacementMat * vec4(aNormal, 0)).xyz;
    vPosition = pos;

    vTexCoord = aTexCoord;
    vTexCoord2_animated = texCoord2;
    vTexCoord2 = aTexCoord2;

    gl_Position = scene.uPMatrix * cameraPoint;
}
