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
#include "../../../../wowViewerLib/src/engine/camera/m2TiedCamera.h"


void updateCameraPosOnLoad(const std::shared_ptr<M2Object> &m2Object, const std::shared_ptr<ICamera> &camera, std::vector<std::shared_ptr<ICamera>> &cameraList) {
    if (m2Object->isMainDataLoaded()) {
        CAaBox aabb = m2Object->getColissionAABB();
        if ((mathfu::vec3(aabb.max) - mathfu::vec3(aabb.min)).LengthSquared() < 0.001 ) {
            aabb = m2Object->getAABB();
        }

        auto max = aabb.max;
        auto min = aabb.min;

        if ((mathfu::vec3(aabb.max) - mathfu::vec3(aabb.min)).LengthSquared() < 20000) {

            mathfu::vec3 modelCenter = mathfu::vec3(
                ((max.x + min.x) / 2.0f),
                ((max.y + min.y) / 2.0f),
                ((max.z + min.z) / 2.0f)
            );

            if ((max.z - modelCenter.z) > (max.y - modelCenter.y)) {
                camera->setCameraPos((max.z - modelCenter.z) / tan(M_PI * 19.0f / 180.0f), 0, 0);

            } else {
                camera->setCameraPos((max.y - modelCenter.y) / tan(M_PI * 19.0f / 180.0f), 0, 0);
            }
            camera->setCameraLookAt(0,0,0);
            camera->setCameraOffset(modelCenter.x, modelCenter.y, modelCenter.z);
        } else {
            camera->setCameraPos(1.0,0,0);
            camera->setCameraOffset(0,0,0);
        }

        //Create cameras
        {
            auto cameraNum = m2Object->getCameraNum();
            for (int i = 0; i < cameraNum; i++) {
                auto newCamera = std::make_shared<m2TiedCamera>(m2Object, i);
                cameraList.push_back(newCamera);
            }
        }

#ifdef __EMSCRIPTEN__
        std::vector <int> availableAnimations;
        m2Object->getAvailableAnimation(availableAnimations);

        supplyAnimationList(&availableAnimations[0], availableAnimations.size());

        std::vector<int> meshIds;
        m2Object->getMeshIds(meshIds);

        supplyMeshIds(&meshIds[0], meshIds.size());
#endif
    }
}

mathfu::mat4 getInfZMatrix(float f, float aspect) {
    return mathfu::mat4(
        f / aspect, 0.0f,  0.0f,  0.0f,
        0.0f,    f,  0.0f,  0.0f,
        0.0f, 0.0f,  1, -1.0f,
        0.0f, 0.0f, 1,  0.0f);
}

inline HMapSceneParams createMapSceneParams(const HApiContainer &apiContainer,
                                     const std::shared_ptr<ICamera> &camera,
                                     float fov,
                                     const std::vector<RenderTargetParameters> &renderTargetParams,
                                     const std::shared_ptr<IScene> &currentScene) {

    auto result = std::make_shared<MapSceneParams>();
    result->scene = currentScene;

    float farPlaneRendering = apiContainer->getConfig()->farPlane;
    float farPlaneCulling = apiContainer->getConfig()->farPlaneForCulling;

    float nearPlane = 1.0;
    float fovR = toRadian(fov);

    {
        auto width = renderTargetParams[0].dimensions.maxs[0];
        auto height = renderTargetParams[0].dimensions.maxs[1];
        float canvasAspect = (float) width / (float) height;

        result->matricesForCulling = camera->getCameraMatrices(fovR, canvasAspect, nearPlane,
                                                                            farPlaneCulling);
    }

    bool isInfZSupported = camera->isCompatibleWithInfiniteZ();
    auto assignInfiniteZ = [&](auto renderTarget, auto canvasAspect) {
        float f = 1.0f / tan(fovR / 2.0f);
        renderTarget.cameraMatricesForRendering->perspectiveMat = getInfZMatrix(f, canvasAspect);
    };

    for (auto &targetParam : renderTargetParams) {
        auto width = targetParam.dimensions.maxs[0];
        auto height = targetParam.dimensions.maxs[1];
        float canvasAspect = (float)width / (float)height;

        auto &renderTarget = result->renderTargets.emplace_back();
        renderTarget.cameraMatricesForRendering = targetParam.camera->getCameraMatrices(fovR, canvasAspect, nearPlane, farPlaneCulling);
        renderTarget.viewPortDimensions = targetParam.dimensions;
        renderTarget.target = targetParam.target;
        if (isInfZSupported) assignInfiniteZ(renderTarget, canvasAspect);
    }

    result->clearColor = apiContainer->getConfig()->clearColor;

    return result;
}

SceneWindow::SceneWindow(const HApiContainer &api, bool renderToSwapChain) : m_api(api), m_renderToSwapChain(renderToSwapChain){
}

/*
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
*/

void SceneWindow::openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<Map>(m_api, mapId, wdtFileId);

    m_camera = std::make_shared<FirstPersonCamera>();
    m_camera->setCameraPos(x,y,z);
    m_camera->setMovementSpeed(movementSpeed);
}
void SceneWindow::openM2SceneByfdid(int m2Fdid, const std::vector<int> &replacementTextureIds) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    auto m2Scene = std::make_shared<M2Scene>(m_api, m2Fdid);
    m_currentScene = m2Scene;
    m2Scene->setReplaceTextureArray(m_sceneRenderer, replacementTextureIds);

    m_camera = std::make_shared<FirstPersonCamera>();
    m_camera->setMovementSpeed(movementSpeed);
    m_api->getConfig()->BCLightHack = false;
