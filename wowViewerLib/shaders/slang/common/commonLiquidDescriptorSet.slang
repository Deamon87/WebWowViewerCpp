#ifndef COMMON_LIQUID_DESCRIPTORSET_SLANG
#define COMMON_LIQUID_DESCRIPTORSET_SLANG

import glsl;

#include "commonLiquidStructs.slang"

layout(std140, set=1, binding=1) uniform liquidInstanceBlockPS {
    LiquidInstanceData liquidInstanceData;
};

#ifdef WATER_LIQUID
#include "liquid/liquidWaterMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    WaterData waterData;
};
#endif //WATER_LIQUID

#ifdef MAGMA_LIQUID
#include "liquid/liquidMagmaMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    MagmaData magmaData;
};
#endif //MAGMA_LIQUID

#ifdef MERCURY_LIQUID
#include "liquid/liquidMercuryMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    MercuryData mercuryData;
};
#endif //MERCURY_LIQUID

#ifdef FOG_LIQUID
#include "liquid/liquidFogMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    FogData liquidFogData;
};
#endif //FOG_LIQUID

#ifdef LEYLINE_LIQUID
#include "liquid/liquidLeyLineMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    LeyLineData leyLineData;
};
#endif //LEYLINE_LIQUID

#ifdef FEL_LIQUID
#include "liquid/liquidFelMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    FelData felData;
};
#endif //FEL_LIQUID

#ifdef SWAMP_LIQUID
#include "liquid/liquidSwampMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    SwampData swampData;
};
#endif //SWAMP_LIQUID

#ifdef AZERITHE_LIQUID
#include "liquid/liquidAzeriteMat.slang"

layout(std140, set=2, binding=0) uniform meshWideBlockPS {
    AzeritheData azeritheData;
};
#endif //AZERITHE_LIQUID

layout(set=2,binding=1) uniform sampler2D uTexture0;
layout(set=2,binding=2) uniform sampler2D uTexture1;
layout(set=2,binding=3) uniform sampler2D uTexture2;
layout(set=2,binding=4) uniform sampler2D uTexture3;
layout(set=2,binding=5) uniform sampler2D uTexture4;
layout(set=2,binding=6) uniform sampler2D uTexture5;
layout(set=2,binding=7) uniform sampler2D uTexture6;
layout(set=2,binding=8) uniform sampler2D uTexture7;
layout(set=2,binding=9) uniform sampler2D uTexture8;

#endif //COMMON_WATER_INDIRECT