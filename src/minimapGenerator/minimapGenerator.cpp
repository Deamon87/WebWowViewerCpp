//
// Created by Deamon on 9/8/2020.
//
//#include "../../wowViewerLib/src/engine/SceneScenario.h"
//#include <filesystem>
#include "../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include "minimapGenerator.h"
#include "../../wowViewerLib/src/engine/algorithms/mathHelper.h"
#include "../../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../../wowViewerLib/src/persistence/RequestProcessor.h"
#include "../screenshots/screenshotMaker.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonOrthoStaticCamera.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonOrthoStaticTopDownCamera.h"


MinimapGenerator::MinimapGenerator(HWoWFilesCacheStorage cacheStorage, const HGDevice &hDevice,
                                   HRequestProcessor processor, std::shared_ptr<IClientDatabase> dbhandler) {
    m_apiContainer = std::make_shared<ApiContainer>();

    m_apiContainer->hDevice = hDevice;
    m_apiContainer->cacheStorage = cacheStorage;
    //TODO:
//    m_apiContainer->camera = std::make_shared<FirstPersonOrthoStaticCamera>();
    m_apiContainer->databaseHandler = dbhandler;

    m_processor = processor;

    auto config = m_apiContainer->getConfig();
    config->glowSource = EParameterSource::eConfig;
    config->globalFog = EParameterSource::eConfig;
    config->skyParams = EParameterSource::eConfig;
    config->globalLighting = EParameterSource::eConfig;
    config->waterColorParams = EParameterSource::eConfig;
    config->useMinimapWaterColor = false;
    config->useCloseRiverColorForDB = true;
//    config->adtTTLWithoutUpdate = 500; //0.5 sec
    config->adtFreeStrategy = EFreeStrategy::eFrameBase; //

    config->exteriorColors.exteriorAmbientColor =         mathfu::vec3(0.5f,0.5f,0.5f);
    config->exteriorColors.exteriorHorizontAmbientColor = mathfu::vec3(0.5f,0.5f,0.5f);
    config->exteriorColors.exteriorGroundAmbientColor =   mathfu::vec3(0.5f,0.5f,0.5f);
    config->exteriorColors.exteriorDirectColor =          mathfu::vec3(0.5f,0.5f,0.5f);

    config->adtSpecMult = 0.0;

    config->disableFog = true;
    config->renderSkyDom = false;
    config->currentGlow = 0;

    config->farPlane = 10000;
    config->farPlaneForCulling = 10000;
}
void MinimapGenerator::startScenarios(std::vector<ScenarioDef> &scenarioList) {
    m_mgMode = EMGMode::eScreenshotGeneration;

    setZoom(1.0f);

    scenarioListToProcess = {};
    for (auto &scenario : scenarioList) {
//        auto boundingBoxHolder = scenario.boundingBoxHolder;
//        if (boundingBoxHolder == nullptr && scenario.doBoundingBoxPreCalc) {
//            scenario.boundingBoxHolder = boundingBoxHolder;
//        }

        scenarioListToProcess.push_back(scenario);

//        if (scenario.doBoundingBoxPreCalc) {
//            auto scenarionCopy = scenario;
//            scenarionCopy.mode = EMGMode::eBoundingBoxCalculation;
//            scenarionCopy.orientation = ScenarioOrientation::soTopDownOrtho;
//
//            scenarionCopy.minWowWorldCoord = mathfu::vec2(-MathHelper::TILESIZE * 32, -MathHelper::TILESIZE * 32);
//            scenarionCopy.maxWowWorldCoord = mathfu::vec2(MathHelper::TILESIZE * 32, MathHelper::TILESIZE * 32);
//
//            scenarionCopy.doBoundingBoxPreCalc = false;
//            scenarionCopy.zoom = 1.0f;
//            scenarionCopy.imageWidth = 512;
//            scenarionCopy.imageHeight = 512;
//            scenarionCopy.boundingBoxHolder = boundingBoxHolder;
//
//            scenarioListToProcess.push_back(scenarionCopy);
//        }
    }

    startNextScenario();
}

void MinimapGenerator::startPreview(ScenarioDef &scenarioDef) {
    m_mgMode = EMGMode::ePreview;
    currentScenario = scenarioDef;

    setupScenarioData();
}
void MinimapGenerator::stopPreview() {
    m_mgMode = EMGMode::eNone;
}
void MinimapGenerator::startBoundingBoxCalc(ScenarioDef &scenarioDef) {
    m_mgMode = EMGMode::eBoundingBoxCalculation;
    currentScenario = scenarioDef;

    currentScenario.orientation = ScenarioOrientation::soTopDownOrtho;
//    currentScenario.minWowWorldCoord = mathfu::vec2(-MathHelper::TILESIZE * 32, -MathHelper::TILESIZE * 32);
    currentScenario.minWowWorldCoord = mathfu::vec2(-MathHelper::TILESIZE * 32, -MathHelper::TILESIZE * 32);
    currentScenario.maxWowWorldCoord = mathfu::vec2(MathHelper::TILESIZE * 32, MathHelper::TILESIZE * 32);
    currentScenario.zoom = 1.0f;

    setupScenarioData();
}
void MinimapGenerator::stopBoundingBoxCalc() {
    m_mgMode = EMGMode::eNone;
}


