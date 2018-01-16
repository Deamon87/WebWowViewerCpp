//
// Created by deamon on 16.01.18.
//

#include "CRndSeed.h"

float CRndSeed::Uniform() {
    union {
        float f;
        uint32_t i;
    } fi;
    uint32_t u = state;
    // [1, 2)
    fi.i = 0x3f800000 | (0x7fffff & u);
    if (u & 0x80000000) {
        return 2.0f - fi.f;
    } else {
        return fi.f - 2.0f;
    }
}

float CRndSeed::UniformPos() {
    union {
        float f;
        uint32_t i;
    } fi;
    uint32_t u = state;
    // [1, 2)
    fi.i = 0x3f800000 | (0x7fffff & u);
    if (u & 0x80000000) {
        return 1.0f - fi.f;
    } else {
        return fi.f - 1.0f;
    }
}
