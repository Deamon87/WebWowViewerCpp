//
// Created by deamon on 16.01.18.
//

#include <random>
#include <assert.h>
#include "CRndSeed.h"

const uint8_t noiseArray[256] = {
    0x8E, 0x14, 0x27, 0x99, 0xFD, 0xAA, 0xC7, 0x08, 0xD5, 0xE6,
    0x3E, 0x1F, 0xF6, 0xBB, 0x55, 0xDA, 0x75, 0xA0, 0x4A, 0x6A,
    0xE8, 0xBD, 0x97, 0xFF, 0xDE, 0x9B, 0xBC, 0x9F, 0x81, 0x8A,
    0xA1, 0x46, 0x6E, 0x0B, 0xE3, 0x63, 0x76, 0x7A, 0x6C, 0x5D,
    0x88, 0xD3, 0x69, 0xCA, 0xC3, 0x47, 0xB9, 0x25, 0x83, 0xAB,
    0xA2, 0x3F, 0xA6, 0x41, 0x7C, 0xBA, 0xE5, 0xAC, 0x95, 0x01,
    0x7E, 0xCF, 0x09, 0xC1, 0xD9, 0x62, 0x70, 0x71, 0x8D, 0xDB,
    0x05, 0x02, 0x24, 0x87, 0xEF, 0x54, 0xC6, 0xD4, 0x37, 0x30,
    0xD0, 0x1B, 0xCB, 0x7B, 0xB8, 0xE4, 0xD8, 0xEC, 0x49, 0xCE,
    0xAD, 0xDC, 0x13, 0xA9, 0x94, 0xC4, 0x8F, 0x39, 0xAE, 0x0D,
    0x18, 0x52, 0xDD, 0x0E, 0x78, 0xFA, 0xF5, 0x85, 0x58, 0xD2,
    0xAF, 0x6D, 0xA4, 0xB2, 0x53, 0x3B, 0x51, 0xA5, 0x50, 0xBE,
    0xFC, 0x2D, 0xF4, 0x11, 0x48, 0x98, 0x16, 0xF1, 0x86, 0xDF,
    0x3D, 0x66, 0x5E, 0x44, 0x2E, 0x2F, 0x36, 0x07, 0x6B, 0x17,
    0x8B, 0x29, 0x4C, 0xB6, 0xE2, 0x89, 0x5F, 0xE7, 0xCD, 0xA7,
    0x21, 0xE1, 0x4D, 0xC9, 0x65, 0xED, 0xFE, 0xEE, 0x9C, 0x23,
    0x33, 0x7D, 0xB7, 0x04, 0x9E, 0x9A, 0x2A, 0x40, 0xB3, 0x10,
    0x5B, 0xF3, 0x82, 0x77, 0x1C, 0x92, 0x20, 0x4E, 0x1E, 0x57,
    0x22, 0x72, 0x06, 0x8C, 0x67, 0x2C, 0x73, 0xFB, 0x59, 0xC2,
    0x0A, 0xBF, 0x79, 0x5C, 0xF9, 0x0C, 0x28, 0x1A, 0x12, 0x68,
    0x74, 0x34, 0x19, 0x42, 0xB1, 0xC0, 0x84, 0xF8, 0x38, 0xF0,
    0x15, 0x9D, 0x60, 0xF2, 0x3A, 0x6F, 0xB4, 0x90, 0xEB, 0x91,
    0x1D, 0x7F, 0x35, 0x61, 0x5A, 0x32, 0x03, 0x56, 0xA3, 0xC5,
    0x2B, 0x93, 0x80, 0x0F, 0x4B, 0x43, 0xF7, 0xA8, 0xE0, 0x3C,
    0x96, 0xD1, 0x64, 0x26, 0xD7, 0x45, 0xCC, 0x4F, 0xC8, 0xB0,
    0xE9, 0xB5, 0x00, 0xD6, 0x31, 0xEA
};

inline uint32_t ROTL32(uint32_t val, uint8_t shift) {
    return (val << shift) | (val >> (32 - shift));
}

