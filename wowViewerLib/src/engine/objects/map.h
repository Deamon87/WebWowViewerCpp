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

private:
    void checkExterior(mathfu::vec4 &cameraPos,
            std::vector<mathfu::vec4> &frustumPlanes,
            std::vector<mathfu::vec3> &frustumPoints,
            std::vector<mathfu::vec3> &hullLines,
            mathfu::mat4 &lookAtMat4,
            std::set<AdtObject> &adtRenderedThisFrame,
            std::set<M2Object> &m2RenderedThisFrame,
            std::set<WmoObject> &wmoRenderedThisFrame);

};


#endif //WEBWOWVIEWERCPP_MAP_H
