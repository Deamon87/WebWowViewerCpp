//
// Created by deamon on 16.01.18.
//

#ifndef WEBWOWVIEWERCPP_CRNDSEED_H
#define WEBWOWVIEWERCPP_CRNDSEED_H


#include <cstdint>

class CRndSeed {
    uint32_t seed;
    uint32_t state;

public:
    CRndSeed(uint32_t seed) {
        this->seed = seed;
    }

    float Uniform();
    float UniformPos();
};


#endif //WEBWOWVIEWERCPP_CRNDSEED_H
