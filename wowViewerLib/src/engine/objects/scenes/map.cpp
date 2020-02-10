//
// Created by Deamon on 7/16/2017.
//
#include <algorithm>
#include <iostream>
#include <set>
#include <cmath>
#include "map.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/grahamScan.h"
#include "../../persistance/wdtFile.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../wowFrameData.h"
#include "../../algorithms/quick-sort-omp.h"

//#include "../../algorithms/quicksort-dualpivot.h"

void Map::checkCulling(HCullStage cullStage) {
//    std::cout << "Map::checkCulling finished called" << std::endl;
//    std::cout << "m_wdtfile->getIsLoaded() = " << m_wdtfile->getIsLoaded() << std::endl;
    if (m_wdtfile->getStatus() != FileStatus::FSLoaded) return;

    Config* config = this->m_api->getConfig();

    mathfu::vec4 cameraPos = cullStage->matricesForCulling->cameraPos;
    mathfu::mat4 &frustumMat = cullStage->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = cullStage->matricesForCulling->lookAtMat;

    size_t adtRenderedThisFramePrev = cullStage->adtArray.size();
    cullStage->adtArray = {};
    cullStage->adtArray.reserve(adtRenderedThisFramePrev);

    size_t m2RenderedThisFramePrev = cullStage->m2Array.size();
    cullStage->m2Array = {};
    cullStage->m2Array.reserve(m2RenderedThisFramePrev);

    size_t wmoRenderedThisFramePrev = cullStage->wmoArray.size();
    cullStage->wmoArray = {};
    cullStage->wmoArray.reserve(wmoRenderedThisFramePrev);


    mathfu::mat4 viewPerspectiveMat = frustumMat*lookAtMat4;

    mathfu::vec4 &camera4 = cameraPos;
    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    cullStage->exteriorView = ExteriorView();
    cullStage->interiorViews = std::vector<InteriorView>();
    m_viewRenderOrder = 0;

    cullStage->m_currentInteriorGroups = {};
    cullStage->m_currentWMO = nullptr;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    cullStage->m_currentWmoGroup = -1;

    //Get center of near plane


    //Get potential WMO
    std::vector<std::shared_ptr<WmoObject>> potentialWmo;
    std::vector<std::shared_ptr<M2Object>> potentialM2;

    if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
        int adt_x = worldCoordinateToAdtIndex(camera4.y);
        int adt_y = worldCoordinateToAdtIndex(camera4.x);
        cullStage->adtAreadId = -1;
        std::shared_ptr<AdtObject> adtObjectCameraAt = this->mapTiles[adt_x][adt_y];
        if (adtObjectCameraAt != nullptr) {
            ADTObjRenderRes tempRes;
            tempRes.adtObject = adtObjectCameraAt;
            adtObjectCameraAt->checkReferences(
                tempRes,
                camera4,
                frustumPlanes,
                frustumPoints,
                lookAtMat4,
                5,
                potentialM2,
                potentialWmo,
                0,
                0,
                16,
                16
            );

            int adt_global_x = worldCoordinateToGlobalAdtChunk(cameraPos.y) % 16;
            int adt_global_y = worldCoordinateToGlobalAdtChunk(cameraPos.x) % 16;

            cullStage->adtAreadId = adtObjectCameraAt->getAreaId(adt_global_x, adt_global_y);
        }
    } else {
        if (wmoMap == nullptr) {
            wmoMap = std::make_shared<WmoObject>(m_api);
            wmoMap->setLoadingParam(*m_wdtfile->wmoDef);
            wmoMap->setModelFileId(m_wdtfile->wmoDef->nameId);
        }

        potentialWmo.push_back(wmoMap);
    }


    for (auto &checkingWmoObj : potentialWmo) {
        WmoGroupResult groupResult;
        bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(camera4, groupResult);

        if (result) {
            cullStage->m_currentWMO = checkingWmoObj;
            cullStage->m_currentWmoGroup = groupResult.groupIndex;
            if (checkingWmoObj->isGroupWmoInterior(groupResult.groupIndex)) {
                cullStage->m_currentInteriorGroups.push_back(groupResult);
                interiorGroupNum = groupResult.groupIndex;
            } else {
            }
            bspNodeId = groupResult.nodeId;
            break;
        }
    }

    auto lcurrentWMO = cullStage->m_currentWMO;
    auto currentWmoGroup = cullStage->m_currentWmoGroup;

    if ((lcurrentWMO != nullptr) && (!cullStage->m_currentInteriorGroups.empty()) && (lcurrentWMO->isLoaded())) {
        lcurrentWMO->resetTraversedWmoGroups();
        if (lcurrentWMO->startTraversingWMOGroup(
            cameraPos,
            viewPerspectiveMat,
            cullStage->m_currentInteriorGroups[0].groupIndex,
            0,
            m_viewRenderOrder,
            true,
            cullStage->interiorViews,
            cullStage->exteriorView)) {

            cullStage->wmoArray.push_back(cullStage->m_currentWMO);
        }

        if (cullStage->exteriorView.viewCreated) {
            checkExterior(cameraPos, frustumPoints, hullines, lookAtMat4, viewPerspectiveMat, m_viewRenderOrder, cullStage);
        }
    } else {
        //Cull exterior
        cullStage->exteriorView.viewCreated = true;
        cullStage->exteriorView.frustumPlanes.push_back(frustumPlanes);
        checkExterior(cameraPos, frustumPoints, hullines, lookAtMat4, viewPerspectiveMat, m_viewRenderOrder, cullStage);
    }
    if (  cullStage->exteriorView.viewCreated && (m_currentSkyFDID != 0)) {
        cullStage->exteriorView.drawnM2s.push_back(m_exteriorSkyBox);
    }

    //Fill M2 objects for views from WmoGroups
    for (auto &view : cullStage->interiorViews) {
        view.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    }
    cullStage->exteriorView.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    for (auto &adtRes : cullStage->exteriorView.drawnADTs) {
        adtRes->adtObject->collectMeshes(*adtRes.get(),  cullStage->exteriorView.drawnChunks, cullStage->exteriorView.renderOrder);
    }

    //Collect M2s for update
    cullStage->m2Array.clear();
    auto inserter = std::back_inserter(cullStage->m2Array);
    for (auto &view : cullStage->interiorViews) {
        std::copy(view.drawnM2s.begin(), view.drawnM2s.end(), inserter);
    }
    std::copy(cullStage->exteriorView.drawnM2s.begin(), cullStage->exteriorView.drawnM2s.end(), inserter);

    //Sort and delete duplicates
    std::sort( cullStage->m2Array.begin(), cullStage->m2Array.end() );
    cullStage->m2Array.erase( unique( cullStage->m2Array.begin(), cullStage->m2Array.end() ), cullStage->m2Array.end() );
    cullStage->m2Array = std::vector<std::shared_ptr<M2Object>>(cullStage->m2Array.begin(), cullStage->m2Array.end());

    std::sort( cullStage->wmoArray.begin(), cullStage->wmoArray.end() );
    cullStage->wmoArray.erase( unique( cullStage->wmoArray.begin(), cullStage->wmoArray.end() ), cullStage->wmoArray.end() );
    cullStage->wmoArray = std::vector<std::shared_ptr<WmoObject>>(cullStage->wmoArray.begin(), cullStage->wmoArray.end());

    cullStage->adtArray = std::vector<std::shared_ptr<ADTObjRenderRes>>(cullStage->adtArray.begin(), cullStage->adtArray.end());

