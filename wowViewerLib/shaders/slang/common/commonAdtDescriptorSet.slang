#ifndef COMMON_ADT_DS_SLANG
#define COMMON_ADT_DS_SLANG

layout(std140, set=1, binding=0) uniform AdtMeshWideVSPS {
    vec4 uGlobalPosOffset;
    ivec4 globalChunkIndex;
    ivec4 uUseHeightMixFormula_useWeightedBlend;
    vec4 uHeightScale[2];
    vec4 uHeightOffset[2];
} adtMeshWideVSPS;

layout(std140, set=1, binding=1) uniform AdtMeshWidePS {
    vec4 scaleFactorPerLayer[2];
    ivec4 animation_rotationPerLayer[2];
    ivec4 animation_speedPerLayer[2];
} adtMeshWidePS;

layout(set=2, binding=0) uniform sampler2D uLayer0;
layout(set=2, binding=1) uniform sampler2D uLayer1;
layout(set=2, binding=2) uniform sampler2D uLayer2;
layout(set=2, binding=3) uniform sampler2D uLayer3;
layout(set=2, binding=4) uniform sampler2D uAlphaTexture;
layout(set=2, binding=5) uniform sampler2D uLayerHeight0;
layout(set=2, binding=6) uniform sampler2D uLayerHeight1;
layout(set=2, binding=7) uniform sampler2D uLayerHeight2;
layout(set=2, binding=8) uniform sampler2D uLayerHeight3;

layout(set=2, binding=9) uniform sampler2D uLayer4;
layout(set=2, binding=10) uniform sampler2D uLayer5;
layout(set=2, binding=11) uniform sampler2D uLayer6;
layout(set=2, binding=12) uniform sampler2D uLayer7;
layout(set=2, binding=13) uniform sampler2D uAlphaTexture2;
layout(set=2, binding=14) uniform sampler2D uLayerHeight4;
layout(set=2, binding=15) uniform sampler2D uLayerHeight5;
layout(set=2, binding=16) uniform sampler2D uLayerHeight6;
layout(set=2, binding=17) uniform sampler2D uLayerHeight7;

#endif //COMMON_ADT_DS_SLANG