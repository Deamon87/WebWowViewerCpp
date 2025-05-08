#ifndef COMMON_WMO_INDIRECT_DS_SLANG
#define COMMON_WMO_INDIRECT_DS_SLANG

import glsl;

#include "commonWmoMaterial.slang"

layout(std430, set=1, binding=1) StructuredBuffer<mat4> uPlacementMats;

layout(std430, set=1, binding=2) StructuredBuffer<WmoVertMeshWide> wmoVertMeshWide;

layout(std430, set=1, binding=3) StructuredBuffer<WmoFragMeshWide> wmoFragMeshWides;

struct WmoMeshWideBindless {
    ivec4 placementMat_meshWideIndex_blockVSIndex_texture9;
    ivec4 text1_text2_text3_text4;
    ivec4 text5_text6_text7_text8;
};

layout(std430, set=1, binding=4) StructuredBuffer<WmoMeshWideBindless> wmoMeshWideBindlesses;

layout(std430, set=1, binding=5) StructuredBuffer<WmoInteriorBlockData> interiorData;

struct WMOPerMeshData {
    ivec4 meshWideBindlessIndex_groupNum_canHaveExterior;
};

layout(std430, set=1, binding=6) StructuredBuffer<WMOPerMeshData> perMeshDatas;

layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

#endif //COMMON_WMO_INDIRECT_DS