//
    m_camera->setCameraPos(0, 0, 0);

    {
        //Post load event for m2
        auto m2Object = m2Scene->getSceneM2();
        m2Object->addPostLoadEvent([m2Object, l_camera = m_camera, &l_cameraList = m_cameraList]() {
            updateCameraPosOnLoad(m2Object, l_camera, l_cameraList);
        });
    }
}

void SceneWindow::openM2SceneByName(const std::string &m2FileName, const std::vector<int> &replacementTextureIds) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());

    auto m2Scene = std::make_shared<M2Scene>(m_api, m2FileName);
    m_currentScene = m2Scene;
    m2Scene->setReplaceTextureArray(m_sceneRenderer, replacementTextureIds);

    m_camera = std::make_shared<FirstPersonCamera>();
    m_camera->setCameraPos(0, 0, 0);
    m_camera->setMovementSpeed(movementSpeed);

    {
        //Post load event for m2
        auto m2Object = m2Scene->getSceneM2();
        m2Object->addPostLoadEvent([m2Object, l_camera = m_camera, &l_cameraList = m_cameraList]() {
            updateCameraPosOnLoad(m2Object, l_camera, l_cameraList);
        });
    }
}

void SceneWindow::openWMOSceneByfdid(int WMOFdid) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<WmoScene>(m_api, WMOFdid);

    m_camera = std::make_shared<FirstPersonCamera>();
    m_camera->setCameraPos(0, 0, 0);
}
void SceneWindow::openWMOSceneByFilename(const std::string &wmoFileName) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<WmoScene>(m_api, wmoFileName);

    m_camera = std::make_shared<FirstPersonCamera>();
    m_camera->setCameraPos(0, 0, 0);
}

void SceneWindow::openMapByIdAndFilename(int mapId, const std::string &mapName, float x, float y, float z) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<Map>(m_api, mapId, mapName);

    m_camera = std::make_shared<FirstPersonCamera>();
    m_camera->setCameraPos(x,y,z);
    m_camera->setMovementSpeed(movementSpeed);
}



void SceneWindow::unload() {
    m_sceneRenderer = nullptr;
    m_currentScene = std::make_shared<NullScene>();
}

std::shared_ptr<MapRenderPlan> SceneWindow::getLastPlan() {
    return (m_sceneRenderer) ? m_sceneRenderer->getLastCreatedPlan() : nullptr;
}
const std::shared_ptr<ICamera> SceneWindow::getCamera() {
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex <= m_cameraList.size()) {
        return m_cameraList[m_currentCameraIndex];
    }

    return m_camera;
}

bool SceneWindow::hasRenderer() {
    return m_sceneRenderer != nullptr;
}

std::shared_ptr<IRenderView> SceneWindow::createRenderView() {
    if (m_sceneRenderer == nullptr) return nullptr;

    return m_sceneRenderer->createRenderView(true);
}

void
SceneWindow::render(double deltaTime,
                    float fov,
                    const HFrameScenario &scenario,
                    const std::shared_ptr<SceneWindow> &debugWindow,
                    const std::function<uint32_t()> &updateFrameNumberLambda
                    )
{
    if (!hasRenderer()) return;

    auto currentCamera = getCamera();
    if (!currentCamera) return;

    if (!m_renderToSwapChain && !m_renderView) {
        m_renderView = m_sceneRenderer->createRenderView(true);
    }

    currentCamera->tick(deltaTime * 1000.0f);

    if (m_api->getConfig()->pauseAnimation) {
        deltaTime = 0.0;
    }

    auto wowSceneFrameInput = std::make_shared<FrameInputParams<MapSceneParams>>();
    wowSceneFrameInput->delta = deltaTime * (1000.0f);

    std::shared_ptr<IRenderView> target = m_renderView;
    std::shared_ptr<IRenderView> debugTarget = nullptr;

    ViewPortDimensions l_dimension = m_dimension;
    ViewPortDimensions debugViewDimension = m_dimension;
    if (debugWindow && debugWindow->m_renderView) {
        debugTarget = debugWindow->m_renderView;

        debugViewDimension = debugWindow->m_dimension;

        if (m_api->getConfig()->swapMainAndDebug) {
            std::swap(target, debugTarget);
            std::swap(l_dimension, debugViewDimension);
        }
    }

    std::vector<RenderTargetParameters> renderTargetParams = {
        {
            currentCamera,
            l_dimension,
            target
        }
    };

    if (m_api->getConfig()->doubleCameraDebug) {
        auto &debugParams = renderTargetParams.emplace_back();
        debugParams.camera = debugWindow->getCamera();
        debugParams.dimensions = debugViewDimension;
        debugParams.target = debugTarget;
    }

    wowSceneFrameInput->frameParameters = createMapSceneParams(m_api,
                                                               currentCamera,
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
    auto screenShotRenderView = m_sceneRenderer->createRenderView(true);
    auto wowSceneScreenshotFrameInput = std::make_shared<FrameInputParams<MapSceneParams>>();
    wowSceneScreenshotFrameInput->delta = 0;

    wowSceneScreenshotFrameInput->frameParameters = createMapSceneParams(
        m_api,
        m_camera,
        fov,
        {{
             m_camera,
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
