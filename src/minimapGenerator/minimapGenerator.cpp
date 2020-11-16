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



    mathfu::vec4 ambient = mathfu::vec4(0.7,0.7,0.7,1.0);
    config->exteriorAmbientColor = ambient;
    config->exteriorHorizontAmbientColor = ambient;
    config->exteriorGroundAmbientColor = ambient;


    config->disableFog = true;
    config->renderSkyDom = false;
    config->currentGlow = 0;
//    m_apiContainer.getConfig()->setDisableFog()
}

void MinimapGenerator::startScenario(int scenarioId) {
    XToYCoefCalculated = false;
    calcXtoYCoef();

    if (scenarioId == 0) {
//        m_currentScene = std::make_shared<Map>(m_apiContainer, 0, 775971);
        m_currentScene = std::make_shared<Map>(m_apiContainer, 1, 782779);
//        m_currentScene = std::make_shared<Map>(&m_apiContainer, 30, 790112); // Alterac Valley
//        m_currentScene = std::make_shared<Map>(&m_apiContainer, 2106, 2205463); // warsong 2
//        m_currentScene = std::make_shared<Map>(&m_apiContainer, 2107, 	2205417); // arathi 2


//        m_chunkStartX = -32*4;
//        m_chunkWidth = 64*4;
//        m_chunkStartY = -32*4;
//        m_chunkHeight = 64*4;

//        mathfu::vec2 minWowWorldCoord = {-10534, 766 };
//        mathfu::vec2 maxWowWorldCoord = {-8284, 2979};
//        mathfu::vec2 minWowWorldCoord = {3558, -4688 };
//        mathfu::vec2 maxWowWorldCoord = { 5804,-1063};

        mathfu::vec2 minWowWorldCoord = {-12150, -9000};
        mathfu::vec2 maxWowWorldCoord = {11900, 5100};

        mathfu::vec2 minWowWorldCoordTransf =
            (MathHelper::RotationZ(M_PI/4).Inverse() * mathfu::vec4(minWowWorldCoord.x, minWowWorldCoord.y, 0, 1.0)).xy();
        mathfu::vec2 minMaxWowWorldCoordTransf =
            (MathHelper::RotationZ(M_PI/4).Inverse() * mathfu::vec4(minWowWorldCoord.x, maxWowWorldCoord.y, 0, 1.0)).xy();
        mathfu::vec2 maxMinWowWorldCoordTransf =
            (MathHelper::RotationZ(M_PI/4).Inverse() * mathfu::vec4(maxWowWorldCoord.x, minWowWorldCoord.y, 0, 1.0)).xy();
        mathfu::vec2 maxWowWorldCoordTransf =
            (MathHelper::RotationZ(M_PI/4).Inverse() * mathfu::vec4(maxWowWorldCoord.x, maxWowWorldCoord.y, 0, 1.0)).xy();

        mathfu::vec2 minOrthoMapCoord = mathfu::vec2(
            std::min(std::min(minWowWorldCoordTransf.x, maxWowWorldCoordTransf.x), std::min(minMaxWowWorldCoordTransf.x, maxMinWowWorldCoordTransf.x)),
            std::min(std::min(minWowWorldCoordTransf.y, maxWowWorldCoordTransf.y), std::min(minMaxWowWorldCoordTransf.y, maxMinWowWorldCoordTransf.y))
        );
        mathfu::vec2 maxOrthoMapCoord = mathfu::vec2(
            std::max(std::max(minWowWorldCoordTransf.x, maxWowWorldCoordTransf.x), std::max(minMaxWowWorldCoordTransf.x, maxMinWowWorldCoordTransf.x)),
            std::max(std::max(minWowWorldCoordTransf.y, maxWowWorldCoordTransf.y), std::max(minMaxWowWorldCoordTransf.y, maxMinWowWorldCoordTransf.y))
        );

        std::cout << "XToYCoef = " << XToYCoef << std::endl;

        m_chunkStartX = std::floor(((minOrthoMapCoord.y) / (2.0f*GetOrthoDimension()))) ;
        m_chunkWidth = std::floor(((maxOrthoMapCoord.y-minOrthoMapCoord.y) / (2.0f*GetOrthoDimension())) + 0.99 ) + 1;
        m_chunkStartY = std::floor(((minOrthoMapCoord.x) / (2.0f*GetOrthoDimension()*XToYCoef))) +1 ; //Y goes in reverse, that's why there is - here instead of +
        m_chunkHeight = std::floor(((maxOrthoMapCoord.x-minOrthoMapCoord.x) /(2.0f*GetOrthoDimension()*XToYCoef))+ 0.99) +1;

        std::cout << "m_chunkStartX = " << m_chunkStartX << " m_chunkWidth = " << m_chunkWidth
            << " m_chunkStartY = " << m_chunkStartY << " m_chunkHeight = " << m_chunkHeight << std::endl;
    }

    m_x = m_chunkStartX;
    m_y = m_chunkStartY;

//    m_x = -8;
//    m_y = -1;

    currentScenario = scenarioId;

    setupCameraData();
}

