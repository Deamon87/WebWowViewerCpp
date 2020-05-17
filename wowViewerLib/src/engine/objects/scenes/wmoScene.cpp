//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"

void WmoScene::checkCulling(HCullStage cullStage) {
    cullStage->m2Array = std::vector<std::shared_ptr<M2Object>>();
    cullStage->wmoArray = std::vector<std::shared_ptr<WmoObject>>();

    mathfu::vec4 cameraPos = cullStage->matricesForCulling->cameraPos;
    mathfu::vec3 cameraVec3 = cullStage->matricesForCulling->cameraPos.xyz();
    mathfu::mat4 &frustumMat = cullStage->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = cullStage->matricesForCulling->lookAtMat;

    mathfu::mat4 projectionModelMat = frustumMat * lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    cullStage->exteriorView = ExteriorView();
    cullStage->interiorViews = std::vector<InteriorView>();
    m_viewRenderOrder = 0;

    //6. Check what WMO instance we're in
    cullStage->m_currentInteriorGroups = {};
    this->m_currentWMO = nullptr;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    int currentWmoGroup = -1;

    std::shared_ptr<WmoObject> checkingWmoObj = this->m_wmoObject;
    WmoGroupResult groupResult;
    bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(mathfu::vec4(cameraVec3, 1), groupResult);

    if (result) {
        this->m_currentWMO = checkingWmoObj;
        currentWmoGroup = groupResult.groupIndex;

        if (checkingWmoObj->isGroupWmoInterior(groupResult.groupIndex)) {
            cullStage->m_currentInteriorGroups.push_back(groupResult);
            interiorGroupNum = groupResult.groupIndex;
        } else {
//            std::cout << "not found! duh!";
        }

        bspNodeId = groupResult.nodeId;
//            break;
    } else {
//        std::cout << "not found! duh!";
    }
//    }

    if (!cullStage->m_currentInteriorGroups.empty() && this->m_wmoObject->isLoaded()) {
        this->m_wmoObject->resetTraversedWmoGroups();
        if (this->m_wmoObject->startTraversingWMOGroup(
            cameraPos,
            projectionModelMat,
            cullStage->m_currentInteriorGroups[0].groupIndex,
            0,
            m_viewRenderOrder,
            true,
            cullStage->interiorViews,
            cullStage->exteriorView)) {

            cullStage->wmoArray.push_back(this->m_wmoObject);
        }

        if (cullStage->exteriorView.viewCreated) {
            cullExterior(cullStage, m_viewRenderOrder);
        }
    } else {
        //Cull exterior
        cullExterior(cullStage, m_viewRenderOrder);
    }

    //Fill M2 objects for views from WmoGroups
    for (auto &view : cullStage->interiorViews) {
        view.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    }
    cullStage->exteriorView.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);

    //Collect M2s for update
    size_t prev_size = cullStage->m2Array.size();
    cullStage->m2Array.clear();
    cullStage->m2Array.reserve(prev_size);
    auto inserter = std::back_inserter(cullStage->m2Array);
    for (auto &view : cullStage->interiorViews) {
        std::copy(view.drawnM2s.begin(), view.drawnM2s.end(), inserter);
    }
    std::copy(cullStage->exteriorView.drawnM2s.begin(), cullStage->exteriorView.drawnM2s.end(), inserter);

    //Sort and delete duplicates
    std::unordered_set<std::shared_ptr<M2Object>> m2Set;
    for (auto i : cullStage->m2Array) {
        if (!i) continue;
        m2Set.insert(i);
    }

    cullStage->m2Array.assign( m2Set.begin(), m2Set.end() );



//    std::sort( frameData->wmoArray.begin(), frameData->wmoArray.end() );
//    frameData->wmoArray.erase( unique( frameData->wmoArray.begin(), frameData->wmoArray.end() ), frameData->wmoArray.end() );
}

void WmoScene::cullExterior(HCullStage cullStage, int viewRenderOrder) {
    mathfu::vec4 cameraVec4 = cullStage->matricesForCulling->cameraPos;

    mathfu::mat4 &frustumMat = cullStage->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = cullStage->matricesForCulling->lookAtMat;

    mathfu::mat4 projectionModelMat = frustumMat * lookAtMat4;

    if (m_currentWMO != m_wmoObject || cullStage->m_currentInteriorGroups.size() <= 0) {
        this->m_wmoObject->resetTraversedWmoGroups();
    }
    if (m_wmoObject->startTraversingWMOGroup(
        cameraVec4,
        projectionModelMat,
        -1,
        0,
        viewRenderOrder,
        false,
        cullStage->interiorViews,
        cullStage->exteriorView)) {

        cullStage->wmoArray.push_back(m_wmoObject);
    }
}

