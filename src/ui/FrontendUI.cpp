//
// Created by deamon on 20.12.19.
//

#include "FrontendUI.h"

#ifndef __ANDROID_API__
#include "imguiLib/imguiImpl/imgui_impl_glfw.h"
#else
#include <imguiImpl/imgui_impl_android.h>
#endif

#include <iostream>
#include <mathfu/glsl_mappings.h>
#include <groupPanel/groupPanel.h>
#include <disablableButton/disablableButton.h>
#include <compactColorPicker/compactColorPicker.h>
#include <imageButton2/imageButton2.h>
#include <stateSaver/stateSaver.h>
#include "imguiLib/fileBrowser/imfilebrowser.h"
#include "../persistance/CascRequestProcessor.h"
#include "../../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../wowViewerLib/src/engine/objects/scenes/wmoScene.h"
#include "../../wowViewerLib/src/engine/objects/scenes/m2Scene.h"
#include "../screenshots/screenshotMaker.h"
#include "../persistance/HttpRequestProcessor.h"
#include "../exporters/gltfExporter/GLTFExporter.h"
#include "../../wowViewerLib/src/engine/objects/scenes/NullScene.h"
#include "../database/CSqliteDB.h"
#include "../database/CEmptySqliteDB.h"
#include "../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "renderer/uiScene/IFrontendUIBufferCreate.h"
#include "renderer/uiScene/FrontendUIRendererFactory.h"
#include "../../wowViewerLib/src/renderer/mapScene/MapSceneRendererFactory.h"
#include "wheelCapture/wheelCapture.h"
#include "childWindow/keysUpdateWorkflow/KeysUpdateWorkflow.h"
#include "imgui_notify.h"
#include "hasFocus/imguiHasFocus.h"

FrontendUI::FrontendUI(HApiContainer api) {
    m_api = api;

    this->createDatabaseHandler();
    m_uiRenderer = FrontendUIRendererFactory::createForwardRenderer(m_api->hDevice);
    this->createDefaultprocessor();

    m_backgroundScene = std::make_shared<SceneWindow>(api, true, m_uiRenderer);
}

