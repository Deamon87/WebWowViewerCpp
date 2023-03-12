//
// Created by Deamon on 11/26/2022.
//

#include "MapSceneRenderer.h"
#include "../../engine/objects/scenes/map.h"
#include "../../gapi/interface/sortLambda.h"

std::shared_ptr<MapRenderPlan>
MapSceneRenderer::processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) {
//    auto &scenes = frameInputParams->frameParameters->scene;

//    for (auto &scene : scenes) {
        auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);
        auto mapPlan = std::make_shared<HMapRenderPlan::element_type>();
        mapScene->makeFramePlan(*frameInputParams, mapPlan);
//    }

    return mapPlan;
}

void MapSceneRenderer::collectMeshes(const std::shared_ptr<MapRenderPlan> &renderPlan) {
    mapProduceUpdateCounter.beginMeasurement();

    //Create meshes
    auto hopaqueMeshes = std::make_shared<std::vector<HGMesh>>();
    auto htransparentMeshes = std::make_shared<std::vector<HGMesh>>();

    auto opaqueMeshes = *hopaqueMeshes;
    auto transparentMeshes = *htransparentMeshes;

    opaqueMeshes.reserve(30000);
    transparentMeshes.reserve(30000);

    const auto& cullStage = renderPlan;
    auto fdd = cullStage->frameDependentData;

    int m_viewRenderOrder = 0;

    //TODO: find a way to forward this data here
    /*
    if (m_api->getConfig()->renderSkyDom && !m_suppressDrawingSky &&
        (cullStage->viewsHolder.getExterior() || cullStage->currentWmoGroupIsExtLit)) {
        if (fdd->overrideValuesWithFinalFog) {
            if (skyMesh0x4Sky != nullptr) {
                transparentMeshes.push_back(skyMesh0x4Sky);
                skyMesh0x4Sky->setSortDistance(0);

            }
        }
        if ((m_skyConeAlpha > 0) ) {
            if (skyMesh != nullptr)
                opaqueMeshes.push_back(skyMesh);
        }
    }
     */

    // Put everything into one array and sort
    interiorViewCollectMeshCounter.beginMeasurement();
    bool renderPortals = m_config->renderPortals;
    for (auto &view : cullStage->viewsHolder.getInteriorViews()) {
        view->collectMeshes(opaqueMeshes, transparentMeshes);
        if (renderPortals) {
//            view->produceTransformedPortalMeshes(m_api, opaqueMeshes, transparentMeshes);
        }
    }
    interiorViewCollectMeshCounter.endMeasurement();

    exteriorViewCollectMeshCounter.beginMeasurement();
    {
        auto exteriorView = cullStage->viewsHolder.getExterior();
        if (exteriorView != nullptr) {
            exteriorView->collectMeshes(opaqueMeshes, transparentMeshes);
            if (renderPortals) {
//                exteriorView->produceTransformedPortalMeshes(m_api, opaqueMeshes, transparentMeshes);
            }
        }
    }
    exteriorViewCollectMeshCounter.endMeasurement();

    m2CollectMeshCounter.beginMeasurement();
    if (m_config->renderM2) {
        for (auto &m2Object : cullStage->m2Array.getDrawn()) {
            if (m2Object == nullptr) continue;
            m2Object->collectMeshes(opaqueMeshes, transparentMeshes, m_viewRenderOrder);
            m2Object->drawParticles(opaqueMeshes, transparentMeshes, m_viewRenderOrder);
        }
    }
    m2CollectMeshCounter.endMeasurement();

    //No need to sort array which has only one element
    sortMeshCounter.beginMeasurement();
    if (transparentMeshes.size() > 1) {
        tbb::parallel_sort(transparentMeshes.begin(), transparentMeshes.end(), SortMeshes);
    }
    sortMeshCounter.endMeasurement();

    mapProduceUpdateCounter.endMeasurement();


    m_config->mapProduceUpdateTime = mapProduceUpdateCounter.getTimePerFrame();
    m_config->interiorViewCollectMeshTime = interiorViewCollectMeshCounter.getTimePerFrame();
    m_config->exteriorViewCollectMeshTime = exteriorViewCollectMeshCounter.getTimePerFrame();
    m_config->m2CollectMeshTime = m2CollectMeshCounter.getTimePerFrame();
    m_config->sortMeshTime = sortMeshCounter.getTimePerFrame();
    m_config->collectBuffersTime = collectBuffersCounter.getTimePerFrame();
    m_config->sortBuffersTime = sortBuffersCounter.getTimePerFrame();
}
