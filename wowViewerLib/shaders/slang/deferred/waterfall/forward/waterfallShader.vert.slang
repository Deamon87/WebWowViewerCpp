#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier: require

precision highp float;
precision highp int;

#define M2_WATERFALLSHADER

#include "../../../common/commonFunctions.slang"
#include "../../../common/commonM2Material.slang"
#include "../../../common/commonLightFunctions.slang"
#include "../../../common/commonFogFunctions.slang"
#include "../../../common/commonUboSceneData.slang"

precision highp float;

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;


#include "../../../common/commonM2DescriptorSet.slang"
#include "../../../common/commonM2WaterfallDescriptorSet.slang"


//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord2_animated;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec3 vPosition;
layout(location=5) out flat int meshInd;

[shader("vertex")]
void main() {
    float bumpScale = waterfallCommon.bumpScale_textTransformInd1_textTransformInd2.x;
    int textMatIndex1 = floatBitsToInt(waterfallCommon.bumpScale_textTransformInd1_textTransformInd2.y);
    int textMatIndex2 = floatBitsToInt(waterfallCommon.bumpScale_textTransformInd1_textTransformInd2.z);

    mat4 textMat[2];
    textMat[0] = textMatIndex1 < 0 ? ident4() : textureMatrix[textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? ident4() : textureMatrix[textMatIndex2];

    vec2 texCoord2 = (textMat[0] * vec4(aTexCoord2, 0, 1)).xy;

    vec4 bumpValue = texture2DLod(uBumpTexture, texCoord2, 0);
    vec3 pos = (aNormal * bumpScale) * bumpValue.z + aPosition;

//    mat4 boneTransformMat =  mat4(0.0);

//    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[boneMatInd + bones.x];
//    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[boneMatInd + bones.y];
//    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[boneMatInd + bones.z];
//    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[boneMatInd + bones.w];

    mat4 placementMat = placementMats.placementMat;
//    mat4 viewModelMat = scene.uLookAtMat * placementMat * boneTransformMat ;
    mat4 viewModelMat = scene.uLookAtMat * placementMat;
    vec4 cameraPoint = viewModelMat * vec4(pos, 1.0);

    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));
    vec3 normal = normalize((viewModelMatForNormal * vec4(aNormal, 0.0)).xyz);

    vNormal = normal;
    vPosition = cameraPoint.xyz;

    vTexCoord = aTexCoord;
    vTexCoord2_animated = texCoord2;
    vTexCoord2 = aTexCoord2;

    gl_Position = scene.uPMatrix * cameraPoint;

    meshInd = gl_InstanceIndex;
}
