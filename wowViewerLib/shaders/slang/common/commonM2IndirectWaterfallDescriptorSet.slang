#ifndef COMMON_M2_WATERFALL_INDIRECT_DESCRIPTOR_SET_SLANG
#define COMMON_M2_WATERFALL_INDIRECT_DESCRIPTOR_SET_SLANG

import glsl;

struct WaterfallCommon {
    vec4 bumpScale_textTransformInd1_textTransformInd2;
    vec4 values0;
    vec4 values1;
    vec4 values2;
    vec4 values3;
    vec4 values4;
    vec4 baseColor;
};

layout(std430, set=2, binding=0) StructuredBuffer<WaterfallCommon> waterfallCommons;

struct WaterfallBindless {
    ivec4 instanceIndex_waterfallInd_bumpTextureInd_maskInd;
    ivec4 whiteWaterInd_noiseInd_normalTexInd;
};

layout(std430, set=2, binding=1) StructuredBuffer<WaterfallBindless> waterfallBindlesses;

layout(set=3,binding=0) uniform sampler2D s_Textures[];

#endif //COMMON_M2_WATERFALL_INDIRECT_DESCRIPTOR_SET_SLANG