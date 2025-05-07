//
// Created by Deamon on 10/29/2022.
//

#include "MinimapGenerationWindow.h"
#include "imgui.h"
#include "imageButton2/imageButton2.h"
#include "groupPanel/groupPanel.h"
#include "compactColorPicker/compactColorPicker.h"
#include "disablableButton/disablableButton.h"

void MinimapGenerationWindow::restartMinimapGenPreview() {
    minimapGenerator->stopPreview();
    minimapGenerator->startPreview(*sceneDef);
    minimapGenerator->setZoom(previewZoom);
    minimapGenerator->setLookAtPoint(previewX, previewY);
}

void MinimapGenerationWindow::render() {
    ImGui::Begin("Minimap Generator settings", &m_showMinimapGeneratorSettings);
    ImGui::Columns(2, NULL, true);
    //Left panel
    ImGui::BeginTabBar("MinimapGenTabs");
    {
        renderListTab();
        if (sceneDef != nullptr) {
            if (editTabOpened) {
                renderEditTab();
            } else {
                //sceneDef = nullptr;
            }
        }

        ImGui::EndTabBar();
    }

    //Right panel
    ImGui::NextColumn();
    {
        ImGui::BeginChild("Minimap Gen Preview", ImVec2(0, 0));
        {
            bool changed = false;
            bool readOnly = minimapGenerator->getCurrentMode() != EMGMode::ePreview;

            const char * fmt = "%.3f";
            changed |= ImGui::InputFloat("x", &previewX, 0.0f, 0.0f, fmt, readOnly ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly: 0);
            changed |= ImGui::InputFloat("y", &previewY, 0.0f, 0.0f, fmt, readOnly ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly: 0);

            if (changed && minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                minimapGenerator->setLookAtPoint(previewX, previewY);
            }
            if (ImGui::SliderFloat("Zoom", &previewZoom, 0.1, 10)) {
                if (minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                    minimapGenerator->setZoom(previewZoom);
                }
            }
            if (ImGui::Button("Reload")) {
                minimapGenerator->reload();
            }

            ImGui::BeginChild("Minimap Gen Preview image", ImVec2(0, 0),
                              true, ImGuiWindowFlags_AlwaysHorizontalScrollbar |
                                    ImGuiWindowFlags_AlwaysVerticalScrollbar);

            auto lastFrameBuffer = minimapGenerator->getLastFrameBuffer();
            if (lastFrameBuffer != nullptr) {
                auto texture = lastFrameBuffer->getAttachment(0);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,1.0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,1.0));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,1.0));

                const int imageSize = 512;
//TODO:
//                if (ImGui::ImageButton2(m_renderer->createUIMaterial({texture}), "previewImage",
//                                        ImVec2(imageSize, imageSize),
//                                        ImVec2(0,1),
//                                        ImVec2(1,0)))
//                {
//                    auto mousePos = ImGui::GetMousePos();
//                    ImGuiStyle &style = ImGui::GetStyle();
//
//                    mousePos.x += -ImGui::GetWindowPos().x - style.WindowPadding.x;
//                    mousePos.y += -ImGui::GetWindowPos().y - style.WindowPadding.y;
//
//
//                    previewX = ((0.5f - (mousePos.y / (float)imageSize)) * minimapGenerator->GetOrthoDimension()) + previewX;
//                    previewY = ((0.5f - (mousePos.x / (float)imageSize)) * minimapGenerator->GetOrthoDimension()) + previewY;
//
//
//
//                    minimapGenerator->setLookAtPoint(previewX, previewY);
//                };

                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar(3);
            }
            ImGui::EndChild();

        }
        ImGui::EndChild();
    }
    ImGui::Columns(1);

    ImGui::End();
}

