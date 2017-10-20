//
// Created by Deamon on 7/16/2017.
//

#include <set>
#include <cmath>
#include "map.h"
#include "../algorithms/mathHelper.h"
#include "../algorithms/grahamScan.h"
#include "m2Instancing/m2InstancingObject.h"

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
    adtRenderedThisFrame = std::set<AdtObject*>();
    m2RenderedThisFrame = std::set<M2Object*>();
    wmoRenderedThisFrame = std::set<WmoObject*>();

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    if (this->m_currentInteriorGroups.size() > 0 && m_api->getConfig()->getUsePortalCulling()) {
        if (this->m_currentWMO->startTraversingFromInteriorWMO(
                this->m_currentInteriorGroups,
                cameraPos,
                projectionModelMat,
                m2RenderedThisFrame)) {

            wmoRenderedThisFrame.insert(this->m_currentWMO);

            if (this->m_currentWMO->exteriorPortals.size() > 0) {
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
    Config* config = this->m_api->getConfig();
    if (config->getRenderM2()) {
        for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
            M2Object *m2Object = this->m2RenderedThisFrameArr[i];
            m2Object->update(deltaTime, cameraVec3, lookAtMat);
//            if (this.isM2Scene && m2Object.objectUpdate) {
//                m2Object.objectUpdate(deltaTime, cameraVec4, lookAtMat);
//            }
        }
//    }
    }

    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
        this->wmoRenderedThisFrameArr[i]->update();
    }

    //2. Calc distance every 100 ms
    if (this->m_currentTime + deltaTime - this->m_lastTimeDistanceCalc > 100) {
        for (int j = 0; j < this->m2RenderedThisFrameArr.size(); j++) {
            this->m2RenderedThisFrameArr[j]->calcDistance(cameraVec3);
        }

        this->m_lastTimeDistanceCalc = this->m_currentTime;
    }

    //3. Sort m2 by distance every 100 ms
    if (this->m_currentTime + deltaTime - this->m_lastTimeSort > 100) {
        std::sort(
            this->m2RenderedThisFrameArr.begin(),
            this->m2RenderedThisFrameArr.end(),
            [] (M2Object *a, M2Object *b) -> bool const {
                return b->getCurrentDistance() - a->getCurrentDistance() > 0;
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

                for (auto m2Object : this->m2RenderedThisFrameArr) {
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
    nearPlaneCenter *= 0.25;

    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
        WmoObject *checkingWmoObj = this->wmoRenderedThisFrameArr[i];
        WmoGroupResult groupResult;
        bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(mathfu::vec4(nearPlaneCenter, 1), groupResult);

        if (result) {
            this->m_currentWMO = checkingWmoObj;
            currentWmoGroup = groupResult.groupId;
            if (checkingWmoObj->isGroupWmoInterior(groupResult.groupId)) {
                this->m_currentInteriorGroups.push_back(groupResult);
                interiorGroupNum = groupResult.groupId;
            } else {
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
//    //8. Check fog color every 2 seconds
//    var fogRecordWasFound = false;
//    if (this.currentTime + deltaTime - this.lastFogParamCheck > 2000) {
//        if (this.currentWMO) {
//            var wmoFile = this.currentWMO.wmoObj;
//            var cameraLocal = vec4.create();
//            vec4.transformMat4(cameraLocal, this.position, this.currentWMO.placementInvertMatrix);
//
//            for (var i = wmoFile.mfogArray.length-1; i >= 0; i--) {
//                var fogRecord = wmoFile.mfogArray[i];
//                var fogPosVec = vec4.fromValues(fogRecord.pos.x,fogRecord.pos.y,fogRecord.pos.z,1);
//
//                var distanceToFog = vec4.distance(fogPosVec, cameraLocal);
//                if ((distanceToFog < fogRecord.larger_radius) /*|| fogRecord.larger_radius == 0*/) {
//                    this.sceneApi.setFogColor(fogRecord.fog_colorF);
//                    //this.sceneApi.setFogStart(wmoFile.mfog.fog_end);
//                    this.sceneApi.setFogEnd(fogRecord.fog_end);
//                    fogRecordWasFound = true;
//                    break;
//                }
//            }
//        }
//        var lightIntBandDBC = this.sceneApi.dbc.getLightIntBandDBC();
//        if (!fogRecordWasFound && lightIntBandDBC) {
//            //Check areaRecord
//            /*
//            //It's always 0 in WotLK
//            if (areaRecord && lightTableDBC) {
//                var lightRecord = lightTableDBC[areaRecord.lightId];
//            }
//            */
//            //Query Light Record
//            var result = this.sceneApi.findLightRecord(this.position);
//            if (result && result.length > 0) {
//                result.sort(function(a,b){
//                    if (a.distance < b.distance) return -1;
//                    if (a.distance > b.distance) return 1;
//                    return 0;
//                });
//                var fogIntRec = lightIntBandDBC[result[0].record.skyAndFog*18 - 17 + 7];
//                this.sceneApi.setFogColor(fogIntRec.floatValues[0]);
//
//                //Take fog params from here
//
//            }
//
//        }
//        this.lastFogParamCheck = this.currentTime;
//    }
    this->m_currentTime += deltaTime;
}

void Map::checkExterior(mathfu::vec4 &cameraPos,
                        std::vector<mathfu::vec4> &frustumPlanes,
                        std::vector<mathfu::vec3> &frustumPoints,
                        std::vector<mathfu::vec3> &hullLines,
                        mathfu::mat4 &lookAtMat4,
                        mathfu::mat4 &projectionModelMat,
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
        m2Object->setLoadParams(fileName, 0, {},{});
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
        wmoObject->setLoadingParam(fileName, mapObjDef);
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
                    m2Object->draw(false, diffuseNon);;
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
                    m2Object->draw(true, diffuseNon);
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
            m2Object->draw(false, diffuseNon);
        }

        for (int i = this->m2RenderedThisFrameArr.size()-1; i >= 0; i--) {
            M2Object *m2Object = this->m2RenderedThisFrameArr[i];

            m2Object->draw(true, diffuseNon);
        }
        this->m_api->deactivateM2Shader();
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

