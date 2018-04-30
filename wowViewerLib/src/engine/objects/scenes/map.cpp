//
// Created by Deamon on 7/16/2017.
//

#include <iostream>
#include <set>
#include <cmath>
#include "map.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/grahamScan.h"
#include "../m2/m2Instancing/m2InstancingObject.h"
#include "../../persistance/wdtFile.h"
#include "../../persistance/db2/DB2WmoAreaTable.h"

void Map::addM2ObjectToInstanceManager(M2Object * m2Object) {
    std::string fileIdent = m2Object->getModelIdent();
    M2InstancingObject* instanceManager = this->m_instanceMap[fileIdent];

    //1. Create Instance manager for this type of file if it was not created yet
    if (instanceManager == nullptr) {
        instanceManager = new M2InstancingObject(this->m_api);
        this->m_instanceMap[fileIdent] = instanceManager;
        this->m_instanceList.push_back(instanceManager);
    }

    //2. Add object to instance
    instanceManager->addMDXObject(m2Object);
}

void Map::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    if (!m_wdtfile->getIsLoaded()) return;


    adtRenderedThisFrame = std::unordered_set<AdtObject*>();
    m2RenderedThisFrame = std::unordered_set<M2Object*>();
    wmoRenderedThisFrame = std::unordered_set<WmoObject*>();

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    if (!this->m_currentInteriorGroups.empty() && m_api->getConfig()->getUsePortalCulling()) {
        if (this->m_currentWMO->startTraversingFromInteriorWMO(
                this->m_currentInteriorGroups,
                cameraPos,
                projectionModelMat,
                m2RenderedThisFrame)) {

            wmoRenderedThisFrame.insert(this->m_currentWMO);

            if (!this->m_currentWMO->exteriorPortals.empty()) {
                std::vector<std::vector<mathfu::vec4>> portalsToExt;
                for (auto &a : this->m_currentWMO->exteriorPortals) {
                    portalsToExt.push_back(a.frustumPlanes);
                }

                checkExterior(cameraPos, frustumPlanes, frustumPoints, hullines, lookAtMat4, projectionModelMat,
                              adtRenderedThisFrame, m2RenderedThisFrame, wmoRenderedThisFrame);
            }
        }
    } else {
        checkExterior(cameraPos, frustumPlanes, frustumPoints, hullines, lookAtMat4, projectionModelMat,
                      adtRenderedThisFrame, m2RenderedThisFrame, wmoRenderedThisFrame);
    }
    adtRenderedThisFrameArr = std::vector<AdtObject*>(adtRenderedThisFrame.begin(), adtRenderedThisFrame.end());
    m2RenderedThisFrameArr = std::vector<M2Object*>(m2RenderedThisFrame.begin(), m2RenderedThisFrame.end());
    wmoRenderedThisFrameArr = std::vector<WmoObject*>(wmoRenderedThisFrame.begin(), wmoRenderedThisFrame.end());
}

