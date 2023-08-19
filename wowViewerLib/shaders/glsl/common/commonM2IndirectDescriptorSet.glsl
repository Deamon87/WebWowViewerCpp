#include "../common/commonLightFunctions.glsl"

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 256
#endif

// Whole model
layout(std430, set=1, binding=1) buffer modelWideBlockVS {
    mat4 uPlacementMats[];
};

struct modelWideBlockPSStruct {
    InteriorLightParam intLight;
    LocalLight pc_lights[4];
    ivec4 lightCountAndBcHack;
    vec4 interiorExteriorBlend;
};

layout(std430, set=1, binding=2) buffer modelWideBlockPS {
    modelWideBlockPSStruct modelWides[];
};

layout(std430, set=1, binding=3) buffer boneMats {
    mat4 uBoneMatrixes[];
};

layout(std430, set=1, binding=4) buffer m2Colors {
    vec4 colors[];
};

layout(std430, set=1, binding=5) buffer textureWeights {
    vec4 textureWeight[];
};

layout(std430, set=1, binding=6) buffer textureMatrices {
    mat4 textureMatrix[];
};

struct M2InstanceRecordBindless {
    ivec4 placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd;
    ivec4 textureMatricesInd_modelFragmentDatasInd;
};
layout(std430, set=1, binding=7) buffer m2Instances {
    M2InstanceRecordBindless instances[];
};