void MinimapGenerator::startNextScenario() {
    if (scenarioListToProcess.size() == 0) {
        m_mgMode = EMGMode::eNone;
        return;
    }

    currentScenario = scenarioListToProcess.back();

    scenarioListToProcess.pop_back();

    setupScenarioData();
}

bool MinimapGenerator::loadMaps() {
    mapRuntimeInfo.clear();

    for ( int i = 0; i < currentScenario.maps.size(); i++) {
        auto &mapDef = currentScenario.maps[i];
        MapRecord mapRecord;
        if (!m_apiContainer->databaseHandler->getMapById(mapDef.mapId, mapRecord)) {
            std::cout << "Couldnt get data for mapId " << mapDef.mapId << std::endl;
            startNextScenario();
            return false;
        }

        auto &mapRuntime = mapRuntimeInfo.emplace_back();
        mapRuntime.mapIndex = i;
        mapRuntime.scene = std::make_shared<Map>(m_apiContainer, mapRecord.ID, mapRecord.WdtFileID);

        if (m_mgMode == EMGMode::eScreenshotGeneration) {
            mapRuntime.scene->setAdtConfig(mapDef.adtConfigHolder);
        }
    }
    setMinMaxXYWidthHeight(currentScenario.minWowWorldCoord, currentScenario.maxWowWorldCoord);

    for (auto &mapData : mapRuntimeInfo) {
        mapData.mandatoryADTMap.resize(0);
    }
    //Assign mandatory adt for every cell on the screen
    if (m_mgMode == EMGMode::eScreenshotGeneration ) {
        //currentScenario.boundingBoxHolder
        for (auto &mapData : mapRuntimeInfo) {
            mapData.mandatoryADTMap.resize(m_chunkWidth);
            for (int i = 0; i < m_chunkWidth; i++) {
                mapData.mandatoryADTMap[i].resize(m_chunkHeight);
            }

            for (int adt_x = 0; adt_x < 64; adt_x++) {
                for (int adt_y = 0; adt_y < 64; adt_y++) {
                    mathfu::vec3 adtMin = currentScenario.maps[mapData.mapIndex].adtConfigHolder->adtMin[adt_x][adt_y];
                    mathfu::vec3 adtMax = currentScenario.maps[mapData.mapIndex].adtConfigHolder->adtMax[adt_x][adt_y];
                    //Project aaBB into screenSpace coordinates

                    std::array<float, 2> minMaxX = {AdtIndexToWorldCoordinate(adt_y + 1), AdtIndexToWorldCoordinate(adt_x + 1)};
                    std::array<float, 2> minMaxY = {AdtIndexToWorldCoordinate(adt_y) , AdtIndexToWorldCoordinate(adt_x)};

//                    std::array<float, 2> minMaxX = {adtMin.x, adtMax.x};
//                    std::array<float, 2> minMaxY = {adtMin.y, adtMax.y};
                    std::array<float, 2> minMaxZ = {adtMin.z, adtMax.z};
                    std::vector<mathfu::vec4> corners;

                    if (minMaxZ[0] > minMaxZ[1]) continue;

                    minMaxX[0] = std::max<double>(minMaxX[0] - (MathHelper::TILESIZE/2.0f), adtMin.x );
                    minMaxX[1] = std::min<double>(minMaxX[1] + (MathHelper::TILESIZE/2.0f), adtMax.x );
                    minMaxY[0] = std::max<double>(minMaxY[0] - (MathHelper::TILESIZE/2.0f), adtMin.y );
                    minMaxY[1] = std::min<double>(minMaxY[1] + (MathHelper::TILESIZE/2.0f), adtMax.y );


                    for (int i = 0; i < 2; i++) {
                        for (int j = 0; j < 2; j++) {
                            for (int k = 0; k < 2; k++) {
                                corners.push_back(
                                    mathfu::vec4(
                                        minMaxX[i],
                                        minMaxY[j],
                                        minMaxZ[k],
                                        1.0f
                                    )
                                );
                            }
                        }
                    }

                    mathfu::mat4 viewProj = genTempProjectMatrix();
                    for (auto & corner : corners) {
                        corner = viewProj * corner;
                        corner = corner * (1.0f / corner.w);
                    }

                    std::array<float, 2> minMaxScreenX = {20000, -20000};
                    std::array<float, 2> minMaxScreenY = {20000, -20000};
                    std::array<float, 2> minMaxScreenZ = {20000, -20000};

                    for (auto & corner : corners) {
                        minMaxScreenX[0] = std::min<float>(corner.x, minMaxScreenX[0]);
                        minMaxScreenX[1] = std::max<float>(corner.x, minMaxScreenX[1]);

                        minMaxScreenY[0] = std::min<float>(corner.y, minMaxScreenY[0]);
                        minMaxScreenY[1] = std::max<float>(corner.y, minMaxScreenY[1]);

                        minMaxScreenZ[0] = std::min<float>(corner.z, minMaxScreenZ[0]);
                        minMaxScreenZ[1] = std::max<float>(corner.z, minMaxScreenZ[1]);
                    }

                    //Add adt to all occupied cells
                    for (int i = std::floor(minMaxScreenX[0]);
                         i < std::ceil((minMaxScreenX[1] - minMaxScreenX[0]) / 2.0f); i++) {
                        for (int j = std::floor(minMaxScreenY[0]);
                             j < std::ceil((minMaxScreenY[1] - minMaxScreenY[0]) / 2.0f); j++) {
                            if (i < m_chunkStartX)
                                continue;
                            if (j < m_chunkStartY)
                                continue;
                            if (i >= m_chunkStartX + m_chunkWidth)
                                continue;
                            if (j >= m_chunkStartY + m_chunkHeight)
                                continue;

                            std::array<uint8_t, 2> adtCoord = {static_cast<unsigned char>(adt_x),
                                                               static_cast<unsigned char>(adt_y)};
                            mapData.mandatoryADTMap[i - m_chunkStartX][j - m_chunkStartY].push_back(adtCoord);
                        }
                    }
                }
            }
        }
    }

    return true;
}