uint32_t CRndSeed::uint32t() {
    uint8_t byte0 =  static_cast<uint8_t>( currentValue        & 0xFF );
    uint8_t byte1 =  static_cast<uint8_t>((currentValue >>   8) & 0xFF);
    uint8_t byte2 =  static_cast<uint8_t>((currentValue >>  16) & 0xFF);
    uint8_t byte3 =  static_cast<uint8_t>((currentValue >>  24) & 0xFF);

    //    idx0 = (byte0 − 28) mod 244
    int idx4 = static_cast<int>(byte0) - 28;
    if (idx4 < 0) idx4 += 244;

    //    idx1 = (byte3 − 4) mod 188
    int idx1 = static_cast<int>(byte3) - 4;
    if (idx1 < 0) idx1 += 188;

    //    idx2 = (byte2 − 12) mod 212
    int idx2 = static_cast<int>(byte2) - 12;
    if (idx2 < 0) idx2 += 212;

    //    idx3 = (byte1 − 24) mod 236
    int idx3 = static_cast<int>(byte1) - 24;
    if (idx3 < 0) idx3 += 236;

    uint32_t val1 = *reinterpret_cast<const uint32_t*>(&noiseArray[idx1]);
    uint32_t val2 = *reinterpret_cast<const uint32_t*>(&noiseArray[idx2]);
    uint32_t val3 = *reinterpret_cast<const uint32_t*>(&noiseArray[idx3]);
    uint32_t val4 = *reinterpret_cast<const uint32_t*>(&noiseArray[idx4]);

    uint32_t newAccumulator = accumulator + val4 ^ ROTL32(val1, 1) ^ ROTL32(val2, 2) ^ ROTL32(val3, 3);

    currentValue   =
              (static_cast<uint32_t>(idx1) << 24)
            | (static_cast<uint32_t>(idx2) << 16)
            | (static_cast<uint32_t>(idx3) <<  8)
            |  static_cast<uint32_t>(idx4);

    accumulator = newAccumulator;

    return newAccumulator;
}

int64_t lattice_(uint32_t x) {
    uint32_t rotated_left = ROTL32(x,11);
    uint32_t rotated_right = ROTL32(x, 21); // ROTR32(11)
    uint32_t hash_input = rotated_right ^ rotated_left ^ x;

    uint32_t index0 = (hash_input << 2) & 0xFF;           // Bits 0-7, shifted for 32-bit alignment
    uint32_t index1 = (hash_input >> 4) &  252;           // Bits 4-11, masked and aligned
    uint32_t index2 = (hash_input >> 10) & 252;          // Bits 10-17, masked and aligned
    uint32_t index3 = (hash_input >> 16) & 252;          // Bits 16-23 (HIWORD), masked and aligned

    uint32_t noise0 = *(uint32_t*)((char*)noiseArray + index0);
    uint32_t noise1 = *(uint32_t*)((char*)noiseArray + index1);
    uint32_t noise2 = *(uint32_t*)((char*)noiseArray + index2);
    uint32_t noise3 = *(uint32_t*)((char*)noiseArray + index3);

    uint32_t result0 = noise0;                  // No rotation
    uint32_t result1 = ROTL32(noise1, 1);       // Rotate left by 1
    uint32_t result2 = ROTL32(noise2, 2);       // Rotate left by 2
    uint32_t result3 = ROTL32(noise3, 3);       // Rotate left by 3

    // Step 5: Combine all results with XOR
    return result3 ^ result2 ^ result1 ^ result0;
}

float CRndSeed::noise_(double x) {
    int xi = static_cast<int>(x);
    if (x < -x) {
        --xi;
    }

    float pct = x - static_cast<double>(xi);
    uint32_t n1 = lattice_(xi);
    uint32_t n2 = lattice_(xi + 1);

    union {
        float f;
        uint32_t i;
    }  buf[4];

    buf[0].i = ((n1 >> 10) & 0x3FFFC0) + 0x40400000;
    buf[1].i = ((n1 << 7) & 0x7FFF80) + 0x40000000;
    buf[2].i = ((n2 >> 10) & 0x3FFFC0) - ((n1 >> 10) & 0x3FFFC0) + 0x40400000;
    buf[3].i = ((n2 << 7) & 0x7FFF80) + 0x40000000;

    float pctSquared = pct * pct;
    float xx0 = buf[0].f - 3.0f;
    float dx0 = buf[1].f - 3.0f;
    float xx1 = buf[2].f - 3.0f;
    float dx1 = buf[3].f - 3.0f;
    float xx1m = pctSquared * (3.0f - 2.0f * pct);
    float dx0m = pctSquared * (pct - 2.0f) + pct;
    float dx1m = pctSquared * pct - pctSquared;

    float val = xx0 + xx1 * xx1m + dx0 * dx0m + dx1 * dx1m;
    val *= 0.75f;
    val += 0.125f;

    float result = (3.0f - 2.0f * val) * (val * val);

    return result;
}

float CRndSeed::Uniform() {
    union {
        float f;
        uint32_t i;
    } fi;
    uint32_t u = this->uint32t();
    // [1, 2)
    fi.i = 0x3f800000 | (0x7fffff & u);
    float result;
    if (u & 0x80000000) {
        result = 2.0f - fi.f;
    } else {
        result = fi.f - 2.0f;
    }

    assert(result <= 1.0 && result >= -1.0);
    return result;
}

//Return  [0, 1]
float CRndSeed::UniformPos() {
    union {
        float f;
        uint32_t i;
    } fi;
    uint32_t u = this->uint32t();
    // [1, 2)
    fi.i = 0x3f800000 | (0x7fffff & u);
    float result = fi.f - 1.0;

    assert(result >= 0.0 && result <= 1.0);
    return result;
}

