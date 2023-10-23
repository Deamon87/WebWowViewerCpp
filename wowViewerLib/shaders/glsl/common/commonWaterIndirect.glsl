#ifndef COMMON_WATER_INDIRECT
#define COMMON_WATER_INDIRECT

struct WaterData {
    ivec4 materialId_liquidFlags;
    vec4 matColor;
    vec4 float0_float1;
};

//Individual meshes
layout(std430, set=1, binding=0) buffer waterData {
    WaterData waterDatas[];
};

layout(std430, set=1, binding=1) buffer placementMats {
    mat4 uPlacementMats[];
};

struct WaterBindless {
    ivec4 waterDataInd_placementMatInd_textureInd;
};

layout(std430, set=1, binding=2) buffer waterBindless {
    WaterBindless waterBindlesses[];
};

layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

#endif //COMMON_WATER_INDIRECT