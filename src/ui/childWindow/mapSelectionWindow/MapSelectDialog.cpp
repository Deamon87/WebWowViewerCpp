//
// Created by Deamon on 6/3/2023.
//

#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "MapSelectDialog.h"
#include "../imguiLib/imgui.h"
#include "wheelCapture/wheelCapture.h"

MapSelectDialog::MapSelectDialog(const HApiContainer &api,
                                 const std::shared_ptr<FrontendUIRenderer> &uiRenderer,
                                 const SceneWindowAccessor &getSceneWindow) : m_api(api),
                                                                              m_uiRenderer(uiRenderer),
                                                                              m_getSceneWindow(getSceneWindow)
{
    emptyMinimap();

    mapList = {};
    mapListStringMap = {};
    filteredMapList = {};
}

void MapSelectDialog::setCurrentMap(const MapRecord &mapRec) {
    minimapZoom = 0.1;
    mapCanBeOpened = true;
    adtMinimapFilled = false;
    prevMapRec = mapRec;
    prevMapId = mapRec.ID;

    limitZoneLight = -1;
    m_zoneLights = {};

    isWmoMap = false;
    adtSelectionMinimapTextures = {};
    adtSelectionMinimapMaterials = {};
    if (mapRec.WdtFileID > 0) {
        getAdtSelectionMinimap(mapRec.WdtFileID);
    } else {
        getAdtSelectionMinimap(
            "world/maps/" + mapRec.MapDirectory + "/" + mapRec.MapDirectory + ".wdt");
    }

    m_zoneLights = loadZoneLightRecs(m_api->databaseHandler, mapRec.ID);
    m_mapLights.clear();
    m_api->databaseHandler->getAllLightByMap(mapRec.ID, m_mapLights);
}