//    //Limit M2 count based on distance/m2 height
//    for (auto it = this->m2RenderedThisFrameArr.begin();
//         it != this->m2RenderedThisFrameArr.end();) {
//        if ((*it)->getCurrentDistance() > ((*it)->getHeight() * 5)) {
//            it = this->m2RenderedThisFrameArr.erase(it);
//        } else {
//            ++it;
//        }
//    }
}

void Map::checkExterior(mathfu::vec4 &cameraPos,
                        std::vector<mathfu::vec3> &frustumPoints,
                        std::vector<mathfu::vec3> &hullLines,
                        mathfu::mat4 &lookAtMat4,
                        mathfu::mat4 &viewPerspectiveMat,
                        int viewRenderOrder,
                        HCullStage cullStage
) {
//    std::cout << "Map::checkExterior finished called" << std::endl;
    if (m_wdlObject == nullptr) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            m_wdlObject = std::make_shared<WdlObject>(m_api, m_wdtfile->mphd->wdlFileDataID);
            m_wdlObject->setMapApi(this);
        }

        return;
    }


    std::vector<std::shared_ptr<M2Object>> m2ObjectsCandidates;
    std::vector<std::shared_ptr<WmoObject>> wmoCandidates;

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
    int adt_x_min = std::max(worldCoordinateToAdtIndex(maxy), 0);
    int adt_x_max = std::min(worldCoordinateToAdtIndex(miny), 63);

    int adt_y_min = std::max(worldCoordinateToAdtIndex(maxx), 0);
    int adt_y_max = std::min(worldCoordinateToAdtIndex(minx), 63);

    int adt_global_x = worldCoordinateToGlobalAdtChunk(cameraPos.y);
    int adt_global_y = worldCoordinateToGlobalAdtChunk(cameraPos.x);

