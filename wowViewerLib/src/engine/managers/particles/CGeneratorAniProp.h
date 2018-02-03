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
    float verticalRange;
    float horizontalRange;
};


#endif //WEBWOWVIEWERCPP_CGENERATORANIPROP_H