void MinimapGenerator::calcXtoYCoef() {

    //Code that tries to calc the step on y and x
    if (!XToYCoefCalculated)
    {
        auto orthoProjection = getOrthoMatrix();

        auto xyDelta= mathfu::vec4(0,0,0,0);
        const mathfu::vec4 vec4Top = {1, 1, 1, 1};
        const mathfu::vec4 vec4TopBottom = {1, -1, 1, 1};
        const mathfu::vec4 vec4Bottom = {-1, -1, 1, 1};
        const mathfu::vec4 vec4BottomTop = {-1, 1, 1, 1};


        //Round1
        {
            mathfu::vec3 point = mathfu::vec3(0,
                                              0,
                                              2000);
            point = MathHelper::RotationZ(M_PI / 4) * point;
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


            auto viewProj = cameraMatricesRendering->lookAtMat * orthoProjection;
//        auto viewProj =  cameraMatrices->lookAtMat * orthoProjection;

            mathfu::vec4 vec4TopTrans = viewProj.Inverse() * vec4Top;
            vec4TopTrans *= (1.0f / vec4TopTrans[3]);

            mathfu::vec4 vec4TopBottomTrans = viewProj.Inverse() * vec4TopBottom;
            vec4TopBottomTrans *= (1.0f / vec4TopBottomTrans[3]);

            mathfu::vec4 vec4BotTrans = viewProj.Inverse() * vec4Bottom;
            vec4BotTrans *= (1.0f / vec4BotTrans[3]);

            mathfu::vec4 vec4BottomTopTrans = viewProj.Inverse() * vec4BottomTop;
            vec4BottomTopTrans *= (1.0f / vec4BottomTopTrans[3]);

            vec4TopTrans = MathHelper::RotationZ(M_PI / 4).Inverse() * vec4TopTrans;
            vec4TopBottomTrans = MathHelper::RotationZ(M_PI / 4).Inverse() * vec4TopBottomTrans;
            vec4BotTrans = MathHelper::RotationZ(M_PI / 4).Inverse() * vec4BotTrans;
            vec4BottomTopTrans = MathHelper::RotationZ(M_PI / 4).Inverse() * vec4BottomTopTrans;

            float minX = std::min<float>(std::min<float>(vec4TopTrans.x, vec4TopBottomTrans.x),
                                         std::min<float>(vec4BotTrans.x, vec4BottomTopTrans.x));
            float minY = std::min<float>(std::min<float>(vec4TopTrans.y, vec4TopBottomTrans.y),
                                         std::min<float>(vec4BotTrans.y, vec4BottomTopTrans.y));
            float maxX = std::max<float>(std::max<float>(vec4TopTrans.x, vec4TopBottomTrans.x),
                                         std::max<float>(vec4BotTrans.x, vec4BottomTopTrans.x));
            float maxY = std::max<float>(std::max<float>(vec4TopTrans.y, vec4TopBottomTrans.y),
                                         std::max<float>(vec4BotTrans.y, vec4BottomTopTrans.y));

            XToYCoef = (maxY - minY) / (maxX - minX);
            XToYCoefCalculated = true;


            std::cout << "vec4TopTrans1 = (" << vec4TopTrans[0] << " " << vec4TopTrans[1] << " " << vec4TopTrans[2]
                      << std::endl;

            std::cout << "vec4TopBottomTrans1 = (" << vec4TopBottomTrans[0] << " " << vec4TopBottomTrans[1] << " " << vec4TopBottomTrans[2]
                      << std::endl;
            std::cout << "vec4BotTrans1 = (" << vec4BotTrans[0] << " " << vec4BotTrans[1] << " " << vec4BotTrans[2]
                      << std::endl;
            std::cout << "vec4BottomTopTrans1 = (" << vec4BottomTopTrans[0] << " " << vec4BottomTopTrans[1] << " " << vec4BottomTopTrans[2]
                      << std::endl;


            xyDelta = mathfu::vec4(
                vec4TopTrans[0] - vec4BotTrans[0],
                vec4TopTrans[1] - vec4BotTrans[1],
                0, 1);

            std::cout << "xyDelta = " << xyDelta[0] << " " << xyDelta[1] << std::endl;
        }

//
//        //Round 2
//        {
//            auto point = mathfu::vec3(MathHelper::TILESIZE *0.25* (1.73f),
//            0,
//                                 2000);
//            point = MathHelper::RotationZ(M_PI / 4) * point;
//
//            std::cout << "point = (" << point.x << " " << point.y << " " << point.z << std::endl;
//
//            m_apiContainer->camera->setCameraPos(
//                point.x, point.y, point.z
//            );
//            ((FirstPersonOrthoStaticCamera *) m_apiContainer->camera.get())->setCameraLookAt(
//                point.x + 1, point.y + 1, point.z - 1
//            );
//            m_apiContainer->camera->tick(0);
//
//            float nearPlane = 1.0;
//            float fov = toRadian(45.0);
//
//            float canvasAspect = (float)m_width / (float)m_height;
//            float farPlaneRendering = m_apiContainer->getConfig()->farPlane;
//            float farPlaneCulling = m_apiContainer->getConfig()->farPlaneForCulling;
//
//            HCameraMatrices cameraMatricesRendering = m_apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
//
//            auto viewProj = cameraMatricesRendering->lookAtMat * orthoProjection;
//
//            mathfu::vec4 vec4TopTrans = viewProj.Inverse() * vec4Top;
//            vec4TopTrans *= (1.0f / vec4TopTrans[3]);
//
//            mathfu::vec4 vec4TopBottomTrans = viewProj.Inverse() * vec4TopBottom;
//            vec4TopBottomTrans *= (1.0f / vec4TopBottomTrans[3]);
//
//            mathfu::vec4 vec4BotTrans = viewProj.Inverse() * vec4Bottom;
//            vec4BotTrans *= (1.0f / vec4BotTrans[3]);
//
//            mathfu::vec4 vec4BottomTopTrans = viewProj.Inverse() * vec4BottomTop;
//            vec4BottomTopTrans *= (1.0f / vec4BottomTopTrans[3]);
//
//            std::cout << "vec4TopTrans2 = (" << vec4TopTrans[0] << " " << vec4TopTrans[1] << " " << vec4TopTrans[2]
//                      << std::endl;
//
//            std::cout << "vec4TopBottomTrans2 = (" << vec4TopBottomTrans[0] << " " << vec4TopBottomTrans[1] << " " << vec4TopBottomTrans[2]
//                      << std::endl;
//            std::cout << "vec4BotTrans2 = (" << vec4BotTrans[0] << " " << vec4BotTrans[1] << " " << vec4BotTrans[2]
//                      << std::endl;
//            std::cout << "vec4BottomTopTrans2 = (" << vec4BottomTopTrans[0] << " " << vec4BottomTopTrans[1] << " " << vec4BottomTopTrans[2]
//                      << std::endl;
//        }

    }


}

