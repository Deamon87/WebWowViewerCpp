//
// Created by deamon on 20.12.19.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUI_H
#define AWEBWOWVIEWERCPP_FRONTENDUI_H

#ifdef __ANDROID_API__
#include <android/native_window.h>
#else
#include <GLFW/glfw3.h>
#endif

#include "imguiLib/imgui.h"
#include <fileBrowser/imfilebrowser.h>
#include "../../wowViewerLib/src/include/database/dbStructs.h"
#include "../../wowViewerLib/src/engine/objects/iScene.h"
#include "childWindow/mapConstructionWindow/mapConstructionWindow.h"
#include "../minimapGenerator/minimapGenerator.h"
#include "../persistance/CascRequestProcessor.h"
#include "../minimapGenerator/storage/CMinimapDataDB.h"
#include "../exporters/dataExporter/DataExporterClass.h"
#include "childWindow/databaseUpdateWorkflow/DatabaseUpdateWorkflow.h"
#include "childWindow/minimapGeneratonWindow/MinimapGenerationWindow.h"


class FrontendUI : public IScene, public std::enable_shared_from_this<FrontendUI> {
//Implementation of iInnerSceneApi
public:
    void createDefaultprocessor();
    void createDatabaseHandler();

    FrontendUI(HApiContainer api, HRequestProcessor processor) {
        m_api = api;
        m_processor = processor;

        this->createDatabaseHandler();
        //this->createDefaultprocessor();

    }
    ~FrontendUI() override {
        fileDialog.Close();
        createFileDialog.Close();
        ImGui::DestroyContext(this->imguiContext);
    };
    std::shared_ptr<FrontendUI> getShared()
    {
        return shared_from_this();
    }

    HRequestProcessor getProcessor() {
        return m_processor;
    }

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void setMeshIdArray(std::vector<uint8_t> &meshIds) override {};
    void setAnimationId(int animationId) override {};
    void setMeshIds(std::vector<uint8_t> &meshIds) override {};

    void produceDrawStage(HDrawStage &resultDrawStage, std::vector<HUpdateStage> &updateStages) override;
    void produceUpdateStage(HUpdateStage &updateStage) override;

    void checkCulling(HCullStage &cullStage) override {};


    void doPostLoad(HCullStage &cullStage) override {};

    void update(HUpdateStage &updateStage) {};
    void updateBuffers(HUpdateStage &updateStage) override {};


    int getCameraNum() override {return 0;};
    std::shared_ptr<ICamera> createCamera(int cameraNum) override {return nullptr;};
    void resetAnimation() override {};

    HFrameScenario createFrameScenario(int canvWidth, int canvHeight, double deltaTime);

    void setUIScale(float scale) {
        uiScale = scale;
    }
    int getWindowWidth() const {
        return windowWidth;
    }
    int getWindowHeight() const {
        return windowHeight;
    }
    void setWindowSize(int width, int height) {
        windowWidth = width;
        windowHeight = height;
    }
private:
    ImGuiContext* imguiContext = nullptr;

    std::array<HCullStage, 4> m_cullstages = {};

    float uiScale = 1;

    std::shared_ptr<IScene> currentScene = nullptr;

    bool tryOpenCasc(std::string &cascPath, BuildDefinition &buildDef);

    void openWMOSceneByfdid(int WMOFdid);
    void openM2SceneByfdid(int m2Fdid, std::vector<int> &replacementTextureIds);
    void openM2SceneByName(std::string m2FileName, std::vector<int> &replacementTextureIds);

    void getCameraPos(float &cameraX,float &cameraY,float &cameraZ);
    void getDebugCameraPos(float &cameraX,float &cameraY,float &cameraZ);
    void makeScreenshotCallback(std::string fileName, int width, int height);

    void getAdtSelectionMinimap(int wdtFileDataId);
    void getAdtSelectionMinimap(std::string wdtFilePath);
    void getMapList(std::vector<MapRecord> &mapList);
    std::string getCurrentAreaName();
    bool fillAdtSelectionminimap(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap, bool &wdtFileExists);


    void unloadScene();
    int getCameraNumCallback();
    bool setNewCameraCallback(int cameraNum);
    void resetAnimationCallback();

