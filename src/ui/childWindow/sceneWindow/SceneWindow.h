//
// Created by Deamon on 1/29/2024.
//

#ifndef AWEBWOWVIEWERCPP_SCENEWINDOW_H
#define AWEBWOWVIEWERCPP_SCENEWINDOW_H

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <array>
#include <map>
#include <set>
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/renderer/mapScene/MapSceneRenderer.h"
#include "../../../../wowViewerLib/src/engine/objects/iScene.h"
#include "../../../../wowViewerLib/src/renderer/frame/SceneScenario.h"
#include "../../renderer/uiScene/materials/UIMaterial.h"
#include "../../renderer/uiScene/FrontendUIRenderer.h"

struct RenderTargetParameters {
    std::shared_ptr<ICamera> camera;
    ViewPortDimensions dimensions;
    std::shared_ptr<IRenderView> target;
};

class WorldObjectManager;

class SceneWindow : public std::enable_shared_from_this<SceneWindow> {
public:
    SceneWindow(const HApiContainer &api, bool renderToSwapChain, const std::shared_ptr<FrontendUIRenderer> &uiRenderer, bool forceForwardRendering);
    virtual ~SceneWindow();

    void openMapByIdAndFilename(int mapId, const std::string &mapName, float x, float y, float z, int timeOverride);
    void openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z, int timeOverride);

    void openWMOSceneByfdid(int WMOFdid);
    void openWMOSceneByFilename(const std::string &wmoFileName);
    void openM2SceneByfdid(int m2Fdid, const std::vector<int> &replacementTextureIds, float size = 1.0f);
    void openM2SceneByName(const std::string &m2FileName, const std::vector<int> &replacementTextureIds);

    void unload();

    int getMapId() { return m_mapId; };

    std::shared_ptr<MapRenderPlan> getLastPlan();
    const std::shared_ptr<ICamera> &getCamera();

    // Returns the current Map scene's WorldObjectManager, or nullptr if the current
    // scene isn't a Map (or has none set, e.g. M2/WMO preview scenes).
    std::shared_ptr<WorldObjectManager> getWorldObjectManager();

    // Sky scene ids of the current Map scene's WDL, grouped by the player condition
    // that gates them (SkySceneXPlayerCondition db2). Empty when the current scene
    // isn't a Map or the map has no WDL. Built once at WDL load and static for the
    // WDL's lifetime.
    const std::map<int, std::set<int>> &getSkyScenesByPlayerCondition();

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

    std::shared_ptr<IVideoRecordingContext> startVideoRecording(uint32_t framebufferWidth, uint32_t framebufferHeight,
                                                                uint32_t outputWidth, uint32_t outputHeight,
                                                                const std::string &videoFilename);
    void processVideoRecording(const std::shared_ptr<IVideoRecordingContext> &videoRecordingContext,
                               const HFrameScenario &scenario,
                               float fov,
                               const std::function<uint32_t()> &updateFrameNumberLambda);

    void setViewPortDimensions(const ViewPortDimensions &dimensions) {
        m_dimension = dimensions;
    }

    // Request an object-id pick at the given pixel (in this window's render-target pixel space),
    // consumed and cleared on the next render() call. isHoverPeek marks a passive hover-only
    // query (e.g. nameplate tooltips) that must not change the actual selection.
    //
    // An explicit request (right-click / Ctrl-hover-select) always takes priority: a hover-peek
    // request will not overwrite an explicit request that's still pending/unconsumed, since the
    // continuous per-frame hover-peek call would otherwise always win the race against the
    // event-driven (mouse-move/click) explicit request and silently swallow every selection.
    void requestPick(int x, int y, bool isHoverPeek = false) {
        if (isHoverPeek && m_pendingPickRequest && !m_pendingPickIsHoverPeek) {
            return;
        }
        m_pendingPickRequest = {{x, y}};
        m_pendingPickIsHoverPeek = isHoverPeek;
    }

    int getCurrentCameraIndex() {return m_currentCameraIndex;}
    int getCurrentCameraCount() {return m_cameraList.size();}
    void setCurrentCameraIndex(int i) {
        if (i < m_cameraList.size())
            m_currentCameraIndex = i;
        else
            m_currentCameraIndex = -1;
    }

    virtual void setSelectedMat(uint8_t matIndex) {};
    virtual std::vector<std::tuple<std::string, std::shared_ptr<IUIMaterial>>> getMaterials() {
        return {};
    };
private:
    std::shared_ptr<MapSceneRenderer> m_sceneRenderer = nullptr;
    std::shared_ptr<IScene> m_currentScene = nullptr;
    bool m_renderToSwapChain = true;

    int m_mapId = -1;

    HApiContainer createNewApiContainer();

private:
    void createMaterials();

    bool m_needToUpdateMaterials = true;
    typedef std::list<std::function<void()>>::const_iterator OnUpdateIter;
    std::unique_ptr<OnUpdateIter> iteratorUnique = nullptr;
protected:
    float movementSpeed = 0.3;
    HApiContainer m_api;
    ViewPortDimensions m_dimension = {{0,0}, {0,0}};
    std::shared_ptr<ICamera> m_camera = nullptr;

    int m_currentCameraIndex = -1;
    std::vector<std::shared_ptr<ICamera>> m_cameraList;

    std::shared_ptr<FrontendUIRenderer> m_uiRenderer;

    bool m_forceForwardRendering = false;

    std::shared_ptr<IRenderView> m_renderView = nullptr;
    std::vector<std::tuple<std::string, std::array<std::shared_ptr<IUIMaterial>, IDevice::MAX_FRAMES_IN_FLIGHT>>> materials = {};

    std::optional<std::array<int, 2>> m_pendingPickRequest;
    bool m_pendingPickIsHoverPeek = false;
};


#endif //AWEBWOWVIEWERCPP_SCENEWINDOW_H
