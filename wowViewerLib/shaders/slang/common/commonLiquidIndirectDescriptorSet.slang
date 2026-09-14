#ifndef COMMON_LIQUID_INDIRECT_DESCRIPTORSET_SLANG
#define COMMON_LIQUID_INDIRECT_DESCRIPTORSET_SLANG

import glsl;

#include "commonLiquidStructs.slang"

// Liquid material (shader) types — must match Liquid::LiquidMaterialType in UniformBufferStructures.h
static const int LIQUID_MAT_WATER = 0;
static const int LIQUID_MAT_MAGMA = 1;
static const int LIQUID_MAT_MERCURY = 2;
static const int LIQUID_MAT_FOG = 3;
static const int LIQUID_MAT_LEYLINE = 4;
static const int LIQUID_MAT_FEL = 5;
static const int LIQUID_MAT_SWAMP = 6;
static const int LIQUID_MAT_AZERITHE = 7;

struct LiquidBindless {
    ivec4 liquidDataInd_placementMatInd_liquidMaterialType; // x=data idx, y=placementMat idx, z=material type, w=unused
    // Indices into the bindless liquid texture array (s_Textures), one per sampler slot
    // (slots 0-5 = animated frames, 6 = extra, 7 = extraTexture2, 8 = extraTexture3).
    // Kept as ivec4s so the std430 layout is unambiguous; matches Liquid::LiquidBindless
    // in UniformBufferStructures.h (4 ints + 9 slot ints + 3 pad ints = 64 bytes).
    ivec4 textureInd0_3;
    ivec4 textureInd4_7;
    ivec4 textureInd8_unused; // .x = slot 8
};

// Explicit per-slot accessor for the texture indices above
int getLiquidTextureIndex(LiquidBindless rec, int slot) {
    switch (slot) {
        case 0: return rec.textureInd0_3.x;
        case 1: return rec.textureInd0_3.y;
        case 2: return rec.textureInd0_3.z;
        case 3: return rec.textureInd0_3.w;
        case 4: return rec.textureInd4_7.x;
        case 5: return rec.textureInd4_7.y;
        case 6: return rec.textureInd4_7.z;
        case 7: return rec.textureInd4_7.w;
        case 8: return rec.textureInd8_unused.x;
        default: return 0;
    }
}

// Placement matrices for all liquids
layout(std430, set=1, binding=0) StructuredBuffer<mat4> uPlacementMats;

// Bindless data for all liquids
layout(std430, set=1, binding=1) StructuredBuffer<LiquidBindless> liquidBindlesses;

// Water liquid data
layout(std430, set=1, binding=2) StructuredBuffer<WaterData> waterDatas;

// Magma liquid data
layout(std430, set=1, binding=3) StructuredBuffer<MagmaData> magmaDatas;

// Mercury liquid data
layout(std430, set=1, binding=4) StructuredBuffer<MercuryData> mercuryDatas;

// Fog liquid data
layout(std430, set=1, binding=5) StructuredBuffer<FogData> fogDatas;

// Ley Line liquid data
layout(std430, set=1, binding=6) StructuredBuffer<LeyLineData> leyLineDatas;

// Fel liquid data
layout(std430, set=1, binding=7) StructuredBuffer<FelData> felDatas;

// Swamp liquid data
layout(std430, set=1, binding=8) StructuredBuffer<SwampData> swampDatas;

// Azerithe liquid data
layout(std430, set=1, binding=9) StructuredBuffer<AzeritheData> azeritheDatas;

//Liquid Instance Data
layout(std430, set=1, binding=10) StructuredBuffer<LiquidInstanceData> liquidInstanceDatas;

layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

#endif //COMMON_LIQUID_INDIRECT_DESCRIPTORSET_SLANG