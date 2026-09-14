#include "../common/commonLightFunctions.slang"

import glsl;

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 256
#endif

#include "commonM2Material.slang"

// Whole model
layout(std430, set=1, binding=1) StructuredBuffer<PlacementMatStruct> placementMats;

struct modelWideBlockPSStruct {
    InteriorLightParam intLight;
    vec4 modelAlpha;
    uvec4 objectId_pad;
};

layout(std430, set=1, binding=2) StructuredBuffer<modelWideBlockPSStruct> modelWides;

layout(std430, set=1, binding=3) StructuredBuffer<mat4> uBoneMatrixes;

layout(std430, set=1, binding=4) StructuredBuffer<vec4> colors;

layout(std430, set=1, binding=5) StructuredBuffer<vec4> textureWeight;

layout(std430, set=1, binding=6) StructuredBuffer<mat4> textureMatrix;

struct M2InstanceRecordBindless {
    ivec4 placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd;
    ivec4 textureMatricesInd_modelFragmentDatasInd;
};
layout(std430, set=1, binding=7) StructuredBuffer<M2InstanceRecordBindless> instances;

struct meshWideBlockVSPS {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight_txac1_txac2;
};

layout(std430, set=1, binding=8) StructuredBuffer<meshWideBlockVSPS> meshWides;

struct meshWideBlockVSPSBindless {
    ivec4 instanceIndex_meshIndex;
    ivec4 textureIndicies;
};

layout(std430, set=1, binding=9) StructuredBuffer<meshWideBlockVSPSBindless> meshWideBindleses;

//Used only by projective meshes



layout(std430, set=1, binding=10) StructuredBuffer<ProjectiveData> projectiveData;

