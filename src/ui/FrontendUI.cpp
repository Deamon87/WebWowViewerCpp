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
#include "../../wowViewerLib/src/engine/shader/ShaderDefinitions.h"
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

FrontendUI::FrontendUI(HApiContainer api, HRequestProcessor processor) {
    m_api = api;

    this->createDatabaseHandler();
    m_uiRenderer = FrontendUIRendererFactory::createForwardRenderer(m_api->hDevice);
    //this->createDefaultprocessor();

}

void FrontendUI::composeUI() {
    ZoneScoped;

    if (m_dataExporter != nullptr) {
        m_dataExporter->process();
        if (m_dataExporter->isDone())
            m_dataExporter = nullptr;
    }
    if (mapCanBeOpened) {
        if (!adtMinimapFilled && fillAdtSelectionminimap(isWmoMap, mapCanBeOpened )) {
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


    static bool show_demo_window = true;
//    if (show_demo_window)
//        ImGui::ShowDemoWindow(&show_demo_window);

    if (m_databaseUpdateWorkflow != nullptr) {
        if (m_databaseUpdateWorkflow->isDatabaseUpdated()) {
            m_databaseUpdateWorkflow = nullptr;
            createDatabaseHandler();
        } else {
            m_databaseUpdateWorkflow->render();
        }
    }
    if (m_keyUpdateWorkFlow != nullptr) {
        if (!m_keyUpdateWorkFlow->render())
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
    if (m_debugRenderWindow)
        m_debugRenderWindow->draw();

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

void FrontendUI::showCurrentStatsDialog() {
    static float f = 0.0f;
    static int counter = 0;

    if (showCurrentStats) {
        ImGui::Begin("Current stats",
                     &showCurrentStats);                          // Create a window called "Hello, world!" and append into it.

        if (ImGui::CollapsingHeader("Camera position")) {
            static float cameraPosition[3] = {0, 0, 0};
            getCameraPos(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

            ImGui::Text("Current camera position: (%.1f,%.1f,%.1f)", cameraPosition[0], cameraPosition[1],
                        cameraPosition[2]);

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
//            if(getCurrentAreaName) {
            ImGui::Text("Current area name: %s", getCurrentAreaName().c_str());

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

        if (m_sceneRenderer) {
            auto mapPlan = m_sceneRenderer->getLastCreatedPlan();
            if (mapPlan) {
                auto &cullStageData = mapPlan;

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
                        ImGui::Text("List of current Light.db2 ids:");

                        for (auto lightId : cullStageData->frameDependentData->currentLightIds) {
                            ImGui::Text("%d", lightId);
                        }

                        ImGui::Separator();

                        ImGui::Text("List of current LightParams.db2 ids:");
                        for (auto lightParamId : cullStageData->frameDependentData->currentLightParamIds) {
                            ImGui::Text("%d", lightParamId);
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

void FrontendUI::showFileList() {
    if (!m_fileListWindow) return;

    if (!m_fileListWindow->draw()) {
        m_fileListWindow = nullptr;
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
void FrontendUI::filterMapList(const std::string &text) {
    filteredMapList = {};
    for (int i = 0; i < mapList.size(); i++) {
        auto &currentRec = mapList[i];
        if (text == "" ||
            (
                (ci_find_substr(currentRec.MapName, text) != std::string::npos) ||
                (ci_find_substr(currentRec.MapDirectory, text) != std::string::npos) ||
                (ci_find_substr(std::to_string(currentRec.ID), text) != std::string::npos)
            )
            ) {
            filteredMapList.push_back(currentRec);
        }
    }
}
void FrontendUI::fillMapListStrings() {
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
}
void FrontendUI::showAdtSelectionMinimap() {
    ImGui::BeginChild("Adt selection minimap", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar |
                                                       ImGuiWindowFlags_AlwaysVerticalScrollbar);

    float wheel = 0.0f;

    float pivotForZoomX;
    float pivotForZoomY;
    {
        auto windowSize = ImGui::GetWindowSize();
        const ImGuiStyle &style = ImGui::GetStyle();

        if (ImGui::CaptureMouseWheel(wheel)) {
            minimapZoom = pow(2.0f, (std::log(minimapZoom)/std::log(2.0f)) + wheel * 0.1f);
            auto windowPos = ImGui::GetWindowPos();
            auto mousePos = ImGui::GetMousePos();

            float windowRelativeMouseX = mousePos.x - windowPos.x - style.WindowPadding.x / 2.0f;
            float windowRelativeMouseY = mousePos.y - windowPos.y - style.WindowPadding.y / 2.0f;

            pivotForZoomX = windowRelativeMouseX;
            pivotForZoomY = windowRelativeMouseY;
        } else {
            pivotForZoomX = ((windowSize.x - style.WindowPadding.x) / 2.0f);
            pivotForZoomY = ((windowSize.x - style.WindowPadding.y) / 2.0f);
        }
    }

    if (minimapZoom < 0.1)
        minimapZoom = 0.1;




    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

    const float defaultImageDimension = 100.0f;
    float newZoomedSize = defaultImageDimension * minimapZoom;
    if (prevZoomedSize == 0)
        prevZoomedSize = defaultImageDimension *  prevMinimapZoom;

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            if (adtSelectionMinimapMaterials[i][j] != nullptr) {
                if (ImGui::ImageButton(std::to_string(i*64+j).c_str(),
                                       adtSelectionMinimapMaterials[i][j]->uniqueId,
                                       ImVec2(prevZoomedSize, prevZoomedSize))) {
                    auto mousePos = ImGui::GetMousePos();
                    const ImGuiStyle &style = ImGui::GetStyle();
                    auto content = ImGui::GetWindowContentRegionMin();
                    auto windowPos = ImGui::GetWindowPos();
                    auto windowSize = ImGui::GetWindowSize();

                    float windowRelativeMouseX = mousePos.x - windowPos.x - style.WindowPadding.x / 2.0f;
                    float windowRelativeMouseY = mousePos.y - windowPos.y - style.WindowPadding.y / 2.0f;

                    float screenSpaceCoordX = ImGui::GetScrollX() + windowRelativeMouseX - style.WindowPadding.x / 2.0f;
                    float screenSpaceCoordY = ImGui::GetScrollY() + windowRelativeMouseY - style.WindowPadding.y / 2.0f;

                    float adtIndexX = (screenSpaceCoordX) / (prevZoomedSize);
                    float adtIndexY = (screenSpaceCoordY) / (prevZoomedSize);

                    worldPosX = AdtIndexToWorldCoordinate(adtIndexY); //?
                    worldPosY = AdtIndexToWorldCoordinate(adtIndexX);

                    ImGui::OpenPopup("AdtWorldCoordsTest");
                    std::cout << "world coords : x = " << worldPosX << " y = " << worldPosY
                              << std::endl;

                }
            } else {
                ImGui::Dummy(ImVec2(prevZoomedSize, prevZoomedSize));
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

            if (prevMapRec.WdtFileID > 0) {
                openMapByIdAndWDTId(prevMapId, prevMapRec.WdtFileID, worldPosX, worldPosY, 200);
            } else {
                openMapByIdAndFilename(prevMapId, prevMapRec.MapDirectory, worldPosX, worldPosY, 200);
            }
            showSelectMap = false;

            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (prevZoomedSize != newZoomedSize && prevMinimapZoom != minimapZoom) {
        auto windowSize = ImGui::GetWindowSize();
        auto scrollX = ImGui::GetScrollX();
        auto scrollY = ImGui::GetScrollY();
        const ImGuiStyle &style = ImGui::GetStyle();

        float pivotX = ImGui::GetScrollX() + pivotForZoomX - style.WindowPadding.x / 2.0f;
        float pivotY = ImGui::GetScrollY() + pivotForZoomY - style.WindowPadding.y / 2.0f;

        float newScrollX = (pivotX) *  minimapZoom / prevMinimapZoom - pivotForZoomX + style.WindowPadding.x / 2.0f;
        float newScrollY = (pivotY) *  minimapZoom / prevMinimapZoom - pivotForZoomY + style.WindowPadding.y / 2.0f;

        ImGui::SetScrollX(newScrollX);
        ImGui::SetScrollY(newScrollY);
        prevZoomedSize = newZoomedSize;
        prevMinimapZoom = minimapZoom;
    }

    ImGui::EndChild();
}


void FrontendUI::showMapSelectionDialog() {
    if (!showSelectMap) {
//        for (int i = 0; i < 64; i++)
//        for (int j = 0; j < 64; j++)
//            adtSelectionMinimapMaterials[i][j] = nullptr;
        return;
    }

    enum MyMapTableColumnID {
        MyItemColumnID_ID,
        MyItemColumnID_Name,
        MyItemColumnID_Directory,
        MyItemColumnID_WDTId,
        MyItemColumnID_MapType
    };

    if (mapList.size() == 0) {
        getMapList(mapList);
        refilterIsNeeded = true;
    }
    if (refilterIsNeeded) {
        filterMapList(std::string(&filterText[0]));
        fillMapListStrings();

        refilterIsNeeded = false;
        resortIsNeeded = true;
    }

    ImGui::Begin("Map Select Dialog", &showSelectMap);
    {
        ImGui::Columns(2, NULL, true);
        //Left panel
        {
            //Filter
            if (ImGui::InputText("Filter: ", filterText.data(), filterText.size(),
                                 ImGuiInputTextFlags_AlwaysOverwrite)) {
                refilterIsNeeded = true;
            }
            //The table

            ImGui::BeginChild("Map Select Dialog Left panel");
            if (ImGui::BeginTable("MapListSelector", 5, ImGuiTableFlags_Resizable |
                                                        ImGuiTableFlags_Reorderable |
                                                        ImGuiTableFlags_Sortable |
                                                        ImGuiTableFlags_NoHostExtendX |
                                                        ImGuiTableFlags_NoHostExtendY |
                                                        ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                                  ImVec2(-1, -1)
            )) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None, 0.0f, MyItemColumnID_ID);
                ImGui::TableSetupColumn("MapName", ImGuiTableColumnFlags_None, 0.0f, MyItemColumnID_Name);
                ImGui::TableSetupColumn("MapDirectory", ImGuiTableColumnFlags_None, 0.0f, MyItemColumnID_Directory);
                ImGui::TableSetupColumn("WdtFileID", ImGuiTableColumnFlags_None, 0.0f, MyItemColumnID_WDTId);
                ImGui::TableSetupColumn("MapType", ImGuiTableColumnFlags_None, 0.0f, MyItemColumnID_MapType);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs();
                if (sorts_specs && sorts_specs->SpecsDirty)
                    resortIsNeeded = true;

                if (sorts_specs && resortIsNeeded) {
                    std::sort(filteredMapList.begin(), filteredMapList.end(),
                              [sorts_specs](const auto &a, const auto &b) -> bool {

                                  for (int n = 0; n < sorts_specs->SpecsCount; n++) {
                                      const ImGuiTableColumnSortSpecs *sort_spec = &sorts_specs->Specs[n];
                                      int delta = 0;

                                      switch (sort_spec->ColumnUserID) {

                                          case MyItemColumnID_ID:
                                              delta = a.ID - b.ID;
                                              break;
                                          case MyItemColumnID_Name:
                                              delta = a.MapName.compare(b.MapName);
                                              break;
                                          case MyItemColumnID_Directory:
                                              delta = a.MapDirectory.compare(b.MapDirectory);
                                              break;
                                          case MyItemColumnID_WDTId:
                                              delta = a.WdtFileID - b.WdtFileID;
                                              break;
                                          case MyItemColumnID_MapType:
                                              delta = a.MapType - b.MapType;
                                              break;

                                          default:
                                              IM_ASSERT(0);
                                              break;
                                      }
                                      if (delta > 0)
                                          return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? true
                                                                                                            : false;
                                      if (delta < 0)
                                          return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? false
                                                                                                            : true;
                                  }

                                  return (a.ID - b.ID) > 0;
                              });

                    sorts_specs->SpecsDirty = false;
                    resortIsNeeded = false;
                    fillMapListStrings();
                }

                static int selected = -1;
                for (int i = 0; i < filteredMapList.size(); i++) {
                    ImGui::TableNextRow();
                    auto mapRec = filteredMapList[i];

                    ImGui::TableSetColumnIndex(0);

                    if (ImGui::Selectable(mapListStringMap[i][0].c_str(), selected == i,
                                          ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowItemOverlap)) {
                        if (mapRec.ID != prevMapId) {
                            minimapZoom = 0.1;
                            mapCanBeOpened = true;
                            adtMinimapFilled = false;
                            prevMapRec = mapRec;

                            isWmoMap = false;
                            adtSelectionMinimapTextures = {};
                            adtSelectionMinimapMaterials = {};
                            if (mapRec.WdtFileID > 0) {
                                getAdtSelectionMinimap(mapRec.WdtFileID);
                            } else {
                                getAdtSelectionMinimap(
                                    "world/maps/" + mapRec.MapDirectory + "/" + mapRec.MapDirectory + ".wdt");
                            }

                        }
                        prevMapId = mapRec.ID;
                        selected = i;
                    }
                    bool hovered = ImGui::IsItemHovered();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", mapListStringMap[i][1].c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", mapListStringMap[i][2].c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%s", mapListStringMap[i][3].c_str());
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%s", mapListStringMap[i][4].c_str());
                }
                ImGui::EndTable();
            }
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
                        if (prevMapRec.WdtFileID > 0) {
                            openMapByIdAndWDTId(prevMapId, prevMapRec.WdtFileID, 17066.6641f, 17066.67380f, 0);
                        } else {
                            //Try to open map by fileName
                            openMapByIdAndFilename(prevMapId, prevMapRec.MapDirectory, 17066.6641f, 17066.67380f,
                                                   0);
                        }
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
                    m_fileListWindow = std::make_shared<FileListWindow>(m_api);
            }
            if (ImGui::MenuItem("Open current stats")) { showCurrentStats = true; }
            ImGui::Separator();
            if (ImGui::MenuItem("Open settings")) {showSettings = true;}
            if (ImGui::MenuItem("Open QuickLinks")) {showQuickLinks = true;}
            if (ImGui::MenuItem("Open MapConstruction")) {showMapConstruction = true;}
            if (ImGui::MenuItem("Open minimap generator", "", false, cascOpened)) {
                showMinimapGeneratorSettings = true;
            }
            if (ImGui::MenuItem("Open BLP viewer", "", false, cascOpened)) {
                if (!m_blpViewerWindow)
                    m_blpViewerWindow = std::make_shared<BLPViewer>(m_api, m_uiRenderer);
            }

            if (ImGui::MenuItem("Test export")) {
                if (m_currentScene != nullptr) {
                    exporter = std::make_shared<GLTFExporter>("./gltf/");
//                    currentScene->exportScene(exporter.get());
                    exporterFramesReady = 0;
                }
            }
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

    emptyMinimap();

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
                   std::string currPath = fileDialog.GetSelected();
                   buf->appendf("lastCascDir=%s\n", currPath.c_str());
                   buf->appendf("windowWidth=%s\n", std::to_string(windowWidth).c_str());
                   buf->appendf("windowHeight=%s\n", std::to_string(windowHeight).c_str());
               }
       );

    ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);

    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
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

bool FrontendUI::getStopMouse() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool FrontendUI::getStopKeyboard() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

std::shared_ptr<IScene> setScene(const HApiContainer& apiContainer, int sceneType, const std::string& name, int cameraNum) {
    apiContainer->camera = std::make_shared<FirstPersonCamera>();
    if (sceneType == -1) {
        return std::make_shared<NullScene>();
    } else if (sceneType == 0) {
//        m_usePlanarCamera = cameraNum == -1;


        return std::make_shared<M2Scene>(apiContainer, name);
    } else if (sceneType == 1) {
        return std::make_shared<WmoScene>(apiContainer, name);
    } else if (sceneType == 2) {
        auto &adtFileName = name;

        size_t lastSlashPos = adtFileName.find_last_of("/");
        size_t underscorePosFirst = adtFileName.find_last_of("_");
        size_t underscorePosSecond = adtFileName.find_last_of("_", underscorePosFirst-1);
        std::string mapName = adtFileName.substr(lastSlashPos+1, underscorePosSecond-lastSlashPos-1);

        int i = std::stoi(adtFileName.substr(underscorePosSecond+1, underscorePosFirst-underscorePosSecond-1));
        int j = std::stoi(adtFileName.substr(underscorePosFirst+1, adtFileName.size()-underscorePosFirst-5));

        float adt_x_min = AdtIndexToWorldCoordinate(j);
        float adt_x_max = AdtIndexToWorldCoordinate(j+1);

        float adt_y_min = AdtIndexToWorldCoordinate(i);
        float adt_y_max = AdtIndexToWorldCoordinate(i+1);

        apiContainer->camera = std::make_shared<FirstPersonCamera>();
        apiContainer->camera->setCameraPos(
            (adt_x_min+adt_x_max) / 2.0,
            (adt_y_min+adt_y_max) / 2.0,
            200
        );

        return std::make_shared<Map>(apiContainer, adtFileName, i, j, mapName);
    }

    return nullptr;
}

void FrontendUI::showQuickLinksDialog() {
    if (!showQuickLinks) return;
    std::vector<int> replacementTextureFDids = {};

    ImGui::Begin("Quick Links", &showQuickLinks);
    if (ImGui::Button("model without skin", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        openM2SceneByfdid(5099010, replacementTextureFDids);
    }
    if (ImGui::Button("crystal song bush", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        openM2SceneByfdid(194418, replacementTextureFDids);
    }
    if (ImGui::Button("bugged decal", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        openM2SceneByfdid(946969, replacementTextureFDids);
    }
    if (ImGui::Button("Some model", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 4952373;
        replacementTextureFDids[12] = 4952379;
        openM2SceneByfdid(4870631, replacementTextureFDids);
    }
    if (ImGui::Button("nightborne model", ImVec2(-1, 0))) {
        openM2SceneByfdid(1810676, replacementTextureFDids);
    }
    if (ImGui::Button("Tomb of sargares hall", ImVec2(-1, 0))) {
        openMapByIdAndWDTId(1676, 1532459, 6289, -801, 3028);
    }
    if (ImGui::Button("Legion Dalaran", ImVec2(-1, 0))) {
        openWMOSceneByfdid(1120838);
    }
    if (ImGui::Button("8du_zuldazarraid_antiportal01.wmo", ImVec2(-1, 0))) {
        openWMOSceneByfdid(2574165);
    }
    if (ImGui::Button("someShip.wmo", ImVec2(-1, 0))) {
        openWMOSceneByfdid(4638404);
    }
    if (ImGui::Button("Vanilla karazhan", ImVec2(-1, 0))) {
        m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
        m_currentScene = std::make_shared<WmoScene>(m_api, "world/wmo/dungeon/az_karazahn/karazhan.wmo");
        m_api->camera->setCameraPos(0, 0, 0);
    }
    if (ImGui::Button("10xt_exterior_glacialspike01.wmo (parallax)", ImVec2(-1, 0))) {
        openWMOSceneByfdid(4419436);
    }
    if (ImGui::Button("14654.wmo (parallax)", ImVec2(-1, 0))) {
        openWMOSceneByfdid(4222547);
    }
    if (ImGui::Button("10.0 Raid WMO", ImVec2(-1, 0))) {
        openWMOSceneByfdid(4282557);
    }
    if (ImGui::Button("(WMO) Model with broken portal culling", ImVec2(-1, 0))) {
        openWMOSceneByfdid(4217818);
    }
    if (ImGui::Button("(WMO) NPE Ship with waterfall model", ImVec2(-1, 0))) {
        openWMOSceneByfdid(3314067);
    }
    if (ImGui::Button("(WMO) Gazebo 590182", ImVec2(-1, 0))) {
        openWMOSceneByfdid(590182);
    }
    if (ImGui::Button("Hearthstone Tavern", ImVec2(-1, 0))) {
        openWMOSceneByfdid(2756726);
    }
    if (ImGui::Button("Original WVF1 model", ImVec2(-1, 0))) {
        openM2SceneByfdid(2445860, replacementTextureFDids);
    }
    if (ImGui::Button("Stormwind mage portal", ImVec2(-1, 0))) {
        openM2SceneByfdid(2394711, replacementTextureFDids);
    }
    if (ImGui::Button("kodobeasttame", ImVec2(-1, 0))) {
        openM2SceneByfdid(124697, replacementTextureFDids);
    }

//    if (ImGui::Button("Azeroth map: Lion's Rest (Legion)", ImVec2(-1, 0))) {
//        openMapByIdAndFilename(0, "azeroth", -8739, 944, 200);
//    }
    if (ImGui::Button("Nyalotha map", ImVec2(-1, 0))) {
        openMapByIdAndWDTId(2217, 2842322, -11595, 9280, 260);
    }
    if (ImGui::Button("WMO 1247268", ImVec2(-1, 0))) {
        openWMOSceneByfdid(1247268);
    }
    if (ImGui::Button("Ironforge.wmo", ImVec2(-1, 0))) {
        openWMOSceneByfdid(113992);
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
    if (ImGui::Button("Steamscale mount", ImVec2(-1, 0))) {
        openM2SceneByfdid(2843110, replacementTextureFDids);
    }
    if (ImGui::Button("Spline emitter", ImVec2(-1, 0))) {
        openM2SceneByfdid(1536145, replacementTextureFDids);
    }
    if (ImGui::Button("Nether collector top", ImVec2(-1, 0))) {
        openM2SceneByfdid(193157, replacementTextureFDids);
    }
    if (ImGui::Button("Сollector top", ImVec2(-1, 0))) {
        openWMOSceneByfdid(113540);
    }
    if (ImGui::Button("10.0 unk model", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);

        openM2SceneByfdid(4519090, replacementTextureFDids);
    }
    if (ImGui::Button("10.0 strange shoulders", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[2] = 4615508;
//        replacementTextureFDids[3] = 4615508;



        openM2SceneByfdid(4614814, replacementTextureFDids);
    }
    if (ImGui::Button("DF chicken", ImVec2(-1, 0))) {
        replacementTextureFDids = std::vector<int>(17);
        replacementTextureFDids[11] = 4007136;

        openM2SceneByfdid(4005446, replacementTextureFDids);
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

    if (ImGui::Button("Vanilla login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(131970, replacementTextureFDids);
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

    if (ImGui::Button("DragonLands login screen", ImVec2(-1, 0))) {
            openM2SceneByfdid(4684877, replacementTextureFDids);
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
    if (ImGui::Button("Torghast raid skybox", ImVec2(-1, 0))) {

        openM2SceneByfdid(4001212, replacementTextureFDids);

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
    if (ImGui::Button("Bugged ADT (SL)", ImVec2(-1, 0))) {
        m_currentScene = setScene(m_api, 2, "world/maps/2363/2363_31_31.adt", 0);
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

static HGTexture blpText = nullptr;

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
        if (ImGui::SliderFloat("fov", &fov, 10, 150)) {
        }

        if (ImGui::SliderFloat("Far plane", &farPlane, 200, 2000)) {
            m_api->getConfig()->farPlane = farPlane;
            m_api->getConfig()->farPlaneForCulling = farPlane+50;
        }

        ImGui::Text("Time: %02d:%02d", (int)(currentTime/120), (int)((currentTime/2) % 60));

        if (ImGui::SliderInt("Current time", &currentTime, 0, 2880)) {
            m_api->getConfig()->currentTime = currentTime;
        }

        if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.3, 100)) {
            m_api->camera->setMovementSpeed(movementSpeed);
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

            bool renderADT = m_api->getConfig()->renderAdt;
            if (ImGui::Checkbox("Render ADT", &renderADT)) {
                m_api->getConfig()->renderAdt = renderADT;
            }

            bool renderM2 = m_api->getConfig()->renderM2;
            if (ImGui::Checkbox("Render M2", &renderM2)) {
                m_api->getConfig()->renderM2 = renderM2;
            }

            bool renderWMO = m_api->getConfig()->renderWMO;
            if (ImGui::Checkbox("Render WMO", &renderWMO)) {
                m_api->getConfig()->renderWMO = renderWMO;
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
            if (useDoubleCameraDebug && (m_debugRenderWindow == nullptr || m_debugRenderView == nullptr) && m_sceneRenderer != nullptr) {
                m_debugRenderView = m_sceneRenderer->createRenderView(640, 480, true);
                m_debugRenderWindow = std::make_shared<DebugRendererWindow>(m_api, m_uiRenderer, m_debugRenderView);
            }


            if (useDoubleCameraDebug) {
                if (m_api->debugCamera == nullptr) {
                    m_api->debugCamera = std::make_shared<FirstPersonCamera>();
                    m_api->debugCamera->setMovementSpeed(movementSpeed);
                    float currentCameraPos[4] = {0, 0, 0, 0};
                    m_api->camera->getCameraPosition(&currentCameraPos[0]);


                    m_api->debugCamera->setCameraPos(currentCameraPos[0],
                                                     currentCameraPos[1],
                                                     currentCameraPos[2]);
                }

                bool controlSecondCamera = m_api->getConfig()->controlSecondCamera;
                if (ImGui::Checkbox("Control debug camera", &controlSecondCamera)) {
                    m_api->getConfig()->controlSecondCamera = controlSecondCamera;
                }

                bool swapMainAndDebug = m_api->getConfig()->swapMainAndDebug;
                if (ImGui::Checkbox("Swap main and debug cameras", &swapMainAndDebug)) {
                    m_api->getConfig()->swapMainAndDebug = swapMainAndDebug;
                }
            } else {
                m_api->debugCamera = nullptr;
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
            auto mapPlan = (m_sceneRenderer != nullptr) ?
                m_sceneRenderer->getLastCreatedPlan() :
                nullptr;

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


        ImGui::End();
    }
}

//void FrontendUI::produceDrawStage(HDrawStage &resultDrawStage, std::vector<HUpdateStage> &updateStages) {

//}
void FrontendUI::getAdtSelectionMinimap(int wdtFileDataId) {
    m_wdtFile = m_api->cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);
}

void FrontendUI::getAdtSelectionMinimap(std::string wdtFilePath) {
    m_wdtFile = m_api->cacheStorage->getWdtFileCache()->get(wdtFilePath);
}

void FrontendUI::getMapList(std::vector<MapRecord> &mapList) {
    if (m_api->databaseHandler == nullptr)  return;

    m_api->databaseHandler->getMapArray(mapList);
}

bool FrontendUI::fillAdtSelectionminimap(bool &isWMOMap, bool &wdtFileExists) {
    if (m_wdtFile == nullptr) return false;

    if (m_wdtFile->getStatus() == FileStatus::FSRejected) {
        wdtFileExists = false;
        isWMOMap = false;
        return false;
    }

    if (m_wdtFile->getStatus() != FileStatus::FSLoaded) return false;

    isWMOMap = m_wdtFile->mphd->flags.wdt_uses_global_map_obj != 0;

    if (!isWMOMap) {
        if (m_wdtFile->mphd->flags.wdt_has_maid) {
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    if (m_wdtFile->mapFileDataIDs[i * 64 + j].minimapTexture > 0) {
                        auto blpTexture = m_api->cacheStorage->getTextureCache()->getFileId(
                            m_wdtFile->mapFileDataIDs[i * 64 + j].minimapTexture);
                        auto textureObj = m_api->hDevice->createBlpTexture(blpTexture, false, false);
                        adtSelectionMinimapTextures[i][j] = textureObj;
                        adtSelectionMinimapMaterials[i][j] = m_uiRenderer->createUIMaterial({textureObj});
                    } else {
                        adtSelectionMinimapTextures[i][j] = nullptr;
                        adtSelectionMinimapMaterials[i][j] = nullptr;
                    }
                }

            }
        }
        else {
            std::cout << "wdtId = " << prevMapRec.WdtFileID << std::endl;
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

mathfu::mat4 getInfZMatrix(float f, float aspect) {
    return mathfu::mat4(
        f / aspect, 0.0f,  0.0f,  0.0f,
        0.0f,    f,  0.0f,  0.0f,
        0.0f, 0.0f,  1, -1.0f,
        0.0f, 0.0f, 1,  0.0f);
}

struct RenderTargetParameters {
    std::shared_ptr<ICamera> camera;
    ViewPortDimensions dimensions;
    std::shared_ptr<IRenderView> target;
};

HMapSceneParams createMapSceneParams(ApiContainer &apiContainer,
                                                     float fov,
                                                     const std::vector<RenderTargetParameters> &renderTargetParams,
                                                     const std::shared_ptr<IScene> &currentScene) {

    auto result = std::make_shared<MapSceneParams>();
    result->scene = currentScene;

    float farPlaneRendering = apiContainer.getConfig()->farPlane;
    float farPlaneCulling = apiContainer.getConfig()->farPlaneForCulling;

    float nearPlane = 1.0;
    float fovR = toRadian(fov);

    {
        auto width = renderTargetParams[0].dimensions.maxs[0];
        auto height = renderTargetParams[0].dimensions.maxs[1];
        float canvasAspect = (float) width / (float) height;

        result->matricesForCulling = apiContainer.camera->getCameraMatrices(fovR, canvasAspect, nearPlane,
                                                                            farPlaneCulling);
    }

    bool isInfZSupported = apiContainer.camera->isCompatibleWithInfiniteZ();
    auto assignInfiniteZ = [&](auto renderTarget, auto canvasAspect) {
        float f = 1.0f / tan(fovR / 2.0f);
        renderTarget.cameraMatricesForRendering->perspectiveMat = getInfZMatrix(f, canvasAspect);
    };

    for (auto &targetParam : renderTargetParams) {
        auto width = targetParam.dimensions.maxs[0];
        auto height = targetParam.dimensions.maxs[1];
        float canvasAspect = (float)width / (float)height;

        auto &renderTarget = result->renderTargets.emplace_back();
        renderTarget.cameraMatricesForRendering = targetParam.camera->getCameraMatrices(fovR, canvasAspect, nearPlane, farPlaneCulling);;
        renderTarget.viewPortDimensions = targetParam.dimensions;
        renderTarget.target = targetParam.target;
        if (isInfZSupported) assignInfiniteZ(renderTarget, canvasAspect);
    }

    result->clearColor = apiContainer.getConfig()->clearColor;

    return result;
}

HFrameScenario FrontendUI::createFrameScenario(int canvWidth, int canvHeight, double deltaTime) {
    ZoneScoped ;
    if (m_minimapGenerationWindow != nullptr) {
        m_minimapGenerationWindow->process();
    }
    auto l_device = m_api->hDevice;
    auto processingFrame = l_device->getFrameNumber();
    auto updateFrameNumberLambda = [l_device](unsigned int frame) -> void {l_device->setCurrentProcessingFrameNumber(frame);};


    HFrameScenario scenario = std::make_shared<HFrameScenario::element_type>();
    {
        ViewPortDimensions dimension = {
            {0,     0},
            {static_cast<unsigned int>(canvWidth), static_cast<unsigned int>(canvHeight)}
        };
        ViewPortDimensions debugViewDimension = dimension;

        if (m_sceneRenderer != nullptr) {
            auto wowSceneFrameInput = std::make_shared<FrameInputParams<MapSceneParams>>();
            wowSceneFrameInput->delta = deltaTime * (1000.0f);

            std::shared_ptr<IRenderView> target = nullptr;
            std::shared_ptr<IRenderView> debugTarget = nullptr;
            if (m_api->debugCamera && m_api->getConfig()->doubleCameraDebug && m_debugRenderView) {
                debugTarget = m_debugRenderView;

                debugViewDimension = {
                    {0,     0},
                    {static_cast<unsigned int>(m_debugRenderWindow->getWidth()),
                     static_cast<unsigned int>(m_debugRenderWindow->getHeight())}
                };

                if (m_api->getConfig()->swapMainAndDebug) {
                    std::swap(target, debugTarget);
                    std::swap(dimension, debugViewDimension);
                }
            }

            std::vector<RenderTargetParameters> renderTargetParams = {
                {
                    m_api->camera,
                    dimension,
                    target
                }
            };
            if (m_api->getConfig()->doubleCameraDebug) {
                auto &debugParams = renderTargetParams.emplace_back();
                debugParams.camera = m_api->debugCamera;
                debugParams.dimensions = debugViewDimension;
                debugParams.target = debugTarget;
            }

            wowSceneFrameInput->frameParameters = createMapSceneParams(*m_api,
                                                                       fov,
                                                                       renderTargetParams,
                                                                       m_currentScene);
            scenario->cullFunctions.push_back(
                m_sceneRenderer->createCullUpdateRenderChain(wowSceneFrameInput, processingFrame, updateFrameNumberLambda));

            //----------------------
            // Screenshot part
            //----------------------

            if (needToMakeScreenshot) {
                auto screenShotRenderView = m_sceneRenderer->createRenderView(screenShotWidth, screenShotHeight, true);
                auto wowSceneScreenshotFrameInput = std::make_shared<FrameInputParams<MapSceneParams>>();
                wowSceneScreenshotFrameInput->delta = 0;

                wowSceneScreenshotFrameInput->frameParameters = createMapSceneParams(
                    *m_api,
                    fov,
                    {{
                         m_api->camera,
                         {
                             {0, 0},
                             {static_cast<unsigned int>(screenShotWidth), static_cast<unsigned int>(screenShotHeight)}
                         },
                         screenShotRenderView
                    }},
                    m_currentScene
                );
                scenario->onFinish.push_back([screenShotRenderView, this, processingFrame]() {
                    saveDataFromDrawStage([screenShotRenderView, processingFrame](int x, int y, int width, int height, uint8_t* data){
                        screenShotRenderView->readRGBAPixels(processingFrame, x, y, width, height, data);
                    }, screenshotFilename, screenShotWidth, screenShotHeight);
                });
                needToMakeScreenshot = false;

                scenario->cullFunctions.push_back(
                    m_sceneRenderer->createCullUpdateRenderChain(wowSceneScreenshotFrameInput, processingFrame, updateFrameNumberLambda));
            }
        }

        auto uiFrameInput = std::make_shared<FrameInputParams<ImGuiFramePlan::ImGUIParam>>();
        uiFrameInput->delta = deltaTime * (1000.0f);
        uiFrameInput->frameParameters = std::make_shared<ImGuiFramePlan::ImGUIParam>( ImGui::GetDrawData(), dimension);

        auto clearColor = m_api->getConfig()->clearColor;

        scenario->cullFunctions.push_back(m_uiRenderer->createCullUpdateRenderChain(uiFrameInput, processingFrame, updateFrameNumberLambda));
    }

    return scenario;
}

bool FrontendUI::tryOpenCasc(std::string &cascPath, BuildDefinition &buildDef) {
    HRequestProcessor newProcessor = nullptr;
    std::shared_ptr<WoWFilesCacheStorage> newStorage = nullptr;

    try {
        newProcessor = std::make_shared<CascRequestProcessor>(cascPath, buildDef);
        newStorage = std::make_shared<WoWFilesCacheStorage>(newProcessor.get());
        newProcessor->setFileRequester(newStorage.get());
    } catch (...){
        return false;
    };

    m_api->cacheStorage = newStorage;
    m_api->requestProcessor = newProcessor;

    return true;
}

void FrontendUI::openWMOSceneByfdid(int WMOFdid) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<WmoScene>(m_api, WMOFdid);
    m_api->camera->setCameraPos(0, 0, 0);
}
void FrontendUI::openMapByIdAndFilename(int mapId, std::string mapName, float x, float y, float z) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<Map>(m_api, mapId, mapName);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(x,y,z);
    m_api->camera->setMovementSpeed(movementSpeed);
}
void FrontendUI::openMapByIdAndWDTId(int mapId, int wdtFileId, float x, float y, float z) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    m_currentScene = std::make_shared<Map>(m_api, mapId, wdtFileId);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(x,y,z);
    m_api->camera->setMovementSpeed(movementSpeed);
}
void FrontendUI::openM2SceneByfdid(int m2Fdid, std::vector<int> &replacementTextureIds) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());
    auto m2Scene = std::make_shared<M2Scene>(m_api, m2Fdid);
    m_currentScene = m2Scene;
    m2Scene->setReplaceTextureArray(m_sceneRenderer, replacementTextureIds);


    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setMovementSpeed(movementSpeed);
    m_api->getConfig()->BCLightHack = false;
//
    m_api->camera->setCameraPos(0, 0, 0);
}

void FrontendUI::openM2SceneByName(std::string m2FileName, std::vector<int> &replacementTextureIds) {
    m_sceneRenderer = MapSceneRendererFactory::createForwardRenderer(m_api->hDevice, m_api->getConfig());

    auto m2Scene = std::make_shared<M2Scene>(m_api, m2FileName);
    m_currentScene = m2Scene;
    m2Scene->setReplaceTextureArray(m_sceneRenderer, replacementTextureIds);

    m_api->camera = std::make_shared<FirstPersonCamera>();
    m_api->camera->setCameraPos(0, 0, 0);
    m_api->camera->setMovementSpeed(movementSpeed);
}

void FrontendUI::unloadScene() {
    if (m_api->cacheStorage) {
        m_api->cacheStorage->actuallDropCache();
    }
    m_sceneRenderer = nullptr;
    m_currentScene = std::make_shared<NullScene>();
}



int FrontendUI::getCameraNumCallback() {
//    if (currentScene != nullptr) {
//        return currentScene->getCameraNum();
//    }

    return 0;
}

bool FrontendUI::setNewCameraCallback(int cameraNum) {
    return false;
//    if (currentScene == nullptr) return false;
//
//    auto newCamera = currentScene->createCamera(cameraNum);
//    if (newCamera == nullptr) {
//        m_api->camera = std::make_shared<FirstPersonCamera>();
//        m_api->camera->setMovementSpeed(movementSpeed);
//        return false;
//    }
//
//    m_api->camera = newCamera;
//    return true;
}

void FrontendUI::resetAnimationCallback() {
//    currentScene->resetAnimation();
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


void FrontendUI::getDebugCameraPos(float &cameraX, float &cameraY, float &cameraZ) {
    if (m_api->debugCamera == nullptr) {
        cameraX = 0; cameraY = 0; cameraZ = 0;
        return;
    }
    float currentCameraPos[4] = {0,0,0,0};
    m_api->debugCamera->getCameraPosition(&currentCameraPos[0]);
    cameraX = currentCameraPos[0];
    cameraY = currentCameraPos[1];
    cameraZ = currentCameraPos[2];
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
    m_api->requestProcessor = std::make_shared<HttpRequestProcessor>(url, urlFileId);
//    m_processor = std::make_shared<CascRequestProcessor>("e:\\games\\wow beta\\World of Warcraft Beta\\:wowt");
////        processor->setThreaded(false);
////
    m_api->cacheStorage = std::make_shared<WoWFilesCacheStorage>(m_api->requestProcessor.get());
    m_api->requestProcessor->setFileRequester(m_api->cacheStorage.get());
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

    mapList = {};
    mapListStringMap = {};
    filteredMapList = {};
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