void MinimapGenerator::setupScenarioData() {
    m_mapIndex = 0;
    m_zoom = currentScenario.zoom;


    auto config = m_apiContainer->getConfig();

    config->currentTime = currentScenario.time;

    m_width = currentScenario.imageWidth;
    m_height = currentScenario.imageHeight;


    //TODO:
    if (currentScenario.orientation == ScenarioOrientation::soTopDownOrtho) {
//        m_apiContainer->camera = std::make_shared<FirstPersonOrthoStaticTopDownCamera>();
    } else {
//        m_apiContainer->camera = std::make_shared<FirstPersonOrthoStaticCamera>();
    }

    if (!loadMaps())
        return;

    m_x = 0;
    //To make AABB work for sure!
    m_y = m_chunkHeight - 1;

    prepearCandidate = false;
    setupCameraData();
}


void
MinimapGenerator::setMinMaxXYWidthHeight(const mathfu::vec2 &minWowWorldCoord, const mathfu::vec2 &maxWowWorldCoord) {
    calcXtoYCoef();

    bool useZCoord = false;
    mathfu::mat4 viewProj = genTempProjectMatrix();

    std::array<float, 2> minMaxScreenX = {20000, -20000};
    std::array<float, 2> minMaxScreenY = {20000, -20000};

    if (m_mgMode != EMGMode::eBoundingBoxCalculation) {
        std::cout << "Using bounding box data to calc minMax Z" << std::endl;
        useZCoord = true;

        for (auto &mapData : mapRuntimeInfo) {
            auto &mapDef = currentScenario.maps[mapData.mapIndex];

            for (int adt_y = worldCoordinateToAdtIndex(maxWowWorldCoord.x + mapDef.deltaX);
                 adt_y <= worldCoordinateToAdtIndex(minWowWorldCoord.x+ mapDef.deltaX); adt_y++) {

                if (adt_y < 0 || adt_y > 63) continue;

                for (int adt_x = worldCoordinateToAdtIndex(maxWowWorldCoord.y + mapDef.deltaY);
                    adt_x <= worldCoordinateToAdtIndex(minWowWorldCoord.y + mapDef.deltaY); adt_x++) {

                    if (adt_x < 0 || adt_x > 63) continue;

                    auto adtMin = mapDef.adtConfigHolder->adtMin[adt_x][adt_y] + mathfu::vec3(mapDef.deltaX, mapDef.deltaY, mapDef.deltaZ);
                    auto adtMax = mapDef.adtConfigHolder->adtMin[adt_x][adt_y] + mathfu::vec3(mapDef.deltaX, mapDef.deltaY, mapDef.deltaZ);;

                    if (adtMin.z > adtMax.z)
                        continue;

                    std::array<double, 2> minMaxX = {AdtIndexToWorldCoordinate(adt_y + 1) + mapDef.deltaX, AdtIndexToWorldCoordinate(adt_y) + mapDef.deltaX};
                    std::array<double, 2> minMaxY = {AdtIndexToWorldCoordinate(adt_x + 1) + mapDef.deltaY, AdtIndexToWorldCoordinate(adt_x) + mapDef.deltaY};

                    minMaxX[0] = std::max<double>(minMaxX[0] - (MathHelper::TILESIZE/2.0f), adtMin.x);
                    minMaxX[1] = std::min<double>(minMaxX[1] + (MathHelper::TILESIZE/2.0f), adtMax.x);

                    minMaxY[0] = std::max<double>(minMaxY[0] - (MathHelper::TILESIZE/2.0f), adtMin.y);
                    minMaxY[1] = std::min<double>(minMaxY[1] + (MathHelper::TILESIZE/2.0f), adtMax.y);

                    std::array<double, 2> minMaxZ = {adtMin.z + mapDef.deltaZ, adtMax.z + mapDef.deltaZ};

                    std::vector<mathfu::vec4> corners;
                    for (int i = 0; i < 2; i++)
                    for (int j = 0; j < 2; j++)
                    for (int k = 0; k < 2; k++) {
                        corners.push_back(
                            mathfu::vec4(
                                minMaxX[i],
                                minMaxY[j],
                                minMaxZ[k],
                                1.0f
                            )
                        );
                    }

                    for (int i = 0; i < corners.size(); i++) {
                        corners[i] = viewProj * corners[i];
                        corners[i] = corners[i] * (1.0f / corners[i].w);
                    }

                    for (int i = 0; i < corners.size(); i++) {
                        minMaxScreenX[0] = std::min<float>(corners[i].x, minMaxScreenX[0]);
                        minMaxScreenX[1] = std::max<float>(corners[i].x, minMaxScreenX[1]);

                        minMaxScreenY[0] = std::min<float>(corners[i].y, minMaxScreenY[0]);
                        minMaxScreenY[1] = std::max<float>(corners[i].y, minMaxScreenY[1]);
                    }
                }
            }
        }
    } else {
        minMaxScreenX = {-64, 64};
        minMaxScreenY = {-64, 64};
    }

    if (minMaxScreenX[0] > minMaxScreenX[1]) {
        std::cout << "apparently BB were not calculated. exiting" << std::endl;
        m_chunkStartX = 0;
        m_chunkStartY = 0;
        m_chunkWidth  = 0;
        m_chunkHeight  = 0;
    }

    mathfu::vec2 minOrthoMapCoord = mathfu::vec2(
        minMaxScreenX[0],
        minMaxScreenY[0]
    );
    mathfu::vec2 maxOrthoMapCoord = mathfu::vec2(
        minMaxScreenX[1],
        minMaxScreenY[1]
    );

    std::cout <<"Orient = " << (int)currentScenario.orientation << std::endl;

    m_chunkStartX = std::floor((minOrthoMapCoord.x) / 2.0f) ;
    m_chunkStartY = std::floor((minOrthoMapCoord.y) / 2.0f);
    m_chunkWidth = std::ceil(((maxOrthoMapCoord.x - minOrthoMapCoord.x) / 2.0f)) ;
    m_chunkHeight = std::ceil(((maxOrthoMapCoord.y - minOrthoMapCoord.y)/ 2.0f));

    std::cout << "m_chunkStartX = " << m_chunkStartX << " m_chunkWidth = " << m_chunkWidth
              << " m_chunkStartY = " << m_chunkStartY << " m_chunkHeight = " << m_chunkHeight << std::endl;
}

