//
// Created by Deamon on 1/29/2024.
//

#include "SceneWindow.h"
#include "../../../../wowViewerLib/src/renderer/mapScene/MapSceneRendererFactory.h"
#include "../../../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../../../wowViewerLib/src/engine/objects/scenes/m2Scene.h"
#include "../../../../wowViewerLib/src/engine/objects/scenes/wmoScene.h"
#include "../../../../wowViewerLib/src/engine/objects/scenes/NullScene.h"
#include "../../../screenshots/screenshotMaker.h"

mathfu::mat4 getInfZMatrix(float f, float aspect) {
    return mathfu::mat4(
        f / aspect, 0.0f,  0.0f,  0.0f,
        0.0f,    f,  0.0f,  0.0f,
        0.0f, 0.0f,  1, -1.0f,
        0.0f, 0.0f, 1,  0.0f);
}

HMapSceneParams createMapSceneParams(ApiContainer &apiContainer,
                                     float fov,
                                     const std::vector<RenderTargetParameters> &renderTargetParams,
                                     const std::shared_ptr<IScene> &currentScene) {

    auto result = std::make_shared<MapSceneParams>();
    result->scene = currentScene;

    float farPlaneRendering = apiContainer.getConfig()->farPlane;
    float farPlaneCulling = apiContainer.getConfig()->farPlaneForCulling;

    float nearPlane = 1.0;
    float fovR = toRadian(fov);

    {
        auto width = renderTargetParams[0].dimensions.maxs[0];
        auto height = renderTargetParams[0].dimensions.maxs[1];
        float canvasAspect = (float) width / (float) height;

        result->matricesForCulling = apiContainer.camera->getCameraMatrices(fovR, canvasAspect, nearPlane,
                                                                            farPlaneCulling);
    }

    bool isInfZSupported = apiContainer.camera->isCompatibleWithInfiniteZ();
    auto assignInfiniteZ = [&](auto renderTarget, auto canvasAspect) {
        float f = 1.0f / tan(fovR / 2.0f);
        renderTarget.cameraMatricesForRendering->perspectiveMat = getInfZMatrix(f, canvasAspect);
    };

    for (auto &targetParam : renderTargetParams) {
        auto width = targetParam.dimensions.maxs[0];
        auto height = targetParam.dimensions.maxs[1];
        float canvasAspect = (float)width / (float)height;

        auto &renderTarget = result->renderTargets.emplace_back();
        renderTarget.cameraMatricesForRendering = targetParam.camera->getCameraMatrices(fovR, canvasAspect, nearPlane, farPlaneCulling);;
        renderTarget.viewPortDimensions = targetParam.dimensions;
        renderTarget.target = targetParam.target;
        if (isInfZSupported) assignInfiniteZ(renderTarget, canvasAspect);
    }

    result->clearColor = apiContainer.getConfig()->clearColor;

    return result;
}

SceneWindow::SceneWindow(HApiContainer api) : m_api(api) {
}

bool SceneWindow::draw() {
    return false;
};

std::shared_ptr<IScene> setScene(const HApiContainer& apiContainer, int sceneType, const std::string& name, int cameraNum) {
    apiContainer->camera = std::make_shared<FirstPersonCamera>();
    if (sceneType == -1) {
        return std::make_shared<NullScene>();
    } else if (sceneType == 0) {
//        m_usePlanarCamera = cameraNum == -1;


        return std::make_shared<M2Scene>(apiContainer, name);
    } else if (sceneType == 1) {
        return std::make_shared<WmoScene>(apiContainer, name);
    } else if (sceneType == 2) {
        auto &adtFileName = name;

        size_t lastSlashPos = adtFileName.find_last_of("/");
        size_t underscorePosFirst = adtFileName.find_last_of("_");
        size_t underscorePosSecond = adtFileName.find_last_of("_", underscorePosFirst-1);
        std::string mapName = adtFileName.substr(lastSlashPos+1, underscorePosSecond-lastSlashPos-1);

        int i = std::stoi(adtFileName.substr(underscorePosSecond+1, underscorePosFirst-underscorePosSecond-1));
        int j = std::stoi(adtFileName.substr(underscorePosFirst+1, adtFileName.size()-underscorePosFirst-5));

        float adt_x_min = AdtIndexToWorldCoordinate(j);
        float adt_x_max = AdtIndexToWorldCoordinate(j+1);

        float adt_y_min = AdtIndexToWorldCoordinate(i);
        float adt_y_max = AdtIndexToWorldCoordinate(i+1);

        apiContainer->camera = std::make_shared<FirstPersonCamera>();
        apiContainer->camera->setCameraPos(
            (adt_x_min+adt_x_max) / 2.0,
            (adt_y_min+adt_y_max) / 2.0,
            200
        );

        return std::make_shared<Map>(apiContainer, adtFileName, i, j, mapName);
    }

    return nullptr;
}

void SceneWindow::openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<Map>(m_api, mapId, wdtFileId);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(x,y,z);
    m_api->camera->setMovementSpeed(movementSpeed);
}
void SceneWindow::openM2SceneByfdid(int m2Fdid, const std::vector<int> &replacementTextureIds) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    auto m2Scene = std::make_shared<M2Scene>(m_api, m2Fdid);
    m_currentScene = m2Scene;
    m2Scene->setReplaceTextureArray(m_sceneRenderer, replacementTextureIds);


    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setMovementSpeed(movementSpeed);
    m_api->getConfig()->BCLightHack = false;
//
    m_api->camera->setCameraPos(0, 0, 0);
}



