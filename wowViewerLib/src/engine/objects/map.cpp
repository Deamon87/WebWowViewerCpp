//
// Created by Deamon on 7/16/2017.
//

#include <set>
#include "map.h"
#include "../algorithms/mathHelper.h"
#include "../algorithms/grahamScan.h"



void Map::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    std::set<AdtObject> adtRenderedThisFrame();
    std::set<M2Object> m2RenderedThisFrame();
    std::set<WmoObject> wmoRenderedThisFrame();

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    //grahamScan(frustumPoints);

}