void MinimapGenerationWindow::renderMapConfigSubWindow(int mapIndex) {
    auto &mapRenderDef = sceneDef->maps[mapIndex];
    ImGui::BeginGroupPanel(("Map" + std::to_string(mapIndex)).c_str());

    if (ImGui::InputInt(("Map Id##"+ std::to_string(mapIndex)).c_str(), &mapRenderDef.mapId)) {
        m_minimapDB->getAdtBoundingBoxes(mapRenderDef);
    }
    ImGui::InputDouble(("Delta X##"+ std::to_string(mapIndex)).c_str(), &mapRenderDef.deltaX);
    ImGui::InputDouble(("Delta Y##"+ std::to_string(mapIndex)).c_str(), &mapRenderDef.deltaY);
    ImGui::InputDouble(("Delta Z##"+ std::to_string(mapIndex)).c_str(), &mapRenderDef.deltaZ);

    const auto redColor = ImVec4{1.0f, 0.f, 0.f, 1.0f};
    ImGui::PushStyleColor(ImGuiCol_Button, redColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, redColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, redColor);
    if (ImGui::Button(("Delete##"+std::to_string(mapIndex)).c_str())) {
        if (mapIndex > 0) {
            sceneDef->maps.erase(std::next(sceneDef->maps.begin(), mapIndex - 1));
        } else {
            sceneDef->maps.erase(sceneDef->maps.begin());
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::EndGroupPanel();

    ImGui::SameLine();
    ImGui::BeginGroupPanel("");

    bool active = mapIndexExcludeADT == mapIndex;
    if (ImGui::RadioButton(("Exclude ADT##" + std::to_string(mapIndex)).c_str(), active)) {
        mapIndexExcludeADT = mapIndex;
        mapIndexExcludeADTChunk = -1;
    }
    bool active2 = mapIndexExcludeADTChunk == mapIndex;
    if (ImGui::RadioButton(("Exclude ADT Chunk##" + std::to_string(mapIndex)).c_str(), active2)) {
        mapIndexExcludeADT = -1;
        mapIndexExcludeADTChunk = mapIndex;
    }
    ImGui::EndGroupPanel();
}

void MinimapGenerationWindow::editComponentsForConfig(Config * config) {
    if (config == nullptr) return;

    ImGui::BeginGroupPanel("Exterior Lighting");



    ImGui::EndGroupPanel();
}

void MinimapGenerationWindow::createMinimapGenerator() {
    riverColorOverrides = std::make_shared<RiverColorOverrideHolder>();

    minimapGenerator = std::make_shared<MinimapGenerator>(
        m_api->cacheStorage,
        m_api->hDevice,
        m_processor,
        m_api->databaseHandler
    );

    minimapGenerator->getConfig()->colorOverrideHolder = riverColorOverrides;

    minimapGenerator->setZoom(previewZoom);
    minimapGenerator->setLookAtPoint(previewX, previewY);


//    sceneDef = {
//        EMGMode::eScreenshotGeneration,
//        0,
//        mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//        mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//        mathfu::vec2(0, 0),
//        mathfu::vec2(MathHelper::TILESIZE*2, MathHelper::TILESIZE*2),
//        1024,
//        1024,
//        1.0f,
//        false,
//        ScenarioOrientation::so45DegreeTick3,
//        "azeroth/topDown1"
//    };

//    sceneDefList = {
//        {
//            -1,
//            530,
//            "Netherstorm",
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec2(1627, -1175),
//            mathfu::vec2(6654 , 4689 ),
//            1024,
//            1024,
//            1.0f,
//            ScenarioOrientation::so45DegreeTick0,
//            "outland/netherstorm"
//        },
//        {
//            -1,
//            1,
//            "Kalimdor, rot 0",
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec2(-12182, -8803 ),
//            mathfu::vec2(12058, 4291),
//            1024,
//            1024,
//            1.0f,
//            ScenarioOrientation::so45DegreeTick0,
//            "kalimdor/rotation0"
//        }
//    };

//    std::vector<ScenarioDef> scenarios = {
////        {
////            2222,
////            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
////            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
////            mathfu::vec2(-9750, -8001   ),
////            mathfu::vec2(8333, 9500 ),
////            ScenarioOrientation::so45DegreeTick0,
////            "shadowlands/orient0"
////        }
//        {
//            1643,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(291 , 647 ),
//            mathfu::vec2(2550, 2895),
//            256,
//            256,
//            4.0f,
//            ScenarioOrientation::so45DegreeTick0,
//            "kultiras/orient0"
//        },
////        {
////            530,
////            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
////            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
////            mathfu::vec2(-5817, -1175),
////            mathfu::vec2(1758, 10491),
////            ScenarioOrientation::so45DegreeTick0,
////            "outland/topDown1"
////        }
////        {
////            0,
////            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
////            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
////            mathfu::vec2(-9081, -20),
////            mathfu::vec2(-8507, 1296),
////            ScenarioOrientation::soTopDownOrtho,
////            "azeroth/topDown"
////        }
////    };
//
////        std::vector<ScenarioDef> scenarios = {
////        {
////            1,
////            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
////            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
////            mathfu::vec2(-12182, -8803 ),
////            mathfu::vec2(12058, 4291),
////            ScenarioOrientation::so45DegreeTick0,
////            "kalimdor/rotation0"
////        },
//        {
//            1,
//            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
//            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
//            mathfu::vec2(-12182, -8803),
//            mathfu::vec2(12058, 4291),
//            256,
//            256,
//            4.0f,
//            ScenarioOrientation::so45DegreeTick1,
//            "kalimdor/rotation1_new"
//        }
//    };
////        {
////            1,
////            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
////            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
////            mathfu::vec2(-12182, -8803 ),
////            mathfu::vec2(12058, 4291),
////            ScenarioOrientation::so45DegreeTick2,
////            "kalimdor/rotation2"
////        },
////        {
////            1,
////            mathfu::vec4(0.0671968088, 0.294095874, 0.348881632, 0),
////            mathfu::vec4(0.345206976, 0.329288304, 0.270450264, 0),
////            mathfu::vec2(-12182, -8803 ),
////            mathfu::vec2(12058, 4291),
////            ScenarioOrientation::so45DegreeTick3,
////            "kalimdor/rotation3"
////        },
////    };
}

void MinimapGenerationWindow::process() {
    if (minimapGenerator != nullptr && minimapGenerator->getCurrentMode() != EMGMode::eNone)
        minimapGenerator->process();
}

void MinimapGenerationWindow::getDrawStage(HFrameScenario sceneScenario) {
    if (minimapGenerator->getCurrentMode() != EMGMode::eNone) {
        return minimapGenerator->createSceneDrawStage(sceneScenario);
    }
}

template<class T, size_t N, class V>
inline std::array<T, N> to_array(const V& v)
{
    assert(v.size() <= N);
    std::array<T, N> d = {0};
    using std::begin; using std::end;
    std::copy( begin(v), end(v), begin(d) ); // this is the recommended way
    return d;
}

void MinimapGenerationWindow::renderEditTab() {
    if (ImGui::BeginTabItem("Edit", &editTabOpened, ImGuiTabItemFlags_SetSelected)) {
        {
            auto scenarioName = std::array<char,128>();
            if (sceneDef->name.size() > 128) sceneDef->name.resize(128);
            std::copy(sceneDef->name.begin(), sceneDef->name.end(), scenarioName.data());
            if (ImGui::InputText("Scenario name", scenarioName.data(), 128)) {
                scenarioName[127] = 0;
                sceneDef->name = std::string(scenarioName.data());
            }

            if (sceneDef->folderToSave.size() > 128) sceneDef->folderToSave.resize(128);
            std::array<char,128> folderToSave = to_array<char, 128>(sceneDef->folderToSave);

            if (ImGui::InputText("Folder for saving", folderToSave.data(), 128)) {
                folderToSave[127] = 0;
                sceneDef->folderToSave = std::string(folderToSave.data());
                trim(sceneDef->folderToSave);
            }
            {
                ImGui::BeginGroupPanel("Maps");

                for (int i = 0; i < sceneDef->maps.size(); i++) {
                    renderMapConfigSubWindow(i);
                }

                if (ImGui::Button("Add Map")) {
                    sceneDef->maps.emplace_back();
                }
                if (ImGui::Button("Reset Exclude")) {
                    mapIndexExcludeADT = -1;
                    mapIndexExcludeADTChunk = -1;
                }
                ImGui::EndGroupPanel();
            }

            ImGui::BeginGroupPanel("Orientation");
            {
                if (ImGui::RadioButton("Ortho projection", &sceneDef->orientation, ScenarioOrientation::soTopDownOrtho)) {
                    if (minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                        restartMinimapGenPreview();
                    }
                }
                if (ImGui::RadioButton("At 45° tick 0", &sceneDef->orientation, ScenarioOrientation::so45DegreeTick0)) {
                    if (minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                        restartMinimapGenPreview();
                    }
                }
                if (ImGui::RadioButton("At 45° tick 1", &sceneDef->orientation, ScenarioOrientation::so45DegreeTick1)) {
                    if (minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                        restartMinimapGenPreview();
                    }
                }
                if (ImGui::RadioButton("At 45° tick 2", &sceneDef->orientation, ScenarioOrientation::so45DegreeTick2)) {
                    if (minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                        restartMinimapGenPreview();
                    }
                }
                if (ImGui::RadioButton("At 45° tick 3", &sceneDef->orientation, ScenarioOrientation::so45DegreeTick3)) {
                    if (minimapGenerator->getCurrentMode() == EMGMode::ePreview) {
                        restartMinimapGenPreview();
                    }
                }
            }
            ImGui::EndGroupPanel();
            ImGui::SameLine();
            ImGui::BeginGroupPanel("Generation boundaries");
            {
                ImGui::Text("In world coordinates");
                ImGui::InputFloat("Min x", &sceneDef->minWowWorldCoord.x);
                ImGui::InputFloat("Min y", &sceneDef->minWowWorldCoord.y);
                ImGui::InputFloat("Max x", &sceneDef->maxWowWorldCoord.x);
                ImGui::InputFloat("Max y", &sceneDef->maxWowWorldCoord.y);
                ImGui::EndGroupPanel();
            }
            ImGui::BeginGroupPanel("Ocean color override");
            {
                ImGui::CompactColorPicker("Close Ocean Color", sceneDef->closeOceanColor, false);
                ImGui::EndGroupPanel();
            }
            ImGui::BeginGroupPanel("Image settings");
            {
                ImGui::PushItemWidth(100);
                ImGui::InputInt("Image Width", &sceneDef->imageWidth);
                ImGui::InputInt("Image Height", &sceneDef->imageHeight);
                ImGui::PopItemWidth();
                ImGui::EndGroupPanel();
            }
            ImGui::InputFloat("Zoom", &sceneDef->zoom, 0.1);



            ImGui::BeginGroupPanel("Global map settings");
            {
//                            ImGui::BeginGroupPanel("River color overrides");
//                            {
//                                for (int i = 0; i < riverColorOverrides->size(); i++) {
//                                    auto &riverColorOverride = riverColorOverrides->operator[](i);
//
//                                    ImGui::PushID(i);
//                                    if (ImGui::Button("Copy from current")) {
//                                        int areaId, parentAreaId;
//                                        mathfu::vec4 riverColor;
//
//                                        minimapGenerator->getCurrentFDData(areaId, parentAreaId, riverColor);
//                                        riverColorOverride.areaId = areaId;
//                                        riverColorOverride.color = riverColor;
//                                    }
//                                    ImGui::SameLine();
//                                    ImGui::CompactColorPicker("River Color Override", riverColorOverride.color);
//                                    ImGui::SameLine();
//                                    ImGui::PushItemWidth(100);
//                                    ImGui::InputInt("Area Id", &riverColorOverride.areaId);
//                                    ImGui::PopItemWidth();
//
//
//                                    ImGui::PopID();
//                                }
//
//                                if (ImGui::Button("Add  override")) {
//                                    riverColorOverrides->push_back({});
//                                }
//                            }
//                            ImGui::EndGroupPanel();
            }
            ImGui::EndGroupPanel();

            ImGui::BeginGroupPanel("Current stats");
            {
                int areaId, parentAreaId;
                mathfu::vec4 riverColor;

                minimapGenerator->getCurrentFDData(areaId, parentAreaId, riverColor);
                ImGui::Text("Current areaId %d", areaId);
                ImGui::Text("Current parent areaId %d", parentAreaId);
                ImGui::CompactColorPicker("Current River Color", riverColor, false);

                ImGui::EndGroupPanel();
            }

            auto currentTime = sceneDef->time;
            ImGui::Text("Time: %02d:%02d", (int)(currentTime/120), (int)((currentTime/2) % 60));
            if (ImGui::SliderInt("Current time", &sceneDef->time, 0, 2880)) {
//                sceneDef->time = sceneDef->time;
            }

            editComponentsForConfig(minimapGenerator->getConfig());

            if (minimapGenerator->getCurrentMode() != EMGMode::eScreenshotGeneration) {
                bool isDisabled = minimapGenerator->getCurrentMode() != EMGMode::eNone;
                if (ImGui::ButtonDisablable("Start Screenshot Gen", isDisabled)) {
                    std::vector<ScenarioDef> list = {*sceneDef};

                    minimapGenerator->startScenarios(list);
                }
            } else {
                if (ImGui::Button("Stop Screenshot Gen")) {
                    minimapGenerator->stopPreview();
                }
            }
            ImGui::SameLine();
            if (minimapGenerator->getCurrentMode() != EMGMode::ePreview) {
                bool isDisabled = minimapGenerator->getCurrentMode() != EMGMode::eNone;
                if (ImGui::ButtonDisablable("Start Preview", isDisabled)) {
                    minimapGenerator->startPreview(*sceneDef);
                    minimapGenerator->setZoom(previewZoom);
                    minimapGenerator->setLookAtPoint(previewX, previewY);
                }
            } else {
                if (ImGui::Button("Stop Preview")) {
                    minimapGenerator->stopPreview();
                }
            }
            ImGui::SameLine();
            if (minimapGenerator->getCurrentMode() != EMGMode::eBoundingBoxCalculation) {
                bool isDisabled = minimapGenerator->getCurrentMode() != EMGMode::eNone;
                if (ImGui::ButtonDisablable("Start BBox calc", isDisabled)) {
                    minimapGenerator->startBoundingBoxCalc(*sceneDef);
                }
            } else {
                if (ImGui::Button("Stop BBox calc")) {
                    minimapGenerator->stopBoundingBoxCalc();
                }
            }

            if (minimapGenerator->getCurrentMode() != EMGMode::eNone && minimapGenerator->getCurrentMode() != EMGMode::ePreview) {
                int x, y, maxX, maxY;
                minimapGenerator->getCurrentTileCoordinates(x, y, maxX, maxY);

                ImGui::Text("X: % 03d out of % 03d", x, maxX);
                ImGui::Text("Y: % 03d out of % 03d", y, maxY);

            }

            if (ImGui::Button("Save")) {
                m_minimapDB->saveScenarios(sceneDefList);
            }
        }

        ImGui::EndTabItem();
    }
}

void MinimapGenerationWindow::renderListTab() {
    if (ImGui::BeginTabItem("List"))
    {
        //The table
        {
            ImGui::BeginChild("Scenario List", ImVec2(0, -(100 + ImGui::GetStyle().ItemSpacing.y)), false,
                              0); // Use avail width, use 100 for height
            ImGui::Columns(3, "scenarioListcolumns"); // 3-ways, with border
            ImGui::Separator();
            ImGui::Text("");
            ImGui::NextColumn();
            ImGui::Text("Name");
            ImGui::NextColumn();
            ImGui::Text("Actions");
            ImGui::NextColumn();
            ImGui::Separator();
            for (int i = 0; i < this->sceneDefList.size(); i++) {
                auto &l_sceneDef = sceneDefList[i];
                bool checked = true;
                ImGui::Checkbox("", &checked);
                ImGui::NextColumn();
                ImGui::Text("%s", l_sceneDef.name.c_str());
                ImGui::NextColumn();
                if (ImGui::Button(("Edit##" + std::to_string(i)).c_str())) {
                    this->sceneDef = &l_sceneDef;
                    editTabOpened = true;
                }
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::EndChild();
        }

        {
            ImGui::BeginChild("BottomBar", ImVec2(0, 0), false, 0); // Use avail width/height
            if (ImGui::Button("Add new scenario")) {
                this->sceneDef = &sceneDefList.emplace_back();
                editTabOpened = true;
            }
            ImGui::Button("Render selected");
            ImGui::EndChild();
        }

        ImGui::EndTabItem();
    }
}
