//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"

void WmoScene::checkCulling(WoWFrameData *frameData) {
    frameData->m2Array = std::vector<M2Object*>();
    frameData->wmoArray = std::vector<WmoObject*>();

    mathfu::vec4 cameraPos = mathfu::vec4(frameData->m_cameraVec3, 1.0);
    mathfu::mat4 &frustumMat = frameData->m_perspectiveMatrixForCulling;
    mathfu::mat4 &lookAtMat4 = frameData->m_lookAtMat4;

    mathfu::mat4 projectionModelMat = frustumMat * lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    frameData->exteriorView = ExteriorView();
    frameData->interiorViews = std::vector<InteriorView>();
    m_viewRenderOrder = 0;

    if (!frameData->currentInteriorGroups.empty() && this->m_wmoObject->isLoaded()) {
        this->m_wmoObject->resetTraversedWmoGroups();
        if (this->m_wmoObject->startTraversingWMOGroup(
            cameraPos,
            projectionModelMat,
            frameData->currentInteriorGroups[0].groupIndex,
            0,
            m_viewRenderOrder,
            true,
            frameData->interiorViews,
            frameData->exteriorView)) {

            frameData->wmoArray.push_back(this->m_wmoObject);
        }

        if (frameData->exteriorView.viewCreated) {
            cullExterior(frameData, m_viewRenderOrder);
        }
    } else {
        //Cull exterior
        frameData->exteriorView.viewCreated = true;
        frameData->exteriorView.frustumPlanes.push_back(frustumPlanes);
        cullExterior(frameData, m_viewRenderOrder);
    }

    //Fill M2 objects for views from WmoGroups
    for (auto &view : frameData->interiorViews) {
        view.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    }
    frameData->exteriorView.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);

    //Collect M2s for update
    frameData->m2Array.clear();
    auto inserter = std::back_inserter(frameData->m2Array);
    for (auto &view : frameData->interiorViews) {
        std::copy(view.drawnM2s.begin(), view.drawnM2s.end(), inserter);
    }
    std::copy(frameData->exteriorView.drawnM2s.begin(), frameData->exteriorView.drawnM2s.end(), inserter);

    //Sort and delete duplicates
    std::sort( frameData->m2Array.begin(), frameData->m2Array.end() );
    frameData->m2Array.erase( unique( frameData->m2Array.begin(), frameData->m2Array.end() ), frameData->m2Array.end() );

    frameData->m2Array= std::vector<M2Object*>(frameData->m2Array.begin(), frameData->m2Array.end());

    std::sort( frameData->wmoArray.begin(), frameData->wmoArray.end() );
    frameData->wmoArray.erase( unique( frameData->wmoArray.begin(), frameData->wmoArray.end() ), frameData->wmoArray.end() );
    frameData->wmoArray = std::vector<WmoObject*>(frameData->wmoArray.begin(), frameData->wmoArray.end());
}

void WmoScene::cullExterior(WoWFrameData *frameData, int viewRenderOrder) {
    mathfu::vec4 cameraVec4 = mathfu::vec4(frameData->m_cameraVec3, 1.0);

    mathfu::mat4 &frustumMat = frameData->m_perspectiveMatrixForCulling;
    mathfu::mat4 &lookAtMat4 = frameData->m_lookAtMat4;

    mathfu::mat4 projectionModelMat = frustumMat * lookAtMat4;

    this->m_wmoObject->resetTraversedWmoGroups();
    if (m_wmoObject->startTraversingWMOGroup(
        cameraVec4,
        projectionModelMat,
        -1,
        0,
        viewRenderOrder,
        false,
        frameData->interiorViews,
        frameData->exteriorView)) {

        frameData->wmoArray.push_back(m_wmoObject);
    }
}

void WmoScene::draw(WoWFrameData *frameData) {
    m_api->getDevice()->drawMeshes(frameData->renderedThisFrame);
}

void WmoScene::doPostLoad(WoWFrameData *frameData) {
    int processedThisFrame = 0;
    int groupsProcessedThisFrame = 0;

    for (int i = 0; i < frameData->m2Array.size(); i++) {
        M2Object *m2Object = frameData->m2Array[i];
        if (m2Object == nullptr) continue;
        if (m2Object->doPostLoad()) processedThisFrame++;
        if (processedThisFrame > 3) return;
    }
//    }

    for (auto &wmoObject : frameData->wmoArray) {
        if (wmoObject == nullptr) continue;
        if (wmoObject->doPostLoad(groupsProcessedThisFrame)) processedThisFrame++;

        if (processedThisFrame > 3) return;
    }
}

void WmoScene::update(WoWFrameData *frameData)  {
    mathfu::vec3 &cameraVec3 = frameData->m_cameraVec3;
    mathfu::mat4 &frustumMat = frameData->m_perspectiveMatrix;
    mathfu::mat4 &lookAtMat4 = frameData->m_lookAtMat4;

    for (int i = 0; i < frameData->m2Array.size(); i++) {
        M2Object *m2Object = frameData->m2Array[i];
        if (m2Object == nullptr) continue;
        m2Object->update(frameData->deltaTime, cameraVec3, lookAtMat4);
    }

    for (auto &wmoObject : frameData->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->update();
    }

    //2. Calc distance every 100 ms
    //if (this->m_currentTime + deltaTime - this->m_lastTimeDistanceCalc > 100) {
        for (int j = 0; j < frameData->m2Array.size(); j++) {
            frameData->m2Array[j]->calcDistance(cameraVec3);
        }

        this->m_lastTimeDistanceCalc = this->m_currentTime;
    //}

    //6. Check what WMO instance we're in
    frameData->currentInteriorGroups = {};
    this->m_currentWMO = nullptr;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    int currentWmoGroup = -1;

    //Get center of near plane
    mathfu::mat4 viewPerspectiveMat = frustumMat*lookAtMat4;
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
                frameData->currentInteriorGroups.push_back(groupResult);
                interiorGroupNum = groupResult.groupIndex;
            } else {
            }

            bspNodeId = groupResult.nodeId;
//            break;
        }
//    }


    this->m_currentTime += frameData->deltaTime;
}

void WmoScene::collectMeshes(WoWFrameData * frameData) {
    frameData->renderedThisFrame = std::vector<HGMesh>();

    // Put everything into one array and sort
    std::vector<GeneralView *> vector;
    for (auto & interiorView : frameData->interiorViews) {
        if (interiorView.viewCreated) {
            vector.push_back(&interiorView);
        }
    }
    if (frameData->exteriorView.viewCreated) {
        vector.push_back(&frameData->exteriorView);
    }

    for (auto &view : vector) {
        view->collectMeshes(frameData->renderedThisFrame);
    }

    std::vector<M2Object *> m2ObjectsRendered;
    for (auto &view : vector) {
        std::copy(view->drawnM2s.begin(),view->drawnM2s.end(), std::back_inserter(m2ObjectsRendered));
    }
    std::sort( m2ObjectsRendered.begin(), m2ObjectsRendered.end() );
    m2ObjectsRendered.erase( unique( m2ObjectsRendered.begin(), m2ObjectsRendered.end() ), m2ObjectsRendered.end() );

    for (auto &m2Object : m2ObjectsRendered) {
        if (m2Object == nullptr) continue;
        m2Object->collectMeshes(frameData->renderedThisFrame, m_viewRenderOrder);
        m2Object->drawParticles(frameData->renderedThisFrame, m_viewRenderOrder);
    }

    std::sort(frameData->renderedThisFrame.begin(),
              frameData->renderedThisFrame.end(),
              IDevice::sortMeshes
    );

}