//    //FOR DEBUG
//    adt_x_min = worldCoordinateToAdtIndex(cameraPos.y) - 2;
//    adt_x_max = adt_x_min + 4;
//    adt_y_min = worldCoordinateToAdtIndex(cameraPos.x) - 2;
//    adt_y_max = adt_y_min + 4;

//    for (int i = 0; i < 64; i++)
//        for (int j = 0; j < 64; j++) {
//            if (this->m_wdtfile->mapTileTable->mainInfo[i][j].Flag_AllWater) {
//                std::cout << AdtIndexToWorldCoordinate(j) <<" "<<  AdtIndexToWorldCoordinate(i) << std::endl;
//            }
//        }

//    std::cout << AdtIndexToWorldCoordinate(adt_y_min) <<" "<<  AdtIndexToWorldCoordinate(adt_x_min) << std::endl;

    m_wdlObject->checkFrustumCulling(
        cameraPos, cullStage->exteriorView.frustumPlanes[0],
        frustumPoints,
        hullLines,
        lookAtMat4, m2ObjectsCandidates, wmoCandidates);

    if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
        for (int i = adt_x_min; i <= adt_x_max; i++) {
            for (int j = adt_y_min; j <= adt_y_max; j++) {
                if ((i < 0) || (i > 64)) continue;
                if ((j < 0) || (j > 64)) continue;

                auto adtObject = this->mapTiles[i][j];
                if (adtObject != nullptr) {

                    std::shared_ptr<ADTObjRenderRes> adtFrustRes = std::make_shared<ADTObjRenderRes>();
                    adtFrustRes->adtObject = adtObject;


                    bool result = adtObject->checkFrustumCulling(
                        *adtFrustRes.get(),
                        cameraPos,
                        adt_global_x,
                        adt_global_y,
                        cullStage->exteriorView.frustumPlanes[0], //TODO:!
                        frustumPoints,
                        hullLines,
                        lookAtMat4, m2ObjectsCandidates, wmoCandidates);
                    if (result) {
                        cullStage->exteriorView.drawnADTs.push_back(adtFrustRes);
                        cullStage->adtArray.push_back(adtFrustRes);
                    }
                } else if (!m_lockedMap && true) { //(m_wdtfile->mapTileTable->mainInfo[j][i].Flag_HasADT > 0) {
                    if (m_wdtfile->mphd->flags.wdt_has_maid) {
                        adtObject = std::make_shared<AdtObject>(m_api, i, j, m_wdtfile->mapFileDataIDs[j * 64 + i],
                                                                m_wdtfile);
                    } else {
                        std::string adtFileTemplate =
                            "world/maps/" + mapName + "/" + mapName + "_" + std::to_string(i) + "_" + std::to_string(j);
                        adtObject = std::make_shared<AdtObject>(m_api, adtFileTemplate, mapName, i, j, m_wdtfile);
                    }


                    adtObject->setMapApi(this);
                    this->mapTiles[i][j] = adtObject;
                }
            }
        }
    } else {
        wmoCandidates.push_back(wmoMap);
    }

    //Sort and delete duplicates
    std::sort( wmoCandidates.begin(), wmoCandidates.end() );
    wmoCandidates.erase( unique( wmoCandidates.begin(), wmoCandidates.end() ), wmoCandidates.end() );

    //Frustum cull
    for (auto it = wmoCandidates.begin(); it != wmoCandidates.end(); ++it) {
        auto wmoCandidate = *it;

        if (!wmoCandidate->isLoaded()) {
            cullStage->wmoArray.push_back(wmoCandidate);
            continue;
        }
        wmoCandidate->resetTraversedWmoGroups();
        if (wmoCandidate->startTraversingWMOGroup(
            cameraPos,
            viewPerspectiveMat,
            -1,
            0,
            viewRenderOrder,
            false,
            cullStage->interiorViews,
            cullStage->exteriorView)) {

            cullStage->wmoArray.push_back(wmoCandidate);
        }
    }

    //Sort and delete duplicates
    std::sort( m2ObjectsCandidates.begin(), m2ObjectsCandidates.end() );
    m2ObjectsCandidates.erase( unique( m2ObjectsCandidates.begin(), m2ObjectsCandidates.end() ), m2ObjectsCandidates.end() );

    //3.2 Iterate over all global WMOs and M2s (they have uniqueIds)
    {
        int numThreads = m_api->getConfig()->getThreadCount();

        #pragma omp parallel for num_threads(numThreads)
        for (size_t i = 0; i < m2ObjectsCandidates.size(); i++) {
            auto m2ObjectCandidate = m2ObjectsCandidates[i];
            bool frustumResult = m2ObjectCandidate->checkFrustumCulling(
                cameraPos,
                cullStage->exteriorView.frustumPlanes[0], //TODO:!
                frustumPoints);
        }
    }
