//
// Created by deamon on 20.12.19.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUI_H
#define AWEBWOWVIEWERCPP_FRONTENDUI_H


#include "imguiLib/imgui.h"
#include <fileBrowser/imfilebrowser.h>
#include "../../wowViewerLib/src/include/database/dbStructs.h"
#include "../../wowViewerLib/src/engine/objects/iScene.h"


class FrontendUI : public IScene {
//Implementation of iInnerSceneApi
public:
    FrontendUI(ApiContainer *api) {
        m_api = api;
    }
    ~FrontendUI() override {};
    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void setAnimationId(int animationId) override {};

    void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) override;

    void checkCulling(HCullStage cullStage) override {};


    void doPostLoad(HCullStage cullStage) override {};
    void update(HUpdateStage updateStage) override {};
    void updateBuffers(HCullStage cullStage) override {};

    int getCameraNum() override {return 0;};
    std::shared_ptr<ICamera> createCamera(int cameraNum) {return nullptr;};

private:
    std::function <bool(std::string cascPath)> openCascCallback = nullptr;

    std::function <void(int mapId, int wdtFileId, float x, float y, float z)> openSceneByfdid = nullptr;
    std::function <void(int WMOFdid)> openWMOSceneByfdid = nullptr;
    std::function <void(int m2Fdid, std::vector<int> &replacementTextureIds)> openM2SceneByfdid = nullptr;
    std::function <void(std::string m2FileName, std::vector<int> &replacementTextureIds)> openM2SceneByName = nullptr;

    std::function <void(float &cameraX,float &cameraY,float &cameraZ)> getCameraPos = nullptr;

    void getAdtSelectionMinimap(int wdtFileDataId) {
        m_wdtFile = m_api->cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);
    };
    void getMapList(std::vector<MapRecord> &mapList);
    std::string getCurrentAreaName();
    bool fillAdtSelectionminimap(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap, bool &wdtFileExists);


    std::function<void()> unloadScene;
    std::function<int()> getCameraNumCallback;
    std::function<bool(int cameraNum)> setNewCameraCallback;

    std::array<std::array<HGTexture, 64>, 64> adtSelectionMinimap;

    void emptyMinimap() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                adtSelectionMinimap[i][j] = nullptr;
            }
        }
    }


    std::array<char, 128> filterText = {0};
    bool refilterIsNeeded = false;
    void filterMapList(std::string text);

    ImGui::FileBrowser fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory);

    bool show_demo_window = true;
    bool show_another_window = true;
    bool showCurrentStats = true;
    bool showSelectMap = false;
    bool showSettings = false;
    bool showQuickLinks = false;
    bool showAboutWindow = false;
//  c bool showWorldPosTooltip = false;

    bool cascOpened = false;
    bool mapCanBeOpened = true;
    bool adtMinimapFilled = false;

    float minimapZoom = 1;
    float farPlane = 200;
    int currentTime = 0;
    float movementSpeed = 1;
    int  threadCount = 4;
    int  quickSortCutoff = 100;
    float prevMinimapZoom = 1;
    int prevMapId = -1;
    bool isWmoMap = false;
    bool useGaussBlur = true;

    bool useTimedGlobalLight = true;
    bool useM2AmbientLight = false;

    int currentCameraNum = -1;

    std::array<float, 3> exteriorAmbientColor = {1, 1, 1};
    std::array<float, 3> exteriorHorizontAmbientColor = {1, 1, 1};
    std::array<float, 3> exteriorGroundAmbientColor= {1, 1, 1};
    std::array<float, 3> exteriorDirectColor = {0, 0, 0};

    MapRecord prevMapRec;

    float worldPosX = 0;
    float worldPosY = 0;
    float worldPosZ = 0;

    ApiContainer * m_api;
    HGTexture fontTexture;

    HWdtFile m_wdtFile = nullptr;

    std::vector<MapRecord> mapList = {};
    std::vector<MapRecord> filteredMapList = {};
    std::vector<std::vector<std::string>> mapListStringMap = {};

public:
    void overrideCascOpened(bool value) {
        cascOpened = value;
    }
    void initImgui(GLFWwindow* window);

    void composeUI();
    void newFrame();

    bool getStopMouse();
    bool getStopKeyboard();

    void setOpenCascStorageCallback(std::function <bool(std::string cascPath)> callback);
    void setOpenSceneByfdidCallback(std::function <void(int mapId, int wdtFileId, float x, float y, float z)> callback);
    void setOpenWMOSceneByfdidCallback(std::function <void(int wmoFDid)> callback);
    void setOpenM2SceneByfdidCallback(std::function <void(int m2FDid, std::vector<int> &replacementTextureIds)> callback);
    void setOpenM2SceneByFilenameCallback(std::function<void(std::string, std::vector<int>&)> callback);
    void setGetCameraPos( std::function <void(float &cameraX,float &cameraY,float &cameraZ)> callback);

    void setUnloadScene( std::function <void()> callback) {
        unloadScene = callback;
    };

    void setGetCameraNum(std::function <int()> callback);
    void setSelectNewCamera(std::function <bool(int cameraNum)> callback);
    void showMainMenu();

    void showMapSelectionDialog();

    void showAdtSelectionMinimap();
    void showSettingsDialog();
    void showQuickLinksDialog();
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