void Map::update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) {
    if (!m_wdtfile->getIsLoaded()) return;

    Config* config = this->m_api->getConfig();
    if (config->getRenderM2()) {
        for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
            M2Object *m2Object = this->m2RenderedThisFrameArr[i];
            m2Object->update(deltaTime, cameraVec3, lookAtMat);
        }
    }

    for (auto &wmoObject : this->wmoRenderedThisFrameArr) {
        wmoObject->update();
    }

    //2. Calc distance every 100 ms
    if (this->m_currentTime + deltaTime - this->m_lastTimeDistanceCalc > 100) {
        for (auto &m2Object : this->m2RenderedThisFrameArr) {
            m2Object->calcDistance(cameraVec3);
        }

        //Limit M2 count based on distance/m2 height
        for (auto it = this->m2RenderedThisFrameArr.begin();
             it != this->m2RenderedThisFrameArr.end();) {
            if ((*it)->getCurrentDistance() > ((*it)->getHeight() * 5)) {
                it = this->m2RenderedThisFrameArr.erase(it);
            } else {
                ++it;
            }
        }

        this->m_lastTimeDistanceCalc = this->m_currentTime;
    }

    //3. Sort m2 by distance every 100 ms
    if (this->m_currentTime + deltaTime - this->m_lastTimeSort > 100) {
        std::sort(
            this->m2RenderedThisFrameArr.begin(),
            this->m2RenderedThisFrameArr.end(),
            [] (M2Object *a, M2Object *b) -> bool const {
                return a->getCurrentDistance() - b->getCurrentDistance() > 0;
            }        
        );

        this->m_lastTimeSort = this->m_currentTime;
    }

    //4. Collect m2 into instances every 200 ms
    //
    if (config->getUseInstancing()) {
        if (this->m_currentTime + deltaTime - this->lastInstanceCollect > 30) {
            std::unordered_map<std::string, M2Object *> map;
            if (true /*this.sceneApi.extensions.getInstancingExt()*/) {
                //Clear instance lists
                for (auto &j : this->m_instanceList) {
                    j->clearList();
                }

                for (auto &m2Object : this->m2RenderedThisFrameArr) {
                    if (!m2Object->getGetIsLoaded()) continue;
                    if (m2Object->getHasBillboarded() || !m2Object->getIsInstancable()) continue;

                    std::string fileIdent = m2Object->getModelIdent();

                    if (map.find(fileIdent) != map.end()) {
                        M2Object *m2ObjectInstanced = map.at(fileIdent);
                        this->addM2ObjectToInstanceManager(m2Object);
                        this->addM2ObjectToInstanceManager(m2ObjectInstanced);
                    } else {
                        map[fileIdent] = m2Object;
                    }
                }
            }


            //4.1 Update placement matrix buffers in Instance
            for (auto instanceManager : this->m_instanceList) {
                instanceManager->updatePlacementVBO();
            }

            this->lastInstanceCollect = this->m_currentTime;
        }
    }
    //6. Check what WMO instance we're in
    this->m_currentInteriorGroups = {};
    this->m_currentWMO = nullptr;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    int currentWmoGroup = -1;

    //Get center of near plane
    mathfu::mat4 viewPerspectiveMat = frustumMat*lookAtMat;
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);

    mathfu::vec3 nearPlaneCenter(0,0,0);
    nearPlaneCenter += frustumPoints[0];
    nearPlaneCenter += frustumPoints[1];
    nearPlaneCenter += frustumPoints[6];
    nearPlaneCenter += frustumPoints[7];
    nearPlaneCenter = nearPlaneCenter * 0.25f;

    for (auto &checkingWmoObj : this->wmoRenderedThisFrameArr) {
        WmoGroupResult groupResult;
        bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(mathfu::vec4(nearPlaneCenter, 1), groupResult);

        if (result) {
            this->m_currentWMO = checkingWmoObj;
            currentWmoGroup = groupResult.groupIndex;
            if (checkingWmoObj->isGroupWmoInterior(groupResult.groupIndex)) {
                this->m_currentInteriorGroups.push_back(groupResult);
                interiorGroupNum = groupResult.groupIndex;
            } else {
            }

            if (m_api->getDB2WmoAreaTable()->getIsLoaded()) {
                DBWmoAreaTableRecord areaTableRecord;
                if (m_api->getDB2WmoAreaTable()->findRecord(
                    this->m_currentWMO->getWmoHeader()->wmoID,
                    this->m_currentWMO->getNameSet(),
                    groupResult.WMOGroupID,
                    areaTableRecord
                )) {
                    config->setAreaName(areaTableRecord.AreaName);
                }

            }

            bspNodeId = groupResult.nodeId;
            break;
        }
    }