mathfu::mat4 MinimapGenerator::genTempProjectMatrix() {
    auto orthoProjection = getOrthoMatrix();

    mathfu::vec3 lookAtPoint = mathfu::vec3(0, 0, 0);

    std::shared_ptr<ICamera> tempCamera;
    if (currentScenario.orientation == ScenarioOrientation::soTopDownOrtho) {
        tempCamera = std::make_shared<FirstPersonOrthoStaticTopDownCamera>();
    } else {
        tempCamera = std::make_shared<FirstPersonOrthoStaticCamera>();
    }

    setupCamera(lookAtPoint, tempCamera);

    tempCamera->tick(0);

    float nearPlane = 1.0;
    float fov = toRadian(45.0);

    float canvasAspect = (float) m_width / (float) m_height;
    float farPlaneRendering = m_apiContainer->getConfig()->farPlane;

    HCameraMatrices cameraMatricesRendering = tempCamera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
    auto viewProj = orthoProjection * cameraMatricesRendering->lookAtMat;
    return viewProj;
}

void MinimapGenerator::calcXtoYCoef() {

    //Code that tries to calc the step on y and x
    {
        auto orthoProjection = getOrthoMatrix();

        //
        const mathfu::vec4 vec4Top = {1, 1, 1, 1};
        const mathfu::vec4 vec4TopBottom = {1, -1, 1, 1};
        const mathfu::vec4 vec4Bottom = {-1, -1, 1, 1};
        const mathfu::vec4 vec4BottomTop = {-1, 1, 1, 1};


        //Round1
        {
            auto viewProj = genTempProjectMatrix();

            mathfu::vec4 vec4TopTrans = viewProj.Inverse() * vec4Top;
            vec4TopTrans *= (1.0f / vec4TopTrans.w);

            mathfu::vec4 vec4TopBottomTrans = viewProj.Inverse() * vec4TopBottom;
            vec4TopBottomTrans *= (1.0f / vec4TopBottomTrans.w);

            mathfu::vec4 vec4BotTrans = viewProj.Inverse() * vec4Bottom;
            vec4BotTrans *= (1.0f / vec4BotTrans.w);

            mathfu::vec4 vec4BottomTopTrans = viewProj.Inverse() * vec4BottomTop;
            vec4BottomTopTrans *= (1.0f / vec4BottomTopTrans.w);


            float minX = std::min<float>(std::min<float>(vec4TopTrans.x, vec4TopBottomTrans.x),
                                         std::min<float>(vec4BotTrans.x, vec4BottomTopTrans.x));
            float minY = std::min<float>(std::min<float>(vec4TopTrans.y, vec4TopBottomTrans.y),
                                         std::min<float>(vec4BotTrans.y, vec4BottomTopTrans.y));
            float maxX = std::max<float>(std::max<float>(vec4TopTrans.x, vec4TopBottomTrans.x),
                                         std::max<float>(vec4BotTrans.x, vec4BottomTopTrans.x));
            float maxY = std::max<float>(std::max<float>(vec4TopTrans.y, vec4TopBottomTrans.y),
                                         std::max<float>(vec4BotTrans.y, vec4BottomTopTrans.y));

            std::cout << "vec4TopTrans1 = (" << vec4TopTrans[0] << " " << vec4TopTrans[1] << " " << vec4TopTrans[2]
                      << std::endl;
            std::cout << "vec4TopBottomTrans1 = (" << vec4TopBottomTrans[0] << " " << vec4TopBottomTrans[1] << " " << vec4TopBottomTrans[2]
                      << std::endl;
            std::cout << "vec4BotTrans1 = (" << vec4BotTrans[0] << " " << vec4BotTrans[1] << " " << vec4BotTrans[2]
                      << std::endl;
            std::cout << "vec4BottomTopTrans1 = (" << vec4BottomTopTrans[0] << " " << vec4BottomTopTrans[1] << " " << vec4BottomTopTrans[2]
                      << std::endl;

        }

    }
}
void MinimapGenerator::setupCameraData() {
    //Do reverse transform here
    auto min1 = mathfu::vec4(
        (m_chunkStartX + m_x) * 2.0f,
        (m_chunkStartY + m_chunkHeight - (m_y + 1)) * 2.0f,
        0,
        1
    );

    auto min2 = mathfu::vec4(
        (m_chunkStartX + m_x) * 2.0f,
        (m_chunkStartY + m_chunkHeight - (m_y + 1)) * 2.0f,
        1,
        1
    );

    auto max1 = mathfu::vec4(
        (m_chunkStartX + m_x + 1) * 2.0f,
        (m_chunkStartY + m_chunkHeight - m_y) * 2.0f,
        0,
        1
    );

    auto max2 = mathfu::vec4(
        (m_chunkStartX + m_x + 1) * 2.0f,
        (m_chunkStartY + m_chunkHeight - m_y) * 2.0f,
        1,
        1
    );

    auto tempViewProjMatInv = genTempProjectMatrix().Inverse();

    min1 = tempViewProjMatInv * min1;
    min1 = min1 * ( 1.0f / min1.w);

    min2 = tempViewProjMatInv * min2;
    min2 = min2 * ( 1.0f / min2.w);

    max1 = tempViewProjMatInv * max1;
    max1 = max1 * ( 1.0f / max1.w);

    max2 = tempViewProjMatInv * max2;
    max2 = max2 * ( 1.0f / max2.w);

    //Reproject to get proper zero in z
    float alphaMin = -min2.z / (min1.z - min2.z);
    auto min = (min1 - min2) * alphaMin + min2;

    float alphaMax = -max2.z / (max1.z - max2.z);
    auto max = (max1 - max2) * alphaMax + max2;

    for (auto &mapRuntime : mapRuntimeInfo) {
        if (mapRuntime.mandatoryADTMap.size() > 0) {
            auto adtVec = mapRuntime.mandatoryADTMap[m_x][m_y];
            mapRuntime.scene->setMandatoryADTs(adtVec);
        }
    }

//    std::cout << "(Debug) m_x = " << m_x << " m_y = " << m_y <<
//        ", x = " << (max.x + min.x) * 0.5f <<
//        ", y = " << (max.y + min.y) * 0.5f << std::endl;

    setLookAtPoint(
        (max.x + min.x) * 0.5,
        (max.y + min.y) * 0.5
    );
}