void SceneWindow::openM2SceneByName(const std::string &m2FileName, const std::vector<int> &replacementTextureIds) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());

    auto m2Scene = std::make_shared<M2Scene>(m_api, m2FileName);
    m_currentScene = m2Scene;
    m2Scene->setReplaceTextureArray(m_sceneRenderer, replacementTextureIds);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(0, 0, 0);
    m_api->camera->setMovementSpeed(movementSpeed);
}

void SceneWindow::openWMOSceneByfdid(int WMOFdid) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<WmoScene>(m_api, WMOFdid);
    m_api->camera->setCameraPos(0, 0, 0);
}
void SceneWindow::openWMOSceneByFilename(const std::string &wmoFileName) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<WmoScene>(m_api, wmoFileName);
    m_api->camera->setCameraPos(0, 0, 0);
}

void SceneWindow::openMapByIdAndFilename(int mapId, const std::string &mapName, float x, float y, float z) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<Map>(m_api, mapId, mapName);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(x,y,z);
    m_api->camera->setMovementSpeed(movementSpeed);
}



void SceneWindow::unload() {
    m_sceneRenderer = nullptr;
    m_currentScene = std::make_shared<NullScene>();
}

std::shared_ptr<MapRenderPlan> SceneWindow::getLastPlan() {
    return (m_sceneRenderer) ? m_sceneRenderer->getLastCreatedPlan() : nullptr;
}

bool SceneWindow::hasRenderer() {
    return m_sceneRenderer != nullptr;
}

std::shared_ptr<IRenderView> SceneWindow::createRenderView() {
    if (m_sceneRenderer == nullptr) return nullptr;

    return m_sceneRenderer->createRenderView(640, 480, true);
}

void
SceneWindow::render(double deltaTime,
                    float fov,
                    const HFrameScenario &scenario,
                    const ViewPortDimensions &dimension,
                    const std::function<uint32_t()> &updateFrameNumberLambda,
                    const std::shared_ptr<IRenderView> &debugRenderView,
                    uint32_t debugViewWidth,
                    uint32_t debugViewHeight)
{
    if (!hasRenderer())  return;

    auto wowSceneFrameInput = std::make_shared<FrameInputParams<MapSceneParams>>();
    wowSceneFrameInput->delta = deltaTime * (1000.0f);

    std::shared_ptr<IRenderView> target = nullptr;
    std::shared_ptr<IRenderView> debugTarget = nullptr;

    ViewPortDimensions l_dimension = dimension;
    ViewPortDimensions debugViewDimension = dimension;
    if (debugRenderView) {
        debugTarget = debugRenderView;

        ViewPortDimensions debugViewDimension = {
            {0, 0},
            {static_cast<unsigned int>(debugViewWidth),
                static_cast<unsigned int>(debugViewHeight)}
        };

        if (m_api->getConfig()->swapMainAndDebug) {
            std::swap(target, debugTarget);
            std::swap(l_dimension, debugViewDimension);
        }
    }

    std::vector<RenderTargetParameters> renderTargetParams = {
        {
            m_api->camera,
            dimension,
            target
        }
    };

    if (m_api->getConfig()->doubleCameraDebug) {
        auto &debugParams = renderTargetParams.emplace_back();
        debugParams.camera = m_api->debugCamera;
        debugParams.dimensions = debugViewDimension;
        debugParams.target = debugTarget;
    }

    wowSceneFrameInput->frameParameters = createMapSceneParams(*m_api,
                                                               fov,
                                                               renderTargetParams,
                                                               m_currentScene);

    scenario->cullFunctions.push_back(
        m_sceneRenderer->createCullUpdateRenderChain(wowSceneFrameInput, updateFrameNumberLambda)
    );
}

void
SceneWindow::makeScreenshot(float fov,
                            uint32_t screenShotWidth, uint32_t screenShotHeight,
                            const std::string &screenshotFilename,
                            const HFrameScenario &scenario,
                            const std::function<uint32_t()> &updateFrameNumberLambda)
{
    auto screenShotRenderView = m_sceneRenderer->createRenderView(screenShotWidth, screenShotHeight, true);
    auto wowSceneScreenshotFrameInput = std::make_shared<FrameInputParams<MapSceneParams>>();
    wowSceneScreenshotFrameInput->delta = 0;

    wowSceneScreenshotFrameInput->frameParameters = createMapSceneParams(
        *m_api,
        fov,
        {{
             m_api->camera,
             {
                 {0, 0},
                 {static_cast<unsigned int>(screenShotWidth), static_cast<unsigned int>(screenShotHeight)}
             },
             screenShotRenderView
         }},
        m_currentScene
    );

    auto processingFrame = updateFrameNumberLambda();

    scenario->cullFunctions.push_back(
        m_sceneRenderer->createCullUpdateRenderChain(wowSceneScreenshotFrameInput, updateFrameNumberLambda)
    );

    scenario->onFinish.push_back([screenShotRenderView, screenShotWidth, screenShotHeight, screenshotFilename, processingFrame]() {
        saveDataFromDrawStage([screenShotRenderView, processingFrame](int x, int y, int width, int height, uint8_t* data){
            screenShotRenderView->readRGBAPixels(processingFrame, x, y, width, height, data);
        }, screenshotFilename, screenShotWidth, screenShotHeight);
    });


    scenario->onFinish.push_back([screenShotRenderView, screenShotWidth, screenShotHeight, screenshotFilename, processingFrame]() {
        saveDataFromDrawStage([screenShotRenderView, processingFrame](int x, int y, int width, int height, uint8_t* data){
            screenShotRenderView->readRGBAPixels(processingFrame, x, y, width, height, data);
        }, screenshotFilename, screenShotWidth, screenShotHeight);
    });
}