//    //7. Get AreaId and Area Name
//    var currentAreaName = '';
//    var wmoAreaTableDBC = this.sceneApi.dbc.getWmoAreaTableDBC();
//    var areaTableDBC = this.sceneApi.dbc.getAreaTableDBC();
//    var areaRecord = null;
//    if (wmoAreaTableDBC && areaTableDBC) {
//        if (this.currentWMO) {
//            var wmoFile = this.currentWMO.wmoObj;
//            var wmoId = wmoFile.wmoId;
//            var wmoGroupId = this.currentWMO.wmoGroupArray[currentWmoGroup].wmoGeom.wmoGroupFile.mogp.groupID;
//            var nameSetId = this.currentWMO.nameSet;
//
//            var wmoAreaTableRecord = wmoAreaTableDBC.findRecord(wmoId, nameSetId, wmoGroupId);
//            if (wmoAreaTableRecord) {
//                var areaRecord = areaTableDBC[wmoAreaTableRecord.areaId];
//                if (wmoAreaTableRecord) {
//                    if (wmoAreaTableRecord.name == '') {
//                        var areaRecord = areaTableDBC[wmoAreaTableRecord.areaId];
//                        if (areaRecord) {
//                            currentAreaName = areaRecord.name
//                        }
//                    } else {
//                        currentAreaName = wmoAreaTableRecord.name;
//                    }
//                }
//            }
//        }
//        if (currentAreaName == '' && mcnkChunk) {
//            var areaRecord = areaTableDBC[mcnkChunk.areaId];
//            if (areaRecord) {
//                currentAreaName = areaRecord.name
//            }
//        }
//    }
//
    //8. Check fog color every 2 seconds
    bool fogRecordWasFound = false;
    if (this->m_currentTime + deltaTime - this->m_lastTimeLightCheck > 30) {
        mathfu::vec3 ambientColor = mathfu::vec3(0.0,0.0,0.0);
        mathfu::vec3 directColor = mathfu::vec3(0.0,0.0,0.0);
        mathfu::vec3 endFogColor = mathfu::vec3(0.0,0.0,0.0);

        if (this->m_currentWMO != nullptr) {
            CImVector sunFogColor;
            fogRecordWasFound = this->m_currentWMO->checkFog(cameraVec3, sunFogColor);
            if (fogRecordWasFound) {
                endFogColor =
                  mathfu::vec3((sunFogColor.r & 0xFF) / 255.0f,
                               ((sunFogColor.g ) & 0xFF) / 255.0f,
                               ((sunFogColor.b ) & 0xFF) / 255.0f);
            }
        }

        if (m_api->getDB2Light()->getIsLoaded() && m_api->getDB2LightData()->getIsLoaded()) {
            //Check areaRecord

            //Query Light Record
            std::vector<FoundLightRecord> result = m_api->getDB2Light()->findRecord(m_mapId, cameraVec3);
            float totalSummator = 0.0f;

            for (int i = 0; i < result.size() && totalSummator < 1.0f; i++) {
                DB2LightDataRecord lightData = m_api->getDB2LightData()->getRecord(result[i].lightRec.lightParamsID[0]);
                float blendPart = result[i].blendAlpha < 1.0f ? result[i].blendAlpha : 1.0f;
                blendPart = blendPart + totalSummator < 1.0f ? blendPart : 1.0f - totalSummator;

                ambientColor = ambientColor +
                    mathfu::vec3((lightData.ambientColor & 0xFF) / 255.0f,
                    ((lightData.ambientColor >> 8) & 0xFF) / 255.0f,
                    ((lightData.ambientColor >> 16) & 0xFF) / 255.0f) * blendPart ;
                directColor = directColor +
                    mathfu::vec3((lightData.directColor & 0xFF) / 255.0f,
                        ((lightData.directColor >> 8) & 0xFF) / 255.0f,
                        ((lightData.directColor >> 16) & 0xFF) / 255.0f) * blendPart ;

                if (!fogRecordWasFound) {
                    endFogColor = endFogColor +
                                  mathfu::vec3((lightData.sunFogColor & 0xFF) / 255.0f,
                                               ((lightData.sunFogColor >> 8) & 0xFF) / 255.0f,
                                               ((lightData.sunFogColor >> 16) & 0xFF) / 255.0f) * blendPart;
                }

                totalSummator += blendPart;
            }
//            DB2LightDataRecord lightData = m_api->getDB2LightData()->getRecord(20947);

//            for (int jk = 0; jk < 10000; jk++) {
//                DB2LightDataRecord lightData = m_api->getDB2LightData()->getRecord(20947+jk);
//                std::cout << "lightParamID = " << lightData.lightParamID << ", time = " << lightData.time << std::endl;
//                if (lightData.lightParamID == 379) {
//                    std::cout << "found :D" << std::endl;
//                }
//            }
        }
        config->setAmbientColor(
                ambientColor.x,
                ambientColor.y,
                ambientColor.z,
                1.0
        );
        config->setSunColor(
                directColor.x,
                directColor.y,
                directColor.z,
                1.0
        );
        config->setFogColor(
                endFogColor.x,
                endFogColor.y,
                endFogColor.z,
                1.0
        );

        this->m_lastTimeLightCheck = this->m_currentTime;
    }
    this->m_currentTime += deltaTime;
}

