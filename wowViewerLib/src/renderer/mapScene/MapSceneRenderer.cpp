//
// Created by Deamon on 11/26/2022.
//

#include "MapSceneRenderer.h"
#include "../../engine/objects/scenes/map.h"
#include "../../gapi/interface/sortLambda.h"
#include "../frame/FrameProfile.h"

std::shared_ptr<MapRenderPlan>
MapSceneRenderer::processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) {
//    for (auto &scene : scenes) {
        auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);
        auto mapPlan = std::make_shared<HMapRenderPlan::element_type>();
        mapScene->makeFramePlan(*frameInputParams, mapPlan);
//    }

    return mapPlan;
}

void MapSceneRenderer::collectMeshes(const std::shared_ptr<MapRenderPlan> &renderPlan,
                                     COpaqueMeshCollector &opaqueMeshCollector,
                                     COpaqueMeshCollector &skyOpaqueMeshCollector,
                                     const std::shared_ptr<framebased::vector<HGSortableMesh>> &htransparentMeshes,
                                     const std::shared_ptr<framebased::vector<HGSortableMesh>> &hSkyTransparentMeshes) {
    ZoneScoped;

    auto &transparentMeshes = *htransparentMeshes;

    auto &skyTransparentMeshes = *hSkyTransparentMeshes;

    transparentMeshes.reserve(30000);
    skyTransparentMeshes.reserve(1000);

    const auto& cullStage = renderPlan;
    auto fdd = cullStage->frameDependentData;

    int m_viewRenderOrder = 0;

    // Put everything into one array and sort

    bool renderPortals = m_config->renderPortals;
    bool renderADT = m_config->renderAdt;
    bool renderWMO = m_config->renderWMO;

    for (auto &view : cullStage->viewsHolder.getInteriorViews()) {
        ZoneScopedN("Collect interiors");
        view->collectMeshes(renderADT, true, renderWMO, opaqueMeshCollector, transparentMeshes);
    }

    {
        auto exteriorView = cullStage->viewsHolder.getExterior();
        if (exteriorView != nullptr) {
            ZoneScopedN("Collect Exterior");
            exteriorView->collectMeshes(renderADT, true, renderWMO, opaqueMeshCollector, transparentMeshes);

            {
                ZoneScopedN("adt mesh collect");
                for (auto &adtRes: renderPlan->adtArray) {
                    adtRes.adtObject->collectMeshes(adtRes, opaqueMeshCollector, transparentMeshes);
                }
            }
        }
    }

    if (m_config->renderM2) {
        ZoneScopedN("collect m2s");

        auto threadsAvailable = m_config->hardwareThreadCount();
        auto &m2ToDraw = cullStage->m2Array.getDrawn();
        int granSize = m2ToDraw.size() / (2 * threadsAvailable);



        if (granSize > 0) {
            std::mutex mergeMtx;
            oneapi::tbb::task_arena arena(std::min<uint32_t>(threadsAvailable, 16), 1);
            arena.execute([&] {
                tbb::static_partitioner ap;

                tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), granSize),
                                  [&](tbb::blocked_range<size_t> r) {
                                      transp_vec<HGSortableMesh> transpVec;
                                      auto lCollector = opaqueMeshCollector.clone();
                                      for (size_t i = r.begin(); i != r.end(); ++i) {
                                          auto *m2Object = m2Factory.getObjectById<0>(m2ToDraw[i]);
                                          if (m2Object != nullptr) {
                                              m2Object->collectMeshes(*lCollector, transpVec);
                                              m2Object->drawParticles(*lCollector, transpVec,
                                                                      m_viewRenderOrder);
                                          }
                                      }

                                      {
                                          std::lock_guard<std::mutex> lock(mergeMtx);
                                          opaqueMeshCollector.merge(*lCollector);
                                          transparentMeshes.insert(transparentMeshes.end(), transpVec.begin(), transpVec.end());
                                      }
                                      delete lCollector;

                                  }, ap);
            });
        } else {
            for (auto m2ObjectId : cullStage->m2Array.getDrawn()) {
                auto m2Object = m2Factory.getObjectById<0>(m2ObjectId);
                if (m2Object == nullptr) continue;
                m2Object->collectMeshes(opaqueMeshCollector, transparentMeshes);
                m2Object->drawParticles(opaqueMeshCollector, transparentMeshes, m_viewRenderOrder);
            }
        }



        auto skyBoxView = cullStage->viewsHolder.getSkybox();
        if (skyBoxView) {
            transp_vec<HGSortableMesh> skyTranspVec;

            ZoneScopedN("collect skyBox");
            for (auto &m2ObjectId : skyBoxView->m2List.getDrawn()) {
                auto m2Object = m2Factory.getObjectById<0>(m2ObjectId);

                if (m2Object == nullptr) continue;
                m2Object->collectMeshes(skyOpaqueMeshCollector, skyTranspVec);
                m2Object->drawParticles(skyOpaqueMeshCollector, skyTranspVec, m_viewRenderOrder);
            }
            skyTransparentMeshes.insert(skyTransparentMeshes.end(), skyTranspVec.begin(), skyTranspVec.end());
        }
    }

    //No need to sort array which has only one element
    if (transparentMeshes.size() > 1) {
        ZoneScopedN("sort transparent");
        std::sort(transparentMeshes.begin(), transparentMeshes.end(), SortMeshes);
    }
    if (skyTransparentMeshes.size() > 1) {
        ZoneScopedN("sky transparent");
        std::sort(skyTransparentMeshes.begin(), skyTransparentMeshes.end(), SortMeshes);
    }
}

