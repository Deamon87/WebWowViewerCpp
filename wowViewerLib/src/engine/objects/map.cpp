//
// Created by Deamon on 7/16/2017.
//

#include <set>
#include "map.h"
#include "../algorithms/mathHelper.h"
#include "../algorithms/grahamScan.h"



void Map::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    std::set<AdtObject*> adtRenderedThisFrame;
    std::set<M2Object*> m2RenderedThisFrame;
    std::set<WmoObject*> wmoRenderedThisFrame;

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    checkExterior(cameraPos, frustumPlanes, frustumPoints, hullines, lookAtMat4,
                  adtRenderedThisFrame, m2RenderedThisFrame, wmoRenderedThisFrame);

}

void Map::checkExterior(mathfu::vec4 &cameraPos,
                        std::vector<mathfu::vec4> &frustumPlanes,
                        std::vector<mathfu::vec3> &frustumPoints,
                        std::vector<mathfu::vec3> &hullLines,
                        mathfu::mat4 &lookAtMat4,
                        std::set<AdtObject*> &adtRenderedThisFrame,
                        std::set<M2Object*> &m2RenderedThisFrame,
                        std::set<WmoObject*> &wmoRenderedThisFrame) {

    std::set<M2Object*> m2ObjectsCandidates;
    std::set<WmoObject*> wmoCandidates;

    float adt_x = floor((32 - (cameraPos[1] / 533.33333)));
    float adt_y = floor((32 - (cameraPos[0] / 533.33333)));

    for (int i = adt_x-1; i <= adt_x+1; i++) {
        for (int j = adt_y-1; j <= adt_y+1; j++) {
            if ((i < 0) || (i > 64)) continue;
            if ((j < 0) || (j > 64)) continue;

            AdtObject *adtObject = this->mapTiles[i][j];
            if (adtObject != nullptr) {
                bool result = adtObject->checkFrustumCulling(
                        cameraPos, frustumPlanes,
                        frustumPoints,
                        hullLines,
                        lookAtMat4, m2ObjectsCandidates, wmoCandidates);
                if (result) {
                    adtRenderedThisFrame.insert(adtObject);
                }
            } else {
                std::string adtFileName = "world/maps/"+mapName+"/"+mapName+"_"+std::to_string(i)+"_"+std::to_string(j)+".adt";
                adtObject = m_api->getAdtGeomCache()->get(adtFileName);
                adtObject->setApi(m_api);
                adtObject->setMapApi(this);
                this->mapTiles[i][j] = adtObject;
            }
        }
    }
}

M2Object *Map::getM2Object(std::string fileName, SMDoodadDef &doodadDef) {
    M2Object * m2Object = m_m2MapObjects.get(doodadDef.uniqueId);
    if (m2Object == nullptr) {
        m2Object = new M2Object(m_api);
        m2Object->setLoadParams(fileName, 0, {},{});
        m2Object->createPlacementMatrix(doodadDef);
        m_m2MapObjects.put(doodadDef.uniqueId, m2Object);
    }
    return m2Object;
}

WmoObject *Map::getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) {
    WmoObject * wmoObject = m_wmoMapObjects.get(mapObjDef.uniqueId);
    if (wmoObject == nullptr) {
        wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(fileName, mapObjDef);
        m_wmoMapObjects.put(mapObjDef.uniqueId, wmoObject);
    }
    return wmoObject;
}