inline int worldCoordinateToAdtIndex(float x) {
    return floor((32.0f - (x / 533.33333f)));
}

inline int worldCoordinateToGlobalAdtChunk(float x) {
    return floor(( (32.0f*16.0f) - (x / (533.33333f / 16.0f)   )));
}

float AdtIndexToWorldCoordinate(int x) {
    return (32 - x) * 533.33333;
}

void Map::checkExterior(mathfu::vec4 &cameraPos,
                        std::vector<mathfu::vec4> &frustumPlanes,
                        std::vector<mathfu::vec3> &frustumPoints,
                        std::vector<mathfu::vec3> &hullLines,
                        mathfu::mat4 &lookAtMat4,
                        mathfu::mat4 &projectionModelMat,
                        std::unordered_set<AdtObject*> &adtRenderedThisFrame,
                        std::unordered_set<M2Object*> &m2RenderedThisFrame,
                        std::unordered_set<WmoObject*> &wmoRenderedThisFrame) {

    std::set<M2Object *> m2ObjectsCandidates;
    std::set<WmoObject *> wmoCandidates;

//    float adt_x = floor((32 - (cameraPos[1] / 533.33333)));
//    float adt_y = floor((32 - (cameraPos[0] / 533.33333)));

    //Get visible area that should be checked
    float minx = 99999, maxx = -99999;
    float miny = 99999, maxy = -99999;

    for (int i = 0; i < frustumPoints.size(); i++) {
        mathfu::vec3 &frustumPoint = frustumPoints[i];

        minx = std::min(frustumPoint.x, minx);
        maxx = std::max(frustumPoint.x, maxx);
        miny = std::min(frustumPoint.y, miny);
        maxy = std::max(frustumPoint.y, maxy);
    }
    int adt_x_min = worldCoordinateToAdtIndex(maxy);
    int adt_x_max = worldCoordinateToAdtIndex(miny);

    int adt_y_min = worldCoordinateToAdtIndex(maxx);
    int adt_y_max = worldCoordinateToAdtIndex(minx);

    int adt_global_x = worldCoordinateToGlobalAdtChunk(cameraPos.y);
    int adt_global_y = worldCoordinateToGlobalAdtChunk(cameraPos.x);


//    for (int i = 0; i < 64; i++)
//        for (int j = 0; j < 64; j++) {
//            if (this->m_wdtfile->mapTileTable->mainInfo[i][j].Flag_AllWater) {
//                std::cout << AdtIndexToWorldCoordinate(j) <<" "<<  AdtIndexToWorldCoordinate(i) << std::endl;
//            }
//        }

//    std::cout << AdtIndexToWorldCoordinate(adt_y_min) <<" "<<  AdtIndexToWorldCoordinate(adt_x_min) << std::endl;

    m_wdlObject->checkFrustumCulling(
            cameraPos, frustumPlanes,
            frustumPoints,
            hullLines,
            lookAtMat4, m2ObjectsCandidates, wmoCandidates);

    for (int i = adt_x_min; i <= adt_x_max; i++) {
        for (int j = adt_y_min; j <= adt_y_max; j++) {
            if ((i < 0) || (i > 64)) continue;
            if ((j < 0) || (j > 64)) continue;

            AdtObject *adtObject = this->mapTiles[i][j];
            if (adtObject != nullptr) {

                bool result = adtObject->checkFrustumCulling(
                        cameraPos,
                        adt_global_x,
                        adt_global_y,
                        frustumPlanes,
                        frustumPoints,
                        hullLines,
                        lookAtMat4, m2ObjectsCandidates, wmoCandidates);
                if (result) {

                    adtRenderedThisFrame.insert(adtObject);
                }
            } else {
                std::string adtFileTemplate = "world/maps/"+mapName+"/"+mapName+"_"+std::to_string(i)+"_"+std::to_string(j);
                adtObject = new AdtObject(m_api, adtFileTemplate, m_wdtfile);

                adtObject->setMapApi(this);
                this->mapTiles[i][j] = adtObject;
            }
        }
    }

    //3.2 Iterate over all global WMOs and M2s (they have uniqueIds)
    for (auto it = m2ObjectsCandidates.begin(); it != m2ObjectsCandidates.end(); ++it) {
        M2Object *m2ObjectCandidate  = *it;
        bool frustumResult = m2ObjectCandidate->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
        if (frustumResult) {
            m2RenderedThisFrame.insert(m2ObjectCandidate);
        }
    }

    for (auto it = wmoCandidates.begin(); it != wmoCandidates.end(); ++it) {
        WmoObject *wmoCandidate = *it;

        if (!wmoCandidate->isLoaded()) {
            wmoRenderedThisFrame.insert(wmoCandidate);
            continue;
        }
        if( wmoRenderedThisFrame.count(wmoCandidate) > 0) continue;

        if ( wmoCandidate->hasPortals() && m_api->getConfig()->getUsePortalCulling() ) {
            if(wmoCandidate->startTraversingFromExterior(cameraPos, projectionModelMat, m2RenderedThisFrame)){
                wmoRenderedThisFrame.insert(wmoCandidate);
            }
        } else {
            if (wmoCandidate->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints, m2RenderedThisFrame)) {
                wmoRenderedThisFrame.insert(wmoCandidate);
            }
        }
    }


}

