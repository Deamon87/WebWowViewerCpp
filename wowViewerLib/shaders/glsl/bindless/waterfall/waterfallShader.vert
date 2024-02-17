#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier: require

precision highp float;
precision highp int;

#include "../../common/commonFunctions.glsl"
#include "../../common/commonM2Material.glsl"
#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"
#include "../../common/commonUboSceneData.glsl"

precision highp float;

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;


#include "../../common/commonM2IndirectDescriptorSet.glsl"
#include "../../common/commonM2WaterfallDescriptorSet.glsl"


//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord2_animated;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec3 vPosition;
layout(location=5) out flat int meshInd;

void main() {

    WaterfallBindless waterfallBindless = waterfallBindlesses[gl_InstanceIndex];
    int instanceIndex = waterfallBindless.instanceIndex_waterfallInd_bumpTextureInd_maskInd.x;

    M2InstanceRecordBindless instanceData = instances[nonuniformEXT(instanceIndex)];
    WaterfallCommon waterfallCommon = waterfallCommons[nonuniformEXT(waterfallBindless.instanceIndex_waterfallInd_bumpTextureInd_maskInd.y)];

    float bumpScale = waterfallCommon.bumpScale_textTransformInd1_textTransformInd2.x;
    int textMatIndex1 = floatBitsToInt(waterfallCommon.bumpScale_textTransformInd1_textTransformInd2.y);
    int textMatIndex2 = floatBitsToInt(waterfallCommon.bumpScale_textTransformInd1_textTransformInd2.z);

    int boneMatInd = instanceData.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.y;
    int textureMatInd = instanceData.textureMatricesInd_modelFragmentDatasInd.x;

    mat4 textMat[2];
    textMat[0] = textMatIndex1 < 0 ? mat4(1.0) : textureMatrix[textureMatInd + textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? mat4(1.0) : textureMatrix[textureMatInd + textMatIndex2];

    vec2 texCoord2 = (textMat[0] * vec4(aTexCoord2, 0, 1)).xy;

    int bumpTextureInd = waterfallBindless.instanceIndex_waterfallInd_bumpTextureInd_maskInd.z;

    vec4 bumpValue = texture(s_Textures[bumpTextureInd], texCoord2);
    vec3 pos = (aNormal * bumpScale.x) * bumpValue.z + aPosition;

    mat4 boneTransformMat =  mat4(0.0);

    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[boneMatInd + bones.x];
    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[boneMatInd + bones.y];
    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[boneMatInd + bones.z];
    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[boneMatInd + bones.w];

    int placementMatInd = instanceData.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    mat4 placementMat = uPlacementMats[placementMatInd];
    mat4 viewModelMat = scene.uLookAtMat * placementMat * boneTransformMat ;
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