void WmoScene::doPostLoad(HCullStage cullStage) {
    int processedThisFrame = 0;
    int groupsProcessedThisFrame = 0;

    for (int i = 0; i < cullStage->m2Array.size(); i++) {
        std::shared_ptr<M2Object> m2Object = cullStage->m2Array[i];
        if (m2Object == nullptr) continue;
        if (m2Object->doPostLoad()) processedThisFrame++;
        if (processedThisFrame > 3) return;
    }
//    }

    for (auto &wmoObject : cullStage->wmoArray) {
        if (wmoObject == nullptr) continue;
        if (wmoObject->doPostLoad(groupsProcessedThisFrame)) processedThisFrame++;

        if (processedThisFrame > 3) return;
    }
}

void WmoScene::update(HUpdateStage updateStage)  {
    mathfu::vec3 cameraVec3 = updateStage->cameraMatrices->cameraPos.xyz();
    mathfu::mat4 &lookAtMat4 = updateStage->cameraMatrices->lookAtMat;

    for (int i = 0; i < updateStage->cullResult->m2Array.size(); i++) {
        auto m2Object = updateStage->cullResult->m2Array[i];
        if (m2Object == nullptr) continue;
        m2Object->update(updateStage->delta, cameraVec3, lookAtMat4);
        m2Object->uploadGeneratorBuffers(lookAtMat4);
    }

    for (auto &wmoObject : updateStage->cullResult->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->update();
    }

    //2. Calc distance every 100 ms
    //if (this->m_currentTime + deltaTime - this->m_lastTimeDistanceCalc > 100) {
        for (int j = 0; j < updateStage->cullResult->m2Array.size(); j++) {
            updateStage->cullResult->m2Array[j]->calcDistance(cameraVec3);
        }

        this->m_lastTimeDistanceCalc = this->m_currentTime;
    //}

     this->m_currentTime += updateStage->delta;
}
void WmoScene::updateBuffers(HCullStage cullStage) {

}

void WmoScene::produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) {
    auto cullStage = updateStage->cullResult;
    auto renderedThisFramePreSort = std::vector<HGMesh>();

    //Create meshes
    resultDrawStage->meshesToRender = std::make_shared<MeshesToRender>();

    //Create scenewide uniform
    auto renderMats = resultDrawStage->matricesForRendering;

    auto config = m_api->getConfig();
    resultDrawStage->sceneWideBlockVSPSChunk = m_api->hDevice->createUniformBufferChunk(sizeof(sceneWideBlockVSPS));
    resultDrawStage->sceneWideBlockVSPSChunk->setUpdateHandler([renderMats, config](IUniformBufferChunk *chunk) -> void {
        auto *blockPSVS = &chunk->getObject<sceneWideBlockVSPS>();
        blockPSVS->uLookAtMat = renderMats->lookAtMat;
        blockPSVS->uPMatrix = renderMats->perspectiveMat;
        blockPSVS->uInteriorSunDir = renderMats->interiorDirectLightDir;
        blockPSVS->uViewUp = renderMats->viewUp;

        blockPSVS->extLight.uExteriorAmbientColor = mathfu::vec4(0.8,0.8,0.8,0.8);
        blockPSVS->extLight.uExteriorHorizontAmbientColor = mathfu::vec4(1.0,1.0,1.0,1.0);
        blockPSVS->extLight.uExteriorGroundAmbientColor = mathfu::vec4(1.0,1.0,1.0,1.0);
        blockPSVS->extLight.uExteriorDirectColor = mathfu::vec4(0.3,0.3,0.3,1.3);
        blockPSVS->extLight.uExteriorDirectColorDir = mathfu::vec4(0.0,0.0,0.0,1.0);
    });


    std::vector<GeneralView *> vector;
    for (auto & interiorView : updateStage->cullResult->interiorViews) {
        if (interiorView.viewCreated) {
            vector.push_back(&interiorView);
        }
    }
    if (updateStage->cullResult->exteriorView.viewCreated) {
        vector.push_back(&updateStage->cullResult->exteriorView);
    }

    for (auto &view : vector) {
        view->collectMeshes(resultDrawStage->meshesToRender->meshes);
    }

    std::vector<std::shared_ptr<M2Object>> m2ObjectsRendered;
    for (auto &view : vector) {
        std::copy(view->drawnM2s.begin(),view->drawnM2s.end(), std::back_inserter(m2ObjectsRendered));
    }
    std::sort( m2ObjectsRendered.begin(), m2ObjectsRendered.end() );
    m2ObjectsRendered.erase( unique( m2ObjectsRendered.begin(), m2ObjectsRendered.end() ), m2ObjectsRendered.end() );

    for (auto &m2Object : m2ObjectsRendered) {
        if (m2Object == nullptr) continue;
        m2Object->collectMeshes(resultDrawStage->meshesToRender->meshes, m_viewRenderOrder);
        m2Object->drawParticles(resultDrawStage->meshesToRender->meshes, m_viewRenderOrder);
    }

    std::sort(resultDrawStage->meshesToRender->meshes.begin(),
              resultDrawStage->meshesToRender->meshes.end(),
              IDevice::sortMeshes
    );
}