    std::array<std::array<HGTexture, 64>, 64> adtSelectionMinimap;

    void emptyMinimap() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                adtSelectionMinimap[i][j] = nullptr;
            }
        }
    }

    auto createMinimapGenerator();

    std::array<char, 128> filterText = {0};
    bool refilterIsNeeded = false;
    void filterMapList(std::string text);

    ImGui::FileBrowser fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory, true);
    ImGui::FileBrowser createFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename);

    bool show_demo_window = true;
    bool show_another_window = true;
    bool showCurrentStats = true;
    bool showSelectMap = false;
    bool showMakeScreenshot = false;
    bool showSettings = false;
    bool showQuickLinks = false;
    bool showAboutWindow = false;
    bool showMinimapGeneratorSettings = false;
//  c bool showWorldPosTooltip = false;

    bool showMapConstruction = false;

    bool cascOpened = false;
    bool mapCanBeOpened = true;
    bool adtMinimapFilled = false;

    int windowWidth = 640;
    int windowHeight = 480;

    float minimapZoom = 1;
    float farPlane = 200;
    int currentTime = 0;
    float movementSpeed = 1;
    int  threadCount = 4;
    int  quickSortCutoff = 100;
    float prevMinimapZoom = 1;
    int prevMapId = -1;
    bool isWmoMap = false;

    bool pauseAnimation = true;

    int lightSource = 0;

    bool disableGlow = false;
    int glowSource = 0;
    float customGlow = 0.0;

    int currentCameraNum = -1;

    std::array<float, 3> exteriorAmbientColor = {1, 1, 1};
    std::array<float, 3> exteriorHorizontAmbientColor = {1, 1, 1};
    std::array<float, 3> exteriorGroundAmbientColor= {1, 1, 1};
    std::array<float, 3> exteriorDirectColor = {0, 0, 0};

    MapRecord prevMapRec;

    float worldPosX = 0;
    float worldPosY = 0;
    float worldPosZ = 0;

    HApiContainer m_api;
    HRequestProcessor m_processor;
    HGTexture fontTexture;

    HWdtFile m_wdtFile = nullptr;

    std::vector<MapRecord> mapList = {};
    std::vector<MapRecord> filteredMapList = {};
    std::vector<std::vector<std::string>> mapListStringMap = {};

    int lastWidth = 100;
    int lastHeight = 100;

    bool needToMakeScreenshot = false;
    std::string screenshotFilename = "";
    HDrawStage screenshotDS = nullptr;
    int screenShotWidth = 100;
    int screenShotHeight = 100;
    int screenshotFrame = -1;

    std::shared_ptr<IExporter> exporter;
    int exporterFramesReady = 0;

   std::shared_ptr<MapConstructionWindow> m_mapConstructionWindow = nullptr;
   std::shared_ptr<MinimapGenerationWindow> m_minimapGenerationWindow = nullptr;


//Test export
    DataExporterClass *dataExporter = nullptr;

    std::shared_ptr<DatabaseUpdateWorkflow> m_databaseUpdateWorkflow = nullptr;

public:
    void overrideCascOpened(bool value) {
        cascOpened = value;
    }

#ifdef __ANDROID_API__
    void initImgui(ANativeWindow* window);
#else
    void initImgui(
#ifdef __ANDROID_API__
    ANativeWindow *window
#else
    GLFWwindow *window
#endif
);
#endif

    void composeUI();
    void newFrame();

    bool getStopMouse();
    bool getStopKeyboard();

    void setExperimentCallback(std::function <void()> callback);
    void showMainMenu();

    void showMapSelectionDialog();
    void showMapConstructionDialog();
    void showMakeScreenshotDialog();

    void showAdtSelectionMinimap();
    void showSettingsDialog();
    void showQuickLinksDialog();

    void showCurrentStatsDialog();

    void restartMinimapGenPreview();
    void showMinimapGenerationSettingsDialog();

    void openMapByIdAndFilename(int mapId, std::string mapName, float x, float y, float z);
    void openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z);

    void editComponentsForConfig(Config *config);
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
