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

    static bool show_demo_window = true;
    static bool show_another_window = true;

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
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Open minimap")) {}
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

//    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
//    if (show_demo_window)
//        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
}

void FrontendUI::renderUI() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void FrontendUI::initImgui(GLFWwindow* window) {

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


#ifdef LINK_VULKAN
void FrontendUI::renderUIVLK(VkCommandBuffer commandBuffer){

};
#endif