void MinimapGenerator::setupCameraData() {
    mathfu::vec3 lookAtPoint = mathfu::vec3(GetOrthoDimension() * 2.0f *XToYCoef* m_y,
                                      GetOrthoDimension() * 2.0f * m_x,
                                      1);

    lookAtPoint = MathHelper::RotationZ(M_PI/4) * lookAtPoint;


    std::cout << "lookAtPoint = (" << lookAtPoint.x << ", " << lookAtPoint.y << ", " << lookAtPoint.z << ") " << std::endl;


    mathfu::vec3 lookAtVec3 = {1,1,-1};
    lookAtPoint -= (4000.0f*lookAtVec3);
    mathfu::vec3 cameraPos = lookAtPoint-(2000.0f*lookAtVec3);

    std::cout << "cameraPos = (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ") " << std::endl;

    m_apiContainer->camera->setCameraPos(
        cameraPos.x, cameraPos.y, cameraPos.z
    );
    ((FirstPersonOrthoStaticCamera*)m_apiContainer->camera.get())->setCameraLookAt(
        lookAtPoint.x, lookAtPoint.y, lookAtPoint.z
    );
    m_apiContainer->camera->tick(0);
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
    std::string fileName = "testMinimap\\test_"+std::to_string(m_y-m_chunkStartY)+"_"+std::to_string((m_x-m_chunkStartX))+".png";

    std::vector<uint8_t> buffer = std::vector<uint8_t>(m_width*m_height*4+1);
    saveDataFromDrawStage(lastFrameIt->target, fileName, m_width, m_height, buffer);


    m_x++;
    if (m_x >= m_chunkWidth + m_chunkStartX)  {
        m_y++;
        m_x = m_chunkStartX;
    }
    std::cout << "m_x = " << m_x << " out of (" << m_chunkStartX+m_chunkWidth << ") m_y = " << m_y << " out of (" << m_chunkStartY+m_chunkHeight << ")" << std::endl;

    setupCameraData();
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

    if (m_currentScene != nullptr) {
        ViewPortDimensions dimensions = {{0, 0}, {m_width, m_height}};

        HFrameBuffer fb = nullptr;
        fb = m_apiContainer->hDevice->createFrameBuffer(m_width, m_height, {ITextureFormat::itRGBA},ITextureFormat::itDepth32, 4);

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
    return (m_y >= (m_chunkHeight + m_chunkStartY));
}

float MinimapGenerator::GetOrthoDimension() {
//    return MathHelper::TILESIZE*0.25*0.5f;
    return MathHelper::TILESIZE*0.25;
}

mathfu::mat4 MinimapGenerator::getOrthoMatrix() {
    return mathfu::mat4::Ortho(
        -GetOrthoDimension(),GetOrthoDimension(),
        -GetOrthoDimension(),GetOrthoDimension(),
        1,8000
    );
}

