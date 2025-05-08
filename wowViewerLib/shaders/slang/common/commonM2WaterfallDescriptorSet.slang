#ifndef COMMON_M2_WATERFALL_DESCRIPTOR_SET_SLANG
#define COMMON_M2_WATERFALL_DESCRIPTOR_SET_SLANG

import glsl;

#ifdef M2_WATERFALLSHADER

layout(std140, set=2, binding=0) uniform WaterfallCommon {
    vec4 bumpScale_textTransformInd1_textTransformInd2;
    vec4 values0;
    vec4 values1;
    vec4 values2;
    vec4 values3;
    vec4 values4;
    vec4 baseColor;
} waterfallCommon;

layout(set=2,binding=1) uniform sampler2D uMask;
layout(set=2,binding=2) uniform sampler2D uWhiteWater;
layout(set=2,binding=3) uniform sampler2D uNoise;
layout(set=2,binding=4) uniform sampler2D uBumpTexture;
layout(set=2,binding=5) uniform sampler2D uNormalTex;

#endif

#endif //COMMON_M2_WATERFALL_DESCRIPTOR_SET