void MapSceneRenderer::updateSceneWideChunk(const std::shared_ptr<IBufferChunkVersioned<sceneWideBlockVSPS>> &sceneWideChunk,
                                            const std::vector<RenderingMatAndSceneSize> &renderingMatricesAndSizes,
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

    if (fdd->fogResults.empty()) fdd->fogResults.emplace_back();

    for (int i = 0; i < renderingMatricesAndSizes.size(); i++) {
        auto const &matAndSceneSize = renderingMatricesAndSizes[i];
        auto const &renderingMatrices = matAndSceneSize.renderingMat;

        auto &blockPSVS = sceneWideChunk->getObject(i);
        blockPSVS.uLookAtMat = renderingMatrices->lookAtMat;
        if (isVulkan) {
            blockPSVS.uPMatrix = vulkanMatrixFix * renderingMatrices->perspectiveMat;
        } else {
            blockPSVS.uPMatrix = renderingMatrices->perspectiveMat;
        }
        blockPSVS.uInteriorSunDir = mathfu::vec4_packed(
            mathfu::vec4(renderingMatrices->interiorDirectLightDir.xyz(), 0)
        );
        blockPSVS.uSceneSize_DisableLightBuffer = mathfu::vec4_packed(mathfu::vec4(
            (float)matAndSceneSize.width,
            (float)matAndSceneSize.height,
            !m_config->enableLightBuffer ? 1.0 : 0.0,
            0.0
        ));

        blockPSVS.uViewUpSceneTime = mathfu::vec4(renderingMatrices->viewUp.xyz(), sceneTime);

        blockPSVS.closeOceanColor = fdd->liquidColors.closeOceanColor_shallowAlpha;
        blockPSVS.farOceanColor = fdd->liquidColors.farOceanColor_deepAlpha;
        blockPSVS.closeRiverColor = fdd->liquidColors.closeRiverColor_shallowAlpha;
        blockPSVS.farRiverColor = fdd->liquidColors.farRiverColor_deepAlpha;

        blockPSVS.extLight.uExteriorAmbientColor = fdd->colors.exteriorAmbientColor;
        blockPSVS.extLight.uExteriorHorizontAmbientColor = fdd->colors.exteriorHorizontAmbientColor;
        blockPSVS.extLight.uExteriorGroundAmbientColor = fdd->colors.exteriorGroundAmbientColor;
        blockPSVS.extLight.uExteriorDirectColor = fdd->colors.exteriorDirectColor;
        blockPSVS.extLight.uExteriorDirectColorDir = mathfu::vec4(fdd->exteriorDirectColorDir, 1.0);
        blockPSVS.extLight.uAdtSpecMult_FogCount = mathfu::vec4(m_config->adtSpecMult, fdd->fogResults.size(), 0, 1.0);

        for (int i = 0; i < std::min<int>(fdd->fogResults.size(), FOG_MAX_SHADER_COUNT); i++) {
            auto &fogResult = fdd->fogResults[i];

            auto &fogData = blockPSVS.fogData;

            mathfu::vec4 heightPlane = mathfu::vec4(
                zUp.xyz(),
                -(mathfu::vec3::DotProduct(zUp.xyz(), (zUp * fogResult.FogHeight).xyz()))
            );
            heightPlane = renderingMatrices->invTranspViewMat * heightPlane;

            float fogEnd = std::min<float>(std::max<float>(m_config->farPlane, 277.5), m_config->farPlane);
            if (m_config->disableFog || !fdd->FogDataFound) {
                fogEnd = 100000000.0f;
                fogResult.FogScaler = 0;
                fogResult.FogDensity = 0;
                fogResult.FogHeightDensity = 0;
            }

            const float densityMultFix = 0.00050000002 * std::pow(10, m_config->fogDensityIncreaser);
            float fogScaler = fogResult.FogScaler;
            if (fogScaler <= 0.00000001f) fogScaler = 0.5f;
            float fogStart = std::min<float>(m_config->farPlane, 3000) * fogScaler;

            fogData.densityParams = mathfu::vec4(
                fogStart,
                fogEnd,
                fogResult.FogDensity * densityMultFix,
                0);
            fogData.classicFogParams = mathfu::vec4(0, 0, 0, 0);
            fogData.heightPlane = heightPlane;
            fogData.color_and_heightRate = mathfu::vec4(fogResult.FogColor, fogResult.FogHeightScaler
    //            * 0.01
            );
            fogData.heightDensity_and_endColor = mathfu::vec4(
                fogResult.FogHeightDensity * densityMultFix,
                fogResult.EndFogColor.x,
                fogResult.EndFogColor.y,
                fogResult.EndFogColor.z
            );

            fogData.sunAngle_and_sunColor = mathfu::vec4(
                fogResult.SunFogAngle,
                fogResult.SunFogColor.x,
                fogResult.SunFogColor.y,
                fogResult.SunFogColor.z
            );
            fogData.heightColor_and_endFogDistance = mathfu::vec4(
                fogResult.FogHeightColor,
                (fogResult.EndFogColorDistance > 0) ?
                fogResult.EndFogColorDistance :
                1000.0f
            );
            fogData.sunPercentage = mathfu::vec4(
                fogResult.SunAngleBlend * fogResult.SunFogStrength,
                0, 1.0, 1.0);
            fogData.sunDirection_and_fogZScalar = mathfu::vec4(
                fdd->sunDirection,
                fogResult.FogZScalar
            );
            fogData.heightFogCoeff = fogResult.FogHeightCoefficients;
            fogData.mainFogCoeff = fogResult.MainFogCoefficients;
            fogData.heightDensityFogCoeff = fogResult.HeightDensityFogCoefficients;

            bool mainFogOk = (fogResult.MainFogStartDist + 0.001 <= fogResult.MainFogEndDist);
            fogData.mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha = mathfu::vec4(
                mainFogOk ? fogResult.MainFogEndDist : fogResult.MainFogStartDist + 0.001,
                fogResult.MainFogStartDist >= 0.0 ? fogResult.MainFogStartDist : 0.0f,
                fogResult.LegacyFogScalar,
                1.0f//fogResult.FogBlendAlpha
            );
            fogData.heightFogEndColor_fogStartOffset = mathfu::vec4(
                fogResult.HeightEndFogColor,
                fogResult.FogStartOffset
            );
        }
        sceneWideChunk->saveVersion(i);
    }
}
