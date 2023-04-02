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

void MapSceneRenderer::collectMeshes(const std::shared_ptr<MapRenderPlan> &renderPlan,
                                     const std::shared_ptr<std::vector<HGMesh>> &hopaqueMeshes,
                                     const std::shared_ptr<std::vector<HGSortableMesh>> &htransparentMeshes,
                                     const std::shared_ptr<std::vector<HGMesh>> &hSkyOpaqueMeshes,
                                     const std::shared_ptr<std::vector<HGSortableMesh>> &hSkyTransparentMeshes) {
    mapProduceUpdateCounter.beginMeasurement();

    auto &opaqueMeshes = *hopaqueMeshes;
    auto &transparentMeshes = *htransparentMeshes;

    auto &skyOpaqueMeshes = *hSkyOpaqueMeshes;
    auto &skyTransparentMeshes = *hSkyTransparentMeshes;

    opaqueMeshes.reserve(30000);
    transparentMeshes.reserve(30000);

    skyOpaqueMeshes.reserve(1000);
    skyTransparentMeshes.reserve(1000);

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

        auto skyBoxView = cullStage->viewsHolder.getSkybox();
        if (skyBoxView) {
            for (auto &m2Object : skyBoxView->m2List.getDrawn()) {
                if (m2Object == nullptr) continue;
                m2Object->collectMeshes(skyOpaqueMeshes, skyTransparentMeshes, m_viewRenderOrder);
                m2Object->drawParticles(skyOpaqueMeshes, skyTransparentMeshes, m_viewRenderOrder);
            }
        }
    }
    m2CollectMeshCounter.endMeasurement();

    //No need to sort array which has only one element
    sortMeshCounter.beginMeasurement();
    if (transparentMeshes.size() > 1) {
        tbb::parallel_sort(transparentMeshes.begin(), transparentMeshes.end(), SortMeshes);
    }
    if (skyTransparentMeshes.size() > 1) {
        tbb::parallel_sort(skyTransparentMeshes.begin(), skyTransparentMeshes.end(), SortMeshes);
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

void MapSceneRenderer::updateSceneWideChunk(const std::shared_ptr<IBufferChunk<sceneWideBlockVSPS>> &sceneWideChunk,
                                            const HCameraMatrices &renderingMatrices,
                                            const HFrameDependantData &fdd,
                                            bool isVulkan
                                            ) {

    static const mathfu::mat4 vulkanMatrixFix = mathfu::mat4(1, 0, 0, 0,
                                                              0, -1, 0, 0,
                                                              0, 0, 1.0/2.0, 1/2.0,
                                                              0, 0, 0, 1).Transpose();


    auto &blockPSVS = sceneWideChunk->getObject();
    blockPSVS.uLookAtMat = renderingMatrices->lookAtMat;
    if (isVulkan) {
        blockPSVS.uPMatrix = vulkanMatrixFix*renderingMatrices->perspectiveMat;
    }
    blockPSVS.uInteriorSunDir = renderingMatrices->interiorDirectLightDir;
    blockPSVS.uViewUp = renderingMatrices->viewUp;

    blockPSVS.extLight.uExteriorAmbientColor = fdd->exteriorAmbientColor;
    blockPSVS.extLight.uExteriorHorizontAmbientColor = fdd->exteriorHorizontAmbientColor;
    blockPSVS.extLight.uExteriorGroundAmbientColor = fdd->exteriorGroundAmbientColor;
    blockPSVS.extLight.uExteriorDirectColor = fdd->exteriorDirectColor;
    blockPSVS.extLight.uExteriorDirectColorDir = mathfu::vec4(fdd->exteriorDirectColorDir, 1.0);
    blockPSVS.extLight.uAdtSpecMult = mathfu::vec4(m_config->adtSpecMult, 0, 0, 1.0);

//        float fogEnd = std::min(config->getFarPlane(), config->getFogEnd());
    float fogEnd = m_config->farPlane;
    if (m_config->disableFog || !fdd->FogDataFound) {
        fogEnd = 100000000.0f;
        fdd->FogScaler = 0;
        fdd->FogDensity = 0;
    }

    float fogStart = std::max<float>(m_config->farPlane - 250, 0);
    fogStart = std::max<float>(fogEnd - fdd->FogScaler * (fogEnd - fogStart), 0);


    blockPSVS.fogData.densityParams = mathfu::vec4(
        fogStart,
        fogEnd,
        fdd->FogDensity / 1000,
        0);
    blockPSVS.fogData.heightPlane = mathfu::vec4(0, 0, 0, 0);
    blockPSVS.fogData.color_and_heightRate = mathfu::vec4(fdd->FogColor, fdd->FogHeightScaler);
    blockPSVS.fogData.heightDensity_and_endColor = mathfu::vec4(
        fdd->FogHeightDensity,
        fdd->EndFogColor.x,
        fdd->EndFogColor.y,
        fdd->EndFogColor.z
    );
    blockPSVS.fogData.sunAngle_and_sunColor = mathfu::vec4(
        fdd->SunFogAngle,
        fdd->SunFogColor.x * fdd->SunFogStrength,
        fdd->SunFogColor.y * fdd->SunFogStrength,
        fdd->SunFogColor.z * fdd->SunFogStrength
    );
    blockPSVS.fogData.heightColor_and_endFogDistance = mathfu::vec4(
        fdd->FogHeightColor,
        (fdd->EndFogColorDistance > 0) ?
        fdd->EndFogColorDistance :
        1000.0f
    );
    blockPSVS.fogData.sunPercentage = mathfu::vec4(
        (fdd->SunFogColor.Length() > 0) ? 0.5f : 0.0f, 0, 0, 0);

    sceneWideChunk->save();
}
