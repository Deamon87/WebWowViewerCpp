//
// Created by deamon on 16.01.18.
//

#ifndef WEBWOWVIEWERCPP_CRNDSEED_H
#define WEBWOWVIEWERCPP_CRNDSEED_H


#include <cstdint>

class CRndSeed {
    uint32_t currentValue;
    uint32_t accumulator = 0xFEEFDFA;

public:
    CRndSeed(uint32_t seed) {
        this->currentValue = seed;
        this->accumulator = accumulator ^ seed;
    }

    static float noise_(double x);

    float Uniform();
    float UniformPos();

    uint32_t uint32t();

    // State access for the GPU particle path (the GPU CRndSeed port continues
    // the exact same streams)
    uint32_t getCurrentValue() const { return currentValue; }
    uint32_t getAccumulator() const { return accumulator; }
};


#endif //WEBWOWVIEWERCPP_CRNDSEED_H
