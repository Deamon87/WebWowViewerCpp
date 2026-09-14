#ifndef COMMON_WMO_DS_SLANG
#define COMMON_WMO_DS_SLANG

import glsl;

#include "commonWmoMaterial.slang"

layout(std140, set=1, binding=0) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};

layout(std140, set=1, binding=1) uniform meshWideBlockVS {
    WmoVertMeshWide vertMeshWide;
};

layout(std140, set=1, binding=2) uniform meshWideBlockPS {
    ivec4 UseLitColor_EnableAlpha_PixelShader_BlendMode;
    vec4 FogColor_AlphaTest;
} wmoMeshWide;

layout(std140, set=1, binding=3) uniform groupWmoInteriorData {
    WmoInteriorBlockData interiorData[MAX_WMO_GROUPS];
};

layout(set=2, binding=0) uniform sampler2D uTexture;
layout(set=2, binding=1) uniform sampler2D uTexture2;
layout(set=2, binding=2) uniform sampler2D uTexture3;
layout(set=2, binding=3) uniform sampler2D uTexture4;
layout(set=2, binding=4) uniform sampler2D uTexture5;
layout(set=2, binding=5) uniform sampler2D uTexture6;
layout(set=2, binding=6) uniform sampler2D uTexture7;
layout(set=2, binding=7) uniform sampler2D uTexture8;
layout(set=2, binding=8) uniform sampler2D uTexture9;



#endif //COMMON_WMO_DS_SLANG