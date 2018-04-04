//
// Created by Deamon on 4/4/2018.
//

#ifndef WEBWOWVIEWERCPP_DB2LIGHTPARAMS_H
#define WEBWOWVIEWERCPP_DB2LIGHTPARAMS_H


#include <cstdint>

class DB2LightParams {
    float glow;
    float waterShallowAlpha;
    float waterDeepAlpha;
    float oceanShallowAlpha;
    float oceanDeepAlpha;
    float overrideCelestialSphere[3];
    int16_t lightSkyboxID;
    int8_t highlightSky;
    int8_t cloudTypeID;
    int8_t flags;
    int32_t ID;
};


#endif //WEBWOWVIEWERCPP_DB2LIGHTPARAMS_H
