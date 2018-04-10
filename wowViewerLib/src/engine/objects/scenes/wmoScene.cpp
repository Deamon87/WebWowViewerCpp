//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"

void WmoScene::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    m2RenderedThisFrame = std::unordered_set<M2Object*>();
    wmoRenderedThisFrame = std::unordered_set<WmoObject*>();

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    if (!this->m_wmoObject->isLoaded()) {
//        this->m_wmoObject->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints, m2RenderedThisFrame);
        wmoRenderedThisFrame.insert(this->m_wmoObject);
    } else
    if (!this->m_currentInteriorGroups.empty() && this->m_wmoObject->isLoaded() && this->m_wmoObject->hasPortals() && m_api->getConfig()->getUsePortalCulling()) {
        if (this->m_currentWMO->startTraversingFromInteriorWMO(
                this->m_currentInteriorGroups,
                cameraPos,
                projectionModelMat,
                m2RenderedThisFrame)) {

            wmoRenderedThisFrame.insert(this->m_currentWMO);
        }
    } else {
        if ( this->m_wmoObject->isLoaded() && this->m_wmoObject->hasPortals() && m_api->getConfig()->getUsePortalCulling() ) {
            if(this->m_wmoObject->startTraversingFromExterior(cameraPos, projectionModelMat, m2RenderedThisFrame)){
                wmoRenderedThisFrame.insert(this->m_wmoObject);
            }
        } else {
            if (this->m_wmoObject->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints, m2RenderedThisFrame)) {
                wmoRenderedThisFrame.insert(this->m_wmoObject);
            }
        }
    }
    m2RenderedThisFrameArr = std::vector<M2Object*>(m2RenderedThisFrame.begin(), m2RenderedThisFrame.end());
    wmoRenderedThisFrameArr = std::vector<WmoObject*>(wmoRenderedThisFrame.begin(), wmoRenderedThisFrame.end());
}

void WmoScene::draw() {
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
//
//    this->m_api->activateDrawPointShader();
//    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
//        this->wmoRenderedThisFrameArr[i]->drawDebugLights();
//    }
//    this->m_api->deactivateDrawPointShader();

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

void WmoScene::drawM2s() {
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
        m2Object->draw(false);
    }

    for (int i = this->m2RenderedThisFrameArr.size() - 1; i >= 0; i--) {
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
//    this->m_api->activateDrawPointShader();
//
//    for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
//        this->m2RenderedThisFrameArr[i]->drawDebugLight();
//    }
//
//    this->m_api->deactivateDrawPointShader();
}


void WmoScene::update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat)  {
    for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
        M2Object *m2Object = this->m2RenderedThisFrameArr[i];
        m2Object->update(deltaTime, cameraVec3, lookAtMat);
//            if (this.isM2Scene && m2Object.objectUpdate) {
//                m2Object.objectUpdate(deltaTime, cameraVec4, lookAtMat);
//            }
    }
//    }

    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
        this->wmoRenderedThisFrameArr[i]->update();
    }

    //2. Calc distance every 100 ms
    if (this->m_currentTime + deltaTime - this->m_lastTimeDistanceCalc > 100) {
        for (int j = 0; j < this->m2RenderedThisFrameArr.size(); j++) {
            //if (this.m2Objects[j].getIsRendered()) {
            this->m2RenderedThisFrameArr[j]->calcDistance(cameraVec3);
            //}
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

//    //4. Collect m2 into instances every 200 ms
////        if (this.currentTime + deltaTime - this.lastInstanceCollect > 30) {
//    var map = new Map();
//    if (this.sceneApi.extensions.getInstancingExt()) {
//        //Clear instance lists
//        for (var j = 0; j < this.instanceList.length; j++) {
//            this.instanceList[j].clearList();
//        }
//
//        for (var j = 0; j < this.m2RenderedThisFrame.length; j++) {
//            var m2Object = this.m2RenderedThisFrame[j];
//
//            if (!m2Object.m2Geom) continue;
//            if (m2Object.getHasBillboarded() || !m2Object.getIsInstancable()) continue;
//
//            var fileIdent = m2Object.getFileNameIdent();
//
//            if (map.has(fileIdent)) {
//                var m2ObjectInstanced = map.get(fileIdent);
//                this.addM2ObjectToInstanceManager(m2Object);
//                this.addM2ObjectToInstanceManager(m2ObjectInstanced);
//            } else {
//                map.set(fileIdent, m2Object);
//            }
//        }
//    }


//    //4.1 Update placement matrix buffers in Instance
//    for (var j = 0; j < this.instanceList.length; j++) {
//        var instanceManager = this.instanceList[j];
//        instanceManager.updatePlacementVBO();
//    }
//
//    this.lastInstanceCollect = this.currentTime;


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
    nearPlaneCenter *= 0.25f;

//    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
        WmoObject *checkingWmoObj = this->m_wmoObject;
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

            bspNodeId = groupResult.nodeId;
//            break;
        }
//    }


    this->m_currentTime += deltaTime;
}