//        bool frustumResult = true;
    {
        for (size_t i = 0; i < m2ObjectsCandidates.size(); i++) {
            auto m2ObjectCandidate = m2ObjectsCandidates[i];
            if (m2ObjectCandidate->m_cullResult) {
                cullStage->exteriorView.drawnM2s.push_back(m2ObjectCandidate);
                cullStage->m2Array.push_back(m2ObjectCandidate);
            }
        }
    }
}
void Map::doPostLoad(HCullStage cullStage){
    int processedThisFrame = 0;
    int groupsProcessedThisFrame = 0;
//    if (m_api->getConfig()->getRenderM2()) {
        for (int i = 0; i < cullStage->m2Array.size(); i++) {
            auto m2Object = cullStage->m2Array[i];
            if (m2Object == nullptr) continue;
            m2Object->doPostLoad();
        }
//    }

    for (auto &wmoObject : cullStage->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->doPostLoad(groupsProcessedThisFrame);
    }

    for (auto &adtObject : cullStage->adtArray) {
        adtObject->adtObject->doPostLoad();
    }
};

void Map::update(HUpdateStage updateStage) {
    if (m_wdtfile->getStatus() != FileStatus::FSLoaded) return;

    mathfu::vec3 cameraVec3 = updateStage->cameraMatrices->cameraPos.xyz();
    mathfu::mat4 &frustumMat = updateStage->cameraMatrices->perspectiveMat;
    mathfu::mat4 &lookAtMat = updateStage->cameraMatrices->lookAtMat;
    animTime_t deltaTime = updateStage->delta;

    Config* config = this->m_api->getConfig();
//    if (config->getRenderM2()) {
//        std::for_each(frameData->m2Array.begin(), frameData->m2Array.end(), [deltaTime, &cameraVec3, &lookAtMat](M2Object *m2Object) {

//    #pragma
    int numThreads = m_api->getConfig()->getThreadCount();
    {
        #pragma omp parallel for num_threads(numThreads) schedule(dynamic, 4)
        for (int i = 0; i < updateStage->cullResult->m2Array.size(); i++) {
            auto m2Object = updateStage->cullResult->m2Array[i];
            if (m2Object != nullptr) {
                m2Object->update(deltaTime, cameraVec3, lookAtMat);
            }
        }
    }

//        });
//    }

    for (auto &wmoObject : updateStage->cullResult->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->update();
    }

    for (auto &adtObject : updateStage->cullResult->adtArray) {
        adtObject->adtObject->update();
    }

    //2. Calc distance every 100 ms
//    #pragma omp parallel for
    for (int i = 0; i < updateStage->cullResult->m2Array.size(); i++) {
        auto m2Object = updateStage->cullResult->m2Array[i];
        if (m2Object == nullptr) continue;
        m2Object->calcDistance(cameraVec3);
    };


    //7. Get AreaId and Area Name
    std::string areaName = "";
    if (updateStage->cullResult->m_currentWMO != nullptr) {
        auto nameId = updateStage->cullResult->m_currentWMO->getNameSet();
        auto wmoId = updateStage->cullResult->m_currentWMO->getWmoId();
        auto groupId = updateStage->cullResult->m_currentWMO->getWmoGroupId(updateStage->cullResult->m_currentWmoGroup);

        areaName = m_api->databaseHandler->getWmoAreaName(wmoId, nameId, groupId);
    };
    if (areaName == "") {
        if (updateStage->cullResult->adtAreadId > 0) {
            areaName = m_api->databaseHandler->getAreaName(updateStage->cullResult->adtAreadId);
        } else {
            areaName = "";
        }
    }
    m_api->getConfig()->setAreaName(areaName);


    //8. Check fog color every 2 seconds
    bool fogRecordWasFound = false;
    if (this->m_currentTime + updateStage->delta - this->m_lastTimeLightCheck > 30) {
        mathfu::vec3 ambientColor = mathfu::vec3(0.0,0.0,0.0);
        mathfu::vec3 directColor = mathfu::vec3(0.0,0.0,0.0);
        mathfu::vec3 endFogColor = mathfu::vec3(0.0,0.0,0.0);

        if (updateStage->cullResult->m_currentWMO != nullptr) {
            CImVector sunFogColor;
            fogRecordWasFound = updateStage->cullResult->m_currentWMO->checkFog(cameraVec3, sunFogColor);
            if (fogRecordWasFound) {
                endFogColor =
                  mathfu::vec3((sunFogColor.r & 0xFF) / 255.0f,
                               ((sunFogColor.g ) & 0xFF) / 255.0f,
                               ((sunFogColor.b ) & 0xFF) / 255.0f);
            }
        }

        LightResult lightResult;
        m_api->databaseHandler->getEnvInfo(m_mapId,
            updateStage->cameraMatrices->cameraPos[0],
            updateStage->cameraMatrices->cameraPos[1],
            updateStage->cameraMatrices->cameraPos[2],
            config->getCurrentTime(),
            lightResult
        );

        if (m_currentSkyFDID != lightResult.skyBoxFdid) {
            if (lightResult.skyBoxFdid == 0) {
                m_exteriorSkyBox = nullptr;
            } else {
                m_exteriorSkyBox = std::make_shared<M2Object>(m_api, true);
                m_exteriorSkyBox->setLoadParams(0, {},{});

                m_exteriorSkyBox->setModelFileId(lightResult.skyBoxFdid);

                m_exteriorSkyBox->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
                m_exteriorSkyBox->calcWorldPosition();
            }
            m_currentSkyFDID = lightResult.skyBoxFdid;
        }

        //Database is in BGRA
        config->setExteriorAmbientColor(lightResult.ambientColor[2], lightResult.ambientColor[1], lightResult.ambientColor[0], 0);
        config->setExteriorDirectColor(lightResult.directColor[2]*3.0, lightResult.directColor[1]*3.0, lightResult.directColor[0]*3.0, 0);
        config->setCloseRiverColor(lightResult.closeRiverColor[2], lightResult.directColor[1], lightResult.directColor[0], 0);

        config->setFogColor(
                endFogColor.x,
                endFogColor.y,
                endFogColor.z,
                1.0
        );

        this->m_lastTimeLightCheck = this->m_currentTime;
    }

    //Cleanup ADT every 10 seconds
    if (this->m_currentTime + updateStage->delta- this->m_lastTimeAdtCleanup > 10000) {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                auto adtObj = mapTiles[i][j];
                //Free obj, if it was unused for 10 secs
                if (adtObj != nullptr && ((this->m_currentTime - adtObj->getLastTimeOfUpdate()) > 10000)) {
                    mapTiles[i][j] = nullptr;
                }
            }
        }

        this->m_lastTimeAdtCleanup = this->m_currentTime;
    }
    this->m_currentTime += updateStage->delta;
}

