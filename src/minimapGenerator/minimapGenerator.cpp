//
// Created by Deamon on 9/8/2020.
//
//#include "../../wowViewerLib/src/engine/SceneScenario.h"
//#include <filesystem>
#include "../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include "minimapGenerator.h"
#include "../../wowViewerLib/src/engine/algorithms/mathHelper.h"
#include "../../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../persistance/RequestProcessor.h"
#include "../screenshots/screenshotMaker.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonOrthoStaticCamera.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonOrthoStaticTopDownCamera.h"


MinimapGenerator::MinimapGenerator(HWoWFilesCacheStorage cacheStorage, std::shared_ptr<IDevice> hDevice, HRequestProcessor processor, IClientDatabase* dbhandler ) {
    m_apiContainer = std::make_shared<ApiContainer>();

    m_apiContainer->hDevice = hDevice;
    m_apiContainer->cacheStorage = cacheStorage;
    m_apiContainer->camera = std::make_shared<FirstPersonOrthoStaticCamera>();
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

    config->exteriorAmbientColor =  mathfu::vec4(0.5f,0.5f,0.5f,1.0);;
    config->exteriorHorizontAmbientColor = mathfu::vec4(0.5f,0.5f,0.5f,1.0);
    config->exteriorGroundAmbientColor = mathfu::vec4(0.0,0.0,0.0,1.0);
    config->exteriorDirectColor = mathfu::vec4(0.5f,0.5f,0.5f,1.0);

    config->adtSpecMult = 0.0;

    config->disableFog = true;
    config->renderSkyDom = false;
    config->currentGlow = 0;
}
void MinimapGenerator::startScenarios(std::vector<ScenarioDef> &scenarioList) {
    m_mgMode = EMGMode::eScreenshotGeneration;

    setZoom(1.0f);

    scenarioListToProcess = {};
    for (auto &scenario : scenarioList) {
        scenarioListToProcess.push_back(scenario);

        if (scenario.doBoundingBoxPreCalc) {
            auto scenarionCopy = scenario;
            scenarionCopy.mode = EMGMode::eBoundingBoxCalculation;
            scenarionCopy.orientation = ScenarioOrientation::soTopDownOrtho;

            scenarionCopy.minWowWorldCoord = mathfu::vec2(-MathHelper::TILESIZE * 32, -MathHelper::TILESIZE * 32);
            scenarionCopy.maxWowWorldCoord = mathfu::vec2(MathHelper::TILESIZE * 32, MathHelper::TILESIZE * 32);

            scenarionCopy.doBoundingBoxPreCalc = false;
            scenarionCopy.zoom = 1.0f;
            scenarionCopy.imageWidth = 512;
            scenarionCopy.imageHeight = 512;
            if (scenarionCopy.boundingBoxHolder == nullptr) {
                scenarionCopy.boundingBoxHolder = std::make_shared<ADTBoundingBoxHolder>();
                scenario.boundingBoxHolder = std::make_shared<ADTBoundingBoxHolder>();
            }
            scenarioListToProcess.push_back(scenarionCopy);
        }

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


void MinimapGenerator::setupScenarioData() {
    m_mgMode = currentScenario.mode;

    m_zoom = currentScenario.zoom;

    MapRecord mapRecord;
    if (!m_apiContainer->databaseHandler->getMapById(currentScenario.mapId, mapRecord)) {
        std::cout << "Couldnt get data for mapId " << currentScenario.mapId << std::endl;
        startNextScenario();
        return;
    }

    if (currentScenario.orientation == ScenarioOrientation::soTopDownOrtho) {
        m_apiContainer->camera = std::make_shared<FirstPersonOrthoStaticTopDownCamera>();
    } else {
        m_apiContainer->camera = std::make_shared<FirstPersonOrthoStaticCamera>();
    }

    m_currentScene = std::make_shared<Map>(m_apiContainer, mapRecord.ID, mapRecord.WdtFileID);
    if (m_mgMode == EMGMode::eScreenshotGeneration) {
        m_currentScene->setAdtBoundingBoxHolder(currentScenario.boundingBoxHolder);
    }

    auto config = m_apiContainer->getConfig();
    config->closeOceanColor = currentScenario.closeOceanColor;//{0.0671968088, 0.294095874, 0.348881632, 0};
    config->closeRiverColor = currentScenario.closeRiverColor;//{0.345206976, 0.329288304, 0.270450264, 0};

    setMinMaxXYWidhtHeight(currentScenario.minWowWorldCoord, currentScenario.maxWowWorldCoord);

    m_x = 0;
    m_y = 0;

    m_width = currentScenario.imageWidth;
    m_height = currentScenario.imageHeight;

    pauseAddingToStack = false;

    setupCameraData();
}


void
MinimapGenerator::setMinMaxXYWidhtHeight(const mathfu::vec2 &minWowWorldCoord, const mathfu::vec2 &maxWowWorldCoord) {
    mathfu::mat4 viewProj = genTempProjectMatrix();

    mathfu::vec4 corner0 =
        (viewProj * mathfu::vec4(minWowWorldCoord.x, minWowWorldCoord.y, 0, 1.0));
    mathfu::vec4 corner1 =
        (viewProj * mathfu::vec4(minWowWorldCoord.x, maxWowWorldCoord.y, 0, 1.0));
    mathfu::vec4 corner2 =
        (viewProj * mathfu::vec4(maxWowWorldCoord.x, minWowWorldCoord.y, 0, 1.0));
    mathfu::vec4 corner3 =
        (viewProj * mathfu::vec4(maxWowWorldCoord.x, maxWowWorldCoord.y, 0, 1.0));

    corner0 = corner0 * (1.0f / corner0.w);
    corner1 = corner1 * (1.0f / corner1.w);
    corner2 = corner2 * (1.0f / corner2.w);
    corner3 = corner3 * (1.0f / corner3.w);


    mathfu::vec2 minOrthoMapCoord = mathfu::vec2(
        std::min<float>(std::min<float>(corner0.x, corner1.x), std::min<float>(corner2.x, corner3.x)),
        std::min<float>(std::min<float>(corner0.y, corner1.y), std::min<float>(corner2.y, corner3.y))
    );
    mathfu::vec2 maxOrthoMapCoord = mathfu::vec2(
        std::max<float>(std::max<float>(corner0.x, corner1.x), std::max<float>(corner2.x, corner3.x)),
        std::max<float>(std::max<float>(corner0.y, corner1.y), std::max<float>(corner2.y, corner3.y))
    );

    std::cout <<"Orient = " << (int)currentScenario.orientation << std::endl;

    m_chunkStartX = std::floor(((minOrthoMapCoord.y))) ;
    m_chunkWidth = std::floor(((maxOrthoMapCoord.y - minOrthoMapCoord.y)/ 2.0f) + 0.99 ) ;
    m_chunkStartY = std::floor((minOrthoMapCoord.x));
    m_chunkHeight = std::floor(((maxOrthoMapCoord.x - minOrthoMapCoord.x)/ 2.0f) + 0.99);

    std::cout << "m_chunkStartX = " << m_chunkStartX << " m_chunkWidth = " << m_chunkWidth
              << " m_chunkStartY = " << m_chunkStartY << " m_chunkHeight = " << m_chunkHeight << std::endl;
}

mathfu::mat4 MinimapGenerator::genTempProjectMatrix() {
    auto orthoProjection = getOrthoMatrix();

    mathfu::vec3 lookAtPoint = mathfu::vec3(0, 0, 0);
    lookAtPoint = lookAtPoint;
    //std::cout << "lookAtPoint = (" << lookAtPoint.x << ", " << lookAtPoint.y << ", " << lookAtPoint.z << ") " << std::endl;

    mathfu::vec3 lookAtVec3 = getLookAtVec3();
    lookAtPoint -= (4000.0f*lookAtVec3);

    mathfu::vec3 cameraPos = lookAtPoint-(2000.0f*lookAtVec3);

    std::shared_ptr<ICamera> tempCamera;
    if (currentScenario.orientation == ScenarioOrientation::soTopDownOrtho) {
        tempCamera = std::make_shared<FirstPersonOrthoStaticTopDownCamera>();
    } else {
        tempCamera = std::make_shared<FirstPersonOrthoStaticCamera>();
    }

    tempCamera->setCameraPos(cameraPos.x, cameraPos.y, cameraPos.z);
    tempCamera->setCameraLookAt(lookAtPoint.x, lookAtPoint.y, lookAtPoint.z);
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
            mathfu::vec3 point = mathfu::vec3(0,
                                              0,
                                              2000);
            std::cout << "point = (" << point.x << " " << point.y << " " << point.z << std::endl;
            m_apiContainer->camera->setCameraPos(
                point.x, point.y, point.z
            );
            ((FirstPersonOrthoStaticCamera *) m_apiContainer->camera.get())->setCameraLookAt(
                point.x + 1, point.y + 1, point.z - 1
            );
            m_apiContainer->camera->tick(0);
            float nearPlane = 1.0;
            float fov = toRadian(45.0);

            float canvasAspect = (float)m_width / (float)m_height;
            float farPlaneRendering = m_apiContainer->getConfig()->farPlane;
            float farPlaneCulling = m_apiContainer->getConfig()->farPlaneForCulling;

            HCameraMatrices cameraMatricesRendering = m_apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);

            auto viewProj = orthoProjection * cameraMatricesRendering->lookAtMat;

            mathfu::vec4 vec4TopTrans = viewProj.Inverse() * vec4Top;
            vec4TopTrans *= (1.0f / vec4TopTrans[3]);

            mathfu::vec4 vec4TopBottomTrans = viewProj.Inverse() * vec4TopBottom;
            vec4TopBottomTrans *= (1.0f / vec4TopBottomTrans[3]);

            mathfu::vec4 vec4BotTrans = viewProj.Inverse() * vec4Bottom;
            vec4BotTrans *= (1.0f / vec4BotTrans[3]);

            mathfu::vec4 vec4BottomTopTrans = viewProj.Inverse() * vec4BottomTop;
            vec4BottomTopTrans *= (1.0f / vec4BottomTopTrans[3]);


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
        m_chunkStartY + (m_y * 2.0f),
        m_chunkStartX + (m_x * 2.0f),
        0,
        1
    );

    auto min2 = mathfu::vec4(
        m_chunkStartY + (m_y * 2.0f),
        m_chunkStartX + (m_x * 2.0f),
        1,
        1
    );

    auto max1 = mathfu::vec4(
        m_chunkStartY + ((m_y + 1) * 2.0f),
        m_chunkStartX + ((m_x + 1) * 2.0f),
        0,
        1
    );

    auto max2 = mathfu::vec4(
        m_chunkStartY + ((m_y + 1) * 2.0f),
        m_chunkStartX + ((m_x + 1) * 2.0f),
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

    setLookAtPoint(
        -(max.x + min.x) * 0.5,
        (max.y + min.y) * 0.5
    );
}

void MinimapGenerator::setZoom(float zoom) {
    m_zoom = zoom;
}
void MinimapGenerator::setLookAtPoint(float x, float y) {
    mathfu::vec3 lookAtPoint = mathfu::vec3(x, y, 0);

    lookAtPoint = lookAtPoint;
    //std::cout << "lookAtPoint = (" << lookAtPoint.x << ", " << lookAtPoint.y << ", " << lookAtPoint.z << ") " << std::endl;

    mathfu::vec3 lookAtVec3 = getLookAtVec3();
    lookAtPoint -= (4000.0f*lookAtVec3);

    mathfu::vec3 cameraPos = lookAtPoint-(2000.0f*lookAtVec3);
//    std::cout << "cameraPos = (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ") " << std::endl;

    m_apiContainer->camera->setCameraPos(
        cameraPos.x, cameraPos.y, cameraPos.z
    );
    m_apiContainer->camera->setCameraLookAt(
        lookAtPoint.x, lookAtPoint.y, lookAtPoint.z
    );
    m_apiContainer->camera->tick(0);
}


const int waitQueueLen = 5;
void MinimapGenerator::process() {
    if (m_processor->completedAllJobs() && !m_apiContainer->hDevice->wasTexturesUploaded()) {
        framesReady++;
    } else {
        framesReady = 0;
        pauseAddingToStack = false;
        drawStageStack.clear();
        cullStageStack.clear();
        return;
    }

    if (framesReady < waitQueueLen) {
        if (drawStageStack.size() > waitQueueLen) {
            drawStageStack.pop_back();
            cullStageStack.pop_back();
        }
        return;
    }
    if (!pauseAddingToStack) {
        pauseAddingToStack = true;
        framesReady = 0;
        return;
    }

    auto lastFrameIt = drawStageStack.front();
    auto lastFrameCull = cullStageStack.front();
    drawStageStack.clear();
    cullStageStack.clear();
    pauseAddingToStack = false;
    framesReady = 0;

    if (m_mgMode == EMGMode::eBoundingBoxCalculation) {
        mathfu::vec3 minCoord = mathfu::vec3(20000, 20000, 20000);
        mathfu::vec3 maxCoord = mathfu::vec3(-20000, -20000, -20000);

        for (auto &m2Object: lastFrameCull->m2Array) {
            auto objBB = m2Object->getAABB();
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

        for (auto &wmoObject: lastFrameCull->wmoArray) {
            auto objBB = wmoObject->getAABB();
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

        int adt_y = ((m_chunkWidth - 1) - (m_x));
        int adt_x = ((m_chunkHeight - 1) - (m_y));
        for (auto &adtObjectRes: lastFrameCull->adtArray) {
            auto adtObj = adtObjectRes->adtObject;
            if (adtObj->getAdtX() != adt_x || adtObj->getAdtY() != adt_y) {
                continue;
            }

            auto objBB = adtObj->calcAABB();
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

//        std::cout << "minCoord = (" << minCoord.x << ", " << minCoord.y << ", " << minCoord.z << ")" << std::endl;
//        std::cout << "maxCoord = (" << maxCoord.x << ", " << maxCoord.y << ", " << maxCoord.z << ")" << std::endl;

        //Set x-y limits according to current adt index, since bounding box counting process is done per ADT
        minCoord = mathfu::vec3(
            AdtIndexToWorldCoordinate(adt_y - 1),
            AdtIndexToWorldCoordinate(adt_x - 1),
            minCoord.z);

        maxCoord = mathfu::vec3(
            AdtIndexToWorldCoordinate(adt_y),
            AdtIndexToWorldCoordinate(adt_x),
            maxCoord.z);

        if (currentScenario.boundingBoxHolder != nullptr) {
            (*currentScenario.boundingBoxHolder)[adt_x][adt_y] = CAaBox(
                C3Vector(minCoord),
                C3Vector(maxCoord)
            );
        }

        saveDrawStageToFile(currentScenario.folderToSave+"/minimap", lastFrameIt);
    } else if (m_mgMode == EMGMode::eScreenshotGeneration) {
        //Make screenshot out of this drawStage
        saveDrawStageToFile(currentScenario.folderToSave, lastFrameIt);
    }

    m_x++;
    if (m_x >= m_chunkWidth)  {
        m_y++;
        m_x = 0;
    }
//    std::cout << "m_x = " << m_x << " out of (" << m_chunkStartX+m_chunkWidth << ") m_y = " << m_y << " out of (" << m_chunkStartY+m_chunkHeight << ")" << std::endl;

    if (m_y >= (m_chunkHeight)) {
        startNextScenario();
    } else {
        setupCameraData();
    }
}

void MinimapGenerator::saveDrawStageToFile(std::string folderToSave, const std::shared_ptr<DrawStage> &lastFrameIt) {
    if (!ghc::filesystem::is_directory(folderToSave) ||
        !ghc::filesystem::exists(folderToSave)) { // Check if src folder exists
        ghc::filesystem::create_directories(folderToSave); // create src folder
    }

    std::string fileName = folderToSave + "/map_" + std::to_string(
        (m_y)
    ) + "_" + std::to_string(
        (m_x)
    ) + ".png";

    std::__1::vector<uint8_t> buffer = std::__1::vector<uint8_t>(m_width * m_height * 4 + 1);
    saveDataFromDrawStage(lastFrameIt->target, fileName, m_width, m_height, buffer);
    if (lastFrameIt->opaqueMeshes != nullptr) {
//        std::cout << "Saved " << fileName << ": opaqueMeshes (" << lastFrameIt->opaqueMeshes->meshes.size() << ") "
//                  << std::endl;
    }
}

HDrawStage MinimapGenerator::createSceneDrawStage(HFrameScenario sceneScenario) {
    float farPlaneRendering = m_apiContainer->getConfig()->farPlane;
    float farPlaneCulling = m_apiContainer->getConfig()->farPlaneForCulling;

    float nearPlane = 1.0;
    float fov = toRadian(45.0);

    float canvasAspect = (float)m_width / (float)m_height;

    HCameraMatrices cameraMatricesCulling = m_apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneCulling);
    HCameraMatrices cameraMatricesRendering = m_apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
    //Frustum matrix with reversed Z

    {
        float f = 1.0f / tan(fov / 2.0f);
        cameraMatricesCulling->perspectiveMat = getOrthoMatrix();
        cameraMatricesRendering->perspectiveMat = getOrthoMatrix();
    }

    if (m_apiContainer->hDevice->getIsVulkanAxisSystem() ) {
        auto &perspectiveMatrix = cameraMatricesRendering->perspectiveMat;

        static const mathfu::mat4 vulkanMatrixFix2 = mathfu::mat4(1, 0, 0, 0,
                                                                  0, -1, 0, 0,
                                                                  0, 0, 1.0/2.0, 1/2.0,
                                                                  0, 0, 0, 1).Transpose();

        perspectiveMatrix = vulkanMatrixFix2 * perspectiveMatrix;
    }

    mathfu::vec4 clearColor = m_apiContainer->getConfig()->clearColor;
    m_lastDraw = nullptr;
    if (m_currentScene != nullptr) {
        ViewPortDimensions dimensions = {{0, 0}, {m_width, m_height}};

        HFrameBuffer fb = nullptr;
        fb = m_apiContainer->hDevice->createFrameBuffer(m_width, m_height,
                                                        {ITextureFormat::itRGBA}, ITextureFormat::itDepth32,
                                                        m_apiContainer->hDevice->getMaxSamplesCnt(), 2*waitQueueLen+1);

        std::vector<HDrawStage> drawStageDependencies = {};

        auto cullStage = sceneScenario->addCullStage(cameraMatricesCulling, m_currentScene);
        std::shared_ptr<UpdateStage> updateStage = sceneScenario->addUpdateStage(cullStage, 0, cameraMatricesRendering);
        HDrawStage sceneDrawStage = sceneScenario->addDrawStage(
            updateStage, m_currentScene, cameraMatricesRendering,
            drawStageDependencies, true, dimensions, true, false, clearColor, fb);

        m_lastDraw = sceneDrawStage;
        //We dont need stack in preview mode
        if (m_mgMode != EMGMode::ePreview && !pauseAddingToStack) {
            drawStageStack.push_front(sceneDrawStage);
            cullStageStack.push_front(cullStage);
        }
        return sceneDrawStage;
    }

    return nullptr;
}

float MinimapGenerator::GetOrthoDimension() {
//    return MathHelper::TILESIZE*0.25*0.5f;
//    return MathHelper::TILESIZE*0.25;
    return MathHelper::TILESIZE / m_zoom;
}

mathfu::mat4 MinimapGenerator::getOrthoMatrix() {
    return mathfu::mat4::Ortho(
        -GetOrthoDimension() / 2.0f,GetOrthoDimension() / 2.0f,
        -GetOrthoDimension() / 2.0f,GetOrthoDimension() / 2.0f,
        1,10000
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
}

HDrawStage MinimapGenerator::getLastDrawStage() {
    return m_lastDraw;
}

Config *MinimapGenerator::getConfig() {
    return m_apiContainer->getConfig();
}

