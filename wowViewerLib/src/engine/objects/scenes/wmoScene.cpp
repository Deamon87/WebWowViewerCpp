//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"

void WmoScene::getPotentialEntities(const mathfu::vec4 &cameraPos, std::vector<std::shared_ptr<M2Object>> &potentialM2,
                                   HCullStage &cullStage, mathfu::mat4 &lookAtMat4, mathfu::vec4 &camera4,
                                   std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                                   std::vector<std::shared_ptr<WmoObject>> &potentialWmo) {
    potentialWmo.push_back(this->m_wmoObject);
}

void WmoScene::getCandidatesEntities(std::vector<mathfu::vec3> &hullLines, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos,
                           std::vector<mathfu::vec3> &frustumPoints, HCullStage &cullStage,
                           std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                           std::vector<std::shared_ptr<WmoObject>> &wmoCandidates) {

    wmoCandidates.push_back(this->m_wmoObject);
};
/*
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
*/
/*

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
*/