void Map::updateBuffers(HCullStage cullStage) {
    for (int i = 0; i < cullStage->m2Array.size(); i++) {
        auto m2Object = cullStage->m2Array[i];
        if (m2Object != nullptr) {
            m2Object->uploadGeneratorBuffers(cullStage->matricesForCulling->lookAtMat);
        }
    }

    for (auto &wmoObject : cullStage->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->uploadGeneratorBuffers();
    }

    for (auto &adtRes: cullStage->adtArray) {
        if (adtRes == nullptr) continue;
        adtRes->adtObject->uploadGeneratorBuffers(*adtRes.get());
    }

}

std::shared_ptr<M2Object> Map::getM2Object(std::string fileName, SMDoodadDef &doodadDef) {
    auto it = m_m2MapObjects[doodadDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto m2Object = std::make_shared<M2Object>(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileName(fileName);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();

        m_m2MapObjects[doodadDef.uniqueId] = std::weak_ptr<M2Object>(m2Object);
        return m2Object;
    }
    return nullptr;
}

std::shared_ptr<M2Object> Map::getM2Object(int fileDataId, SMDoodadDef &doodadDef) {
    auto it = m_m2MapObjects[doodadDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto m2Object = std::make_shared<M2Object>(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileId(fileDataId);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();

        m_m2MapObjects[doodadDef.uniqueId] = std::weak_ptr<M2Object>(m2Object);
        return m2Object;
    }
    return nullptr;
}


std::shared_ptr<WmoObject> Map::getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}
std::shared_ptr<WmoObject> Map::getWmoObject(int fileDataId, SMMapObjDef &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

void Map::collectMeshes(HUpdateStage updateStage) {
    auto cullStage = updateStage->cullResult;
    auto renderedThisFramePreSort = std::vector<HGMesh>();

    // Put everything into one array and sort
    std::vector<GeneralView *> vector;
    for (auto & interiorView : updateStage->cullResult->interiorViews) {
        if (interiorView.viewCreated) {
            vector.push_back(&interiorView);
        }
    }
    if (updateStage->cullResult->exteriorView.viewCreated) {
        vector.push_back(&updateStage->cullResult->exteriorView);
    }

//    if (m_api->getConfig()->getRenderWMO()) {
        for (auto &view : vector) {
            view->collectMeshes(renderedThisFramePreSort);
        }
//    }

    std::vector<std::shared_ptr<M2Object>> m2ObjectsRendered;
    for (auto &view : vector) {
        std::copy(view->drawnM2s.begin(),view->drawnM2s.end(), std::back_inserter(m2ObjectsRendered));
    }

//    std::unordered_set<std::shared_ptr<M2Object>> s;
//    for (auto i : m2ObjectsRendered)
//        s.insert(i);
//    m2ObjectsRendered.assign( s.begin(), s.end() );

    std::sort( m2ObjectsRendered.begin(), m2ObjectsRendered.end() );
    m2ObjectsRendered.erase( unique( m2ObjectsRendered.begin(), m2ObjectsRendered.end() ), m2ObjectsRendered.end() );

//    if (m_api->getConfig()->getRenderM2()) {
        for (auto &m2Object : m2ObjectsRendered) {
            if (m2Object == nullptr) continue;
            m2Object->collectMeshes(renderedThisFramePreSort, m_viewRenderOrder);
            m2Object->drawParticles(renderedThisFramePreSort, m_viewRenderOrder);
        }
//    }

    //No need to sort array which has only one element
    if (renderedThisFramePreSort.size() > 1) {
        auto *sortedArrayPtr = &renderedThisFramePreSort[0];
        std::vector<int> indexArray = std::vector<int>(renderedThisFramePreSort.size());
        for (int i = 0; i < indexArray.size(); i++) {
            indexArray[i] = i;
        }

        auto *config = m_api->getConfig();

//        if (config->getMovementSpeed() > 2) {
            quickSort_parallel(
                indexArray.data(),
                indexArray.size(),
                config->getThreadCount(),
                config->getQuickSortCutoff(),
                #include "../../../gapi/interface/sortLambda.h"
            );
//        } else {
//            quickSort_dualPoint(
//                indexArray.data(),
//                indexArray.size(),
//                config->getThreadCount(),
//                config->getQuickSortCutoff(),
//                #include "../../../gapi/interface/sortLambda.h"
//            );
//        }

        updateStage->meshes->reserve(indexArray.size());
        for (int i = 0; i < indexArray.size(); i++) {
            updateStage->meshes->push_back(renderedThisFramePreSort[indexArray[i]]);
        }
    } else {
        for (int i = 0; i < renderedThisFramePreSort.size(); i++) {
            updateStage->meshes->push_back(renderedThisFramePreSort[i]);
        }
    }
};

animTime_t Map::getCurrentSceneTime() {
    return m_currentTime;
}
