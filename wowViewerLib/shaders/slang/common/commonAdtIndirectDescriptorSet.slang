#ifndef COMMON_ADT_INDIRECT_DS_SLANG
#define COMMON_ADT_INDIRECT_DS_SLANG

import glsl;

struct AdtMeshWideVSPS {
    vec4 uGlobalPosOffset;
    ivec4 globalChunkIndex;
    ivec4 uUseHeightMixFormula_useWeightedBlend_maxLayersCount;
    vec4 uHeightScale[2];
    vec4 uHeightOffset[2];
};

layout(std430, set=1, binding=1) StructuredBuffer<AdtMeshWideVSPS> adtMeshWideVSPSes;

struct AdtMeshWidePS {
    vec4 scaleFactorPerLayer[2];
    ivec4 animation_rotationPerLayer[2];
    ivec4 animation_speedPerLayer[2];
};

layout(std430, set=1, binding=2) StructuredBuffer<AdtMeshWidePS> adtMeshWidePSes;

struct AdtInstanceData {
    ivec4 meshIndexVSPS_meshIndexPS_AlphaTextureInd_AlphaTextureInd2;
    ivec4 Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind;
    ivec4 Layer4Ind_Layer5Ind_Layer6Ind_Layer7Ind;
    ivec4 LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind;
    ivec4 LayerHeight4Ind_LayerHeight5Ind_LayerHeight6Ind_LayerHeight7Ind;
};

layout(std430, set=1, binding=3) StructuredBuffer<AdtInstanceData> adtInstanceDatas;

layout (set = 2, binding = 0) uniform sampler2D s_LayerTextures[];
layout (set = 3, binding = 0) uniform sampler2D s_AlphaTextures[];
layout (set = 4, binding = 0) uniform sampler2D s_LayerHeightTextures[];

#endif //COMMON_ADT_INDIRECT_DS