M2Object *Map::getM2Object(std::string fileName, SMDoodadDef &doodadDef) {
    M2Object * m2Object = m_m2MapObjects.get(doodadDef.uniqueId);
    if (m2Object == nullptr) {
        m2Object = new M2Object(m_api);
        m2Object->setLoadParams(0, {},{});
        m2Object->setModelFileName(fileName);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();
        m_m2MapObjects.put(doodadDef.uniqueId, m2Object);
    }
    return m2Object;
}

M2Object *Map::getM2Object(int fileDataId, SMDoodadDef &doodadDef) {
    M2Object * m2Object = m_m2MapObjects.get(doodadDef.uniqueId);
    if (m2Object == nullptr) {
        m2Object = new M2Object(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileId(fileDataId);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();
        m_m2MapObjects.put(doodadDef.uniqueId, m2Object);
    }
    return m2Object;
}


WmoObject *Map::getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) {
    WmoObject * wmoObject = m_wmoMapObjects.get(mapObjDef.uniqueId);
    if (wmoObject == nullptr) {
        wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);
        m_wmoMapObjects.put(mapObjDef.uniqueId, wmoObject);
    }
    return wmoObject;
}

WmoObject *Map::getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) {
    WmoObject * wmoObject = m_wmoMapObjects.get(mapObjDef.uniqueId);
    if (wmoObject == nullptr) {
        wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);
        m_wmoMapObjects.put(mapObjDef.uniqueId, wmoObject);
    }
    return wmoObject;
}

WmoObject *Map::getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) {
    WmoObject * wmoObject = m_wmoMapObjects.get(mapObjDef.uniqueId);
    if (wmoObject == nullptr) {
        wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);
        m_wmoMapObjects.put(mapObjDef.uniqueId, wmoObject);
    }
    return wmoObject;
}

