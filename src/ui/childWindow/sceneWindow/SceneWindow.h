//
// Created by Deamon on 1/29/2024.
//

#ifndef AWEBWOWVIEWERCPP_SCENEWINDOW_H
#define AWEBWOWVIEWERCPP_SCENEWINDOW_H

#include <vector>
#include <string>
#include <memory>
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/renderer/mapScene/MapSceneRenderer.h"
#include "../../../../wowViewerLib/src/engine/objects/iScene.h"
#include "../../../../wowViewerLib/src/renderer/frame/SceneScenario.h"

struct RenderTargetParameters {
    std::shared_ptr<ICamera> camera;
    ViewPortDimensions dimensions;
    std::shared_ptr<IRenderView> target;
};

class SceneWindow {
public:
    SceneWindow(const HApiContainer &api, bool renderToSwapChain);

    void openMapByIdAndFilename(int mapId, const std::string &mapName, float x, float y, float z);
    void openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z);

    void openWMOSceneByfdid(int WMOFdid);
    void openWMOSceneByFilename(const std::string &wmoFileName);
    void openM2SceneByfdid(int m2Fdid, const std::vector<int> &replacementTextureIds);
    void openM2SceneByName(const std::string &m2FileName, const std::vector<int> &replacementTextureIds);

    void unload();

    std::shared_ptr<MapRenderPlan> getLastPlan();
    const std::shared_ptr<ICamera> &getCamera();

    bool hasRenderer();
    std::shared_ptr<IRenderView> createRenderView();
    void render(double deltaTime,
                float fov,
                const HFrameScenario &scenario,
                const std::shared_ptr<SceneWindow> &debugWindow,
                const std::function<uint32_t()> &updateFrameNumberLambda);

    void makeScreenshot(float fov,
                        uint32_t screenShotWidth, uint32_t screenShotHeight,
                        const std::string &screenshotFilename,
                        const HFrameScenario &scenario,
                        const std::function<uint32_t()> &updateFrameNumberLambda);

    void setViewPortDimensions(const ViewPortDimensions &dimensions) {
        m_dimension = dimensions;
    }

    int getCurrentCameraIndex() {return m_currentCameraIndex;}
    int getCurrentCameraCount() {return m_cameraList.size();}
    void setCurrentCameraIndex(int i) {
        if (i < m_cameraList.size())
            m_currentCameraIndex = i;
        else
            m_currentCameraIndex = -1;
    }
private:
    std::shared_ptr<MapSceneRenderer> m_sceneRenderer = nullptr;
    std::shared_ptr<IScene> m_currentScene = nullptr;
    bool m_renderToSwapChain = true;

protected:
    float movementSpeed = 1;
    HApiContainer m_api;
    ViewPortDimensions m_dimension = {{0,0}, {0,0}};
    std::shared_ptr<ICamera> m_camera = nullptr;

    int m_currentCameraIndex = -1;
    std::vector<std::shared_ptr<ICamera>> m_cameraList;

    std::shared_ptr<IRenderView> m_renderView = nullptr;
};


#endif //AWEBWOWVIEWERCPP_SCENEWINDOW_H
