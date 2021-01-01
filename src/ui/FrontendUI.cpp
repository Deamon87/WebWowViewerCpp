//
// Created by deamon on 20.12.19.
//

#include <GLFW/glfw3.h>
#include "FrontendUI.h"

#include <imguiImpl/imgui_impl_opengl3.h>
#include <iostream>
#include <mathfu/glsl_mappings.h>
#include "imguiLib/imguiImpl/imgui_impl_glfw.h"
#include "imguiLib/fileBrowser/imfilebrowser.h"
#include "../../wowViewerLib/src/engine/shader/ShaderDefinitions.h"
#include "childWindow/mapConstructionWindow.h"
#include "../persistance/CascRequestProcessor.h"
#include "../../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../wowViewerLib/src/engine/objects/scenes/wmoScene.h"
#include "../../wowViewerLib/src/engine/objects/scenes/m2Scene.h"
#include "../screenshots/screenshotMaker.h"
#include "../persistance/HttpRequestProcessor.h"
#include "../../wowViewerLib/src/exporters/gltfExporter/GLTFExporter.h"
#include "../../wowViewerLib/src/engine/objects/scenes/NullScene.h"


static const GBufferBinding imguiBindings[3] = {
    {+imguiShader::Attribute::Position, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, pos)},
    {+imguiShader::Attribute::UV, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, uv)},
    {+imguiShader::Attribute::Color, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, col)},
};