void Map::draw() {
    this->m2OpaqueRenderedThisFrame = {};
    this->m2TranspRenderedThisFrame = {};
//    if (this.currentWMO && this.currentInteriorGroups != null && config.getUsePortalCulling()) {
//        this.sceneApi.shaders.activateWMOShader();
//        this.currentWMO.drawPortalBased(true);
//        this.sceneApi.shaders.deactivateWMOShader();
//
//        if (this.currentWMO.exteriorPortals.length > 0) {
//            this.drawExterior()
//        }
//        //6. Draw WMO portals
//        if (config.getRenderPortals()) {
//            this.sceneApi.shaders.activateDrawPortalShader();
//            for (var i = 0; i < this.wmoRenderedThisFrame.length; i++) {
//                this.wmoRenderedThisFrame[i].drawPortals();
//            }
//        }
//        this.drawM2s();
//
//        this.sceneApi.shaders.activateFrustumBoxShader();
//        //Draw Wmo portal frustums
//        if (this.sceneApi.getIsDebugCamera()) {
//            this.sceneApi.drawCamera()
//        }
//    } else {
        this->drawExterior();
        this->drawM2s();

        //6. Draw WMO portals
//        if (config.getRenderPortals()) {
//            this.sceneApi.shaders.activateDrawPortalShader();
//            for (var i = 0; i < this.wmoRenderedThisFrame.length; i++) {
//                this.wmoRenderedThisFrame[i].drawPortals();
//            }
//        }
        //Draw Wmo portal frustums
        this->m_api->activateFrustumBoxShader();
        if (this->m_api->getIsDebugCamera()) {
            this->m_api->drawCamera();
        }
//    }
}

void Map::drawExterior() {
//    if (config.getRenderAdt()) {
        this->m_api->activateAdtShader();
        for (int i = 0; i < this->adtRenderedThisFrame.size(); i++) {
            this->adtRenderedThisFrameArr[i]->draw();
        }
//    }


    //2.0. Draw WMO bsp highlighted vertices
//    if (config.getRenderBSP()) {
//        this.sceneApi.shaders.activateDrawPortalShader();
//        for (var i = 0; i < this.wmoRenderedThisFrame.length; i++) {
//            this.wmoRenderedThisFrame[i].drawBspVerticles();
//        }
//    }

    //2. Draw WMO
    this->m_api->activateWMOShader();
    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
//        if (config.getUsePortalCulling()) {
//            this.wmoRenderedThisFrame[i].drawPortalBased(false)
//        } else {
            this->wmoRenderedThisFrameArr[i]->draw();
//        }
    }
    this->m_api->deactivateWMOShader();

    this->m_api->activateDrawPortalShader();
    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
//        if (config.getUsePortalCulling()) {
//            this.wmoRenderedThisFrame[i].drawPortalBased(false)
//        } else {
        this->wmoRenderedThisFrameArr[i]->drawTransformedPortalPoints();
        this->wmoRenderedThisFrameArr[i]->drawTransformedAntiPortalPoints();
//        }
    }


    //3. Draw background WDL

    //4. Draw skydom
//    if (this.skyDom) {
//        this.skyDom.draw();
//    }

    //7.1 Draw WMO BBs
//    this.sceneApi.shaders.activateBoundingBoxShader();
//    if (config.getDrawWmoBB()) {
//        for (var i = 0; i < this.wmoRenderedThisFrame.length; i++) {
//            this.wmoRenderedThisFrame[i].drawBB();
//        }
//    }

}

