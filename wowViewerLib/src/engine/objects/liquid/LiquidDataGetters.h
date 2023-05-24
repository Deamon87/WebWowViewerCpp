//
// Created by Deamon on 5/20/2023.
//

#ifndef AWEBWOWVIEWERCPP_LIQUIDDATAGETTERS_H
#define AWEBWOWVIEWERCPP_LIQUIDDATAGETTERS_H


#include <cstdint>
#include <bitset>
#include "../../persistance/header/adtFileHeader.h"

struct lfv_4_5_format {
    union {
        struct {
            int16_t s;
            int16_t t;
        } vert_format_5;

        struct {
            char depth;
            char filler1;
            char filler2;
            char filler3;
        } vert_format_4;
    };
    float height;
};

static_assert(sizeof(lfv_4_5_format) == 8);

inline float getLiquidVertexHeight(int liquidVertexFormat, float *vertexDataPtr, int totalElemSize, int index) {
    constexpr std::bitset<5> lvf_0_1_3 = 0xB;
    constexpr std::bitset<5> lvf_4_5 = 0x30;

    if ( liquidVertexFormat > 5 )
        return 0.0;

    if ( liquidVertexFormat == 0 || liquidVertexFormat == 1 || liquidVertexFormat == 3 ) // lvf == 0, 1, 3
        return vertexDataPtr[index];

    if ( liquidVertexFormat == 4 || liquidVertexFormat == 5 )
        return ((lfv_4_5_format*) vertexDataPtr)[index].height;// lvf == 4,5

    return 0.0;// lfv == 2
}

struct uv_map_entry {
    int16_t s;
    int16_t t;
};

uint8_t * getLiquidExistsTable(const PointerChecker<char> &mH2OBlob, const SMLiquidInstance &liquidInstance) {
    const static uint64_t fullBitmask = 0xFFFFFFFFFFFFFFFF;

    if (liquidInstance.offset_exists_bitmap) {
        return (uint8_t *) &mH2OBlob[liquidInstance.offset_exists_bitmap];
    }

    return (uint8_t *) &fullBitmask;
}
inline mathfu::vec2 getLiquidVertexCoords(int liquidVertexFormat, float *vertexDataPtr, int totalElemSize, int index) {
    int16_t s = 0;
    int16_t t = 0;

    if (liquidVertexFormat == 3 || liquidVertexFormat == 1) {
        auto const * uvStart = ((uv_map_entry *) &vertexDataPtr[totalElemSize]);
        auto const &element = uvStart[index];

        s = element.s;
        t = element.t;
    }
    if (liquidVertexFormat == 5) {
        auto &element = ((lfv_4_5_format*) vertexDataPtr)[index];

        s = element.vert_format_5.s;
        t = element.vert_format_5.t;
    }
    //For all other liquidVertexFormat are default zeroes

    return mathfu::vec2(s * 3.0f / 256.0f, t * 3.0f / 256.0f);
}

inline uint8_t getLiquidDepth(int liquidVertexFormat, float *vertexDataPtr, int totalElemSize, int index) {
    uint8_t depth = 0;
    if (liquidVertexFormat == 2) {
        depth = 255;
        if ( vertexDataPtr )
            depth = ((uint8_t *)vertexDataPtr)[index];
    } else if (liquidVertexFormat == 0) {
        auto const depthStart = (uint8_t *) (&vertexDataPtr[totalElemSize]);
        depth = depthStart[index];
    } else if (liquidVertexFormat == 3) {
        auto const * uvStart = ((uv_map_entry *) &vertexDataPtr[totalElemSize]);
        auto const * depthStart = (uint8_t *) (&uvStart[totalElemSize]);

        depth = depthStart[index];
    } else if (liquidVertexFormat == 4) {
        auto &element = ((lfv_4_5_format*) vertexDataPtr)[index];
        depth = element.vert_format_4.depth;
    }

    return depth;
}

struct LiquidObjectSettings {
    bool generateTexCoordsFromPos;
};

const constexpr LiquidObjectSettings useTexCoordLiquidObject = {
    .generateTexCoordsFromPos = false,
};
const constexpr LiquidObjectSettings usePlanarMapLiquidObject = {
    .generateTexCoordsFromPos = true
};

const constexpr LiquidObjectSettings usePlanarMapLiquidObjectNoSky = {
    .generateTexCoordsFromPos = true
};

const constexpr LiquidObjectSettings oceanLiquidObject = {
    .generateTexCoordsFromPos = true
};


inline LiquidObjectSettings getLiquidSettings(int liquidObjectId, int liquidType, int materialId, bool db_generateTexCoordsFromPos) {
    if (liquidObjectId < 41) {
        if (materialId == 2) {
            return useTexCoordLiquidObject;
        } else if ( materialId == 1) {
            if (liquidType == 2 || liquidType == 14) {
                return oceanLiquidObject;
            } else if (liquidType == 17) {
                return usePlanarMapLiquidObjectNoSky;
            } else {
                return usePlanarMapLiquidObject;
            }
        } else if ( materialId == 5 && (liquidType == 350 || liquidType == 412) ) {
            return usePlanarMapLiquidObjectNoSky;
        } else {
            return usePlanarMapLiquidObject;
        }
    } else if (liquidObjectId == 42 || liquidType == 14) {
        return oceanLiquidObject;
    } else {
        return {.generateTexCoordsFromPos = db_generateTexCoordsFromPos};
    }
}

#endif //AWEBWOWVIEWERCPP_LIQUIDDATAGETTERS_H
