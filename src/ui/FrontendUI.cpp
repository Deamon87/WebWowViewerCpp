//
// Created by deamon on 20.12.19.
//

#include <GLFW/glfw3.h>
#include "FrontendUI.h"
#include "imguiLib/imgui.h"
#include <imguiImpl/imgui_impl_opengl3.h>
#include <iostream>
#include "imguiLib/imguiImpl/imgui_impl_glfw.h"
#include "imguiLib/fileBrowser/imfilebrowser.h"

void FrontendUI::composeUI() {
    // Start the Dear ImGui frame
    static float clear_color[3] = {0,0,0};
    static ImGui::FileBrowser fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory);

    static bool show_demo_window = false;
    static bool show_another_window = true;
    static bool showCurrentStats = true;
    static bool showSelectAdtPoint = false;
    static float minimapZoom = 1;
    static float prevMinimapZoom = 1;

    static float prevScrollX = 1;
    static float prevScrollY = 1;

    if (fillAdtSelectionminimap) {
        if (fillAdtSelectionminimap(adtSelectionMinimap)) {
            fillAdtSelectionminimap = nullptr;

            requiredTextures.clear();
            requiredTextures.reserve(64*64);
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    if (adtSelectionMinimap[i][j] != nullptr) {
                        requiredTextures.push_back(adtSelectionMinimap[i][j]);
                    }
                }
            }

        }
    }


    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
//            ImGui::MenuItem("(dummy menu)", NULL, false, false);
            if (ImGui::MenuItem("Open CASC Storage...")) {
                fileDialog.Open();
            }
            if (ImGui::MenuItem("Open test scene")) {
                if ((openSceneByfdid)){
                    openSceneByfdid(0);
                }
            }
            if (ImGui::MenuItem("Open ADT")) {
                if (getAdtSelectionMinimap) {
                    getAdtSelectionMinimap(0);
                    showSelectAdtPoint = true;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Open minimap")) {}
            if (ImGui::MenuItem("Open current stats")) {showCurrentStats = true;}
            ImGui::Separator();
            if (ImGui::MenuItem("Open settings")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    //Show filePicker
    fileDialog.Display();

    if(fileDialog.HasSelected())
    {
        std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
        if (openCascCallback) {
            openCascCallback(fileDialog.GetSelected().string());
        }
        fileDialog.ClearSelected();
    }

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    {
        if (showSelectAdtPoint) {
            ImGui::Begin("Adt select dialog", &showSelectAdtPoint);
            {
                ImGui::SliderFloat("Sample count", &minimapZoom, 0.1, 10);
                ImGui::BeginChild("left pane", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

                if (minimapZoom < 0.001)
                    minimapZoom = 0.001;
                if (prevMinimapZoom != minimapZoom) {
                    auto windowSize = ImGui::GetWindowSize();
                    ImGui::SetScrollX((ImGui::GetScrollX() + windowSize.x/2.0)*minimapZoom/prevMinimapZoom - windowSize.x/2.0);
                    ImGui::SetScrollY((ImGui::GetScrollY() + windowSize.y/2.0)*minimapZoom/prevMinimapZoom - windowSize.y/2.0);
                }
                prevMinimapZoom = minimapZoom;



                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
                  for (int i = 0; i < 64; i++) {
                    for (int j = 0; j < 64; j++) {
                        if (adtSelectionMinimap[i][j] != nullptr && adtSelectionMinimap[i][j]->getIsLoaded()) {
                            ImGui::Image(adtSelectionMinimap[i][j]->getIdent(), ImVec2(100*minimapZoom, 100*minimapZoom) );
                        }
                        else {
                            ImGui::Dummy(ImVec2(100*minimapZoom, 100*minimapZoom));
                        }

                        ImGui::SameLine(0,0);
                    }
                    ImGui::NewLine();
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleVar();
                ImGui::PopStyleVar();
                ImGui::EndChild();
            }

            ImGui::End();

        }
    }

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
//        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
//        ImGui::Checkbox("Another Window", &show_another_window);
//
//        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
//        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
//
//        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
//            counter++;
//        ImGui::SameLine();
//        ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }
    }

    // 3. Show another simple window.
//    if (show_another_window)
//    {
//        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
//        ImGui::Text("Hello from another window!");
//        if (ImGui::Button("Close Me"))
//            show_another_window = false;
//        ImGui::End();
//    }

    //Minimap
    {

    }

    // Rendering
    ImGui::Render();
}

void FrontendUI::renderUI() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void FrontendUI::initImgui(GLFWwindow* window) {

    emptyMinimap();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
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
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool FrontendUI::getStopKeyboard() {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

void FrontendUI::setOpenCascStorageCallback(std::function<void(std::string cascPath)> callback) {
    openCascCallback = callback;
}

void FrontendUI::setOpenSceneByfdidCallback(std::function<void(int fdid)> callback) {
    openSceneByfdid = callback;
}

void FrontendUI::setGetCameraPos(std::function<void(float &cameraX, float &cameraY, float &cameraZ)> callback) {
    getCameraPos = callback;
}

void FrontendUI::setGetAdtSelectionMinimap(std::function<void(int mapId)> callback) {
    getAdtSelectionMinimap = callback;
}

void FrontendUI::setFillAdtSelectionMinimap(
    std::function<bool (std::array<std::array<HGTexture, 64>, 64> &minimap)> callback) {
    fillAdtSelectionminimap = callback;
}


#ifdef LINK_VULKAN
void FrontendUI::renderUIVLK(VkCommandBuffer commandBuffer){

};
#endif