void MinimapGenerator::setZoom(float zoom) {
    m_zoom = zoom;
}

void MinimapGenerator::setLookAtPoint(float x, float y) {
    mathfu::vec3 lookAtPoint2D = mathfu::vec3(x, y, 0);

    //TODO:
//    setupCamera(lookAtPoint2D, m_apiContainer->camera);

//    m_apiContainer->camera->tick(0);
}

void
MinimapGenerator::setupCamera(const mathfu::vec3 &lookAtPoint2D, std::shared_ptr<ICamera> &camera) {
    mathfu::vec3 lookAtVec3 = getLookAtVec3();

    mathfu::vec3 lookAtPoint = lookAtPoint2D - ((m_minZ - 10) * lookAtVec3);
    mathfu::vec3 cameraPos = lookAtPoint2D - ((m_maxZ + 10) * lookAtVec3);

    camera->setCameraPos(
        cameraPos.x, cameraPos.y, cameraPos.z
    );
    camera->setCameraLookAt(
        lookAtPoint.x, lookAtPoint.y, lookAtPoint.z
    );
}

void MinimapGenerator::resetCandidate() {
    prepearCandidate = false;
    m_candidateCS = {};
    framesReady = 0;
}

void MinimapGenerator::calcBB(const HMapRenderPlan &mapRenderPlan, mathfu::vec3 &minCoord,
                              mathfu::vec3 &maxCoord,
                              const CAaBox &adtBox2d,
                              int adt_x, int adt_y, bool applyAdtChecks) {
    minCoord = mathfu::vec3(20000, 20000, 20000);
    maxCoord = mathfu::vec3(-20000, -20000, -20000);

    for (auto &m2ObjectId: mapRenderPlan->m2Array.getDrawn()) {
        auto objBB = m2Factory->getObjectById<0>(m2ObjectId)->getAABB();

        if (applyAdtChecks && !MathHelper::isAabbIntersect2d(objBB, adtBox2d)) continue;

        if (objBB.max.x > 17000) {
            continue;
        }

        if (objBB.max.z > 10000) {
            std::cout << "what" << std::endl;
        }

        minCoord = mathfu::vec3(
            std::min<float>(minCoord.x, objBB.min.x),
            std::min<float>(minCoord.y, objBB.min.y),
            std::min<float>(minCoord.z, objBB.min.z)
        );
        maxCoord = mathfu::vec3(
            std::max<float>(maxCoord.x, objBB.max.x),
            std::max<float>(maxCoord.y, objBB.max.y),
            std::max<float>(maxCoord.z, objBB.max.z)
        );
    }

    for (auto &wmoObject: mapRenderPlan->wmoGroupArray.getToDraw()) {
        auto objBB = wmoObject->getWorldAABB();

        if (applyAdtChecks && !MathHelper::isAabbIntersect2d(objBB, adtBox2d)) continue;

        if (objBB.max.x > 17000) {
            continue;
        }

        if (objBB.max.z > 10000) {
            std::cout << "what" << std::endl;
        }

        minCoord = mathfu::vec3(
            std::min<float>(minCoord.x, objBB.min.x),
            std::min<float>(minCoord.y, objBB.min.y),
            std::min<float>(minCoord.z, objBB.min.z)
        );
        maxCoord = mathfu::vec3(
            std::max<float>(maxCoord.x, objBB.max.x),
            std::max<float>(maxCoord.y, objBB.max.y),
            std::max<float>(maxCoord.z, objBB.max.z)
        );
    }


    for (auto &adtObjectRes: mapRenderPlan->adtArray) {
        auto const &adtObj = adtObjectRes.adtObject;


        if (applyAdtChecks && (adtObj->getAdtX() != adt_x || adtObj->getAdtY() != adt_y)) {
//                std::cout << "skipping adtObj( " <<
//                adtObj->getAdtX() << "," << adtObj->getAdtY() << " "
//                ") for adt(" << adt_x << ", " << adt_y << ")" << std::endl;

            continue;
        }

        auto objBB = adtObj->calcAABB();

        if (objBB.max.x > 17000) {
            continue;
        }

        if (objBB.max.z > 10000) {
            std::cout << "what" << std::endl;
        }

        minCoord = mathfu::vec3(
            std::min<float>(minCoord.x, objBB.min.x),
            std::min<float>(minCoord.y, objBB.min.y),
            std::min<float>(minCoord.z, objBB.min.z)
        );
        maxCoord = mathfu::vec3(
            std::max<float>(maxCoord.x, objBB.max.x),
            std::max<float>(maxCoord.y, objBB.max.y),
            std::max<float>(maxCoord.z, objBB.max.z)
        );
    }
}