void MapSelectDialog::renderLeftSide() {
    enum MyMapTableColumnID {
        MyItemColumnID_ID,
        MyItemColumnID_Name,
        MyItemColumnID_Directory,
        MyItemColumnID_WDTId,
        MyItemColumnID_MapType
    };

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


            for (int i = 0; i < filteredMapList.size(); i++) {
                ImGui::TableNextRow();
                const auto &mapRec = filteredMapList[i];

                ImGui::TableSetColumnIndex(0);


                if (ImGui::Selectable(mapListStringMap[i][0].c_str(), prevMapId == mapRec.ID,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                      ImGuiSelectableFlags_AllowItemOverlap)) {
                    if (mapRec.ID != prevMapId) {
                        setCurrentMap(mapRec);
                    }
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
}

void MapSelectDialog::renderRightSide() {
    ImGui::BeginChild("Map Select Dialog Right panel", ImVec2(0, 0));
    {
        if (prevMapId == -1) {
            ImGui::Text("Please select a map from the list");
            ImGui::Text("Or update database for maps to appear in the list");
        } else if (!mapCanBeOpened) {
            ImGui::Text("Cannot open this map.");
            ImGui::Text("WDT file either does not exist in CASC repository or is encrypted");
        } else if (!isWmoMap) {
            ImGui::SliderFloat("Zoom", &minimapZoom, 0.1, 10);
            ImGui::Checkbox("Draw Area lights", &drawAreaLights);
            ImGui::Checkbox("Draw ZoneLights", &drawZoneLights);
            ImGui::SameLine();
            ImGui::Text("Limit ZoneLights");
            ImGui::SameLine();
            std::string zoneLightIdStr = limitZoneLight >= 0 ? std::to_string(limitZoneLight) : "None";
            if (ImGui::BeginCombo("##combo", zoneLightIdStr.c_str())) // The second parameter is the label previewed before opening the combo.
            {
                {
                    const std::string caption = "First None";
                    if (ImGui::Selectable(caption.c_str(), limitZoneLight == -1)) {
                        limitZoneLight = -1;
                    }
                }

                for (int n = 0; n < m_zoneLights.size(); n++)
                {
                    bool is_selected = (m_zoneLights[n].ID == limitZoneLight); // You can store your selection however you want, outside or inside your objects
                    std::string caption = std::to_string(m_zoneLights[n].ID);
                    if (ImGui::Selectable(caption.c_str(), is_selected)) {
                        limitZoneLight = m_zoneLights[n].ID;
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
            //                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
            showAdtSelectionMinimap();
        } else {
            worldPosX = 0;
            worldPosY = 0;
            if (ImGui::Button("Open WMO Map", ImVec2(-1, 0))) {
                auto scene = m_getSceneWindow();
                if (scene) {
                    if (prevMapRec.WdtFileID > 0) {
                        scene->openMapByIdAndWDTId(prevMapId, prevMapRec.WdtFileID, 17066.6641f, 17066.67380f, 0, prevMapRec.overrideTime);
                    } else {
                        //Try to open map by fileName
                        scene->openMapByIdAndFilename(prevMapId, prevMapRec.MapDirectory, 17066.6641f, 17066.67380f,
                                                                  0, prevMapRec.overrideTime);
                    }
                }
                m_showWindow = false;
            }
        }

    }
    ImGui::EndChild();
}

void MapSelectDialog::showAdtSelectionMinimap() {
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

    //Draw
    {
        const float zoomConstant = prevZoomedSize;
        const auto windowPos = ImGui::GetWindowPos();
        const auto scrollX = ImGui::GetScrollX();
        const auto scrollY = ImGui::GetScrollY();
        //To Window Space
        const auto toWS = [&zoomConstant](float value) -> float {
            return worldCoordinateToAdtIndexF(value) * zoomConstant;
        };
        //To Window Space Translated
        const auto toWST = [&toWS, windowPos, scrollX, scrollY](float value, int index) -> float {
            return toWS(value) + (index == 0 ? (windowPos.x - scrollX) : (windowPos.y - scrollY));
        };

        if (drawZoneLights) {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            for (const auto &zoneLight: m_zoneLights) {
                if (limitZoneLight != -1 && zoneLight.ID != limitZoneLight) continue;

                if (!zoneLight.points.empty()) {
                    for (int n = 0; n < zoneLight.points.size() - 1; n++)
                        draw_list->AddLine(
                            ImVec2(toWST(zoneLight.points[n].y, 0), toWST(zoneLight.points[n].x, 1)),
                            ImVec2(toWST(zoneLight.points[n + 1].y, 0), toWST(zoneLight.points[n + 1].x, 1)),
                            IM_COL32(255, 255, 0, 255),
                            2.0f
                        );
                    uint32_t maxIdx = zoneLight.points.size() - 1;
                    draw_list->AddLine(
                        ImVec2(toWST(zoneLight.points[0].y, 0), toWST(zoneLight.points[0].x, 1)),
                        ImVec2(toWST(zoneLight.points[maxIdx].y, 0), toWST(zoneLight.points[maxIdx].x, 1)),
                        IM_COL32(255, 255, 0, 255),
                        2.0f
                    );
                }
            }
        }
        if (drawAreaLights) {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            for (auto const &light: m_mapLights) {
                draw_list->AddCircle(
                    ImVec2(toWST(light.pos[1], 0), toWST(light.pos[0], 1)),
                    light.fallbackEnd * prevZoomedSize / MathHelper::TILESIZE,
                    IM_COL32(3, 186, 252, 255)
                );
                draw_list->AddCircle(
                    ImVec2(toWST(light.pos[1], 0), toWST(light.pos[0], 1)),
                    light.fallbackStart * prevZoomedSize / MathHelper::TILESIZE,
                    IM_COL32(58, 138, 126, 255)
                );
            }
        }
    }


    if (ImGui::BeginPopup("AdtWorldCoordsTest", ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Pos: (%.2f,%.2f,200)", worldPosX, worldPosY);
        if (ImGui::Button("Go")) {
            auto scene = m_getSceneWindow();
            if (scene) {
                if (prevMapRec.WdtFileID > 0) {
                    scene->openMapByIdAndWDTId(prevMapId, prevMapRec.WdtFileID, worldPosX, worldPosY, 200, prevMapRec.overrideTime);
                } else {
                    scene->openMapByIdAndFilename(prevMapId, prevMapRec.MapDirectory, worldPosX, worldPosY, 200, prevMapRec.overrideTime);
                }
            }
            m_showWindow = false;

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


void MapSelectDialog::filterMapList(const std::string &text) {
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
void MapSelectDialog::fillMapListStrings() {
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

bool MapSelectDialog::fillAdtSelectionminimap(bool &isWMOMap, bool &wdtFileExists) {
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

void MapSelectDialog::getAdtSelectionMinimap(int wdtFileDataId) {
    m_wdtFile = m_api->cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);
}

void MapSelectDialog::getAdtSelectionMinimap(std::string wdtFilePath) {
    m_wdtFile = m_api->cacheStorage->getWdtFileCache()->get(wdtFilePath);
}

void MapSelectDialog::getMapList(std::vector<MapRecord> &mapList) {
    if (m_api->databaseHandler == nullptr)  return;

    m_api->databaseHandler->getMapArray(mapList);
}

void MapSelectDialog::show() {
    m_showWindow = true;
}

bool MapSelectDialog::draw() {
    if (!m_showWindow) return true;

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

    if (mapCanBeOpened && !adtMinimapFilled && fillAdtSelectionminimap(isWmoMap, mapCanBeOpened )) {
        adtMinimapFilled = true;
    }

    bool windowWasManuallyClosed = false;
    auto prevShowWindow= m_showWindow;
    ImGui::Begin("Map Select Dialog", &m_showWindow);
    {
        windowWasManuallyClosed = prevShowWindow != m_showWindow;
        ImGui::Columns(2, NULL, true);
        this->renderLeftSide();
        ImGui::NextColumn();
        this->renderRightSide();

        ImGui::Columns(1);

        ImGui::End();
    }

    return !windowWasManuallyClosed;
}


