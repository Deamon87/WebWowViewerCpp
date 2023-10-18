#ifndef COMMON_ADT_INDIRECT_DS
#define COMMON_ADT_INDIRECT_DS

struct AdtMeshWideVSPS {
    vec4 uPos;
    ivec4 uUseHeightMixFormula;
    vec4 uHeightScale;
    vec4 uHeightOffset;
};

layout(std430, set=1, binding=1) buffer readonly meshWideBlockVSPS {
    AdtMeshWideVSPS adtMeshWideVSPSes[];
};

struct AdtMeshWidePS {
    vec4 scaleFactorPerLayer;
    ivec4 animation_rotationPerLayer;
    ivec4 animation_speedPerLayer;
};

layout(std430, set=1, binding=2) buffer readonly meshWideBlockPS {
    AdtMeshWidePS adtMeshWidePSes[];
};


struct AdtInstanceData {
    ivec4 meshIndexVSPS_meshIndexPS_AlphaTextureInd;
    ivec4 Layer0Ind_Layer1Ind_Layer2Ind_Layer3Ind;
    ivec4 LayerHeight0Ind_LayerHeight1Ind_LayerHeight2Ind_LayerHeight3Ind;
};

layout(std430, set=1, binding=3) buffer readonly instanceData {
    AdtInstanceData adtInstanceDatas[];
};



#endif //COMMON_ADT_INDIRECT_DS