const int waitQueueLen = 5;
void MinimapGenerator::process() {
    if (m_processor->completedAllJobs()) {
        framesReady++;
    } else {
        resetCandidate();
        return;
    }

    if (!m_candidateCS.empty()) {
        for (auto &cullStage : m_candidateCS) {
            if (cullStage == nullptr) continue;

            for (const auto &adtCullRes : cullStage->adtArray) {
                if (adtCullRes.adtObject == nullptr) continue;
                if (adtCullRes.adtObject->getLoadedStatus() == FileStatus::FSNotLoaded) {
                    resetCandidate();
                    return;
                }
            }

            if (
                    (cullStage->m2Array.getToLoadGeom().size() != 0) ||
                    (cullStage->m2Array.getToLoadMain().size() != 0) ||
                    (cullStage->wmoGroupArray.getToLoad().size() != 0)
                ) {
                resetCandidate();
                return;
            }
        }
    }

//    if (!m_candidateUS.empty()) {
//        for (auto &updateStage : m_candidateUS) {
//            if (updateStage != nullptr && updateStage->texturesForUpload.size() > 0) {
//                resetCandidate();
//                return;
//            }
//        }
//    }
//
//    if (framesReady < waitQueueLen) {
//        return;
//    }
//
//    if (m_candidateDS == nullptr) {
//        resetCandidate();
//        prepearCandidate = true;
//        return;
//    }

    auto lastFrameCull = m_candidateCS;
    auto lastFrameParams = m_candidateCS;
    resetCandidate();

    //Check the BB and adjust minZ-maxZ
    int adt_x = m_x;
    int adt_y = m_y;

    vec2 minAdt = {
        AdtIndexToWorldCoordinate(adt_y + 1),
        AdtIndexToWorldCoordinate(adt_x + 1)
    };
    vec2 maxAdt = {
        AdtIndexToWorldCoordinate(adt_y),
        AdtIndexToWorldCoordinate(adt_x),
    };
    mathfu::vec3 minCoord = mathfu::vec3(20000, 20000, 20000);
    mathfu::vec3 maxCoord = mathfu::vec3(-20000, -20000, -20000);
    {
        CAaBox adtBox2d = {
            mathfu::vec3_packed(mathfu::vec3(minAdt.x, minAdt.y, 0)),
            mathfu::vec3_packed(mathfu::vec3(maxAdt.x, maxAdt.y, 0))
        };

        bool needRestart = false;
        for (auto &cullStage: lastFrameCull) {
            this->calcBB(cullStage, minCoord, maxCoord, adtBox2d, adt_x, adt_y,
                         m_mgMode == EMGMode::eBoundingBoxCalculation);
            if (adt_y == 29 && adt_x == 15) {
                std::cout << "what" << std::endl;
            }

            float zFar = (minCoord - maxCoord).Length();
            float maxZ = maxCoord.z;
            float minZ = minCoord.z;


            if (!cullStage->m2Array.getDrawn().empty() || !cullStage->adtArray.empty() ||
                !cullStage->wmoGroupArray.getToDraw().empty()) {
                if (minCoord.x < 20000 && maxCoord.x > -20000) {
                    if (zFar > m_zFar || maxZ > m_maxZ || minZ < m_minZ) {
                        m_zFar = std::max<float>(zFar, m_zFar);
                        m_maxZ = std::max<float>(maxZ, m_maxZ);
                        m_minZ = std::min<float>(minZ, m_minZ);
                        needRestart = true;
                    }
                }
            }
        }
        if (needRestart) {
            resetCandidate();
            setupCameraData();
            return;
        }

        if (minCoord.x < 20000 && maxCoord.x > -20000) {
            minAdt.x = std::max<double>(minCoord.x, minAdt.x - (MathHelper::TILESIZE/2.0f));
            maxAdt.x = std::min<double>(maxCoord.x, maxAdt.x + (MathHelper::TILESIZE/2.0f));

            minAdt.y = std::max<double>(minCoord.y, minAdt.y - (MathHelper::TILESIZE/2.0f));
            maxAdt.y = std::min<double>(maxCoord.y, maxAdt.y + (MathHelper::TILESIZE/2.0f));
        }
    }

    //Conditional mode stuff
    if (m_mgMode == EMGMode::eBoundingBoxCalculation) {


        minCoord = mathfu::vec3(
            minAdt.x,
            minAdt.y,
            minCoord.z);

        maxCoord = mathfu::vec3(
            maxAdt.x,
            maxAdt.y,
            maxCoord.z);

        currentScenario.maps[m_mapIndex].adtConfigHolder->adtMin[adt_x][adt_y] = minCoord;
        currentScenario.maps[m_mapIndex].adtConfigHolder->adtMax[adt_x][adt_y] = maxCoord;



        saveDrawStageToFile(currentScenario.folderToSave+"/minimap/"+std::to_string(currentScenario.maps[m_mapIndex].mapId), m_lastFrameBuffer);
    } else if (m_mgMode == EMGMode::eScreenshotGeneration) {
        //Make screenshot out of this drawStage
        saveDrawStageToFile(currentScenario.folderToSave, m_lastFrameBuffer);
    }

    //Apply this logic only if it's not a preview mode
    if (m_mgMode != EMGMode::ePreview) {
        m_y--;
        if (m_y < 0) {
            m_x++;
            m_y = m_chunkHeight - 1;
        }
//    std::cout << "m_x = " << m_x << " out of (" << m_chunkStartX+m_chunkWidth << ") m_y = " << m_y << " out of (" << m_chunkStartY+m_chunkHeight << ")" << std::endl;

        if (m_x >= m_chunkWidth) {
            if (m_mgMode == EMGMode::eBoundingBoxCalculation && (m_mapIndex+1) < currentScenario.maps.size()) {
                m_mapIndex++;
                m_x = 0;
                m_y = 0;
                m_zFar = 10000.0f;
                m_maxZ = 1000.0f;
                m_minZ = -1000.0f;
                setupCameraData();
            } else {
                startNextScenario();
            }
        } else {
            //reset
            m_zFar = 10000.0f;
            m_maxZ = 1000.0f;
            m_minZ = -1000.0f;
            setupCameraData();
        }
    }
}

