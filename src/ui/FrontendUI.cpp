//
// Created by deamon on 20.12.19.
//

#include <GLFW/glfw3.h>
#include "FrontendUI.h"

#include <imguiImpl/imgui_impl_opengl3.h>
#include <iostream>
#include "imguiLib/imguiImpl/imgui_impl_glfw.h"
#include "imguiLib/fileBrowser/imfilebrowser.h"

void FrontendUI::composeUI() {

    if (fillAdtSelectionminimap) {
        if (fillAdtSelectionminimap(adtSelectionMinimap)) {
            fillAdtSelectionminimap = nullptr;

            requiredTextures.clear();
            requiredTextures.reserve(64 * 64);
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    if (adtSelectionMinimap[i][j] != nullptr) {
                        requiredTextures.push_back(adtSelectionMinimap[i][j]);
                    }
                }
            }
        }
    }

    showMainMenu();

    //Show filePicker
    fileDialog.Display();

    if (fileDialog.HasSelected()) {
        std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
        if (openCascCallback) {
            openCascCallback(fileDialog.GetSelected().string());
        }
        fileDialog.ClearSelected();
    }

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    showSettingsDialog();

    showMapSelectionDialog();

    {
        static float f = 0.0f;
        static int counter = 0;

        if (showCurrentStats) {
            ImGui::Begin("Current stats",
                         &showCurrentStats);                          // Create a window called "Hello, world!" and append into it.

            static float cameraPosition[3] = {0, 0, 0};
            if (getCameraPos) {
                getCameraPos(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
            }

            ImGui::Text("Current camera position: (%.1f,%.1f,%.1f)", cameraPosition[0], cameraPosition[1],
                        cameraPosition[2]);               // Display some text (you can use a format strings too)

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }
    }

    // Rendering
    ImGui::Render();
}