void FrontendUI::composeUI() {
    ZoneScoped;

    {
        auto processingFrame = m_api->hDevice->getFrameNumber();
        m_api->hDevice->setCurrentProcessingFrameNumber(processingFrame);
    }
    if (!m_backgroundScene) {
        getOrCreateWindow()->openM2SceneByfdid(194418, {});
    }

    if (m_dataExporter != nullptr) {
        m_dataExporter->process();
        if (m_dataExporter->isDone())
            m_dataExporter = nullptr;
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

    if (ImGui::BeginPopupModal("Casc succeed"))
    {
        ImGui::Text("CASC storage successfully opened");
        if (ImGui::Button("Ok", ImVec2(-1, 23))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    //Show filePicker
    fileDialog.Display();
    createFileDialog.Display();

    if (fileDialog.HasSelected()) {
        std::cout << "Selected filename " << fileDialog.GetSelected().string() << std::endl;
        std::string cascPath = fileDialog.GetSelected().string();
        BuildDefinition buildDef = fileDialog.getProductBuild();

        if (!tryOpenCasc(cascPath, buildDef)) {
            ImGui::OpenPopup("Casc failed");
            cascOpened = false;
        } else {
            ImGui::OpenPopup("Casc succeed");
            cascOpened = true;
        }
        fileDialog.ClearSelected();
    }
    if (createFileDialog.HasSelected()) {
        screenshotFilename = createFileDialog.GetSelected().string();
        needToMakeScreenshot = true;

        createFileDialog.ClearSelected();
    }


    static bool show_demo_window = false;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    if (m_databaseUpdateWorkflow != nullptr) {
        if (m_databaseUpdateWorkflow->isDatabaseUpdated()) {
            m_databaseUpdateWorkflow = nullptr;
            createDatabaseHandler();
        } else {
            m_databaseUpdateWorkflow->render();
        }
    }
    if (m_keyUpdateWorkFlow != nullptr && !m_keyUpdateWorkFlow->render()) {
        m_keyUpdateWorkFlow = nullptr;
    }

    showSettingsDialog();
    showQuickLinksDialog();
    showFileList();
    showMapSelectionDialog();
    showMakeScreenshotDialog();
    showCurrentStatsDialog();
    showMinimapGenerationSettingsDialog();
    showBlpViewer();
    showM2Viewer();

    ImGui::RenderNotifications();

    if (m_debugRenderWindow)
        m_debugRenderWindow->draw();

    m_currentActiveScene = nullptr;
    if (!ImGui::HasFocus())
        m_currentActiveScene = m_backgroundScene;

    for (auto &window : m_m2Windows) {
        if (window && window->isActive()) {
            m_currentActiveScene = window;
        }
    }

    if (m_currentActiveScene) {
        m_lastActiveScene = m_currentActiveScene;
    }

    // Rendering
    ImGui::Render();
}

inline void drawColumnF(const std::string &text, const float &value) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text(text.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%.3f", value);
}
inline void drawColumnVec4(const std::string &text, const mathfu::vec4 &value) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text(text.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%.3f, %.3f, %.3f, %.3f", value.x, value.y, value.z, value.w);
}

inline void drawColumnColorVec3(const std::string &text, const std::string &name, const mathfu::vec3 &value) {
    ImGui::TableNextRow();

    const std::string message = "(%.3f, %.3f, %.3f)";
    const std::string varName = name + "##b";
    ImGui::TableNextColumn();
    ImGui::Text(text.c_str());

    ImGui::TableNextColumn();
    ImGui::Text(message.c_str(),
                value.x,
                value.y,
                value.z);
    ImGui::SameLine();
    ImGui::ColorButton(varName.c_str(), ImVec4(value.x, value.y, value.z, 0));
}
inline void drawColumnVec3(const std::string &text, const std::string &name, const mathfu::vec3 &value) {
    ImGui::TableNextRow();

    const std::string message = "(%.3f, %.3f, %.3f)";
    const std::string varName = name + "##b";
    ImGui::TableNextColumn();
    ImGui::Text(text.c_str());

    ImGui::TableNextColumn();
    ImGui::Text(message.c_str(),
                value.x,
                value.y,
                value.z);
}

void FrontendUI::showCurrentStatsDialog() {
    static float f = 0.0f;
    static int counter = 0;

    if (showCurrentStats) {
        ImGui::Begin("Current stats",
                     &showCurrentStats);                          // Create a window called "Hello, world!" and append into it.

        mathfu::vec3 cameraPos, lookAt;
        getCameraPos(cameraPos, lookAt);

        if (ImGui::CollapsingHeader("Camera position")) {


            ImGui::Text("Current camera position: (%.1f,%.1f,%.1f)", cameraPos[0], cameraPos[1], cameraPos[2]);
            ImGui::Text("Current camera lookAt: (%.1f,%.1f,%.1f)", lookAt[0], lookAt[1], lookAt[2]);

            if (m_api->getConfig()->doubleCameraDebug) {
                static float debugCameraPosition[3] = {0, 0, 0};
                getDebugCameraPos(debugCameraPosition[0], debugCameraPosition[1], debugCameraPosition[2]);

                ImGui::Text("Current debug camera position: (%.1f,%.1f,%.1f)",
                            debugCameraPosition[0], debugCameraPosition[1], debugCameraPosition[2]);
            }
            ImGui::Separator();
        }
        if (ImGui::CollapsingHeader("General statistics")) {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            ImGui::Text("Uniform data for GPU: %.3f MB", m_api->hDevice->getUploadSize() / (1024.0f * 1024.0f));

            float m2s = m2SizeLoaded / 1024.0f / 1024.f;
            ImGui::Text("Total size of m2 files loaded %f MB", m2s);
            int l_blpTexturesLoaded = blpTexturesLoaded;
            float l_blpTexturesSizeLoaded = blpTexturesSizeLoaded / 1024.0f / 1024.f;
            ImGui::Text("Current blp files loaded %d", l_blpTexturesLoaded);
            ImGui::Text("Current blp files size %f MB", l_blpTexturesSizeLoaded);

            int l_blpTexturesVulkanLoaded = blpTexturesVulkanLoaded;
            float l_blpTexturesVulkanSizeLoaded = blpTexturesVulkanSizeLoaded / 1024.0f / 1024.f;
            ImGui::Text("Current blp vulkan textures loaded %d", l_blpTexturesVulkanLoaded);
            ImGui::Text("Current blp vulkan textures %f MB", l_blpTexturesVulkanSizeLoaded);
        }

        auto activeScene = m_lastActiveScene.lock();
        if (activeScene && activeScene->hasRenderer()) {
            auto mapPlan = activeScene->getLastPlan();
            if (mapPlan) {
                auto &cullStageData = mapPlan;

                int adt_x = worldCoordinateToAdtIndex(cameraPos.y);
                int adt_y = worldCoordinateToAdtIndex(cameraPos.x);

                if (ImGui::CollapsingHeader("Current Scene Data")) {
                    ImGui::Text("Current area id: %d", mapPlan->areaId);
                    ImGui::Text("Current area file: (%d, %d)", adt_x, adt_y);
                    ImGui::Text("Current parent area id: %d", mapPlan->parentAreaId);
                    ImGui::Text("Current adt area id: %d", mapPlan->adtAreadId);
                    ImGui::Text("Current wmo area name: %s", mapPlan->wmoAreaName.c_str());
                    ImGui::Text("Current area name: %s", mapPlan->areaName.c_str());
                    ImGui::Separator();

                    {
                        int modelFdid = 0;
                        std::string modelFileName = "";
                        if (mapPlan->m_currentWMO != emptyWMO) {
                            auto l_currentWmo = wmoFactory->getObjectById<0>(mapPlan->m_currentWMO);

                            if (l_currentWmo != nullptr) {
                                modelFdid = l_currentWmo->getModelFileId();
                                modelFileName = l_currentWmo->getModelFileName();
                            }
                        }
                        if (modelFileName.empty()) {
                            ImGui::Text("Current WMO: %d", modelFdid);
                        } else {
                            ImGui::Text("Current WMO: %s", modelFileName.c_str());
                        }
                    }

                    ImGui::Text("Current WMO group: %d", mapPlan->m_currentWmoGroup);
                }

                if (ImGui::CollapsingHeader("Objects Drawn/Culled")) {
                    auto &adtArray = mapPlan->adtArray;
                    auto &wmoArray = mapPlan->wmoArray;
                    auto &wmoGroupArray = mapPlan->wmoGroupArray;
                    auto &m2Array = mapPlan->m2Array;

                    ImGui::Text("Candidates WMO %d", wmoArray.getCandidates().size());
                    ImGui::Text("Candidates M2 objects %d", m2Array.getCandidates().size());

                    ImGui::Text("Rendered ADT files %d", adtArray.size());
                    ImGui::Text("Rendered WMO %d", wmoArray.getToDrawn().size());
                    ImGui::Text("Rendered WMO groups %d", wmoGroupArray.getToDraw().size());
                    ImGui::Text("Rendered M2 objects %d", m2Array.getDrawn().size());

                    ImGui::Separator();

                    int m2ObjectsBeforeCullingExterior = 0;
                    if (cullStageData->viewsHolder.getExterior() != nullptr) {
                        m2ObjectsBeforeCullingExterior = cullStageData->viewsHolder.getExterior()->m2List.getCandidates().size();
                    }

                    int wmoGroupsInExterior = 0;
                    if (cullStageData->viewsHolder.getExterior() != nullptr) {
                        wmoGroupsInExterior = cullStageData->viewsHolder.getExterior()->wmoGroupArray.getToDraw().size();
                    }

                    int m2ObjectsDrawn = cullStageData != nullptr ? cullStageData->m2Array.getDrawn().size() : 0;
                    int wmoObjectsBeforeCull =
                        cullStageData != nullptr ? cullStageData->wmoArray.getCandidates().size() : 0;

                    ImGui::Text("M2 objects drawn: %s", std::to_string(m2ObjectsDrawn).c_str());
                    ImGui::Text("WMO Groups in Exterior: %s", std::to_string(wmoGroupsInExterior).c_str());
                    ImGui::Text("Interiors (aka group WMOs): %s",
                                std::to_string(cullStageData->viewsHolder.getInteriorViews().size()).c_str());
                    ImGui::Text("M2 Objects Before Culling in Exterior: %s",
                                std::to_string(m2ObjectsBeforeCullingExterior).c_str());
                    ImGui::Text("WMO objects before culling: %s", std::to_string(wmoObjectsBeforeCull).c_str());

                    ImGui::Separator();
                }

                if (ImGui::CollapsingHeader("Active db2 lights")) {
                    if (cullStageData != nullptr && cullStageData->frameDependentData != nullptr ) {

                        ImGui::Text("List of current ZoneLight.db2 ids:");
                        for (const auto &zoneLight : cullStageData->frameDependentData->stateForConditions.currentZoneLights) {
                            ImGui::Text("%d %f", zoneLight.id, zoneLight.blend);
                        }

                        ImGui::Separator();
                        ImGui::Text("List of current Light.db2 ids:");

                        for (const auto &lightId : cullStageData->frameDependentData->stateForConditions.currentLightIds) {
                            ImGui::Text("%d %f", lightId.id, lightId.blend);
                        }

                        ImGui::Separator();

                        ImGui::Text("List of current LightParams.db2 ids:");
                        for (const auto &lightParamId : cullStageData->frameDependentData->stateForConditions.currentLightParams) {
                            ImGui::Text("%d %f", lightParamId.id, lightParamId.blend);
                        }

                        ImGui::Separator();

                        ImGui::Text("List of current LightSkyBox.db2 ids:");
                        for (const auto &skyboxId : cullStageData->frameDependentData->stateForConditions.currentSkyboxIds) {
                            ImGui::Text("%d %f", skyboxId.id, skyboxId.blend);
                        }
                    }
                }

                if (ImGui::CollapsingHeader("Current fog params")) {
                    if (cullStageData != nullptr && cullStageData->frameDependentData != nullptr &&
                        !cullStageData->frameDependentData->fogResults.empty() )
                    {
                        ImGui::BeginTable("CurrentFogParams", 2);

                        const auto &fogData = cullStageData->frameDependentData->fogResults[0];
                        drawColumnF("Fog End:", fogData.FogEnd);
                        drawColumnF("Fog Scalar:", fogData.FogScaler);
                        drawColumnF("Fog Density:", fogData.FogDensity);
                        drawColumnF("Fog Height:", fogData.FogHeight);
                        drawColumnF("Fog Height Scaler:", fogData.FogHeightScaler);
                        drawColumnF("Fog Height Density:", fogData.FogHeightDensity);
                        drawColumnF("Sun Fog Angle:", fogData.SunFogAngle);
                        drawColumnF("End Fog Color Distance:", fogData.EndFogColorDistance);
                        drawColumnF("Sun Fog Strength:", fogData.SunFogStrength);
                        drawColumnVec4("Fog Height Coefficients:", fogData.FogHeightCoefficients);
                        drawColumnVec4("Main Fog Coefficients:", fogData.MainFogCoefficients);
                        drawColumnVec4("Height Density Fog Coefficients:", fogData.HeightDensityFogCoefficients);
                        drawColumnF("Fog Z Scalar:", fogData.FogZScalar);
                        drawColumnF("Legacy Fog Scalar:", fogData.LegacyFogScalar);
                        drawColumnF("Main Fog Start Dist:", fogData.MainFogStartDist);
                        drawColumnF("Main Fog End Dist:", fogData.MainFogEndDist);
                        drawColumnF("Fog Blend Alpha:", fogData.FogBlendAlpha);
                        drawColumnF("Fog Start Offset:", fogData.FogStartOffset);
                        drawColumnColorVec3("Sun Fog Color:", "SunFogColor", fogData.SunFogColor);
                        drawColumnColorVec3("Fog Color:", "FogColor", fogData.FogColor);
                        drawColumnColorVec3("End Fog Color:", "EndFogColor", fogData.EndFogColor);
                        drawColumnColorVec3("Fog Height Color:", "FogHeightColor", fogData.FogHeightColor);
                        drawColumnColorVec3("Height End Fog Color:", "HeightEndFogColor", fogData.HeightEndFogColor);
                        ImGui::EndTable();

                        ImGui::Separator();
                    }
                }
                if (ImGui::CollapsingHeader("Current Sky colors")) {
                    if (cullStageData->frameDependentData != nullptr) {
                        auto frameDependentData = cullStageData->frameDependentData;

                        ImGui::BeginTable("CurrentSkyColors", 2);
                        drawColumnColorVec3("Sky Top:", "SkyTopColor", frameDependentData->skyColors.SkyTopColor.xyz());
                        drawColumnColorVec3("Sky Middle:", "SkyMiddleColor", frameDependentData->skyColors.SkyMiddleColor.xyz());
                        drawColumnColorVec3("Sky Band1:", "SkyBand1Color", frameDependentData->skyColors.SkyBand1Color.xyz());
                        drawColumnColorVec3("Sky Band2:", "SkyBand2Color", frameDependentData->skyColors.SkyBand2Color.xyz());
                        drawColumnColorVec3("Sky Smog:", "SkySmogColor", frameDependentData->skyColors.SkySmogColor.xyz());
                        drawColumnColorVec3("Sky Fog:", "SkyFogColor", frameDependentData->skyColors.SkyFogColor.xyz());
                        ImGui::EndTable();
                    }
                }
                if (ImGui::CollapsingHeader("Current global light")) {
                    if (cullStageData->frameDependentData != nullptr) {
                        auto frameDependentData = cullStageData->frameDependentData;

                        ImGui::BeginTable("CurrentLightParams", 2);

                        drawColumnColorVec3("Exterior Ambient:", "ExteriorAmbient", frameDependentData->colors.exteriorAmbientColor.xyz());
                        drawColumnColorVec3("Exterior Horizon Ambient:", "ExteriorHorizonAmbient", frameDependentData->colors.exteriorHorizontAmbientColor.xyz());
                        drawColumnColorVec3("Exterior Ground Ambient:", "ExteriorGroundAmbient", frameDependentData->colors.exteriorGroundAmbientColor.xyz());
                        drawColumnColorVec3("Exterior Direct Color:", "ExteriorDirectColor", frameDependentData->colors.exteriorDirectColor.xyz());
                        drawColumnVec3("Exterior Direct Dir:", "ExteriorDirectDir", frameDependentData->exteriorDirectColorDir);
                        drawColumnF("Glow:", cullStageData->frameDependentData->currentGlow);
                        ImGui::EndTable();
                    }
                }
            }
        }
        ImGui::End();
    }
}

void FrontendUI::showBlpViewer() {
    if (!m_blpViewerWindow) return;

    if (!m_blpViewerWindow->draw()) {
        m_blpViewerWindow = nullptr;
    }
}

void FrontendUI::showM2Viewer() {

    for (auto &window : m_m2Windows) {
        if (window && !window->draw()) {
            window = nullptr;
        }
    }
}

void FrontendUI::showFileList() {
    if (m_fileListWindow && !m_fileListWindow->draw()) {
        m_fileListWindow = nullptr;
    }

    if (m_blpFileViewerWindow && !m_blpFileViewerWindow->draw()) {
        m_blpFileViewerWindow = nullptr;
    }
}

void FrontendUI::showMapSelectionDialog() {
    if (m_mapSelectDialog && !m_mapSelectDialog->draw()) {
        m_mapSelectDialog = nullptr;
    }
}

void FrontendUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
//            ImGui::MenuItem("(dummy menu)", NULL, false, false);
            if (ImGui::MenuItem("Open CASC Storage...")) {
                fileDialog.Open();
            }

            if (ImGui::MenuItem("Open Map selection", "", false, cascOpened)) {
                if (!m_mapSelectDialog) {
                    auto weakPtr = weak_from_this();
                    m_mapSelectDialog = std::make_shared<MapSelectDialog>(m_api, m_uiRenderer, [weakPtr] -> std::shared_ptr<SceneWindow> {
                        auto sharedPtr = weakPtr.lock();
                        if (!sharedPtr) return nullptr;

                        return sharedPtr->getOrCreateWindow();
                    });
                }
                m_mapSelectDialog->show();
            }
            if (ImGui::MenuItem("Unload scene", "", false, cascOpened)) {
                unloadScene();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Update database", "", false, cascOpened)) {
                m_databaseUpdateWorkflow = std::make_shared<DatabaseUpdateWorkflow>(
                        m_api,
                        contains(fileDialog.getProductBuild().productName, "classic")
                    );
            }
            if (ImGui::MenuItem("Update keys", "", false)) {
                m_keyUpdateWorkFlow = std::make_shared<KeysUpdateWorkflow>(
                    std::dynamic_pointer_cast<CascRequestProcessor>(m_api->requestProcessor)
                );
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Open File List", "", false, cascOpened)) {
                if (!m_fileListWindow)
                    m_fileListWindow = std::make_shared<FileListWindow>(m_api, [&](int fileId, const std::string &fileType){
                        if (fileType == "blp") {
                            if (m_blpFileViewerWindow == nullptr)
                                m_blpFileViewerWindow = std::make_shared<BLPViewer>(m_api, m_uiRenderer, true);

                            m_blpFileViewerWindow->loadBlp(std::to_string(fileId));
                        } else if (fileType == "m2") {
                            getOrCreateWindow()->openM2SceneByfdid(fileId, {});
                        } else if (fileType == "wmo") {
                            getOrCreateWindow()->openWMOSceneByfdid(fileId);
                        } else if (fileType == "wdt") {
                            getOrCreateWindow()->openMapByIdAndWDTId(0, fileId, 0,0,0, -1);
                        }
                    });
            }
            if (ImGui::MenuItem("Open current stats")) { showCurrentStats = true; }
            if (ImGui::MenuItem("Test notification")) {
                ImGui::InsertNotification({ ImGuiToastType_Info, 3000, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation" });
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open settings")) {showSettings = true;}
            if (ImGui::MenuItem("Open QuickLinks", "", nullptr,cascOpened)) {showQuickLinks = true;}
            if (ImGui::MenuItem("Open MapConstruction")) {showMapConstruction = true;}
            if (ImGui::MenuItem("Open minimap generator", "", false, cascOpened)) {
                showMinimapGeneratorSettings = true;
            }
            if (ImGui::MenuItem("Open BLP viewer", "", false, cascOpened)) {
                if (!m_blpViewerWindow)
                    m_blpViewerWindow = std::make_shared<BLPViewer>(m_api, m_uiRenderer);
            }

            /*
            if (ImGui::MenuItem("Test export")) {
                if (m_currentScene != nullptr) {
                    exporter = std::make_shared<GLTFExporter>("./gltf/");
//                    m_currentScene->exportScene(exporter.get());
                    exporterFramesReady = 0;
                }
            }
             */
            if (ImGui::MenuItem("Test data export")) {
                m_dataExporter = std::make_shared<DataExporter::DataExporterClass>(m_api);
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

void FrontendUI::initImgui(
#ifdef __ANDROID_API__
    ANativeWindow *window
#else
    GLFWwindow *window
#endif
) {

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    this->imguiContext = ImGui::CreateContext();
    auto &fileDialog = this->fileDialog;
    auto &windowWidth = this->windowWidth;
    auto &windowHeight = this->windowHeight;
    addIniCallback(imguiContext,
               "Global Settings",
               [&fileDialog, &windowWidth, &windowHeight](const char* line) -> void {
                    char lastCascDir[256];
                    if (sscanf(line, "lastCascDir=%[^\n\r]", &lastCascDir) == 1) {
                        std::string s = std::string(&lastCascDir[0]);
//                        std::cout << " read string s = " << s << std::endl;

                        if (ghc::filesystem::exists(s)) {
                            fileDialog.SetPwd(s);
                        }
                    }
                   int lastWidth = 0;
                   if (sscanf(line, "windowWidth=%d", &lastWidth) == 1) {
                       windowWidth = lastWidth;
                   }
                   int lastHeight = 0;
                   if (sscanf(line, "windowHeight=%d", &lastHeight) == 1) {
                       windowHeight = lastHeight;
                   }
               },
               [&fileDialog, &windowWidth, &windowHeight](ImGuiTextBuffer* buf) -> void {
                   std::string currPath = fileDialog.GetSelected().string();
                   buf->appendf("lastCascDir=%s\n", currPath.c_str());
                   buf->appendf("windowWidth=%s\n", std::to_string(windowWidth).c_str());
                   buf->appendf("windowHeight=%s\n", std::to_string(windowHeight).c_str());
               }
       );

    ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);

    ImGuiIO &io = ImGui::GetIO();
    (void) io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
#ifdef __ANDROID_API__
    ImGui_ImplAndroid_Init(window);
#else
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#endif

    this->createFontTexture();
}

void FrontendUI::newFrame() {

//    ImGui_ImplOpenGL3_NewFrame();

#ifdef __ANDROID_API__
    ImGui_ImplAndroid_NewFrame();
#else
    ImGui_ImplGlfw_NewFrame();
#endif
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.uiScale = this->uiScale;
    io.DisplaySize = ImVec2((float)io.DisplaySize.x / io.uiScale, (float)io.DisplaySize.y / io.uiScale);
}
void FrontendUI::shutDown() {
    ImGui_ImplGlfw_Shutdown();
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
    if (ImGui::Button("model without skin", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        getOrCreateWindow()->openM2SceneByfdid(5099010, replacementTextureFDids);
    }
    if (ImGui::Button("Model with buggy particles", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        getOrCreateWindow()->openM2SceneByfdid(5587940, replacementTextureFDids);
    }
    if (ImGui::Button("crystal song bush", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        getOrCreateWindow()->openM2SceneByfdid(194418, replacementTextureFDids);
    }
    if (ImGui::Button("bugged decal", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        getOrCreateWindow()->openM2SceneByfdid(946969, replacementTextureFDids);
    }
    if (ImGui::Button("Some model", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 4952373;
        replacementTextureFDids[12] = 4952379;
        getOrCreateWindow()->openM2SceneByfdid(4870631, replacementTextureFDids);
    }
    if (ImGui::Button("nightborne model", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(1810676, replacementTextureFDids);
    }
    if (ImGui::Button("Tomb of sargares hall", ImVec2(-1, 0))) {
        getOrCreateWindow()->openMapByIdAndWDTId(1676, 1532459, 6289, -801, 3028, -1);
    }
    if (ImGui::Button("Legion Dalaran", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(1120838);
    }
    if (ImGui::Button("8du_zuldazarraid_antiportal01.wmo", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(2574165);
    }
    if (ImGui::Button("someShip.wmo", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(4638404);
    }
    if (ImGui::Button("Vanilla karazhan", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByFilename("world/wmo/dungeon/az_karazahn/karazhan.wmo");
    }
    if (ImGui::Button("10xt_exterior_glacialspike01.wmo (parallax)", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(4419436);
    }
    if (ImGui::Button("14654.wmo (parallax)", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(4222547);
    }
    if (ImGui::Button("10.0 Raid WMO", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(4282557);
    }
    if (ImGui::Button("(WMO) Model with broken portal culling", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(4217818);
    }
    if (ImGui::Button("(WMO) NPE Ship with waterfall model", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(3314067);
    }
    if (ImGui::Button("(WMO) Gazebo 590182", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(590182);
    }
    if (ImGui::Button("Hearthstone Tavern", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(2756726);
    }
    if (ImGui::Button("Halls of Awakening", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(5373469);
    }
    if (ImGui::Button("Original WVF1 model", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(2445860, replacementTextureFDids);
    }
    if (ImGui::Button("Stormwind mage portal", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(2394711, replacementTextureFDids);
    }
    if (ImGui::Button("kodobeasttame", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(124697, replacementTextureFDids);
    }

//    if (ImGui::Button("Azeroth map: Lion's Rest (Legion)", ImVec2(-1, 0))) {
//        openMapByIdAndFilename(0, "azeroth", -8739, 944, 200);
//    }
    if (ImGui::Button("Nyalotha map", ImVec2(-1, 0))) {
        getOrCreateWindow()->openMapByIdAndWDTId(2217, 2842322, -11595, 9280, 260, -1);
    }
    if (ImGui::Button("WMO 1247268", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(1247268);
    }
    if (ImGui::Button("Ironforge.wmo", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(113992);
    }

    if (ImGui::Button("Some item", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[1] = 528801;
        for (auto &fdid: replacementTextureFDids) {
            fdid = 1029337;
        }
        getOrCreateWindow()->openM2SceneByfdid(1029334, replacementTextureFDids);
    }
    if (ImGui::Button("IGC Anduin", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3849312, replacementTextureFDids);
    }
    if (ImGui::Button("Steamscale mount", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(2843110, replacementTextureFDids);
    }
    if (ImGui::Button("Spline emitter", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(1536145, replacementTextureFDids);
    }
    if (ImGui::Button("Nether collector top", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(193157, replacementTextureFDids);
    }
    if (ImGui::Button("Сollector top", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(113540);
    }
    if (ImGui::Button("10.0 unk model", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);

        getOrCreateWindow()->openM2SceneByfdid(4519090, replacementTextureFDids);
    }
    if (ImGui::Button("10.0 strange shoulders", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[2] = 4615508;
//        replacementTextureFDids[3] = 4615508;



        getOrCreateWindow()->openM2SceneByfdid(4614814, replacementTextureFDids);
    }
    if (ImGui::Button("DF chicken", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 4007136;

        getOrCreateWindow()->openM2SceneByfdid(4005446, replacementTextureFDids);
    }
    if (ImGui::Button("Fox", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 3071379;

        getOrCreateWindow()->openM2SceneByfdid(3071370, replacementTextureFDids);
    }
    if (ImGui::Button("COT hourglass", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(190850, replacementTextureFDids);
    }
    if (ImGui::Button("Gryphon roost", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(198261, replacementTextureFDids);
    }
    if (ImGui::Button("Northrend Human Inn", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(114998);
    }
    if (ImGui::Button("Strange WMO", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(2342637);
    }
    if (ImGui::Button("Flyingsprite", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);

        replacementTextureFDids[11] = 3059000;
        getOrCreateWindow()->openM2SceneByfdid(3024835, replacementTextureFDids);
    }
    if (ImGui::Button("maldraxxusflyer", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 3196375;
        getOrCreateWindow()->openM2SceneByfdid(3196372, replacementTextureFDids);
    }
    if (ImGui::Button("ridingphoenix", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);

        getOrCreateWindow()->openM2SceneByfdid(125644, replacementTextureFDids);
    }
    if (ImGui::Button("Upright Orc", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[1] = 3844710;
        getOrCreateWindow()->openM2SceneByfdid(1968587, replacementTextureFDids);
    }
    if (ImGui::Button("quillboarbrute.m2", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 1786107;
        getOrCreateWindow()->openM2SceneByfdid(1784020, replacementTextureFDids);
    }
    if (ImGui::Button("WMO With Horde Symbol", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(1846142);
    }
    if (ImGui::Button("WMO 3565693", ImVec2(-1, 0))) {
        getOrCreateWindow()->openWMOSceneByfdid(3565693);
    }

    if (ImGui::Button("Vanilla login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(131970, replacementTextureFDids);
    }
    if (ImGui::Button("BC login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(131982, replacementTextureFDids);
        //        auto ambient = mathfu::vec4(0.3929412066936493f, 0.26823532581329346f, 0.3082353174686432f, 0);
        m_api->getConfig()->BCLightHack = true;
    }
    if (ImGui::Button("Wrath login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(236122, replacementTextureFDids);
    }

    if (ImGui::Button("Cataclysm login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(466614, replacementTextureFDids);
    }
    if (ImGui::Button("Panda login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(631713, replacementTextureFDids);
    }
    if (ImGui::Button("Draenor login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByName("interface/glues/models/ui_mainmenu_warlords/ui_mainmenu_warlords.m2", replacementTextureFDids);
    }
    if (ImGui::Button("Legion Login Screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(1396280, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }
    if (ImGui::Button("BfA login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(2021650, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }
    if (ImGui::Button("Shadowlands login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3846560, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }

    if (ImGui::Button("DragonLands login screen", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(4684877, replacementTextureFDids);
//            m_api->getConfig()->setBCLightHack(true);
    }

    if (ImGui::Button("Shadowlands clouds", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3445776, replacementTextureFDids);
    }

    if (ImGui::Button("Pink serpent", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);

        replacementTextureFDids[11] = 2905480;
        replacementTextureFDids[12] = 2905481;
        replacementTextureFDids[13] = 577442;
        getOrCreateWindow()->openM2SceneByfdid(577443, replacementTextureFDids);
    }
    if (ImGui::Button("Wolf", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);

        replacementTextureFDids[11] = 126494;
        replacementTextureFDids[12] = 126495;
        replacementTextureFDids[13] = 0;
        getOrCreateWindow()->openM2SceneByfdid(126487, replacementTextureFDids);
    }

    if (ImGui::Button("Aggramar", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 1599776;
        getOrCreateWindow()->openM2SceneByfdid(1599045, replacementTextureFDids);
    }
    if (ImGui::Button("M2 3087468", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 3087540;
        getOrCreateWindow()->openM2SceneByfdid(3087468, replacementTextureFDids);
    }

    if (ImGui::Button("Nagrand skybox", ImVec2(-1, 0))) {

        getOrCreateWindow()->openM2SceneByfdid(130575, replacementTextureFDids);

    }
    if (ImGui::Button("Torghast raid skybox", ImVec2(-1, 0))) {

        getOrCreateWindow()->openM2SceneByfdid(4001212, replacementTextureFDids);

    }
    if (ImGui::Button("3445776 PBR cloud sky in Maw", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3445776, replacementTextureFDids);
    }
    if (ImGui::Button("M2 3572296", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3572296, replacementTextureFDids);
    }
    if (ImGui::Button("M2 3487959", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3487959, replacementTextureFDids);
    }
    if (ImGui::Button("M2 1729717 waterfall", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(1729717, replacementTextureFDids);
    }
    if (ImGui::Button("Maw jailer", ImVec2(-1, 0))) {
//        3096499,3096495
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 3096499;
        replacementTextureFDids[12] = 3096495;
        getOrCreateWindow()->openM2SceneByfdid(3095966, replacementTextureFDids);
    }
    if (ImGui::Button("Creature with colors", ImVec2(-1, 0))) {
//        3096499,3096495
        getOrCreateWindow()->openM2SceneByfdid(1612576, replacementTextureFDids);
    }
    if (ImGui::Button("IC new sky", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3159936, replacementTextureFDids);
    }


    if (ImGui::Button("vampire candle", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(3184581, replacementTextureFDids);
    }
    if (ImGui::Button("Bog Creature", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 3732358;
        replacementTextureFDids[12] = 3732360;
        replacementTextureFDids[13] = 3732368;

        getOrCreateWindow()->openM2SceneByfdid(3732303, replacementTextureFDids);
    }
    if (ImGui::Button("Bugged ADT (SL)", ImVec2(-1, 0))) {
//        m_currentScene = setScene(m_api, 2, "world/maps/2363/2363_31_31.adt", 0);
    }
    ImGui::Separator();
    ImGui::Text("Models for billboard checking");
    ImGui::NewLine();
    if (ImGui::Button("Dalaran dome", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(203598, replacementTextureFDids);
    }
    if (ImGui::Button("Gift of Nzoth", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(2432705, replacementTextureFDids);
    }
    if (ImGui::Button("Plagueheart Shoulderpad", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(143343, replacementTextureFDids);
    }
    if (ImGui::Button("Dalaran eye", ImVec2(-1, 0))) {
        getOrCreateWindow()->openM2SceneByfdid(243044, replacementTextureFDids);
    }
    if (ImGui::Button("Hand weapon", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[1] = 528801;
        for (auto &fdid: replacementTextureFDids) {
            fdid = 528801;
        }
        getOrCreateWindow()->openM2SceneByfdid(528797, replacementTextureFDids);
    }

    ImGui::End();
}

static HGTexture blpText = nullptr;

void FrontendUI::showSettingsDialog() {
    if(showSettings) {
        ImGui::Begin("Settings", &showSettings);

        //Camera Selection
        auto activeScene = m_lastActiveScene.lock();
        if (activeScene)
        {
            std::string currentCamera;
            auto currentCameraNum = activeScene->getCurrentCameraIndex();
            if (currentCameraNum == -1) {
                currentCamera = "First person";
            } else {
                currentCamera = "Camera Num " + std::to_string(currentCameraNum);
            }


            ImGui::Text("Camera selection");
            ImGui::SameLine();
            if (ImGui::BeginCombo("##combo", currentCamera.c_str())) // The second parameter is the label previewed before opening the combo.
            {
                int cameraNum = activeScene->getCurrentCameraCount();
                {
                    const std::string caption = "First person";
                    if (ImGui::Selectable(caption.c_str(), currentCameraNum == -1)) {
                        activeScene->setCurrentCameraIndex(-1);
                        currentCameraNum = -1;
                    }
                }

                for (int n = 0; n < cameraNum; n++)
                {
                    bool is_selected = (currentCameraNum == n); // You can store your selection however you want, outside or inside your objects
                    std::string caption = "Camera Num " + std::to_string(n);
                    if (ImGui::Selectable(caption.c_str(), is_selected)) {
                        activeScene->setCurrentCameraIndex(n);
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
        }
        ImGui::Separator();

        {
            float fov = m_api->getConfig()->fov;
            if (ImGui::SliderFloat("fov", &fov, 10, 150)) {
                m_api->getConfig()->fov = fov;
            }
        }

        {
            float farPlane = m_api->getConfig()->farPlane;
            if (ImGui::SliderFloat("Far plane", &farPlane, 200, 2000)) {
                m_api->getConfig()->farPlane = farPlane;
                m_api->getConfig()->farPlaneForCulling = farPlane + 50;
            }
        }

        ImGui::Text("Time: %02d:%02d", (int)(currentTime/120), (int)((currentTime/2) % 60));

        if (ImGui::SliderInt("Current time", &currentTime, 0, 2880)) {
            m_api->getConfig()->currentTime = currentTime;
        }

        {
            auto activeScene = m_lastActiveScene.lock();
            auto camera = (activeScene) ? activeScene->getCamera() : nullptr;
            float movementSpeed = (camera) ? camera->getMovementSpeed() : 1.0;
            movementSpeed *= 30.0f;
            if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.3, 100)) {
                if (camera) {
                    camera->setMovementSpeed(movementSpeed * 1.0f/30.0f);
                }
            }
        }
        auto fogDensityIncreaser = m_api->getConfig()->fogDensityIncreaser;
        if (ImGui::SliderFloat("Fog Density", &fogDensityIncreaser, -4, 4)) {
            m_api->getConfig()->fogDensityIncreaser = fogDensityIncreaser;
        }

        if (ImGui::CollapsingHeader("Option toogles")) {


            if (ImGui::Checkbox("Disable glow", &disableGlow)) {
                m_api->getConfig()->disableGlow = disableGlow;
            }

            bool disableFog = m_api->getConfig()->disableFog;
            if (ImGui::Checkbox("Disable fog", &disableFog)) {
                m_api->getConfig()->disableFog = disableFog;
            }

            bool enableLightBuffer = m_api->getConfig()->enableLightBuffer;
            if (ImGui::Checkbox("Enable lightBuffer", &enableLightBuffer)) {
                m_api->getConfig()->enableLightBuffer = enableLightBuffer;
            }
            bool drawDebugLights = m_api->getConfig()->drawDebugLights;
            if (ImGui::Checkbox("Draw Debug lights", &drawDebugLights)) {
                m_api->getConfig()->drawDebugLights = drawDebugLights;
            }

            bool renderADT = m_api->getConfig()->renderAdt;
            if (ImGui::Checkbox("Render ADT", &renderADT)) {
                m_api->getConfig()->renderAdt = renderADT;
            }

            bool renderM2 = m_api->getConfig()->renderM2;
            if (ImGui::Checkbox("Render M2", &renderM2)) {
                m_api->getConfig()->renderM2 = renderM2;
            }

            {
                bool renderM2Decals = m_api->getConfig()->renderM2Decals;
                if (ImGui::Checkbox("Render M2 Decals", &renderM2Decals)) {
                    m_api->getConfig()->renderM2Decals = renderM2Decals;
                }
            }

            {
                bool renderParticles = m_api->getConfig()->maxParticle >= 0;
                if (ImGui::Checkbox("Render Particles", &renderParticles)) {
                    m_api->getConfig()->maxParticle = renderParticles ? 9999 : -1;
                }
            }
            {
                bool renderRibbons = m_api->getConfig()->renderRibbons;
                if (ImGui::Checkbox("Render Ribbons", &renderRibbons)) {
                    m_api->getConfig()->renderRibbons = renderRibbons;
                }
            }

            bool renderWMO = m_api->getConfig()->renderWMO;
            if (ImGui::Checkbox("Render WMO", &renderWMO)) {
                m_api->getConfig()->renderWMO = renderWMO;
            }

            bool renderSkyDom = m_api->getConfig()->renderSkyDom;
            if (ImGui::Checkbox("Render Sky M2", &renderSkyDom)) {
                m_api->getConfig()->renderSkyDom = renderSkyDom;
            }

            bool renderSkyScene = m_api->getConfig()->renderSkyScene;
            if (ImGui::Checkbox("Render SkyScene Models", &renderSkyScene)) {
                m_api->getConfig()->renderSkyScene = renderSkyScene;
            }

            bool renderLiquid = m_api->getConfig()->renderLiquid;
            if (ImGui::Checkbox("Render Liquid", &renderLiquid)) {
                m_api->getConfig()->renderLiquid = renderLiquid;
            }

            bool drawM2BB = m_api->getConfig()->drawM2BB;
            if (ImGui::Checkbox("Render M2 Bounding Box", &drawM2BB)) {
                m_api->getConfig()->drawM2BB = drawM2BB;
            }


            bool discardInvisibleMeshes = m_api->getConfig()->discardInvisibleMeshes;
            if (ImGui::Checkbox("Discard invisible M2 meshes", &discardInvisibleMeshes)) {
                m_api->getConfig()->discardInvisibleMeshes = discardInvisibleMeshes;
            }

            bool ignoreADTHoles = m_api->getConfig()->ignoreADTHoles;
            if (ImGui::Checkbox("Ignore ADT holes for rendering", &ignoreADTHoles)) {
                m_api->getConfig()->ignoreADTHoles = ignoreADTHoles;
            }

            bool ignoreADTColors = m_api->getConfig()->ignoreADTColoring;
            if (ImGui::Checkbox("Ignore ADT colors for rendering", &ignoreADTColors)) {
                m_api->getConfig()->ignoreADTColoring = ignoreADTColors;
            }


            bool disablePortalCulling = !m_api->getConfig()->usePortalCulling;
            if (ImGui::Checkbox("Disable portal culling", &disablePortalCulling)) {
                m_api->getConfig()->usePortalCulling = !disablePortalCulling;
            }

            bool renderPortals = m_api->getConfig()->renderPortals;
            if (ImGui::Checkbox("Render portals", &renderPortals)) {
                m_api->getConfig()->renderPortals = renderPortals;
            }
            bool renderAntiPortals = m_api->getConfig()->renderAntiPortals;
            if (ImGui::Checkbox("Render anti portals", &renderAntiPortals)) {
                m_api->getConfig()->renderAntiPortals = renderAntiPortals;
            }

            if (renderPortals || renderAntiPortals) {
                bool renderPortalsIgnoreDepth = m_api->getConfig()->renderPortalsIgnoreDepth;
                if (ImGui::Checkbox("Ignore depth test for rendering portals", &renderPortalsIgnoreDepth)) {
                    m_api->getConfig()->renderPortalsIgnoreDepth = renderPortalsIgnoreDepth;
                }
            }

            bool useDoubleCameraDebug = m_api->getConfig()->doubleCameraDebug;
            if (ImGui::Checkbox("Enable second camera(for debug)", &useDoubleCameraDebug)) {
                m_api->getConfig()->doubleCameraDebug = useDoubleCameraDebug;
                if (!useDoubleCameraDebug) {
                    m_debugRenderWindow = nullptr;
                    m_debugRenderView = nullptr;
                }
            }
            if (useDoubleCameraDebug && (m_debugRenderWindow == nullptr || m_debugRenderView == nullptr) && m_backgroundScene->hasRenderer()) {
                m_debugRenderView = m_backgroundScene->createRenderView();
                m_debugRenderWindow = std::make_shared<DebugRendererWindow>(m_api, m_uiRenderer, m_debugRenderView);
            }

            {
                bool stopBufferUpdates = m_api->getConfig()->stopBufferUpdates;
                if (ImGui::Checkbox("Stop buffer updates", &stopBufferUpdates)) {
                    m_api->getConfig()->stopBufferUpdates = stopBufferUpdates;
                }

                if (stopBufferUpdates) {
                    bool stepBufferUpdate = m_api->getConfig()->stepBufferUpdate;
                    if (ImGui::Checkbox("Step buffer update", &stepBufferUpdate)) {
                        m_api->getConfig()->stepBufferUpdate = stepBufferUpdate;
                    }
                }
            }

            pauseAnimation = m_api->getConfig()->pauseAnimation;
            if (ImGui::Checkbox("Pause animation", &pauseAnimation)) {
                m_api->getConfig()->pauseAnimation = pauseAnimation;
            }

            if (ImGui::Button("Reset Animation")) {
                resetAnimationCallback();
            }
        }

        if (ImGui::CollapsingHeader("Light options")) {
            auto activeScene = m_lastActiveScene.lock();
            auto mapPlan = activeScene ? activeScene->getLastPlan() : nullptr;

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
            if (mapPlan != nullptr &&
                mapPlan->frameDependentData != nullptr &&
                m_api->getConfig()->globalLighting == EParameterSource::eDatabase) {

                ImGui::SameLine();
                if (ImGui::Button("Edit current colors")) {
                    m_api->getConfig()->exteriorColors = mapPlan->frameDependentData->colors;
                    m_api->getConfig()->globalLighting = EParameterSource::eConfig;
                }
            }

            if (ImGui::RadioButton("Use ambient light from M2  (only for M2 scenes)", &lightSource, 1)) {
                m_api->getConfig()->globalLighting = EParameterSource::eM2;
            }
            if (ImGui::RadioButton("Manual light", &lightSource, 2)) {
                m_api->getConfig()->globalLighting = EParameterSource::eConfig;
            }

            if (m_api->getConfig()->globalLighting == EParameterSource::eConfig) {
                auto config = m_api->getConfig();

                ImGui::BeginTable("CurrentFogParams", 2);

                ImGui::CompactColorPicker("Exterior Ambient", config->exteriorColors.exteriorAmbientColor,true);
                ImGui::CompactColorPicker("Exterior Horizon Ambient",
                                          config->exteriorColors.exteriorHorizontAmbientColor,true);
                ImGui::CompactColorPicker("Exterior Ground Ambient", config->exteriorColors.exteriorGroundAmbientColor,true);
                ImGui::CompactColorPicker("Exterior Direct Color", config->exteriorColors.exteriorDirectColor,true);

                ImGui::EndTable();
            }

            //Glow source
            switch (m_api->getConfig()->glowSource) {
                case EParameterSource::eDatabase: {
                    glowSource = 0;
                    break;
                }
                case EParameterSource::eConfig: {
                    glowSource = 1;
                    break;
                }
                default:
                    glowSource = 1;
            }
            ImGui::Separator();

            if (ImGui::RadioButton("Use glow from database", &glowSource, 0)) {
                m_api->getConfig()->glowSource = EParameterSource::eDatabase;
            }
            if (ImGui::RadioButton("Manual glow", &glowSource, 1)) {
                m_api->getConfig()->glowSource = EParameterSource::eConfig;
            }

            if (m_api->getConfig()->glowSource == EParameterSource::eConfig) {
                if (ImGui::SliderFloat("Custom glow", &customGlow, 0.0, 10)) {
                    m_api->getConfig()->currentGlow = customGlow;
                }
            }
        }
        if (ImGui::CollapsingHeader("Mat options")) {
            auto activeScene = m_lastActiveScene.lock();
            if (activeScene) {
                auto materials = activeScene->getMaterials();
                for (int i = 0; i < materials.size(); i++) {
                    auto &mat = materials[i];

                    const uint32_t thumbnailDim = 256;

                    auto &textureMat = std::get<1>(mat);
                    if (textureMat) {
                        ImGui::BeginChild((std::string("mat##test_") + std::to_string(i)).c_str(), {thumbnailDim+10, thumbnailDim + 25});

                        auto const &matName = std::get<0>(mat);

                        if (ImGui::ImageButton(matName.c_str(), std::get<1>(mat)->uniqueId, {thumbnailDim, thumbnailDim})) {
                            activeScene->setSelectedMat(i);
                        }
                        ImGui::Text(matName.c_str());
                        ImGui::EndChild();
                    }
                    if ((i+1) < materials.size()) {
                        if (ImGui::GetCursorPos().x +
                            (i + 2) * (thumbnailDim + 10 + ImGui::GetStyle().ItemSpacing.x) < ImGui::GetContentRegionMax().x) {
                            ImGui::SameLine();
                        }
                    }
                }
            }
        }


        ImGui::End();
    }
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


HFrameScenario FrontendUI::createFrameScenario(int canvWidth, int canvHeight, double deltaTime) {
    ZoneScoped ;
    if (m_minimapGenerationWindow != nullptr) {
        m_minimapGenerationWindow->process();
    }
    auto l_device = m_api->hDevice;
    auto processingFrame = l_device->getFrameNumber();
    std::function<uint32_t()> updateFrameNumberLambda = [l_device, frame = processingFrame]() -> uint32_t {
        l_device->setCurrentProcessingFrameNumber(frame);
        return frame;
    };

    HFrameScenario scenario = std::make_shared<HFrameScenario::element_type>();
    {
        ViewPortDimensions dimension = {
            {0,     0},
            {static_cast<unsigned int>(canvWidth), static_cast<unsigned int>(canvHeight)}
        };
        m_backgroundScene->setViewPortDimensions(dimension);

        uint32_t debugWidth = 0; uint32_t debugHeight = 0;
        if (m_debugRenderWindow) {
            debugWidth = m_debugRenderWindow->getWidth();
            debugHeight = m_debugRenderWindow->getHeight();
        }

        m_backgroundScene->render(
            deltaTime, m_api->getConfig()->fov, scenario,
            /*
            m_debugRenderView,
            debugWidth,
            debugHeight
             */ nullptr,
            updateFrameNumberLambda
        );
        for (auto &window : m_m2Windows) {
            if (window) {
                window->render(deltaTime, scenario, updateFrameNumberLambda);
            }
        }

        //----------------------
        // Screenshot part
        //----------------------

        auto activeScene = getCurrentActiveScene();
        if (needToMakeScreenshot && activeScene) {
            activeScene->makeScreenshot(m_api->getConfig()->fov,
                                        screenShotWidth, screenShotHeight,
                                        screenshotFilename,
                                        scenario,
                                        updateFrameNumberLambda);

            needToMakeScreenshot = false;
        }

        auto uiFrameInput = std::make_shared<FrameInputParams<ImGuiFramePlan::ImGUIParam>>();
        uiFrameInput->delta = deltaTime * (1000.0f);
        uiFrameInput->frameParameters = std::make_shared<ImGuiFramePlan::ImGUIParam>( ImGui::GetDrawData(), dimension);

        auto clearColor = m_api->getConfig()->clearColor;

        scenario->cullFunctions.push_back(m_uiRenderer->createCullUpdateRenderChain(uiFrameInput, updateFrameNumberLambda));
    }

    return scenario;
}

bool FrontendUI::tryOpenCasc(std::string &cascPath, BuildDefinition &buildDef) {
    HRequestProcessor newProcessor = nullptr;
    std::shared_ptr<WoWFilesCacheStorage> newStorage = nullptr;

    try {
        newProcessor = std::make_shared<CascRequestProcessor>(cascPath, buildDef);
        newStorage = std::make_shared<WoWFilesCacheStorage>(newProcessor.get());
    } catch (...){
        return false;
    };

    m_api->cacheStorage = newStorage;
    m_api->requestProcessor = newProcessor;

    return true;
}



void FrontendUI::unloadScene() {
    if (m_api->cacheStorage) {
        m_api->cacheStorage->actuallDropCache();
    }

    m_backgroundScene->unload();
}

void FrontendUI::resetAnimationCallback() {
//    currentScene->resetAnimation();
}

void FrontendUI::getCameraPos(mathfu::vec3 &cameraPos, mathfu::vec3 &lookAt) {
    auto activeScene = m_lastActiveScene.lock();
    auto camera = activeScene ? activeScene->getCamera() : nullptr;

    if (camera == nullptr) {
        cameraPos = {0,0,0};
        lookAt = {0,0,0};
        return;
    }

    auto camMatrices = camera->getCameraMatrices(0,0,0,0);
    cameraPos = camMatrices->cameraPos.xyz();
    lookAt = camMatrices->lookAt.xyz();
}


void FrontendUI::getDebugCameraPos(float &cameraX, float &cameraY, float &cameraZ) {
    //TODO:
    /*
    auto camera = m_currentActiveScene ? m_currentActiveScene->getCamera() : nullptr;
    if (m_api->debugCamera == nullptr) {
        cameraX = 0; cameraY = 0; cameraZ = 0;
        return;
    }
    float currentCameraPos[4] = {0,0,0,0};
    m_api->debugCamera->getCameraPosition(&currentCameraPos[0]);
    cameraX = currentCameraPos[0];
    cameraY = currentCameraPos[1];
    cameraZ = currentCameraPos[2];
     */
}

inline bool fileExistsNotNull (const std::string& name) {
#ifdef ANDROID
    return false;
#endif

    ghc::filesystem::path p{name};
    std::error_code errorCode;

    bool fileExists = exists(p,errorCode) && ghc::filesystem::file_size(p) > 10;
    if (errorCode) {
        std::cout << "errorCode = " << errorCode.message() << std::endl;
    }

    return fileExists;
}

void FrontendUI::createDefaultprocessor() {

    const char * url = "https://wow.tools/casc/file/fname?buildconfig=9a77c0cdef71f18aaee8ba081865b6fd&cdnconfig=dd2c07aa3d4621529a93921750262d28&filename=";
    const char * urlFileId = "https://wow.tools/casc/file/fdid?buildconfig=9a77c0cdef71f18aaee8ba081865b6fd&cdnconfig=dd2c07aa3d4621529a93921750262d28&filename=data&filedataid=";
//
//Classics
//        const char * url = "https://wow.tools/casc/file/fname?buildconfig=bf24b9d67a4a9c7cc0ce59d63df459a8&cdnconfig=2b5b60cdbcd07c5f88c23385069ead40&filename=";
//        const char * urlFileId = "https://wow.tools/casc/file/fdid?buildconfig=bf24b9d67a4a9c7cc0ce59d63df459a8&cdnconfig=2b5b60cdbcd07c5f88c23385069ead40&filename=data&filedataid=";
//        processor = new HttpZipRequestProcessor(url);
////        processor = new ZipRequestProcessor(filePath);
////        processor = new MpqRequestProcessor(filePath);
    // m_api->requestProcessor = std::make_shared<HttpRequestProcessor>(url, urlFileId);
    BuildDefinition buildDef;
    m_api->requestProcessor = std::make_shared<CascRequestProcessor>("f:\\games\\World Of Warcraft\\:wowt", buildDef);
////        processor->setThreaded(false);
////
    m_api->cacheStorage = std::make_shared<WoWFilesCacheStorage>(m_api->requestProcessor.get());
    overrideCascOpened(true);
}

void FrontendUI::showMinimapGenerationSettingsDialog() {
    if(showMinimapGeneratorSettings) {
        if (m_minimapGenerationWindow == nullptr)
            m_minimapGenerationWindow = std::make_shared<MinimapGenerationWindow>(m_api,
                                                                                  m_uiRenderer,
                                                                                  showMinimapGeneratorSettings);

        m_minimapGenerationWindow->render();
    } else {
        if (m_minimapGenerationWindow != nullptr) {
            m_minimapGenerationWindow = nullptr;
        }
    }
}

void FrontendUI::createDatabaseHandler() {
    bool useEmptyDatabase = false;
    if (fileExistsNotNull("./export.db3")) {
        try{
            m_api->databaseHandler = std::make_shared<CSqliteDB>("./export.db3");
        } catch(std::exception const& e) {
            std::cout << "Failed to open database: " << e.what() << std::endl;
            useEmptyDatabase = true;
        } catch(...) {
            std::cout << "Exception occurred" << std::endl;
        }
    }

    if (useEmptyDatabase) {
        m_api->databaseHandler = std::make_shared<CEmptySqliteDB>();
    }
}

void FrontendUI::createFontTexture() {
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
    // Upload texture to graphics system

    // Store our identifier
    fontMat = this->m_uiRenderer->createUIMaterial({this->m_uiRenderer->uploadFontTexture(pixels, width, height)});
    io.Fonts->TexID = fontMat->uniqueId;
}

std::shared_ptr<SceneWindow> FrontendUI::getOrCreateWindow() {
    //If shift key is pressed -> create a new window
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyShift) {
        for (int i = 0; i < m_m2Windows.size(); i++) {
            if (m_m2Windows[i] == nullptr) {
                auto newWindow = std::make_shared<M2Window>(m_api, m_uiRenderer, std::to_string(i));
                m_m2Windows[i] = newWindow;
                return m_m2Windows[i];
            }
        }
        // Create entry, cause there are no empty space
        auto newWindow = std::make_shared<M2Window>(m_api, m_uiRenderer, std::to_string(m_m2Windows.size()));
        m_m2Windows.push_back(newWindow);
        return newWindow;
    }

    //Return usual background otherwise
    return m_backgroundScene;
}

