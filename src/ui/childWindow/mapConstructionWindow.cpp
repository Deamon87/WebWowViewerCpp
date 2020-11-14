//
// Created by Deamon on 10/4/2020.
//

#include <imgui.h>
#include <imgui_internal.h>
#include "mapConstructionWindow.h"
#include "../../../wowViewerLib/src/engine/algorithms/mathHelper.h"

MapConstructionWindow::MapConstructionWindow(HApiContainer mApi) : m_api(mApi) {}

bool MapConstructionWindow::render() {
    bool isNotClosed = true;
    if (mapList.size() == 0) {
        m_api->databaseHandler->getMapArray(mapList);
    }

    if (mapCanBeOpened) {
        if (!adtMinimapFilled && fillAdtSelectionminimap(adtSelectionMinimap, isWmoMap, mapCanBeOpened )) {
//            fillAdtSelectionminimap = nullptr;
            adtMinimapFilled = true;
        }
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

    ImGui::Begin("Map construction dialog", &isNotClosed);
    {
        ImGui::Columns(2, NULL, true);
        {
            ImGui::BeginChild("Map Select Dialog for map constr", ImVec2(0, 0));
            {
                if (!mapCanBeOpened) {
                    ImGui::Text("Cannot open this map.");
                    ImGui::Text("WDT file either does not exist in CASC repository or is encrypted");
                } else if (!isWmoMap) {
                    ImGui::SliderFloat("Zoom", &minimapZoom, 0.1, 10);
                    //                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    showAdtSelectionMinimap();
                } else {
                    ImGui::Text("This is WMO map.");
                }

            }
            ImGui::EndChild();
        }

        ImGui::NextColumn();

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
                        currentlySelectedWdtFile = m_api->cacheStorage->getWdtFileCache()->getFileId(mapRec.WdtFileID);
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
    }
    ImGui::End();

    return isNotClosed;
}

bool MapConstructionWindow::fillAdtSelectionminimap(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap,
                                         bool &wdtFileExists) {
    if (currentlySelectedWdtFile == nullptr) return false;

    if (currentlySelectedWdtFile->getStatus() == FileStatus::FSRejected) {
        wdtFileExists = false;
        isWMOMap = false;
        return false;
    }

    if (currentlySelectedWdtFile->getStatus() != FileStatus::FSLoaded) return false;

    isWMOMap = currentlySelectedWdtFile->mphd->flags.wdt_uses_global_map_obj != 0;

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            if (currentlySelectedWdtFile->mapFileDataIDs[i*64 + j].minimapTexture > 0) {
                auto texture = m_api->cacheStorage->getTextureCache()->getFileId(currentlySelectedWdtFile->mapFileDataIDs[i*64 + j].minimapTexture);
                minimap[i][j] = m_api->hDevice->createBlpTexture(texture, false, false);
            } else {
                minimap[i][j] = nullptr;
            }
        }
    }
    return true;
}

template<typename charT>
struct my_equal1 {
    my_equal1( const std::locale& loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};

template<typename T>
int ci_find_substr_1( const T& str1, const T& str2, const std::locale& loc = std::locale() )
{
    typename T::const_iterator it = std::search( str1.begin(), str1.end(),
                                                 str2.begin(), str2.end(), my_equal1<typename T::value_type>(loc) );
    if ( it != str1.end() ) return it - str1.begin();
    else return -1; // not found
}

void MapConstructionWindow::filterMapList(std::string text) {
    filteredMapList = {};
    for (int i = 0; i < mapList.size(); i++) {
        auto &currentRec = mapList[i];
        if (text == "" ||
            (
                (ci_find_substr_1(currentRec.MapName, text) != std::string::npos) ||
                (ci_find_substr_1(currentRec.MapDirectory, text) != std::string::npos)
            )
            ) {
            filteredMapList.push_back(currentRec);
        }
    }
}

void MapConstructionWindow::showAdtSelectionMinimap() {
    ImGui::BeginChild("Adt selection minimap", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar |
                                                                   ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (minimapZoom < 0.001)
        minimapZoom = 0.001f;

    float oldRealScrollX = ImGui::GetScrollX() + (ImGui::GetCurrentWindow()->InnerRect.GetWidth() / 2.0f);
    float oldRealScrollY = ImGui::GetScrollY() + (ImGui::GetCurrentWindow()->InnerRect.GetHeight() / 2.0f);

    float oldScrollPercX = oldRealScrollX / (std::ceil(64*100*prevMinimapZoom) + (ImGui::GetCurrentWindow()->WindowPadding.x * 2.0f));
    float oldScrollPercY = oldRealScrollY / (std::ceil(64*100*prevMinimapZoom) + (ImGui::GetCurrentWindow()->WindowPadding.y * 2.0f));

    if (prevMinimapZoom != minimapZoom) {
        auto windowSize = ImGui::GetWindowSize();
//        ImGui::SetScrollX((windowSize.x / 2.0f) * ((ImGui::GetScrollX() + 1.0f) * minimapZoom / prevMinimapZoom - 1.0f));
//        ImGui::SetScrollY((windowSize.y / 2.0f) * ((ImGui::GetScrollY() + 1.0f) * minimapZoom / prevMinimapZoom - 1.0f));

//        ImGui::SetScrollX((minimapZoom / prevMinimapZoom) * (ImGui::GetScrollX()/ImGui::GetScrollMaxX()));
//        ImGui::SetScrollY((minimapZoom / prevMinimapZoom) * (ImGui::GetScrollY()/ImGui::GetScrollMaxY()));

    }

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

//                    worldPosX = mousePos.y;
//                    worldPosY = mousePos.x;
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

    if (prevMinimapZoom != minimapZoom) {
//        ImGui::BeginChild("Adt selection minimap", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar |
//                                                                       ImGuiWindowFlags_AlwaysVerticalScrollbar);

        float newScrollX = oldScrollPercX * (std::ceil(64*100*minimapZoom) + (ImGui::GetCurrentWindow()->WindowPadding.x * 2.0f)) - (ImGui::GetCurrentWindow()->InnerRect.GetWidth() / 2.0f);
        float newScrollY = oldScrollPercY * (std::ceil(64*100*minimapZoom) + (ImGui::GetCurrentWindow()->WindowPadding.y * 2.0f)) - (ImGui::GetCurrentWindow()->InnerRect.GetHeight() / 2.0f);

        ImGui::SetScrollX(newScrollX);
        ImGui::SetScrollY(newScrollY);
        prevMinimapZoom = minimapZoom;

//        ImGui::EndChild();
    }

    ImGui::EndChild();

}