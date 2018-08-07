//
// Created by deamon on 16.01.18.
//

#ifndef WEBWOWVIEWERCPP_CGENERATORANIPROP_H
#define WEBWOWVIEWERCPP_CGENERATORANIPROP_H


#include "../../persistance/header/commonFileStructs.h"

class CGeneratorAniProp {
public:
    float emissionSpeed;
    float speedVariation;
    float lifespan;
    float emissionRate;
    C3Vector gravity;
    float zSource;
    float emissionAreaX;
    float emissionAreaY;
    //Latitude in 4.1.0
    float verticalRange;
    //Longitude in 4.1.0
    float horizontalRange;
};


#endif //WEBWOWVIEWERCPP_CGENERATORANIPROP_H
