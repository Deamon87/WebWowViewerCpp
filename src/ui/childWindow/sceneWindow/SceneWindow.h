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
    SceneWindow(HApiContainer api);
    bool draw();

    void openMapByIdAndFilename(int mapId, const std::string &mapName, float x, float y, float z);
    void openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z);

    void openWMOSceneByfdid(int WMOFdid);
    void openWMOSceneByFilename(const std::string &wmoFileName);
    void openM2SceneByfdid(int m2Fdid, const std::vector<int> &replacementTextureIds);
    void openM2SceneByName(const std::string &m2FileName, const std::vector<int> &replacementTextureIds);

    void unload();

    std::shared_ptr<MapRenderPlan> getLastPlan();

    bool hasRenderer();
    std::shared_ptr<IRenderView> createRenderView();
    void render(double deltaTime, float fov, const HFrameScenario &scenario,
                const ViewPortDimensions &dimension,
                const std::function<uint32_t()> &updateFrameNumberLambda,
                const std::shared_ptr<IRenderView> &debugRenderView,
                uint32_t debugViewWidth,
                uint32_t debugViewHeight);

    void makeScreenshot(float fov,
                        uint32_t screenShotWidth, uint32_t screenShotHeight,
                        const std::string &screenshotFilename,
                        const HFrameScenario &scenario,
                        const std::function<uint32_t()> &updateFrameNumberLambda);

private:
    std::shared_ptr<MapSceneRenderer> m_sceneRenderer = nullptr;
    std::shared_ptr<IScene> m_currentScene = nullptr;
    HApiContainer m_api;
protected:
    float movementSpeed = 1;

};


#endif //AWEBWOWVIEWERCPP_SCENEWINDOW_H
