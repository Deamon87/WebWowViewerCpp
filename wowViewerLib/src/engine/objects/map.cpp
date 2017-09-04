//
// Created by Deamon on 7/16/2017.
//

#include <set>
#include <cmath>
#include "map.h"
#include "../algorithms/mathHelper.h"
#include "../algorithms/grahamScan.h"

void Map::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    adtRenderedThisFrame = std::set<AdtObject*>();
    m2RenderedThisFrame = std::set<M2Object*>();
    wmoRenderedThisFrame = std::set<WmoObject*>();

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    checkExterior(cameraPos, frustumPlanes, frustumPoints, hullines, lookAtMat4,
                  adtRenderedThisFrame, m2RenderedThisFrame, wmoRenderedThisFrame);

    adtRenderedThisFrameArr = std::vector<AdtObject*>(adtRenderedThisFrame.begin(), adtRenderedThisFrame.end());
    m2RenderedThisFrameArr = std::vector<M2Object*>(m2RenderedThisFrame.begin(), m2RenderedThisFrame.end());
    wmoRenderedThisFrameArr = std::vector<WmoObject*>(wmoRenderedThisFrame.begin(), wmoRenderedThisFrame.end());
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

        if ( false /*wmoCandidate->hasPortals() && config.getUsePortalCulling() */) {
//            if(self.portalCullingAlgo.startTraversingFromExterior(wmoCandidate, self.position,
//                                                                  lookAtMat4, frustumPlanes, m2RenderedThisFrame)){
//                wmoRenderedThisFrame.add(wmoCandidate);
//            }
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
//    this.m2OpaqueRenderedThisFrame = {};
//    this.m2TranspRenderedThisFrame = {};
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
//    this.sceneApi.shaders.activateFrustumBoxShader();
//        if (this.sceneApi.getIsDebugCamera()) {
//            this.sceneApi.drawCamera()
//        }
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
    //if (config.getRenderM2()) {
//        bool lastWasDrawInstanced = false;
//        this->m_api->activateM2Shader();
//        for (var i = 0; i < this.m2RenderedThisFrame.length; i++) {
//            var m2Object = this.m2RenderedThisFrame[i];
//            if (this.m2OpaqueRenderedThisFrame[m2Object.sceneNumber]) continue;
//            var fileIdent = m2Object.getFileNameIdent();
//
//            var drawInstanced = false;
//            if (this.instanceMap.has(fileIdent)) {
//                var instanceManager = this.instanceMap.get(fileIdent);
//                drawInstanced = instanceManager.mdxObjectList.length > 1;
//            }
//            if (drawInstanced) {
//                if (!lastWasDrawInstanced) {
//                    this.sceneApi.shaders.activateM2InstancingShader();
//                }
//
//                instanceManager.drawInstancedNonTransparentMeshes(this.m2OpaqueRenderedThisFrame);
//                lastWasDrawInstanced = true;
//            } else {
//                if (lastWasDrawInstanced) {
//                    this.sceneApi.shaders.deactivateM2InstancingShader();
//                    this.sceneApi.shaders.activateM2Shader();
//                }
//
//                this.m2OpaqueRenderedThisFrame[m2Object.sceneNumber] = true;
//                m2Object.drawNonTransparentMeshes();
//                lastWasDrawInstanced = false;
//            }
//        }
//        if (lastWasDrawInstanced) {
//            this->m_api->deactivateM2InstancingShader();
//        } else {
//            this->m_api->deactivateM2Shader();
//        }
//   // }
//
//    //6. Draw transparent meshes of m2
////    if (config.getRenderM2()) {
//        var lastWasDrawInstanced = false;
//        this.sceneApi.shaders.activateM2Shader();
//        for (var i = this.m2RenderedThisFrame.length-1; i >= 0; i--) {
//            var m2Object = this.m2RenderedThisFrame[i];
//            if (this.m2TranspRenderedThisFrame[m2Object.sceneNumber]) continue;
//            var fileIdent = m2Object.getFileNameIdent();
//
//            var drawInstanced = false;
//            if (this.instanceMap.has(fileIdent)) {
//                var instanceManager = this.instanceMap.get(fileIdent);
//                drawInstanced = instanceManager.mdxObjectList.length > 1;
//            }
//            if (drawInstanced) {
//                if (!lastWasDrawInstanced) {
//                    this.sceneApi.shaders.activateM2InstancingShader();
//                }
//
//                instanceManager.drawInstancedTransparentMeshes(this.m2TranspRenderedThisFrame);
//                lastWasDrawInstanced = true;
//            } else {
//                if (lastWasDrawInstanced) {
//                    this.sceneApi.shaders.deactivateM2InstancingShader();
//                    this.sceneApi.shaders.activateM2Shader();
//                }
//
//                this.m2TranspRenderedThisFrame[m2Object.sceneNumber] = true;
//                m2Object.drawTransparentMeshes();
//                lastWasDrawInstanced = false;
//            }
//        }
//        if (lastWasDrawInstanced) {
//            this->m_api->deactivateM2InstancingShader();
//        } else {
//            this->m_api->deactivateM2Shader();
//        }
    this->m_api->activateM2Shader();
    mathfu::vec4 diffuseNon(1.0, 1.0, 1.0, 1.0);
    for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {

        M2Object *m2Object = this->m2RenderedThisFrameArr[i];

        //TODO: fix this hack
        m2Object->update(0, mathfu::vec3(0,0,0), mathfu::mat4::Identity());

        m2Object->draw(false, diffuseNon);
    }

    for (int i = this->m2RenderedThisFrameArr.size()-1; i >= 0; i--) {
        M2Object *m2Object = this->m2RenderedThisFrameArr[i];

        m2Object->draw(true, diffuseNon);
    }


//    }
}