void FrontendUI::composeUI() {
    if (this->fontTexture == nullptr)
        return;

    if (mapCanBeOpened) {
        if (!adtMinimapFilled && fillAdtSelectionminimap(adtSelectionMinimap, isWmoMap, mapCanBeOpened )) {
//            fillAdtSelectionminimap = nullptr;
            adtMinimapFilled = true;
        }
    }

    showMainMenu();

    if (ImGui::BeginPopupModal("Casc failed"))
    {
        ImGui::Text("Could not open CASC storage at selected folder");
        if (ImGui::Button("Ok", ImVec2(-1, 23))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Casc succesed"))
    {
        ImGui::Text("CASC storage succefully opened");
        if (ImGui::Button("Ok", ImVec2(-1, 23))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    //Show filePicker
    fileDialog.Display();
    createFileDialog.Display();

    if (fileDialog.HasSelected()) {
        std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;

        if (!openCascCallback(fileDialog.GetSelected().string())) {
            ImGui::OpenPopup("Casc failed");
            cascOpened = false;
        } else {
            ImGui::OpenPopup("Casc succesed");
            cascOpened = true;
        }
        fileDialog.ClearSelected();
    }
    if (createFileDialog.HasSelected()) {
        screenshotFilename = createFileDialog.GetSelected().string();
        needToMakeScreenshot = true;

        createFileDialog.ClearSelected();
    }


//    if (show_demo_window)
//        ImGui::ShowDemoWindow(&show_demo_window);

    showSettingsDialog();
    showQuickLinksDialog();

    showMapConstructionDialog();
    showMapSelectionDialog();
    showMakeScreenshotDialog();
    showCurrentStatsDialog();

    // Rendering
    ImGui::Render();
}

void FrontendUI::showCurrentStatsDialog() {
    static float f = 0.0f;
    static int counter = 0;

    if (showCurrentStats) {
        ImGui::Begin("Current stats",
                     &showCurrentStats);                          // Create a window called "Hello, world!" and append into it.

        static float cameraPosition[3] = {0, 0, 0};
        getCameraPos(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

        ImGui::Text("Current camera position: (%.1f,%.1f,%.1f)", cameraPosition[0], cameraPosition[1],
                    cameraPosition[2]);               // Display some text (you can use a format strings too)

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
//            if(getCurrentAreaName) {
        ImGui::Text("Current area name: %s", getCurrentAreaName().c_str());
        ImGui::Text("Uniform data for GPU: %.3f MB", m_api->hDevice->getUploadSize() / (1024.0f * 1024.0f));
        ImGui::NewLine();
        ImGui::Text("Elapsed time on culling : %.3f ms", m_api->getConfig()->cullingTimePerFrame);
        ImGui::Text("Elapsed time on update : %.3f ms",  m_api->getConfig()->updateTimePerFrame);
        ImGui::Text("Elapsed time on m2 update : %.3f ms",  m_api->getConfig()->m2UpdateTime);
        ImGui::Text("Elapsed time on wait for begin update: %.3f ms",  m_api->hDevice->getWaitForUpdate());

        ImGui::Text("Elapsed time on singleUpdateCNT: %.3f ms",  m_api->getConfig()->singleUpdateCNT);
        ImGui::Text("Elapsed time on meshesCollectCNT: %.3f ms",  m_api->getConfig()->meshesCollectCNT);
        ImGui::Text("Elapsed time on updateBuffersCNT: %.3f ms",  m_api->getConfig()->updateBuffersCNT);
        ImGui::Text("Elapsed time on updateBuffersDeviceCNT: %.3f ms",  m_api->getConfig()->updateBuffersDeviceCNT);
        ImGui::Text("Elapsed time on postLoadCNT: %.3f ms",  m_api->getConfig()->postLoadCNT);
        ImGui::Text("Elapsed time on textureUploadCNT: %.3f ms",  m_api->getConfig()->textureUploadCNT);
        ImGui::Text("Elapsed time on drawStageAndDepsCNT: %.3f ms",  m_api->getConfig()->drawStageAndDepsCNT);
        ImGui::Text("Elapsed time on endUpdateCNT: %.3f ms",  m_api->getConfig()->endUpdateCNT);

        int currentFrame = m_api->hDevice->getDrawFrameNumber();
        auto &cullStageData = m_cullstages[currentFrame];

        int m2ObjectsDrawn = cullStageData!= nullptr ? cullStageData->m2Array.size() : 0;
        int wmoObjectsDrawn = cullStageData!= nullptr ? cullStageData->wmoArray.size() : 0;

        ImGui::Text("M2 objects drawn: %s", std::to_string(m2ObjectsDrawn).c_str());
        ImGui::Text("WMO objects drawn: %s", std::to_string(wmoObjectsDrawn).c_str());

//        ImGui::Text("Current Fog scaler: %f", m_api->getConfig()->getFogScaler());
//        ImGui::Text("Current Fog density: %f", m_api->getConfig()->getFogDensity());
//            }
        ImGui::End();
    }
}

// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct my_equal {
    my_equal( const std::locale& loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};

// find substring (case insensitive)
template<typename T>
int ci_find_substr( const T& str1, const T& str2, const std::locale& loc = std::locale() )
{
    typename T::const_iterator it = std::search( str1.begin(), str1.end(),
                                                 str2.begin(), str2.end(), my_equal<typename T::value_type>(loc) );
    if ( it != str1.end() ) return it - str1.begin();
    else return -1; // not found
}

void FrontendUI::filterMapList(std::string text) {
    filteredMapList = {};
    for (int i = 0; i < mapList.size(); i++) {
        auto &currentRec = mapList[i];
        if (text == "" ||
            (
                (ci_find_substr(currentRec.MapName, text) != std::string::npos) ||
                (ci_find_substr(currentRec.MapDirectory, text) != std::string::npos)
            )
            ) {
            filteredMapList.push_back(currentRec);
        }
    }
}
void FrontendUI::showMapConstructionDialog() {
    if (!showMapConstruction) return;

    if (m_mapConstructionWindow == nullptr)
        m_mapConstructionWindow = std::make_shared<MapConstructionWindow>(m_api);

    showMapConstruction = m_mapConstructionWindow->render();

}
void FrontendUI::showMapSelectionDialog() {
    if (showSelectMap) {
        if (mapList.size() == 0) {
            getMapList(mapList);
            refilterIsNeeded = true;
        }
        if (refilterIsNeeded) {
            filterMapList(std::string(&filterText[0]));
            mapListStringMap = {};
            for (int i = 0; i < filteredMapList.size(); i++) {
                auto mapRec = filteredMapList[i];

                std::vector<std::string> mapStrRec;
                mapStrRec.push_back(std::to_string(mapRec.ID));
                mapStrRec.push_back(mapRec.MapName);
                mapStrRec.push_back(mapRec.MapDirectory);
                mapStrRec.push_back(std::to_string(mapRec.WdtFileID));
                mapStrRec.push_back(std::to_string(mapRec.MapType));

                mapListStringMap.push_back(mapStrRec);
            }

            refilterIsNeeded = false;
        }

        ImGui::Begin("Map Select Dialog", &showSelectMap);
        {
            ImGui::Columns(2, NULL, true);
            //Left panel
            {
                //Filter
                if (ImGui::InputText("Filter: ", filterText.data(), filterText.size(), ImGuiInputTextFlags_AlwaysInsertMode)) {
                    refilterIsNeeded = true;
                }
                //The table
                ImGui::BeginChild("Map Select Dialog Left panel");
                ImGui::Columns(5, "mycolumns"); // 5-ways, with border
                ImGui::Separator();
                ImGui::Text("ID");
                ImGui::NextColumn();
                ImGui::Text("MapName");
                ImGui::NextColumn();
                ImGui::Text("MapDirectory");
                ImGui::NextColumn();
                ImGui::Text("WdtFileID");
                ImGui::NextColumn();
                ImGui::Text("MapType");
                ImGui::NextColumn();
                ImGui::Separator();
                static int selected = -1;
                for (int i = 0; i < filteredMapList.size(); i++) {
                    auto mapRec = filteredMapList[i];

                    if (ImGui::Selectable(mapListStringMap[i][0].c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns)) {
                        if (mapRec.ID != prevMapId) {
                            mapCanBeOpened = true;
                            adtMinimapFilled = false;
                            prevMapRec = mapRec;

                            isWmoMap = false;
                            adtSelectionMinimap = {};
                            getAdtSelectionMinimap(mapRec.WdtFileID);

                        }
                        prevMapId = mapRec.ID;
                        selected = i;
                    }
                    bool hovered = ImGui::IsItemHovered();
                    ImGui::NextColumn();
                    ImGui::Text("%s", mapListStringMap[i][1].c_str());
                    ImGui::NextColumn();
                    ImGui::Text("%s", mapListStringMap[i][2].c_str());
                    ImGui::NextColumn();
                    ImGui::Text("%s", mapListStringMap[i][3].c_str());
                    ImGui::NextColumn();
                    ImGui::Text("%s", mapListStringMap[i][4].c_str());
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
                ImGui::Separator();
                ImGui::EndChild();
            }
            ImGui::NextColumn();

            {
                ImGui::BeginChild("Map Select Dialog Right panel", ImVec2(0, 0));
                {
                    if (!mapCanBeOpened) {
                        ImGui::Text("Cannot open this map.");
                        ImGui::Text("WDT file either does not exist in CASC repository or is encrypted");
                    } else if (!isWmoMap) {
                        ImGui::SliderFloat("Zoom", &minimapZoom, 0.1, 10);
//                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                        showAdtSelectionMinimap();
                    } else {
                        worldPosX = 0;
                        worldPosY = 0;
                        if (ImGui::Button("Open WMO Map", ImVec2(-1, 0))) {
                            openSceneByfdid(prevMapId, prevMapRec.WdtFileID, 17066.6641f, 17066.67380f, 0);
                            showSelectMap = false;
                        }
                    }

                }
                ImGui::EndChild();


            }
            ImGui::Columns(1);

            ImGui::End();
        }
    }
}

void FrontendUI::showAdtSelectionMinimap() {
    ImGui::BeginChild("Adt selection minimap", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar |
                                                       ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (minimapZoom < 0.001)
        minimapZoom = 0.001;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

    const float defaultImageDimension = 100;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            if (adtSelectionMinimap[i][j] != nullptr) {
                if (ImGui::ImageButton(adtSelectionMinimap[i][j],
                                       ImVec2(defaultImageDimension * minimapZoom, defaultImageDimension * minimapZoom))) {
                    auto mousePos = ImGui::GetMousePos();
                    ImGuiStyle &style = ImGui::GetStyle();


                    mousePos.x += ImGui::GetScrollX() - ImGui::GetWindowPos().x - style.WindowPadding.x;
                    mousePos.y += ImGui::GetScrollY() - ImGui::GetWindowPos().y - style.WindowPadding.y;

                    mousePos.x = ((mousePos.x / minimapZoom) / defaultImageDimension);
                    mousePos.y = ((mousePos.y / minimapZoom) / defaultImageDimension);

                    mousePos.x = (32.0f - mousePos.x) * MathHelper::TILESIZE;
                    mousePos.y = (32.0f - mousePos.y) * MathHelper::TILESIZE;

                    worldPosX = mousePos.y;
                    worldPosY = mousePos.x;
//                                if ()
                    ImGui::OpenPopup("AdtWorldCoordsTest");
                    std::cout << "world coords : x = " << worldPosX << " y = " << worldPosY
                              << std::endl;

                }
            } else {
                ImGui::Dummy(ImVec2(100 * minimapZoom, 100 * minimapZoom));
            }

            ImGui::SameLine(0, 0);
        }
        ImGui::NewLine();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();


    if (ImGui::BeginPopup("AdtWorldCoordsTest", ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Pos: (%.2f,%.2f,200)", worldPosX, worldPosY);
        if (ImGui::Button("Go")) {

            openSceneByfdid(prevMapId, prevMapRec.WdtFileID, worldPosX, worldPosY, 200);
            showSelectMap = false;

            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (prevMinimapZoom != minimapZoom) {
        auto windowSize = ImGui::GetWindowSize();
        ImGui::SetScrollX((ImGui::GetScrollX() + windowSize.x / 2.0) * minimapZoom / prevMinimapZoom -
                          windowSize.x / 2.0);
        ImGui::SetScrollY((ImGui::GetScrollY() + windowSize.y / 2.0) * minimapZoom / prevMinimapZoom -
                          windowSize.y / 2.0);
    }
    prevMinimapZoom = minimapZoom;

    ImGui::EndChild();
}

void FrontendUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
//            ImGui::MenuItem("(dummy menu)", NULL, false, false);
            if (ImGui::MenuItem("Open CASC Storage...")) {
                fileDialog.Open();
            }

            if (ImGui::MenuItem("Open Map selection", "", false, cascOpened)) {
                showSelectMap = true;
            }
            if (ImGui::MenuItem("Unload scene", "", false, cascOpened)) {
                unloadScene();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Open minimap")) {}
            if (ImGui::MenuItem("Open current stats")) { showCurrentStats = true; }
            ImGui::Separator();
            if (ImGui::MenuItem("Open settings")) {showSettings = true;}
            if (ImGui::MenuItem("Open QuickLinks")) {showQuickLinks = true;}
            if (ImGui::MenuItem("Open MapConstruction")) {showMapConstruction = true;}
            if (ImGui::MenuItem("Start experiment")) {
                startExperimentCallback();
            }
            if (ImGui::MenuItem("Test export")) {
                if (currentScene != nullptr) {
                    std::unique_ptr<GLTFExporter> exporter = std::make_unique<GLTFExporter>();
                    currentScene->exportScene(exporter.get());
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Make screenshot")) {
                showMakeScreenshot = true;
                screenShotWidth = lastWidth;
                screenShotHeight = lastHeight;

            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

//

void FrontendUI::initImgui(GLFWwindow *window) {

    emptyMinimap();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
}

void FrontendUI::newFrame() {
//    ImGui_ImplOpenGL3_NewFrame();
    //Create Font image
    if (this->fontTexture == nullptr)
        return;


    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

bool FrontendUI::getStopMouse() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool FrontendUI::getStopKeyboard() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}



void FrontendUI::showQuickLinksDialog() {
    if (!showQuickLinks) return;
    std::vector<int> replacementTextureFDids = {};

    ImGui::Begin("Quick Links", &showQuickLinks);


    if (ImGui::Button("Hearthstone Tavern", ImVec2(-1, 0))) {
        openWMOSceneByfdid(2756726);
    }
    if (ImGui::Button("Stormwind mage portal", ImVec2(-1, 0))) {
        openM2SceneByfdid(2394711, replacementTextureFDids);
    }

//    if (ImGui::Button("Azeroth map: Lion's Rest (Legion)", ImVec2(-1, 0))) {
//        openMapByIdAndFilename(0, "azeroth", -8739, 944, 200);
//    }
    if (ImGui::Button("Nyalotha map", ImVec2(-1, 0))) {
        openSceneByfdid(2217, 2842322, -11595, 9280, 260);
    }
    if (ImGui::Button("WMO 1247268", ImVec2(-1, 0))) {
        openWMOSceneByfdid(1247268);
    }

    if (ImGui::Button("Some item", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[1] = 528801;
            for (auto &fdid: replacementTextureFDids) {
                fdid = 1029337;
            }
            openM2SceneByfdid(1029334, replacementTextureFDids);
    }
    if (ImGui::Button("IGC Anduin", ImVec2(-1, 0))) {


        openM2SceneByfdid(3849312, replacementTextureFDids);
    }

    if (ImGui::Button("Fox", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[11] = 3071379;

            openM2SceneByfdid(3071370, replacementTextureFDids);
    }
    if (ImGui::Button("COT hourglass", ImVec2(-1, 0))) {
        openM2SceneByfdid(190850, replacementTextureFDids);
    }
    if (ImGui::Button("Gryphon roost", ImVec2(-1, 0))) {
        openM2SceneByfdid(198261, replacementTextureFDids);
    }
    if (ImGui::Button("Northrend Human Inn", ImVec2(-1, 0))) {
            openWMOSceneByfdid(114998);
    }
    if (ImGui::Button("Strange WMO", ImVec2(-1, 0))) {
            openWMOSceneByfdid(2342637);
    }
    if (ImGui::Button("Flyingsprite", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);

            replacementTextureFDids[11] = 3059000;
            openM2SceneByfdid(3024835, replacementTextureFDids);
    }
    if (ImGui::Button("maldraxxusflyer", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[11] = 3196375;
            openM2SceneByfdid(3196372, replacementTextureFDids);
    }
    if (ImGui::Button("ridingphoenix", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);

            openM2SceneByfdid(125644, replacementTextureFDids);
    }
    if (ImGui::Button("Upright Orc", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[1] = 3844710;
            openM2SceneByfdid(1968587, replacementTextureFDids);
    }
    if (ImGui::Button("quillboarbrute.m2", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[11] = 1786107;
            openM2SceneByfdid(1784020, replacementTextureFDids);
    }
    if (ImGui::Button("WMO With Horde Symbol", ImVec2(-1, 0))) {
            openWMOSceneByfdid(1846142);
    }
    if (ImGui::Button("WMO 3565693", ImVec2(-1, 0))) {
            openWMOSceneByfdid(3565693);
    }

    if (ImGui::Button("BC login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(131982, replacementTextureFDids);
            //        auto ambient = mathfu::vec4(0.3929412066936493f, 0.26823532581329346f, 0.3082353174686432f, 0);
            m_api->getConfig()->BCLightHack = true;
    }
    if (ImGui::Button("Wrath login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(236122, replacementTextureFDids);
    }

    if (ImGui::Button("Cataclysm login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(466614, replacementTextureFDids);
    }
    if (ImGui::Button("Panda login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(631713, replacementTextureFDids);
    }
    if (ImGui::Button("Draenor login screen", ImVec2(-1, 0))) {
            openM2SceneByName("interface/glues/models/ui_mainmenu_warlords/ui_mainmenu_warlords.m2", replacementTextureFDids);
    }
    if (ImGui::Button("Legion Login Screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(1396280, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }
    if (ImGui::Button("BfA login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(2021650, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }
    if (ImGui::Button("Shadowlands login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(3846560, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }

    if (ImGui::Button("Shadowlands clouds", ImVec2(-1, 0))) {
            openM2SceneByfdid(3445776, replacementTextureFDids);
    }

    if (ImGui::Button("Pink serpent", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);

            replacementTextureFDids[11] = 2905480;
            replacementTextureFDids[12] = 2905481;
            replacementTextureFDids[13] = 577442;
            openM2SceneByfdid(577443, replacementTextureFDids);
    }
    if (ImGui::Button("Wolf", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);

            replacementTextureFDids[11] = 126494;
            replacementTextureFDids[12] = 126495;
            replacementTextureFDids[13] = 0;
            openM2SceneByfdid(126487, replacementTextureFDids);
    }

    if (ImGui::Button("Aggramar", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[11] = 1599776;
            openM2SceneByfdid(1599045, replacementTextureFDids);
    }
    if (ImGui::Button("M2 3087468", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[11] = 3087540;
            openM2SceneByfdid(3087468, replacementTextureFDids);
    }

    if (ImGui::Button("Nagrand skybox", ImVec2(-1, 0))) {

        openM2SceneByfdid(130575, replacementTextureFDids);

    }
    if (ImGui::Button("3445776 PBR cloud sky in Maw", ImVec2(-1, 0))) {
            openM2SceneByfdid(3445776, replacementTextureFDids);
    }
    if (ImGui::Button("M2 3572296", ImVec2(-1, 0))) {
            openM2SceneByfdid(3572296, replacementTextureFDids);
    }
    if (ImGui::Button("M2 3487959", ImVec2(-1, 0))) {
            openM2SceneByfdid(3487959, replacementTextureFDids);
    }
    if (ImGui::Button("M2 1729717 waterfall", ImVec2(-1, 0))) {
            openM2SceneByfdid(1729717, replacementTextureFDids);
    }
    if (ImGui::Button("Maw jailer", ImVec2(-1, 0))) {
//        3096499,3096495
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 3096499;
        replacementTextureFDids[12] = 3096495;
            openM2SceneByfdid(3095966, replacementTextureFDids);
    }
    if (ImGui::Button("Creature with colors", ImVec2(-1, 0))) {
//        3096499,3096495
            openM2SceneByfdid(1612576, replacementTextureFDids);
    }
    if (ImGui::Button("IC new sky", ImVec2(-1, 0))) {
            openM2SceneByfdid(3159936, replacementTextureFDids);
    }


    if (ImGui::Button("vampire candle", ImVec2(-1, 0))) {
            openM2SceneByfdid(3184581, replacementTextureFDids);
    }
    if (ImGui::Button("Bog Creature", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[11] = 3732358;
            replacementTextureFDids[12] = 3732360;
            replacementTextureFDids[13] = 3732368;

            openM2SceneByfdid(3732303, replacementTextureFDids);
    }

    ImGui::Separator();
    ImGui::Text("Models for billboard checking");
    ImGui::NewLine();
    if (ImGui::Button("Dalaran dome", ImVec2(-1, 0))) {
            openM2SceneByfdid(203598, replacementTextureFDids);
    }
    if (ImGui::Button("Gift of Nzoth", ImVec2(-1, 0))) {
            openM2SceneByfdid(2432705, replacementTextureFDids);
    }
    if (ImGui::Button("Plagueheart Shoulderpad", ImVec2(-1, 0))) {
            openM2SceneByfdid(143343, replacementTextureFDids);
    }
    if (ImGui::Button("Dalaran eye", ImVec2(-1, 0))) {
            openM2SceneByfdid(243044, replacementTextureFDids);
    }
    if (ImGui::Button("Hand weapon", ImVec2(-1, 0))) {
            replacementTextureFDids = std::vector<int>(17);
            replacementTextureFDids[1] = 528801;
            for (auto &fdid: replacementTextureFDids) {
                fdid = 528801;
            }
            openM2SceneByfdid(528797, replacementTextureFDids);
    }

    ImGui::End();
}

void FrontendUI::showSettingsDialog() {
    if(showSettings) {
        ImGui::Begin("Settings", &showSettings);

        {
            std::string currentCamera;
            if (currentCameraNum == -1) {
                currentCamera = "First person";
            } else {
                currentCamera = "Camera Num " + std::to_string(currentCameraNum);
            }


            ImGui::Text("Camera selection");
            ImGui::SameLine();
            if (ImGui::BeginCombo("##combo", currentCamera.c_str())) // The second parameter is the label previewed before opening the combo.
            {
                int cameraNum = getCameraNumCallback();

                {
                    std::string caption = "First person";
                    if (ImGui::Selectable(caption.c_str(), currentCameraNum == -1)) {
                        setNewCameraCallback(-1);
                        currentCameraNum = -1;
                    }
                }

                for (int n = 0; n < cameraNum; n++)
                {
                    bool is_selected = (currentCameraNum == n); // You can store your selection however you want, outside or inside your objects
                    std::string caption = "Camera Num " + std::to_string(n);
                    if (ImGui::Selectable(caption.c_str(), is_selected)) {
                        if (setNewCameraCallback(n)) {
                            currentCameraNum = n;
                        }
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
        }
        ImGui::Separator();

//        {
//            std::string currentMode = std::to_string(m_api->getConfig()->diffuseColorHack);
//            ImGui::Text("Diffuse hack selection");
//            ImGui::SameLine();
//            if (ImGui::BeginCombo("##diffuseCombo", currentMode.c_str())) // The second parameter is the label previewed before opening the combo.
//            {
//
//                for (int n = 0; n < 6; n++)
//                {
//                    bool is_selected = (m_api->getConfig()->diffuseColorHack == n); // You can store your selection however you want, outside or inside your objects
//                    std::string caption =std::to_string(n);
//                    if (ImGui::Selectable(caption.c_str(), is_selected)) {
//                        m_api->getConfig()->diffuseColorHack = n;
//                    }
//
//                    if (is_selected)
//                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
//                }
//                ImGui::EndCombo();
//            }
//        }

        if (ImGui::SliderFloat("Far plane", &farPlane, 200, 2000)) {
            m_api->getConfig()->farPlane = farPlane;
            m_api->getConfig()->farPlaneForCulling = farPlane+50;
        }

        if (ImGui::Checkbox("Use gauss blur", &useGaussBlur)) {
            m_api->getConfig()->useGaussBlur = useGaussBlur;
        }

        bool disableFog = m_api->getConfig()->disableFog;
        if (ImGui::Checkbox("Disable fog", &disableFog)) {
            m_api->getConfig()->disableFog = disableFog;
        }

        pauseAnimation = m_api->getConfig()->pauseAnimation;
        if (ImGui::Checkbox("Pause animation", &pauseAnimation)) {
            m_api->getConfig()->pauseAnimation = pauseAnimation;
        }

        if (ImGui::Button("Reset Animation")) {
                resetAnimationCallback();
        }

        ImGui::Text("Time: %02d:%02d", (int)(currentTime/120), (int)((currentTime/2) % 60));
        if (ImGui::SliderInt("Current time", &currentTime, 0, 2880)) {
            m_api->getConfig()->currentTime = currentTime;
        }

        if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.3, 100)) {
            m_api->camera->setMovementSpeed(movementSpeed);
        }

        switch(m_api->getConfig()->globalLighting) {
            case EParameterSource::eDatabase: {
                lightSource = 0;
                break;
            }
            case EParameterSource::eM2: {
                lightSource = 1;
                break;
            }
            case EParameterSource::eConfig: {
                lightSource = 2;
                break;
            }
        }

        if (ImGui::RadioButton("Use global timed light", &lightSource, 0)) {
            m_api->getConfig()->globalLighting = EParameterSource::eDatabase;
        }
        if (ImGui::RadioButton("Use ambient light from M2  (only for M2 scenes)", &lightSource, 1)) {
            m_api->getConfig()->globalLighting = EParameterSource::eM2;
        }
        if (ImGui::RadioButton("Manual light", &lightSource, 2)) {
            m_api->getConfig()->globalLighting = EParameterSource::eConfig;
        }

        if (m_api->getConfig()->globalLighting == EParameterSource::eConfig) {
            {
                auto ambient = m_api->getConfig()->exteriorAmbientColor;
                exteriorAmbientColor = {ambient.x, ambient.y, ambient.z};
                ImVec4 col = ImVec4(ambient.x, ambient.y, ambient.z, 1.0);
                if (ImGui::ColorButton("ExteriorAmbientColor##3b", col)) {
                    ImGui::OpenPopup("Exterior Ambient picker");
                }
                ImGui::SameLine();
                ImGui::Text("Exterior Ambient");

                if (ImGui::BeginPopup("Exterior Ambient picker")) {
                    if (ImGui::ColorPicker3("Exterior Ambient", exteriorAmbientColor.data())) {
                        m_api->getConfig()->exteriorAmbientColor = mathfu::vec4(
                            exteriorAmbientColor[0], exteriorAmbientColor[1], exteriorAmbientColor[2], 1.0);
                    }
                    ImGui::EndPopup();
                }
            }

            {
                auto horizontAmbient = m_api->getConfig()->exteriorHorizontAmbientColor;
                exteriorHorizontAmbientColor = {horizontAmbient.x, horizontAmbient.y, horizontAmbient.z};
                ImVec4 col = ImVec4(horizontAmbient.x, horizontAmbient.y, horizontAmbient.z, 1.0);
                if (ImGui::ColorButton("ExteriorHorizontAmbientColor##3b", col)) {
                    ImGui::OpenPopup("Exterior Horizont Ambient picker");
                }
                ImGui::SameLine();
                ImGui::Text("Exterior Horizont Ambient");

                if (ImGui::BeginPopup("Exterior Horizont Ambient picker")) {
                    if (ImGui::ColorPicker3("Exterior Horizont Ambient", exteriorHorizontAmbientColor.data())) {
                        m_api->getConfig()->exteriorHorizontAmbientColor = mathfu::vec4 (
                            exteriorHorizontAmbientColor[0],
                            exteriorHorizontAmbientColor[1], exteriorHorizontAmbientColor[2], 1.0);
                    }
                    ImGui::EndPopup();
                }
            }
            {
                auto groundAmbient = m_api->getConfig()->exteriorGroundAmbientColor;
                exteriorGroundAmbientColor = {groundAmbient.x, groundAmbient.y, groundAmbient.z};
                ImVec4 col = ImVec4(groundAmbient.x, groundAmbient.y, groundAmbient.z, 1.0);

                if (ImGui::ColorButton("ExteriorGroundAmbientColor##3b", col)) {
                    ImGui::OpenPopup("Exterior Ground Ambient picker");
                }
                ImGui::SameLine();
                ImGui::Text("Exterior Ground Ambient");

                if (ImGui::BeginPopup("Exterior Ground Ambient picker")) {
                    if (ImGui::ColorPicker3("Exterior Ground Ambient", exteriorGroundAmbientColor.data())) {
                        m_api->getConfig()->exteriorGroundAmbientColor = mathfu::vec4(
                            exteriorGroundAmbientColor[0],
                            exteriorGroundAmbientColor[1], exteriorGroundAmbientColor[2], 1.0);
                    }
                    ImGui::EndPopup();
                }
            }
        }


//        if (ImGui::SliderInt("Thread Count", &threadCount, 2, 16)) {
//            if (setThreadCount){
//                setThreadCount(threadCount);
//            }
//        }
//        if (ImGui::SliderInt("QuickSort cutoff", &quickSortCutoff, 1, 1000)) {
//            if (setQuicksortCutoff){
//                setQuicksortCutoff(quickSortCutoff);
//            }
//        }




        ImGui::End();
    }
}


void FrontendUI::produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) {
    auto m_device = m_api->hDevice;

    if (this->fontTexture == nullptr) {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

        // Upload texture to graphics system

        this->fontTexture = m_device->createTexture(false, false);
        this->fontTexture->loadData(width, height, pixels, ITextureFormat::itRGBA);

        // Store our identifier
        io.Fonts->TexID = this->fontTexture;
        return;
    }


    lastWidth = resultDrawStage->viewPortDimensions.maxs[0];
    lastHeight = resultDrawStage->viewPortDimensions.maxs[1];

    resultDrawStage->opaqueMeshes = std::make_shared<MeshesToRender>();

    auto *draw_data = ImGui::GetDrawData();

    int  fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    //Create projection matrix:
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

    mathfu::mat4 ortho_projection =
        {
            { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
            { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
            { 0.0f,         0.0f,        -1.0f,   0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
        };

    if (m_device->getIsVulkanAxisSystem()) {
        static const mathfu::mat4 vulkanMatrixFix1 = mathfu::mat4(1, 0, 0, 0,
                                                                 0, -1, 0, 0,
                                                                 0, 0, 1.0/2.0, 1/2.0,
                                                                 0, 0, 0, 1).Transpose();
        ortho_projection = vulkanMatrixFix1 * ortho_projection;
    }

    auto uboPart = m_device->createUniformBufferChunk(sizeof(mathfu::mat4));
    uboPart->setUpdateHandler([ortho_projection](IUniformBufferChunk* self, const HFrameDepedantData &frameDepedantData) {
        self->getObject<mathfu::mat4>() = ortho_projection;
    });

    auto shaderPermute = m_device->getShader("imguiShader", nullptr);

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        auto vertexBufferBindings = m_device->createVertexBufferBindings();
        auto vboBuffer = m_device->createVertexBuffer();
        auto iboBuffer = m_device->createIndexBuffer();

        vboBuffer->uploadData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        iboBuffer->uploadData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        //Create vao
        GVertexBufferBinding vertexBufferBinding;
        vertexBufferBinding.bindings = std::vector<GBufferBinding>(&imguiBindings[0], &imguiBindings[3]);
        vertexBufferBinding.vertexBuffer = vboBuffer;

        vertexBufferBindings->setIndexBuffer(iboBuffer);
        vertexBufferBindings->addVertexBufferBinding(vertexBufferBinding);
        vertexBufferBindings->save();

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {


            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
//                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
//                    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
//                else
//                    pcmd->UserCallback(cmd_list, pcmd);
                assert(pcmd->UserCallback == NULL);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    // Create mesh add add it to collected meshes
                    gMeshTemplate meshTemplate(vertexBufferBindings, shaderPermute);
                    meshTemplate.element = DrawElementMode::TRIANGLES;
                    meshTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;
                    meshTemplate.backFaceCulling = false;
                    meshTemplate.depthCulling = false;

                    meshTemplate.scissorEnabled = true;
                    //Vulkan has different clip offset compared to OGL
                    if (!m_device->getIsVulkanAxisSystem()) {
                        meshTemplate.scissorOffset = {(int)clip_rect.x, (int)(fb_height - clip_rect.w)};
                        meshTemplate.scissorSize = {(int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y)};
                    } else {
                        meshTemplate.scissorOffset = {(int)clip_rect.x, (int)(clip_rect.y)};
                        meshTemplate.scissorSize = {(int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y)};
                    }

                    meshTemplate.ubo[1] = uboPart;
                    meshTemplate.textureCount = 1;
                    meshTemplate.texture[0] = pcmd->TextureId;

                    meshTemplate.start = pcmd->IdxOffset * 2;
                    meshTemplate.end = pcmd->ElemCount;

                    resultDrawStage->opaqueMeshes->meshes.push_back(m_device->createMesh(meshTemplate));
                }
            }
        }
    }

    //1. Collect buffers
    auto &bufferChunks = updateStage->uniformBufferChunks;
    int renderIndex = 0;
    for (const auto &mesh : resultDrawStage->opaqueMeshes->meshes) {
        for (int i = 0; i < 5; i++ ) {
            auto bufferChunk = mesh->getUniformBuffer(i);

            if (bufferChunk != nullptr) {
                bufferChunks.push_back(bufferChunk);
            }
        }
    }

    std::sort( bufferChunks.begin(), bufferChunks.end());
    bufferChunks.erase( unique( bufferChunks.begin(), bufferChunks.end() ), bufferChunks.end() );
}


void FrontendUI::getMapList(std::vector<MapRecord> &mapList) {
    if (m_api->databaseHandler == nullptr)  return;

    m_api->databaseHandler->getMapArray(mapList);
}

bool FrontendUI::fillAdtSelectionminimap(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap,
                                         bool &wdtFileExists) {
    if (m_wdtFile == nullptr) return false;

    if (m_wdtFile->getStatus() == FileStatus::FSRejected) {
        wdtFileExists = false;
        isWMOMap = false;
        return false;
    }

    if (m_wdtFile->getStatus() != FileStatus::FSLoaded) return false;

    isWMOMap = m_wdtFile->mphd->flags.wdt_uses_global_map_obj != 0;

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            if (m_wdtFile->mapFileDataIDs[i*64 + j].minimapTexture > 0) {
                auto texture = m_api->cacheStorage->getTextureCache()->getFileId(m_wdtFile->mapFileDataIDs[i*64 + j].minimapTexture);
                minimap[i][j] = m_api->hDevice->createBlpTexture(texture, false, false);
            } else {
                minimap[i][j] = nullptr;
            }
        }
    }
    return true;
}

std::string FrontendUI::getCurrentAreaName() {
    auto conf = m_api->getConfig();
    return conf->areaName;
}

void FrontendUI::showMakeScreenshotDialog() {
   if (showMakeScreenshot) {
       ImGui::Begin("Make screenshot", &showMakeScreenshot);
       {
           ImGui::Text("Width:  ");
           ImGui::SameLine();
           if (ImGui::InputInt("##width", &screenShotWidth)) {
                if (screenShotWidth < 0) {
                    screenShotWidth = 0;
                }
           }

           ImGui::Text("Height: ");
           ImGui::SameLine();
           if (ImGui::InputInt("##height", &screenShotHeight)) {
               if (screenShotHeight < 0) {
                   screenShotHeight = 0;
               }
           }

           if (ImGui::Button("Make screenshot", ImVec2(-1, 23))) {
               createFileDialog.Open();
           }
           ImGui::NewLine();

           ImGui::End();
       }

   }
}

void FrontendUI::produceUpdateStage(HUpdateStage updateStage) {
    this->update(updateStage);


}

HDrawStage createSceneDrawStage(HFrameScenario sceneScenario, int width, int height, double deltaTime, bool isScreenshot,
                                ApiContainer &apiContainer, const std::shared_ptr<IScene> &currentScene, HCullStage &cullStage) {
    float farPlaneRendering = apiContainer.getConfig()->farPlane;
    float farPlaneCulling = apiContainer.getConfig()->farPlaneForCulling;

    float nearPlane = 1.0;
    float fov = toRadian(45.0);

    float canvasAspect = (float)width / (float)height;

    HCameraMatrices cameraMatricesCulling = apiContainer.camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneCulling);
    HCameraMatrices cameraMatricesRendering = apiContainer.camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
    //Frustum matrix with reversed Z

    bool isInfZSupported = apiContainer.camera->isCompatibleWithInfiniteZ();
    if (isInfZSupported)
    {
        float f = 1.0f / tan(fov / 2.0f);
        cameraMatricesRendering->perspectiveMat = mathfu::mat4(
            f / canvasAspect, 0.0f,  0.0f,  0.0f,
            0.0f,    f,  0.0f,  0.0f,
            0.0f, 0.0f,  1, -1.0f,
            0.0f, 0.0f, 1,  0.0f);
    }

    if (apiContainer.hDevice->getIsVulkanAxisSystem() ) {
        auto &perspectiveMatrix = cameraMatricesRendering->perspectiveMat;

        static const mathfu::mat4 vulkanMatrixFix2 = mathfu::mat4(1, 0, 0, 0,
                                                                  0, -1, 0, 0,
                                                                  0, 0, 1.0/2.0, 1/2.0,
                                                                  0, 0, 0, 1).Transpose();

        perspectiveMatrix = vulkanMatrixFix2 * perspectiveMatrix;
    }

    auto clearColor = apiContainer.getConfig()->clearColor;

    if (currentScene != nullptr) {
        ViewPortDimensions dimensions = {{0, 0}, {width, height}};

        HFrameBuffer fb = nullptr;
        if (isScreenshot) {
            fb = apiContainer.hDevice->createFrameBuffer(width, height,
                                                         {ITextureFormat::itRGBA},
                                                         ITextureFormat::itDepth32,
                                                         apiContainer.hDevice->getMaxSamplesCnt(), 4);
        }

        cullStage = sceneScenario->addCullStage(cameraMatricesCulling, currentScene);
        auto updateStage = sceneScenario->addUpdateStage(cullStage, deltaTime*(1000.0f), cameraMatricesRendering);
        std::vector<HDrawStage> drawStageDependencies = {};
        HDrawStage sceneDrawStage = sceneScenario->addDrawStage(updateStage, currentScene, cameraMatricesRendering, drawStageDependencies, true,
                                                                dimensions,
                                                                true, isInfZSupported, clearColor, fb);

        return sceneDrawStage;
    }

    return nullptr;
}

HFrameScenario FrontendUI::createFrameScenario(int canvWidth, int canvHeight, double deltaTime) {
    if ((minimapGenerator != nullptr) && (minimapGenerator->isDone()))
        minimapGenerator = nullptr;

    if (minimapGenerator != nullptr) {
        minimapGenerator->process();
    }

    if (screenshotDS != nullptr) {
        if (screenshotFrame + 5 <= m_api->hDevice->getFrameNumber()) {
            std::vector<uint8_t> buffer = std::vector<uint8_t>(screenShotWidth*screenShotHeight*4+1);

            saveDataFromDrawStage(screenshotDS->target, screenshotFilename, screenShotWidth, screenShotHeight, buffer);

            screenshotDS = nullptr;
        }
    }


    HFrameScenario sceneScenario = std::make_shared<FrameScenario>();
    std::vector<HDrawStage> uiDependecies = {};

    if (needToMakeScreenshot)
    {
        HCullStage temp = nullptr;
        auto drawStage = createSceneDrawStage(sceneScenario, screenShotWidth, screenShotHeight, deltaTime, true, *m_api,
                                              currentScene,temp);
        if (drawStage != nullptr) {
            uiDependecies.push_back(drawStage);
            screenshotDS = drawStage;
            screenshotFrame = m_api->hDevice->getFrameNumber();
        }
        needToMakeScreenshot = false;
    }
    if (minimapGenerator != nullptr) {
        uiDependecies.push_back(minimapGenerator->createSceneDrawStage(sceneScenario));
    }

    //DrawStage for current frame
    bool clearOnUi = true;
    if (currentScene != nullptr && m_api->camera != nullptr)
    {
        int currentFrame = m_api->hDevice->getDrawFrameNumber();
        auto &cullStageData = m_cullstages[currentFrame];
        cullStageData = nullptr;

        auto drawStage = createSceneDrawStage(sceneScenario, canvWidth, canvHeight, deltaTime, false, *m_api,
                                              currentScene, cullStageData);
        if (drawStage != nullptr) {
            uiDependecies.push_back(drawStage);
            clearOnUi = false;
        }
    }
    //DrawStage for UI
    {
        ViewPortDimensions dimension = {
            {0,     0},
            {canvWidth, canvHeight}
        };
        auto clearColor = m_api->getConfig()->clearColor;

        auto uiCullStage = sceneScenario->addCullStage(nullptr, getShared());
        auto uiUpdateStage = sceneScenario->addUpdateStage(uiCullStage, deltaTime * (1000.0f), nullptr);
        HDrawStage frontUIDrawStage = sceneScenario->addDrawStage(uiUpdateStage, getShared(), nullptr, uiDependecies,
                                                                  true, dimension, clearOnUi, false, clearColor, nullptr);
    }

    return sceneScenario;
}

bool FrontendUI::openCascCallback(std::string cascPath) {
    HRequestProcessor newProcessor = nullptr;
    std::shared_ptr<WoWFilesCacheStorage> newStorage = nullptr;
    try {
        newProcessor = std::make_shared<CascRequestProcessor>(cascPath.c_str());
        newStorage = std::make_shared<WoWFilesCacheStorage>(newProcessor.get());
        newProcessor->setThreaded(true);
        newProcessor->setFileRequester(newStorage.get());
    } catch (...){
        return false;
    };

    m_api->cacheStorage = newStorage;
    m_processor = newProcessor;

    return true;
}

void FrontendUI::openSceneByfdid(int mapId, int wdtFileId, float x, float y, float z) {
    if (m_api->cacheStorage) {
//            storage->actuallDropCache();
    }

    currentScene = std::make_shared<Map>(m_api, mapId, wdtFileId);
    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(x, y, z);
    m_api->camera->setMovementSpeed(movementSpeed);
}

void FrontendUI::openWMOSceneByfdid(int WMOFdid) {
    currentScene = std::make_shared<WmoScene>(m_api, WMOFdid);
    m_api->camera->setCameraPos(0, 0, 0);
}

void FrontendUI::openMapByIdAndFilename(int mapId, std::string mapName, float x, float y, float z) {
    currentScene = std::make_shared<Map>(m_api, mapId, mapName);
    m_api->camera->setCameraPos(x,y,z);
}
void FrontendUI::openM2SceneByfdid(int m2Fdid, std::vector<int> &replacementTextureIds) {
    currentScene = std::make_shared<M2Scene>(m_api, m2Fdid, -1);
    currentScene->setReplaceTextureArray(replacementTextureIds);


    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setMovementSpeed(movementSpeed);
    m_api->getConfig()->BCLightHack = false;
//
    m_api->camera->setCameraPos(0, 0, 0);
}
void FrontendUI::openM2SceneByName(std::string m2FileName, std::vector<int> &replacementTextureIds) {
    currentScene = std::make_shared<M2Scene>(m_api, m2FileName, -1);
    currentScene->setReplaceTextureArray(replacementTextureIds);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(0, 0, 0);
    m_api->camera->setMovementSpeed(movementSpeed);
}

void FrontendUI::unloadScene() {
    if (m_api->cacheStorage) {
        m_api->cacheStorage->actuallDropCache();
    }
    currentScene = std::make_shared<NullScene>();
}

int FrontendUI::getCameraNumCallback() {
    if (currentScene != nullptr) {
        return currentScene->getCameraNum();
    }

    return 0;
}

bool FrontendUI::setNewCameraCallback(int cameraNum) {
    if (currentScene == nullptr) return false;

    auto newCamera = currentScene->createCamera(cameraNum);
    if (newCamera == nullptr) {
        m_api->camera = std::make_shared<FirstPersonCamera>();
        m_api->camera->setMovementSpeed(movementSpeed);
        return false;
    }

    m_api->camera = newCamera;
    return true;
}

void FrontendUI::resetAnimationCallback() {
    currentScene->resetAnimation();
}

void FrontendUI::getCameraPos(float &cameraX, float &cameraY, float &cameraZ) {
    if (m_api->camera == nullptr) {
        cameraX = 0; cameraY = 0; cameraZ = 0;
        return;
    }
    float currentCameraPos[4] = {0,0,0,0};
    m_api->camera->getCameraPosition(&currentCameraPos[0]);
    cameraX = currentCameraPos[0];
    cameraY = currentCameraPos[1];
    cameraZ = currentCameraPos[2];
}

void FrontendUI::startExperimentCallback() {
    minimapGenerator = std::make_shared<MinimapGenerator>(
        m_api->cacheStorage,
        m_api->hDevice,
        m_processor,
        m_api->databaseHandler
    );

    std::vector<ScenarioDef> scenarios = {
//        {
//            2222,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-9750, -8001   ),
//            mathfu::vec2(8333, 9500 ),
//            ScenarioOrientation::so45DegreeTick0,
//            "shadowlands/orient0"
//        }
        {
            1643,
            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
            mathfu::vec2(291 , 647 ),
            mathfu::vec2(2550, 2895),
            ScenarioOrientation::so45DegreeTick0,
            "kultiras/orient0"
        },
//        {
//            530,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-5817, -1175),
//            mathfu::vec2(1758, 10491),
//            ScenarioOrientation::so45DegreeTick0,
//            "outland/topDown1"
//        }
//        {
//            0,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-9081, -20),
//            mathfu::vec2(-8507, 1296),
//            ScenarioOrientation::soTopDownOrtho,
//            "azeroth/topDown"
//        }
//    };

//        std::vector<ScenarioDef> scenarios = {
//        {
//            1,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-12182, -8803 ),
//            mathfu::vec2(12058, 4291),
//            ScenarioOrientation::so45DegreeTick0,
//            "kalimdor/rotation0"
//        },
        {
            1,
            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
            mathfu::vec2(-12182, -8803),
            mathfu::vec2(12058, 4291),
            ScenarioOrientation::so45DegreeTick1,
            "kalimdor/rotation1_new"
        }
    };
//        {
//            1,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-12182, -8803 ),
//            mathfu::vec2(12058, 4291),
//            ScenarioOrientation::so45DegreeTick2,
//            "kalimdor/rotation2"
//        },
//        {
//            1,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-12182, -8803 ),
//            mathfu::vec2(12058, 4291),
//            ScenarioOrientation::so45DegreeTick3,
//            "kalimdor/rotation3"
//        },
//    };
    minimapGenerator->startScenarios(scenarios);
}

void FrontendUI::createDefaultprocessor() {

    const char * url = "https://wow.tools/casc/file/fname?buildconfig=6116c52b23b01bf112b67f571749c38f&cdnconfig=86bf8d360110ae471e61d7b9c99c2c08&filename=";
    const char * urlFileId = "https://wow.tools/casc/file/fdid?buildconfig=6116c52b23b01bf112b67f571749c38f&cdnconfig=86bf8d360110ae471e61d7b9c99c2c08&filename=data&filedataid=";
//
//Classics
//        const char * url = "https://wow.tools/casc/file/fname?buildconfig=bf24b9d67a4a9c7cc0ce59d63df459a8&cdnconfig=2b5b60cdbcd07c5f88c23385069ead40&filename=";
//        const char * urlFileId = "https://wow.tools/casc/file/fdid?buildconfig=bf24b9d67a4a9c7cc0ce59d63df459a8&cdnconfig=2b5b60cdbcd07c5f88c23385069ead40&filename=data&filedataid=";
//        processor = new HttpZipRequestProcessor(url);
////        processor = new ZipRequestProcessor(filePath);
////        processor = new MpqRequestProcessor(filePath);
//    m_processor = std::make_shared<HttpRequestProcessor>(url, urlFileId);
    m_processor = std::make_shared<CascRequestProcessor>("e:/games/wow beta/World of Warcraft Beta/");
////        processor->setThreaded(false);
////
    m_processor->setThreaded(true);
    m_api->cacheStorage = std::make_shared<WoWFilesCacheStorage>(m_processor.get());
    m_processor->setFileRequester(m_api->cacheStorage.get());
}
