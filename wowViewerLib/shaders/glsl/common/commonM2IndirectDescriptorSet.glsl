#include "../common/commonLightFunctions.glsl"

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 256
#endif

// Whole model
layout(std430, set=1, binding=1) buffer readonly modelWideBlockVS {
    mat4 uPlacementMats[];
};

struct modelWideBlockPSStruct {
    InteriorLightParam intLight;
    ivec4 lightCountAndBcHack;
};

layout(std430, set=1, binding=2) buffer readonly modelWideBlockPS {
    modelWideBlockPSStruct modelWides[];
};

layout(std430, set=1, binding=3) buffer readonly boneMats {
    mat4 uBoneMatrixes[];
};

layout(std430, set=1, binding=4) buffer readonly m2Colors {
    vec4 colors[];
};

layout(std430, set=1, binding=5) buffer readonly textureWeights {
    vec4 textureWeight[];
};

layout(std430, set=1, binding=6) buffer readonly textureMatrices {
    mat4 textureMatrix[];
};

struct M2InstanceRecordBindless {
    ivec4 placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd;
    ivec4 textureMatricesInd_modelFragmentDatasInd;
};
layout(std430, set=1, binding=7) buffer readonly m2Instances {
    M2InstanceRecordBindless instances[];
};

struct meshWideBlockVSPS {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight;
};

layout(std430, set=1, binding=8) buffer readonly meshWide {
    meshWideBlockVSPS meshWides[];
};

struct meshWideBlockVSPSBindless {
    ivec4 instanceIndex_meshIndex;
    ivec4 textureIndicies;
};

layout(std430, set=1, binding=9) buffer readonly meshWideBindless {
    meshWideBlockVSPSBindless meshWideBindleses[];
};