void MinimapGenerator::saveDrawStageToFile(std::string folderToSave, const HFrameBuffer lastFrameBuffer) {
    if (!ghc::filesystem::is_directory(folderToSave) ||
        !ghc::filesystem::exists(folderToSave)) { // Check if src folder exists
        ghc::filesystem::create_directories(folderToSave); // create src folder
    }

    std::string fileName = folderToSave + "/map_" + std::to_string(
        (m_x)
    ) + "_" + std::to_string(
        (m_y)
    ) + ".png";

    std::vector<uint8_t> buffer = std::vector<uint8_t>(m_width * m_height * 4 + 1);
//TODO:
//    saveDataFromDrawStage(lastFrameBuffer, fileName, m_width, m_height, buffer);
}

void MinimapGenerator::createSceneDrawStage(HFrameScenario sceneScenario) {
//    float farPlaneRendering = m_apiContainer->getConfig()->farPlane;
//    float farPlaneCulling = m_apiContainer->getConfig()->farPlaneForCulling;
//
//    float nearPlane = 1.0f;
//    float fov = toRadian(45.0f);
//
//    float canvasAspect = (float)m_width / (float)m_height;
//
//    HCameraMatrices cameraMatricesCulling = m_apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneCulling);
//    HCameraMatrices cameraMatricesRendering = m_apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
//    //Frustum matrix with reversed Z
//
//    {
//        float f = 1.0f / tan(fov / 2.0f);
//        cameraMatricesCulling->perspectiveMat = getOrthoMatrix();
//        cameraMatricesRendering->perspectiveMat = getOrthoMatrix();
//    }
//
//    mathfu::vec4 clearColor = m_apiContainer->getConfig()->clearColor;
//    m_lastDraw = nullptr;
//
//    if (!mapRuntimeInfo.empty()) {
//        ViewPortDimensions dimensions = {{0, 0}, {m_width, m_height}};
//
//        HFrameBuffer fb = nullptr;
//        int frameWait = 4;
//        if (prepearCandidate && m_candidateFrameBuffer == nullptr ) {
//            frameWait = waitQueueLen+4;
//        }
//
//        fb = m_apiContainer->hDevice->createFrameBuffer(m_width, m_height,
//                                                        {ITextureFormat::itRGBA}, ITextureFormat::itDepth32,
//                                                        m_apiContainer->hDevice->getMaxSamplesCnt(), frameWait);
//
//        std::vector<HDrawStage> drawStageDependencies = {};
//
//        auto &&mainMap = mapRuntimeInfo[m_mapIndex];
//        std::vector<HCullStage> cullStages;
//        std::vector<HUpdateStage> updateStages;
//
//        if (m_mgMode != EMGMode::eBoundingBoxCalculation) {
//            for (auto &mapData: mapRuntimeInfo) {
//                auto cullStage = sceneScenario->addCullStage(cameraMatricesCulling, mapData.scene);
//                HUpdateStage updateStage = sceneScenario->addUpdateStage(cullStage, 0,
//                                                                                         cameraMatricesRendering);
//                cullStages.push_back(cullStage);
//                updateStages.push_back(updateStage);
//            }
//        } else {
//            auto cullStage = sceneScenario->addCullStage(cameraMatricesCulling, mainMap.scene);
//            std::shared_ptr<UpdateStage> updateStage = sceneScenario->addUpdateStage(cullStage, 0,
//                                                                                     cameraMatricesRendering);
//            cullStages.push_back(cullStage);
//            updateStages.push_back(updateStage);
//        }
//
//        HDrawStage sceneDrawStage = sceneScenario->addDrawStage(
//            updateStages, mainMap.scene, cameraMatricesRendering,
//            drawStageDependencies, true, dimensions, true, false, clearColor, fb);
//
//        m_lastDraw = sceneDrawStage;
//        //We dont need stack in preview mode
//        if (prepearCandidate && m_candidateDS == nullptr) {
//            m_candidateDS = sceneDrawStage;
//            m_candidateCS = cullStages;
//            m_candidateUS = updateStages;
//        }
//        stackOfCullStages[m_apiContainer->hDevice->getDrawFrameNumber()] = cullStages[0];
//
//        return sceneDrawStage;
//    }

    return;
}

