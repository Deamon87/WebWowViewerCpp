#include "../common/commonLightFunctions.slang"

import glsl;

#ifndef MAX_MATRIX_NUM_SLANG
#define MAX_MATRIX_NUM 256
#endif

#include "commonM2Material.slang"

// Whole model
layout(std140, set=1, binding=0) uniform modelWideBlockVS {
    mat4 placementMat;
    mat4 invPlacementMat;
} placementMats;

layout(std140, set=1, binding=1) uniform modelWideBlockPS {
    InteriorLightParam intLight;
    vec4 modelAlpha;
    uvec4 objectId_pad;
} modelWide;

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

#ifndef M2_WATERFALLSHADER

//Individual meshes
layout(std140, set=2, binding=0) uniform meshWideBlockVSPS {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight_txac1_txac2;
} meshWide;

layout(set=2,binding=1) uniform sampler2D uTexture;
layout(set=2,binding=2) uniform sampler2D uTexture2;
layout(set=2,binding=3) uniform sampler2D uTexture3;
layout(set=2,binding=4) uniform sampler2D uTexture4;

#ifdef PROJECTIVE_MATERIAL
layout(std140, set=2, binding=5) uniform ProjectiveData {
    ProjectiveData projectiveData;
};
#endif


#endif //M2_WATERFALLSHADER