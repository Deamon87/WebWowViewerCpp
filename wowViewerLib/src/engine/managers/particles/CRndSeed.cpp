//
// Created by deamon on 16.01.18.
//

#include "CRndSeed.h"

uint32_t CRndSeed::uint32t() {
    uint32_t x = this->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    this->state = x;
    return x;
}

float CRndSeed::Uniform() {
    union {
        float f;
        uint32_t i;
    } fi;
    uint32_t u = this->uint32t();
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
    uint32_t u = this->uint32t();
    // [1, 2)
    fi.i = 0x3f800000 | (0x7fffff & u);
    if (u & 0x80000000) {
        return 1.0f - fi.f;
    } else {
        return fi.f - 1.0f;
    }
}
