//
// Created by Deamon on 10/4/2020.
//

#ifndef AWEBWOWVIEWERCPP_MAPCONSTRUCTIONWINDOW_H
#define AWEBWOWVIEWERCPP_MAPCONSTRUCTIONWINDOW_H


#include "../../../wowViewerLib/src/engine/ApiContainer.h"

class MapConstructionWindow {
private:
    ApiContainer * m_api;

    std::vector<MapRecord> mapList = {};
    std::vector<MapRecord> filteredMapList = {};
    std::vector<std::vector<std::string>> mapListStringMap = {};

    bool showMapConstruction = false;


    bool mapCanBeOpened = true;
    bool adtMinimapFilled = false;
    bool isWmoMap = false;

    int prevMapId = -1;
    MapRecord prevMapRec;

    std::shared_ptr<WdtFile> currentlySelectedWdtFile = nullptr;
    std::array<std::array<HGTexture, 64>, 64> adtSelectionMinimap;

    float minimapZoom = 1;
    float prevMinimapZoom = 1;

    std::array<char, 128> filterText = {0};
    bool refilterIsNeeded = false;
    void filterMapList(std::string text);

    bool fillAdtSelectionminimap(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap, bool &wdtFileExists);
    void showAdtSelectionMinimap();
public:
    MapConstructionWindow(ApiContainer *mApi);

    bool render();
};


#endif //AWEBWOWVIEWERCPP_MAPCONSTRUCTIONWINDOW_H
