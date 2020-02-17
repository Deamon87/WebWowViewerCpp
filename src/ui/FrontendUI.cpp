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


static const GBufferBinding imguiBindings[3] = {
    {+imguiShader::Attribute::Position, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, pos)},
    {+imguiShader::Attribute::UV, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, uv)},
    {+imguiShader::Attribute::Color, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, col)},
};

void FrontendUI::composeUI() {

    if (fillAdtSelectionminimap && mapCanBeOpened) {
        if (fillAdtSelectionminimap(adtSelectionMinimap, isWmoMap, mapCanBeOpened )) {
            fillAdtSelectionminimap = nullptr;
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

    if (fileDialog.HasSelected()) {
        std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
        if (openCascCallback) {
            if (!openCascCallback(fileDialog.GetSelected().string())) {
                ImGui::OpenPopup("Casc failed");
                cascOpened = false;
            } else {
                ImGui::OpenPopup("Casc succesed");
                cascOpened = true;
            }
        }
        fileDialog.ClearSelected();
    }

//    if (show_demo_window)
//        ImGui::ShowDemoWindow(&show_demo_window);

    showSettingsDialog();
    showQuickLinksDialog();

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
            if(getCurrentAreaName) {
                ImGui::Text("Current area name: %s", getCurrentAreaName().c_str());
            }
            ImGui::End();
        }
    }

    // Rendering
    ImGui::Render();
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

void FrontendUI::showMapSelectionDialog() {
    if (showSelectMap) {
        if (mapList.size() == 0 && getMapList) {
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
                            prevMapRec = mapRec;
                            if (getAdtSelectionMinimap) {
                                isWmoMap = false;
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
                    ImGui::Text(mapListStringMap[i][4].c_str());
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
                showSelectMap = false;
            }
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
                if (unloadScene) {
                    unloadScene();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Open minimap")) {}
            if (ImGui::MenuItem("Open current stats")) { showCurrentStats = true; }
            ImGui::Separator();
            if (ImGui::MenuItem("Open settings")) {showSettings = true;}
            if (ImGui::MenuItem("Open QuickLinks")) {showQuickLinks = true;}
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
    if (this->fontTexture == nullptr) {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

        // Upload texture to graphics system

        this->fontTexture = m_device->createTexture();
        this->fontTexture->loadData(width, height, pixels);

        // Store our identifier
        io.Fonts->TexID = this->fontTexture;
    }

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

void FrontendUI::setOpenCascStorageCallback(std::function<bool(std::string cascPath)> callback) {
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

void FrontendUI::setFillAdtSelectionMinimap(std::function<bool (std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap, bool &wdtFileExists)> callback) {
    fillAdtSelectionminimap = callback;
}

void FrontendUI::setGetMapList(std::function<void(std::vector<MapRecord> &mapList)> callback) {
    getMapList = callback;
}

void FrontendUI::setGetCurrentAreaName( std::function <std::string()> callback) {
    getCurrentAreaName = callback;
}

void FrontendUI::setFarPlaneChangeCallback(std::function<void(float farPlane)> callback) {
    setFarPlane = callback;
}
void FrontendUI::setSpeedCallback(std::function<void(float speed)> callback) {
    setMovementSpeed = callback;
}

void FrontendUI::showQuickLinksDialog() {
    if (!showQuickLinks) return;

    ImGui::Begin("Quick Links", &showQuickLinks);
    if (ImGui::Button("Hearthstone Tavern", ImVec2(-1, 0))) {
        if (openWMOSceneByfdid) {
            openWMOSceneByfdid(2756726);
        }
    }

    ImGui::End();
}



void FrontendUI::showSettingsDialog() {
    if(showSettings) {
        ImGui::Begin("Settings", &showSettings);
        if (ImGui::SliderFloat("Far plane", &farPlane, 200, 700)) {
            if (setFarPlane){
                setFarPlane(farPlane);
            }
        }
        ImGui::Text("Time: %02d:%02d", (int)(currentTime/120), (int)((currentTime/2) % 60));
        if (ImGui::SliderInt("Current time", &currentTime, 0, 2880)) {
            if (setCurrentTime){
                setCurrentTime(currentTime);
            }
        }

        if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.3, 10)) {
            if (setMovementSpeed){
                setMovementSpeed(movementSpeed);
            }
        }

        if (ImGui::SliderInt("Thread Count", &threadCount, 2, 16)) {
            if (setThreadCount){
                setThreadCount(threadCount);
            }
        }
        if (ImGui::SliderInt("QuickSort cutoff", &quickSortCutoff, 1, 1000)) {
            if (setQuicksortCutoff){
                setQuicksortCutoff(quickSortCutoff);
            }
        }


        ImGui::End();
    }
}

void FrontendUI::setThreadCountCallback(std::function<void(int value)> callback) {
    setThreadCount = callback;
}

void FrontendUI::setQuicksortCutoffCallback(std::function<void(int value)> callback) {
    setQuicksortCutoff = callback;
}

void FrontendUI::setCurrentTimeChangeCallback(std::function<void(int value)> callback) {
    setCurrentTime = callback;
}

void FrontendUI::produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage) {
    resultDrawStage->meshesToRender = std::make_shared<MeshesToRender>();

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
    auto uboPart = m_device->createUniformBufferChunk(sizeof(mathfu::mat4));
    uboPart->setUpdateHandler([ortho_projection](IUniformBufferChunk* self) {
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
                    meshTemplate.scissorOffset = {(int)clip_rect.x, (int)(fb_height - clip_rect.w)};
                    meshTemplate.scissorSize = {(int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y)};

                    meshTemplate.ubo[1] = uboPart;
                    meshTemplate.textureCount = 1;
                    meshTemplate.texture[0] = pcmd->TextureId;

                    meshTemplate.start = pcmd->IdxOffset * 2;
                    meshTemplate.end = pcmd->ElemCount;

                    resultDrawStage->meshesToRender->meshes.push_back(m_device->createMesh(meshTemplate));
                }
            }
        }
    }
}

void FrontendUI::setOpenWMOSceneByfdidCallback(std::function<void(int wmoFDid)> callback) {
    this->openWMOSceneByfdid = callback;
}

