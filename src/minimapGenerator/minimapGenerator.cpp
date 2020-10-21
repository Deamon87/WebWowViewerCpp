//
// Created by Deamon7 on 9/8/2020.
//
//#include "../../wowViewerLib/src/engine/SceneScenario.h"
#include "minimapGenerator.h"
#include "../../wowViewerLib/src/engine/algorithms/mathHelper.h"
#include "../../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../persistance/RequestProcessor.h"
#include "../screenshots/screenshotMaker.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonOrthoStaticCamera.h"


MinimapGenerator::MinimapGenerator(HWoWFilesCacheStorage cacheStorage, std::shared_ptr<IDevice> hDevice, HRequestProcessor processor ) {
    m_apiContainer.hDevice = hDevice;
    m_apiContainer.cacheStorage = cacheStorage;
    m_apiContainer.camera = std::make_shared<FirstPersonOrthoStaticCamera>();
    m_processor = processor;

//    m_apiContainer.getConfig()->setDisableFog()
}

void MinimapGenerator::startScenario(int scenarioId) {
    if (scenarioId == 0) {
        m_currentScene = std::make_shared<Map>(&m_apiContainer, 0, 775971);
//        m_x = -32;
//        m_y = -32;
        m_x = 0;
        m_y = 0;

        setupCameraData();
    }

    currentScenario = scenarioId;
}
void MinimapGenerator::setupCameraData() {
    m_apiContainer.camera->setCameraPos(
        MathHelper::TILESIZE * m_x,
        MathHelper::TILESIZE * m_y,
        1000
    );
    ((FirstPersonOrthoStaticCamera*)m_apiContainer.camera.get())->setCameraLookAt(
        MathHelper::TILESIZE * m_x + 1,
        MathHelper::TILESIZE * m_y + 1,
        1000 - 1
    );
    m_apiContainer.camera->tick(0);
}


void MinimapGenerator::process() {
    if (!m_processor->queuesNotEmpty()) {
        framesReady++;
    } else {
        framesReady = 0;
    }

    if (framesReady < 8) {
        if (drawStageStack.size() > 8)
            drawStageStack.pop_back();
        return;
    }

    auto lastFrameIt = drawStageStack.back();
    drawStageStack.clear();
    framesReady = 0;

    //Make screenshot out of this drawStage
    std::string fileName = "testMinimap\\test_"+std::to_string(m_x+32)+"_"+std::to_string(m_y+32)+".png";

    std::vector<uint8_t> buffer = std::vector<uint8_t>(m_width*m_height*4+1);
    saveDataFromDrawStage(lastFrameIt->target, fileName, m_width, m_height, buffer);

    m_x++;
    if (m_x > 32)  {
        m_y++;
        m_x = -32;
    }

    setupCameraData();
}

HDrawStage MinimapGenerator::createSceneDrawStage(HFrameScenario sceneScenario) {
    float farPlaneRendering = m_apiContainer.getConfig()->farPlane;
    float farPlaneCulling = m_apiContainer.getConfig()->farPlaneForCulling;

    float nearPlane = 1.0;
    float fov = toRadian(45.0);

    float canvasAspect = (float)m_width / (float)m_height;

    HCameraMatrices cameraMatricesCulling = m_apiContainer.camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneCulling);
    HCameraMatrices cameraMatricesRendering = m_apiContainer.camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
    //Frustum matrix with reversed Z

    {
        float f = 1.0f / tan(fov / 2.0f);
        cameraMatricesCulling->perspectiveMat = mathfu::mat4::Ortho(
            -MathHelper::TILESIZE*0.5f,MathHelper::TILESIZE*0.5f,
            -MathHelper::TILESIZE*0.5f,MathHelper::TILESIZE*0.5f,
            1,2000
        );
        cameraMatricesRendering->perspectiveMat = mathfu::mat4::Ortho(
            -MathHelper::TILESIZE*0.5f,MathHelper::TILESIZE*0.5f,
            -MathHelper::TILESIZE*0.5f,MathHelper::TILESIZE*0.5f,
            1,2000
        );
    }

    if (m_apiContainer.hDevice->getIsVulkanAxisSystem() ) {
        auto &perspectiveMatrix = cameraMatricesRendering->perspectiveMat;

        static const mathfu::mat4 vulkanMatrixFix2 = mathfu::mat4(1, 0, 0, 0,
                                                                  0, -1, 0, 0,
                                                                  0, 0, 1.0/2.0, 1/2.0,
                                                                  0, 0, 0, 1).Transpose();

        perspectiveMatrix = vulkanMatrixFix2 * perspectiveMatrix;
    }

    mathfu::vec4 clearColor = m_apiContainer.getConfig()->clearColor;

    if (m_currentScene != nullptr) {
        ViewPortDimensions dimensions = {{0, 0}, {m_width, m_height}};

        HFrameBuffer fb = nullptr;
        fb = m_apiContainer.hDevice->createFrameBuffer(m_width, m_height, {ITextureFormat::itRGBA},ITextureFormat::itDepth32, 4);

        std::vector<HDrawStage> drawStageDependencies = {};

        auto cullStage = sceneScenario->addCullStage(cameraMatricesCulling, m_currentScene);
        std::shared_ptr<UpdateStage> updateStage = sceneScenario->addUpdateStage(cullStage, 0, cameraMatricesRendering);
        HDrawStage sceneDrawStage = sceneScenario->addDrawStage(
            updateStage, m_currentScene, cameraMatricesRendering,
            drawStageDependencies, true, dimensions, true, false, clearColor, fb);


        drawStageStack.push_front(sceneDrawStage);
        return sceneDrawStage;
    }

    return nullptr;
}

bool MinimapGenerator::isDone() {
    return (m_y > 32);
}