float MinimapGenerator::GetOrthoDimension() {
//    return MathHelper::TILESIZE*0.25*0.5f;
//    return MathHelper::TILESIZE*0.25;
    return MathHelper::TILESIZE / m_zoom;
}

mathfu::mat4 MinimapGenerator::getOrthoMatrix() {
    return
        mathfu::mat4::Ortho(
        -GetOrthoDimension() / 2.0f,GetOrthoDimension() / 2.0f,
        -GetOrthoDimension() / 2.0f,GetOrthoDimension() / 2.0f,
        1,m_zFar + 10, 1
    );
}

mathfu::vec3 MinimapGenerator::getLookAtVec3() {
    switch (currentScenario.orientation) {
        case ScenarioOrientation::soTopDownOrtho:
            return mathfu::vec3(0,0,-1);
        case ScenarioOrientation::so45DegreeTick0:
            return mathfu::vec3(1,1,-1);
        case ScenarioOrientation::so45DegreeTick1:
            return mathfu::vec3(1,-1,-1);
        case ScenarioOrientation::so45DegreeTick2:
            return mathfu::vec3(-1,-1,-1);
        case ScenarioOrientation::so45DegreeTick3:
            return mathfu::vec3(-1,1,-1);
    }

    return mathfu::vec3(0,0,0);
}

HFrameBuffer MinimapGenerator::getLastFrameBuffer() {
    return m_lastFrameBuffer;
}

Config *MinimapGenerator::getConfig() {
    return m_apiContainer->getConfig();
}

void MinimapGenerator::reload() {
    loadMaps();
}

void MinimapGenerator::getCurrentFDData(int &areaId, int &parentAreaId, mathfu::vec4 &riverColor) {
    areaId = 0;
    parentAreaId = 0;
    riverColor = mathfu::vec4(0,0,0,0);

//    auto cullStage = stackOfCullStages[m_apiContainer->hDevice->getUpdateFrameNumber()];
//    if (cullStage == nullptr) return;
//
//    areaId = cullStage->adtAreadId;
//    parentAreaId = cullStage->parentAreaId;
//    riverColor = cullStage->frameDependentData->closeRiverColor;
//
//    mathfu::vec4 cameraPos = {0,0,0,1};
//    this->m_apiContainer->camera->getCameraPosition(cameraPos.data_);
//
//    for (auto &adtCullObj : cullStage->adtArray) {
//        auto adt_x = worldCoordinateToAdtIndex(cameraPos.y);
//        auto adt_y = worldCoordinateToAdtIndex(cameraPos.x);
//
//        if (adt_x != adtCullObj->adtObject->getAdtX() || adt_y != adtCullObj->adtObject->getAdtY())
//            continue;
//
//        mathfu::vec3 riverColorV3;
//        adtCullObj->adtObject->getWaterColorFromDB(cameraPos, riverColorV3);
//        riverColor = mathfu::vec4(riverColorV3, 0);
//        break;
//    }
}