void Map::drawM2s() {
    mathfu::vec4 diffuseNon(1.0, 1.0, 1.0, 1.0);
    Config * config = this->m_api->getConfig();

    if (config->getUseInstancing()) {
        if (this->m_api->getConfig()->getRenderM2()) {
            bool lastWasDrawInstanced = false;
            this->m_api->activateM2Shader();
            for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
                M2Object *m2Object = this->m2RenderedThisFrameArr[i];
                if (this->m2OpaqueRenderedThisFrame.find(m2Object) != this->m2OpaqueRenderedThisFrame.end()) continue;
                std::string fileIdent = m2Object->getModelIdent();

                bool drawInstanced = false;
                M2InstancingObject *instanceManager = nullptr;
                auto it = this->m_instanceMap.find(fileIdent);
                if (it != this->m_instanceMap.end()) {
                    instanceManager = (*it).second;
                    drawInstanced = instanceManager->getLastUpdatedNumber() > 1;
                }
                if (drawInstanced) {
                    if (!lastWasDrawInstanced) {
                        this->m_api->activateM2InstancingShader();
                    }

                    instanceManager->drawInstancedNonTransparentMeshes(this->m2OpaqueRenderedThisFrame);
                    lastWasDrawInstanced = true;
                } else {
                    if (lastWasDrawInstanced) {
                        this->m_api->deactivateM2InstancingShader();
                        this->m_api->activateM2Shader();
                    }

                    this->m2OpaqueRenderedThisFrame.insert(m2Object);
                    m2Object->draw(false);
                    lastWasDrawInstanced = false;
                }
            }
            if (lastWasDrawInstanced) {
                this->m_api->deactivateM2InstancingShader();
            } else {
                this->m_api->deactivateM2Shader();
            }
        }

        //6. Draw transparent meshes of m2
        if (this->m_api->getConfig()->getRenderM2()) {
            bool lastWasDrawInstanced = false;
            this->m_api->activateM2Shader();

            for (int i = static_cast<int>(this->m2RenderedThisFrameArr.size() - 1); i >= 0; i--) {
                M2Object *m2Object = this->m2RenderedThisFrameArr[i];
                if (this->m2TranspRenderedThisFrame.find(m2Object)
                    != this->m2TranspRenderedThisFrame.end())
                    continue;
                std::string fileIdent = m2Object->getModelIdent();

                bool drawInstanced = false;
                M2InstancingObject *instanceManager = nullptr;
                auto it = this->m_instanceMap.find(fileIdent);
                if (it != this->m_instanceMap.end()) {
                    instanceManager = (*it).second;
                    drawInstanced = instanceManager->getLastUpdatedNumber() > 1;
                }
                if (drawInstanced) {
                    if (!lastWasDrawInstanced) {
                        this->m_api->activateM2InstancingShader();
                    }

                    instanceManager->drawInstancedTransparentMeshes(this->m2TranspRenderedThisFrame);
                    lastWasDrawInstanced = true;
                } else {
                    if (lastWasDrawInstanced) {
                        this->m_api->deactivateM2InstancingShader();
                        this->m_api->activateM2Shader();
                    }

                    this->m2TranspRenderedThisFrame.insert(m2Object);
                    m2Object->draw(true);
                    lastWasDrawInstanced = false;
                }
            }
            if (lastWasDrawInstanced) {
                this->m_api->deactivateM2InstancingShader();
            } else {
                this->m_api->deactivateM2Shader();
            }
        }
    } else {
        //Old implementation without instancing

        this->m_api->activateM2Shader();
        mathfu::vec4 diffuseNon(1.0, 1.0, 1.0, 1.0);
        for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {

            M2Object *m2Object = this->m2RenderedThisFrameArr[i];
            m2Object->draw(false);
        }

        for (int i = this->m2RenderedThisFrameArr.size()-1; i >= 0; i--) {
            M2Object *m2Object = this->m2RenderedThisFrameArr[i];

            m2Object->draw(true);
        }
        this->m_api->deactivateM2Shader();

        this->m_api->activateM2ParticleShader();
        for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {

            M2Object *m2Object = this->m2RenderedThisFrameArr[i];
            m2Object->drawParticles();
        }
        this->m_api->deactivateM2ParticleShader();
    }

    //7. Draw BBs
    //7.1 Draw M2 BBs
    if (this->m_api->getConfig()->getDrawM2BB()) {
        this->m_api->activateBoundingBoxShader();

        mathfu::vec3 bbColor(0.819607843, 0.058, 0.058);
        for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
            this->m2RenderedThisFrameArr[i]->drawBB(bbColor);
        }
        this->m_api->deactivateBoundingBoxShader();

    }

//    }
}

