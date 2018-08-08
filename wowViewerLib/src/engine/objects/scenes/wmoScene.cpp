//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/meshes/GM2Mesh.h"

void WmoScene::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    m2RenderedThisFrame = std::vector<M2Object*>();
    wmoRenderedThisFrame = std::vector<WmoObject*>();

    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    exteriorView = ExteriorView();
    interiorViews = std::vector<InteriorView>();
    m_viewRenderOrder = 0;

    this->m_wmoObject->resetTraversedWmoGroups();
    if (!this->m_currentInteriorGroups.empty() && this->m_wmoObject->isLoaded()) {
        if (this->m_currentWMO->startTraversingWMOGroup(
            cameraPos,
            projectionModelMat,
            this->m_currentInteriorGroups[0].groupIndex,
            0,
            m_viewRenderOrder,
            true,
            interiorViews,
            exteriorView)) {

            wmoRenderedThisFrame.push_back(this->m_currentWMO);
        }

        if (exteriorView.viewCreated) {
            cullExterior(cameraPos, projectionModelMat, m_viewRenderOrder);
        }
    } else {
        //Cull exterior
        exteriorView.viewCreated = true;
        exteriorView.frustumPlanes.push_back(frustumPlanes);
        cullExterior(cameraPos, projectionModelMat, m_viewRenderOrder);
    }

    //Fill M2 objects for views from WmoGroups
    for (auto &view : interiorViews) {
        view.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    }
    exteriorView.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);

    //Collect M2s for update
    m2RenderedThisFrame.clear();
    auto inserter = std::back_inserter(m2RenderedThisFrame);
    for (auto &view : interiorViews) {
        std::copy(view.drawnM2s.begin(), view.drawnM2s.end(), inserter);
    }
    std::copy(exteriorView.drawnM2s.begin(), exteriorView.drawnM2s.end(), inserter);

    //Sort and delete duplicates
    std::sort( m2RenderedThisFrame.begin(), m2RenderedThisFrame.end() );
    m2RenderedThisFrame.erase( unique( m2RenderedThisFrame.begin(), m2RenderedThisFrame.end() ), m2RenderedThisFrame.end() );

    m2RenderedThisFrameArr = std::vector<M2Object*>(m2RenderedThisFrame.begin(), m2RenderedThisFrame.end());

    std::sort( wmoRenderedThisFrame.begin(), wmoRenderedThisFrame.end() );
    wmoRenderedThisFrame.erase( unique( wmoRenderedThisFrame.begin(), wmoRenderedThisFrame.end() ), wmoRenderedThisFrame.end() );
    wmoRenderedThisFrameArr = std::vector<WmoObject*>(wmoRenderedThisFrame.begin(), wmoRenderedThisFrame.end());
}

void WmoScene::cullExterior(mathfu::vec4 &cameraPos, mathfu::mat4 &projectionModelMat, int viewRenderOrder) {
    if (m_wmoObject->startTraversingWMOGroup(
        cameraPos,
        projectionModelMat,
        -1,
        0,
        viewRenderOrder,
        false,
        interiorViews,
        exteriorView)) {

        wmoRenderedThisFrame.push_back(m_wmoObject);
    }
}

void WmoScene::draw() {
    std::vector<HGMesh> renderedThisFrame;

    // Put everything into one array and sort
    std::vector<GeneralView *> vector;
    for (auto & interiorView : thisFrameInteriorViews) {
        if (interiorView.viewCreated) {
            vector.push_back(&interiorView);
        }
    }
    if (thisFrameExteriorView.viewCreated) {
        vector.push_back(&thisFrameExteriorView);
    }

    for (auto &view : vector) {
        view->collectMeshes(renderedThisFrame);
    }

    std::vector<M2Object *> m2ObjectsRendered;
    for (auto &view : vector) {
        std::copy(view->drawnM2s.begin(),view->drawnM2s.end(), std::back_inserter(m2ObjectsRendered));
    }
    std::sort( m2ObjectsRendered.begin(), m2ObjectsRendered.end() );
    m2ObjectsRendered.erase( unique( m2ObjectsRendered.begin(), m2ObjectsRendered.end() ), m2ObjectsRendered.end() );

    for (auto &m2Object : m2ObjectsRendered) {
        m2Object->collectMeshes(renderedThisFrame, m_viewRenderOrder);
        m2Object->drawParticles(renderedThisFrame, m_viewRenderOrder);
    }

    std::sort(renderedThisFrame.begin(),
              renderedThisFrame.end(),
              GDevice::sortMeshes
    );
    m_api->getDevice()->drawMeshes(renderedThisFrame);
}

void WmoScene::copyToCurrentFrame() {
    currentFrameM2RenderedThisFrameArr = m2RenderedThisFrameArr;
    currentFrameWmoRenderedThisFrameArr = wmoRenderedThisFrameArr;

    thisFrameInteriorViews = interiorViews;
    thisFrameExteriorView = exteriorView;
}
void WmoScene::doPostLoad() {
    for (int i = 0; i < this->currentFrameM2RenderedThisFrameArr.size(); i++) {
        M2Object *m2Object = this->currentFrameM2RenderedThisFrameArr[i];
        m2Object->doPostLoad();
    }

    for (int i = 0; i < this->currentFrameWmoRenderedThisFrameArr.size(); i++) {
        this->currentFrameWmoRenderedThisFrameArr[i]->doPostLoad();
    }
}

void WmoScene::update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat)  {
    for (int i = 0; i < this->m2RenderedThisFrameArr.size(); i++) {
        M2Object *m2Object = this->m2RenderedThisFrameArr[i];
        m2Object->update(deltaTime, cameraVec3, lookAtMat);
    }

    for (int i = 0; i < this->wmoRenderedThisFrameArr.size(); i++) {
        this->wmoRenderedThisFrameArr[i]->update();
    }

    //2. Calc distance every 100 ms
    //if (this->m_currentTime + deltaTime - this->m_lastTimeDistanceCalc > 100) {
        for (int j = 0; j < this->m2RenderedThisFrameArr.size(); j++) {
            this->m2RenderedThisFrameArr[j]->calcDistance(cameraVec3);
        }

        this->m_lastTimeDistanceCalc = this->m_currentTime;
    //}

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
