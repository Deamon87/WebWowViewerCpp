#include "../common/commonLightFunctions.glsl"

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 256
#endif

// Whole model
layout(std140, set=1, binding=0) uniform modelWideBlockVS {
    mat4 uPlacementMat;
    mat4 uInvPlacementMat;
};

layout(std140, set=1, binding=1) uniform modelWideBlockPS {
    InteriorLightParam intLight;
    ivec4 lightCountAndBcHack;
};

layout(std140, set=1, binding=2) uniform boneMats {
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

layout(std140, set=1, binding=3) uniform m2Colors {
    vec4 colors[256];
};

layout(std140, set=1, binding=4) uniform textureWeights {
    vec4 textureWeight[16];
};

layout(std140, set=1, binding=5) uniform textureMatrices {
    mat4 textureMatrix[64];
};