void FrontendUI::showMapSelectionDialog() {
    if (showSelectMap) {
        if (mapList.size() == 0 && getMapList) {
            getMapList(mapList);
            mapListStringMap = {};
            for (int i = 0; i < mapList.size(); i++) {
                auto mapRec = mapList[i];

                std::vector<std::string> mapStrRec;
                mapStrRec.push_back(std::to_string(mapRec.ID));
                mapStrRec.push_back(mapRec.MapName);
                mapStrRec.push_back(std::to_string(mapRec.WdtFileID));
                mapStrRec.push_back(std::to_string(mapRec.MapType));

                mapListStringMap.push_back(mapStrRec);
            }
        }

        ImGui::Begin("Map Select Dialog", &showSelectMap);
        {
            ImGui::Columns(2, NULL, true);
            //Left panel
            {
                ImGui::BeginChild("Map Select Dialog Left panel");
                ImGui::Columns(4, "mycolumns"); // 4-ways, with border
                ImGui::Separator();
                ImGui::Text("ID");
                ImGui::NextColumn();
                ImGui::Text("MapName");
                ImGui::NextColumn();
                ImGui::Text("WdtFileID");
                ImGui::NextColumn();
                ImGui::Text("MapType");
                ImGui::NextColumn();
                ImGui::Separator();
                static int selected = -1;
                for (int i = 0; i < mapList.size(); i++) {
                    auto mapRec = mapList[i];

                    if (ImGui::Selectable(mapListStringMap[i][0].c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns)) {
                        if (mapRec.ID != prevMapId) {
                            prevMapRec = mapRec;
                            if (getAdtSelectionMinimap) {
                                adtSelectionMinimap = {};
                                getAdtSelectionMinimap(mapRec.WdtFileID);
                            }
                        }
                        prevMapId = mapRec.ID;
                        selected = i;
                    }
                    bool hovered = ImGui::IsItemHovered();
                    ImGui::NextColumn();
                    ImGui::Text(mapListStringMap[i][1].c_str());
                    ImGui::NextColumn();
                    ImGui::Text(mapListStringMap[i][2].c_str());
                    ImGui::NextColumn();
                    ImGui::Text(mapListStringMap[i][3].c_str());
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
                    ImGui::SliderFloat("Zoom", &minimapZoom, 0.1, 10);
//                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    showAdtSelectionMinimap();

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
    if (prevMinimapZoom != minimapZoom) {
        auto windowSize = ImGui::GetWindowSize();
        ImGui::SetScrollX((ImGui::GetScrollX() + windowSize.x / 2.0) * minimapZoom / prevMinimapZoom -
                          windowSize.x / 2.0);
        ImGui::SetScrollY((ImGui::GetScrollY() + windowSize.y / 2.0) * minimapZoom / prevMinimapZoom -
                          windowSize.y / 2.0);
    }
    prevMinimapZoom = minimapZoom;


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            if (adtSelectionMinimap[i][j] != nullptr && adtSelectionMinimap[i][j]->getIsLoaded()) {
                if (ImGui::ImageButton(adtSelectionMinimap[i][j]->getIdent(),
                                       ImVec2(100 * minimapZoom, 100 * minimapZoom))) {
                    auto mousePos = ImGui::GetMousePos();
                    ImGuiStyle &style = ImGui::GetStyle();


                    mousePos.x += ImGui::GetScrollX() - ImGui::GetWindowPos().x - style.WindowPadding.x;
                    mousePos.y += ImGui::GetScrollY() - ImGui::GetWindowPos().y - style.WindowPadding.y;

                    mousePos.x = ((mousePos.x / minimapZoom) / 100);
                    mousePos.y = ((mousePos.y / minimapZoom) / 100);

                    mousePos.x = (32.0f - mousePos.x) * 533.33333f;
                    mousePos.y = (32.0f - mousePos.y) * 533.33333f;

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
            if (openSceneByfdid) {
                openSceneByfdid(prevMapId, prevMapRec.WdtFileID, worldPosX, worldPosY, 200);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::EndChild();
}

void FrontendUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
//            ImGui::MenuItem("(dummy menu)", NULL, false, false);
            if (ImGui::MenuItem("Open CASC Storage...")) {
                fileDialog.Open();
            }

            if (ImGui::MenuItem("Open Map selection")) {
                showSelectMap = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Open minimap")) {}
            if (ImGui::MenuItem("Open current stats")) { showCurrentStats = true; }
            ImGui::Separator();
            if (ImGui::MenuItem("Open settings")) {showSettings = true;}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void FrontendUI::renderUI() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void FrontendUI::initImgui(GLFWwindow *window) {

    emptyMinimap();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");


}

void FrontendUI::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
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

void FrontendUI::setOpenCascStorageCallback(std::function<void(std::string cascPath)> callback) {
    openCascCallback = callback;
}

void FrontendUI::setOpenSceneByfdidCallback(std::function<void(int mapId, int wdtFileId, float x, float y, float z)> callback) {
    openSceneByfdid = callback;
}

void FrontendUI::setGetCameraPos(std::function<void(float &cameraX, float &cameraY, float &cameraZ)> callback) {
    getCameraPos = callback;
}

void FrontendUI::setGetAdtSelectionMinimap(std::function<void(int wdtFileDataId)> callback) {
    getAdtSelectionMinimap = callback;
}

void FrontendUI::setFillAdtSelectionMinimap(std::function<bool (std::array<std::array<HGTexture, 64>, 64> &minimap)> callback) {
    fillAdtSelectionminimap = callback;
}

void FrontendUI::setGetMapList(std::function<void(std::vector<MapRecord> &mapList)> callback) {
    getMapList = callback;
}

void FrontendUI::setFarPlaneChangeCallback(std::function<void(float farPlane)> callback) {
    setFarPlane = callback;
}
void FrontendUI::setSpeedCallback(std::function<void(float speed)> callback) {
    setMovementSpeed = callback;
}

void FrontendUI::showSettingsDialog() {
    if(showSettings) {
        ImGui::Begin("Settings", &showSelectMap);
        if (ImGui::SliderFloat("Far plane", &farPlane, 200, 700)) {
            if (setFarPlane){
                setFarPlane(farPlane);
            }
        }
        if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.3, 10)) {
            if (setMovementSpeed){
                setMovementSpeed(movementSpeed);
            }
        }

        ImGui::End();
    }
}


#ifdef LINK_VULKAN
void FrontendUI::renderUIVLK(VkCommandBuffer commandBuffer){

};
#endif