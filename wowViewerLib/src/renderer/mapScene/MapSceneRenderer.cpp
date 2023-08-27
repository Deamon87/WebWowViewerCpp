//
// Created by Deamon on 11/26/2022.
//

#include "MapSceneRenderer.h"
#include "../../engine/objects/scenes/map.h"
#include "../../gapi/interface/sortLambda.h"
#include "../frame/FrameProfile.h"

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
    ZoneScoped;
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

    // Put everything into one array and sort
    interiorViewCollectMeshCounter.beginMeasurement();
    bool renderPortals = m_config->renderPortals;
    bool renderADT = m_config->renderAdt;
    bool renderWMO = m_config->renderWMO;

    for (auto &view : cullStage->viewsHolder.getInteriorViews()) {
        view->collectMeshes(renderADT, true, renderWMO, opaqueMeshes, transparentMeshes);
    }
    interiorViewCollectMeshCounter.endMeasurement();

    exteriorViewCollectMeshCounter.beginMeasurement();
    {
        auto exteriorView = cullStage->viewsHolder.getExterior();
        if (exteriorView != nullptr) {
            exteriorView->collectMeshes(renderADT, true, renderWMO, opaqueMeshes, transparentMeshes);
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
        std::sort(transparentMeshes.begin(), transparentMeshes.end(), SortMeshes);
    }
    if (skyTransparentMeshes.size() > 1) {
        std::sort(skyTransparentMeshes.begin(), skyTransparentMeshes.end(), SortMeshes);
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

void MapSceneRenderer::updateSceneWideChunk(const std::shared_ptr<IBufferChunkVersioned<sceneWideBlockVSPS>> &sceneWideChunk,
                                            const HCameraMatrices &renderingMatrices,
                                            const HFrameDependantData &fdd,
                                            bool isVulkan,
                                            animTime_t sceneTime
                                            ) {
    ZoneScoped;

    static const mathfu::mat4 vulkanMatrixFix = mathfu::mat4(1, 0, 0, 0,
                                                              0, -1, 0, 0,
                                                              0, 0, 1.0/2.0, 1/2.0,
                                                              0, 0, 0, 1).Transpose();

    const static mathfu::vec4 zUp = {0,0,1.0,0};

    mathfu::vec3 zUpPointInView =
        (renderingMatrices->lookAtMat * mathfu::vec4((zUp * fdd->FogHeight).xyz(), 1.0f)).xyz();
    mathfu::vec4 heightPlaneVec =
        renderingMatrices->invTranspViewMat * mathfu::vec4((zUp * fdd->FogHeight).xyz(), 0.0);
    mathfu::vec3 heightPlaneVecNorm = heightPlaneVec.xyz().LengthSquared() > 0 ?
        heightPlaneVec.Normalized().xyz() :
        mathfu::vec3(0,0,0);
    mathfu::vec4 heightPlane = mathfu::vec4(
        heightPlaneVecNorm,
        -(mathfu::vec3::DotProduct(heightPlaneVecNorm, zUpPointInView))
    );

    auto &blockPSVS = sceneWideChunk->getObject(0);
    blockPSVS.uLookAtMat = renderingMatrices->lookAtMat;
    if (isVulkan) {
        blockPSVS.uPMatrix = vulkanMatrixFix*renderingMatrices->perspectiveMat;
    } else {
        blockPSVS.uPMatrix = renderingMatrices->perspectiveMat;
    }
    blockPSVS.uInteriorSunDir = renderingMatrices->interiorDirectLightDir;
    blockPSVS.uViewUpSceneTime = mathfu::vec4(renderingMatrices->viewUp.xyz(), sceneTime);

    blockPSVS.extLight.uExteriorAmbientColor = fdd->exteriorAmbientColor;
    blockPSVS.extLight.uExteriorHorizontAmbientColor = fdd->exteriorHorizontAmbientColor;
    blockPSVS.extLight.uExteriorGroundAmbientColor = fdd->exteriorGroundAmbientColor;
    blockPSVS.extLight.uExteriorDirectColor = fdd->exteriorDirectColor;
    blockPSVS.extLight.uExteriorDirectColorDir = mathfu::vec4(fdd->exteriorDirectColorDir, 1.0);
    blockPSVS.extLight.uAdtSpecMult = mathfu::vec4(m_config->adtSpecMult, 0, 0, 1.0);

    float fogEnd = std::min<float>(std::max<float>(m_config->farPlane, 277.5), m_config->farPlane);
    if (m_config->disableFog || !fdd->FogDataFound) {
        fogEnd = 100000000.0f;
        fdd->FogScaler = 0;
        fdd->FogDensity = 0;
    }

    const float densityMultFix = 1.0/1000; ;
    float fogScaler = fdd->FogScaler;
    if (fogScaler <= 0.00000001f) fogScaler = 0.5f;
    float fogStart = std::min<float>(m_config->farPlane, 3000) * fogScaler;

    blockPSVS.fogData.densityParams = mathfu::vec4(
        fogStart,
        fogEnd,
        fdd->FogDensity * densityMultFix,
        0);
    blockPSVS.fogData.classicFogParams = mathfu::vec4(0, 0, 0, 0);
    blockPSVS.fogData.heightPlane = heightPlane;
    blockPSVS.fogData.color_and_heightRate = mathfu::vec4(fdd->FogColor, fdd->FogHeightScaler);
    blockPSVS.fogData.heightDensity_and_endColor = mathfu::vec4(
        fdd->FogHeightDensity * densityMultFix,
        fdd->EndFogColor.x,
        fdd->EndFogColor.y,
        fdd->EndFogColor.z
    );
    blockPSVS.fogData.sunAngle_and_sunColor = mathfu::vec4(
        fdd->SunFogAngle,
        fdd->SunFogColor.x,
        fdd->SunFogColor.y,
        fdd->SunFogColor.z
    );
    blockPSVS.fogData.heightColor_and_endFogDistance = mathfu::vec4(
        fdd->FogHeightColor,
        (fdd->EndFogColorDistance > 0) ?
            fdd->EndFogColorDistance :
            1000.0f
    );
    blockPSVS.fogData.sunPercentage = mathfu::vec4(
        0.0f, //fdd->SunFogAngle * fdd->SunFogStrength,
        0, 1.0, 1.0);
    blockPSVS.fogData.sunDirection_and_fogZScalar = mathfu::vec4(
        fdd->exteriorDirectColorDir, //TODO: for fog this is calculated from SUN position
        fdd->FogZScalar
    );
    blockPSVS.fogData.heightFogCoeff = fdd->FogHeightCoefficients;
    blockPSVS.fogData.mainFogCoeff = fdd->MainFogCoefficients;
    blockPSVS.fogData.heightDensityFogCoeff = fdd->HeightDensityFogCoefficients;

    bool mainFogOk = (fdd->MainFogStartDist + 0.001 <= fdd->MainFogEndDist);
    blockPSVS.fogData.mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha = mathfu::vec4(
        mainFogOk ? fdd->MainFogEndDist : fdd->MainFogStartDist + 0.001,
        fdd->MainFogStartDist >= 0.0 ? fdd->MainFogStartDist : 0.0f,
        fdd->LegacyFogScalar,
        fdd->FogBlendAlpha
    );
    blockPSVS.fogData.heightFogEndColor_fogStartOffset = mathfu::vec4(
        fdd->HeightEndFogColor,
        fdd->FogStartOffset
    );
    sceneWideChunk->saveVersion(0);
}
