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
#include "../minimapGenerator/minimapGenerator.h"
#include "../persistance/CascRequestProcessor.h"
#include "../minimapGenerator/storage/CMinimapDataDB.h"
#include "../exporters/dataExporter/DataExporterClass.h"
#include "childWindow/databaseUpdateWorkflow/DatabaseUpdateWorkflow.h"
#include "childWindow/minimapGeneratonWindow/MinimapGenerationWindow.h"
#include "../../wowViewerLib/src/exporters/IExporter.h"
#include "renderer/uiScene/IFrontendUIBufferCreate.h"
#include "renderer/uiScene/FrontendUIRenderer.h"
#include "../../wowViewerLib/src/renderer/mapScene/MapSceneRenderer.h"
#include "childWindow/BLPViewer.h"
#include "childWindow/keysUpdateWorkflow/KeysUpdateWorkflow.h"
#include "childWindow/textureRenderer/DebugRendererWindow.h"
#include "childWindow/fileListWindow/FileListWindow.h"
#include "childWindow/sceneWindow/SceneWindow.h"
#include "childWindow/m2Window/M2Window.h"


class FrontendUI : public IScene, public std::enable_shared_from_this<FrontendUI> {
public:
    void createDefaultprocessor();
    void createDatabaseHandler();

    FrontendUI(HApiContainer api);
    ~FrontendUI() override {
        fileDialog.Close();
        createFileDialog.Close();

        ImGui::DestroyContext(this->imguiContext);
    };

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

    std::shared_ptr<SceneWindow> getCurrentActiveScene() {
        return m_currentActiveScene;
    };
private:
    ImGuiContext* imguiContext = nullptr;
    std::shared_ptr<FrontendUIRenderer> m_uiRenderer;

    std::shared_ptr<IUIMaterial> fontMat;

    std::shared_ptr<SceneWindow> m_backgroundScene = nullptr;
    std::shared_ptr<SceneWindow> m_currentActiveScene = nullptr;
    std::weak_ptr<SceneWindow> m_lastActiveScene;

    float uiScale = 1;

    bool tryOpenCasc(std::string &cascPath, BuildDefinition &buildDef);



    void getCameraPos(mathfu::vec3 &camera, mathfu::vec3 &lookAt);
    void getDebugCameraPos(float &cameraX,float &cameraY,float &cameraZ);
    void makeScreenshotCallback(std::string fileName, int width, int height);

    void getAdtSelectionMinimap(int wdtFileDataId);
    void getAdtSelectionMinimap(std::string wdtFilePath);
    void getMapList(std::vector<MapRecord> &mapList);

    bool fillAdtSelectionminimap(bool &isWMOMap, bool &wdtFileExists);

    std::shared_ptr<SceneWindow> getOrCreateWindow();
    void unloadScene();
    void resetAnimationCallback();

    std::array<std::array<HGSamplableTexture, 64>, 64> adtSelectionMinimapTextures;
    std::array<std::array<std::shared_ptr<IUIMaterial>, 64>, 64> adtSelectionMinimapMaterials;
    std::vector<mapInnerZoneLightRecord> m_zoneLights = {};
    std::vector<LightResult> m_mapLights = {};
    int limitZoneLight = -1;

    void emptyMinimap() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                adtSelectionMinimapTextures[i][j] = nullptr;
                adtSelectionMinimapMaterials[i][j] = nullptr;
            }
        }
    }

    auto createMinimapGenerator();

    std::array<char, 128> filterText = {0};
    bool refilterIsNeeded = false;
    bool resortIsNeeded = false;
    void filterMapList(const std::string &text);
    void fillMapListStrings();

    ImGui::FileBrowser fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory, true);
    ImGui::FileBrowser createFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename);

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
    bool drawZoneLights = false;
    bool drawAreaLights = false;

    int currentTime = 0;

    int  threadCount = 4;
    int  quickSortCutoff = 100;
    float prevMinimapZoom = 1;
    float prevZoomedSize = 0;
    int prevMapId = -1;
    bool isWmoMap = false;

    bool pauseAnimation = true;

    int lightSource = 0;

    bool disableGlow = false;
    int glowSource = 0;
    float customGlow = 0.0;



    MapRecord prevMapRec;

    float worldPosX = 0;
    float worldPosY = 0;
    float worldPosZ = 0;

    HApiContainer m_api;

    HWdtFile m_wdtFile = nullptr;

    std::vector<MapRecord> mapList = {};
    std::vector<MapRecord> filteredMapList = {};
    std::vector<std::vector<std::string>> mapListStringMap = {};

    int lastWidth = 100;
    int lastHeight = 100;

    bool needToMakeScreenshot = false;
    std::shared_ptr<IRenderView> m_screenShotRenderView = nullptr;
    std::shared_ptr<IRenderView> m_debugRenderView = nullptr;


    std::string screenshotFilename = "";
    HFrameBuffer screenshotFramebuffer = nullptr;
    int screenShotWidth = 100;
    int screenShotHeight = 100;
    int screenshotFrame = -1;

    std::shared_ptr<IExporter> exporter;
    int exporterFramesReady = 0;

    std::shared_ptr<MinimapGenerationWindow> m_minimapGenerationWindow = nullptr;
    std::shared_ptr<BLPViewer> m_blpViewerWindow = nullptr;
    std::shared_ptr<BLPViewer> m_blpFileViewerWindow = nullptr;
    std::vector<std::shared_ptr<M2Window>> m_m2Windows = {};

    std::shared_ptr<FileListWindow> m_fileListWindow = nullptr;
    std::shared_ptr<DebugRendererWindow> m_debugRenderWindow;


//Test export
    std::shared_ptr<DataExporter::DataExporterClass> m_dataExporter = nullptr;

    std::shared_ptr<DatabaseUpdateWorkflow> m_databaseUpdateWorkflow = nullptr;
    std::shared_ptr<KeysUpdateWorkflow> m_keyUpdateWorkFlow = nullptr;

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
    void shutDown();

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
    void showBlpViewer();
    void showM2Viewer();
    void showFileList();

    void restartMinimapGenPreview();
    void showMinimapGenerationSettingsDialog();



    void editComponentsForConfig(Config *config);

    //Making the frame
    void update(HFrontendUIBufferCreate renderer);

    void createFontTexture();
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
