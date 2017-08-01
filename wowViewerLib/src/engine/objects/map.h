//
// Created by Deamon on 7/16/2017.
//

#ifndef WEBWOWVIEWERCPP_MAP_H
#define WEBWOWVIEWERCPP_MAP_H


#include "adtObject.h"
#include "m2Object.h"
#include "wmoObject.h"

class Map {
private:
    AdtObject *mapTiles[64][64]={};

    std::map<int, M2Object> m_m2MapObjects;
    std::map<int, M2Object> m_wmoMapObjects;
public:
    void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos);



};


#endif //WEBWOWVIEWERCPP_MAP_H
