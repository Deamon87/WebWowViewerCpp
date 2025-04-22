#ifndef AWEBWOWVIEWERCPP_MAPSELECTDIALOG_H
#define AWEBWOWVIEWERCPP_MAPSELECTDIALOG_H

#include <array>
#include <vector>
#include "../include/database/dbStructs.h"
#include "../../renderer/uiScene/materials/UIMaterial.h"
#include "../../renderer/uiScene/FrontendUIRenderer.h"
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"

#include "../sceneWindow/SceneWindow.h"

#include "../../../../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../../../../wowViewerLib/src/engine/objects/scenes/dayNightDataHolder/LightParamCalculate.h"

typedef  std::function<std::shared_ptr<SceneWindow>()> SceneWindowAccessor;

class MapSelectDialog {
public:
    MapSelectDialog(const HApiContainer &api, const std::shared_ptr<FrontendUIRenderer> &uiRenderer,
                    const SceneWindowAccessor &getSceneWindow);

    void setCurrentMap(const MapRecord & mapRec);

    void renderLeftSide();

    void renderRightSide();

    void show();

    bool draw();

    void showAdtSelectionMinimap();
private:
    bool m_showWindow = true;

    HApiContainer m_api;
    std::shared_ptr<FrontendUIRenderer> m_uiRenderer;
    SceneWindowAccessor m_getSceneWindow;

    int prevMapId = -1;

    HWdtFile m_wdtFile = nullptr;

    std::vector<MapRecord> mapList = {};
    std::vector<MapRecord> filteredMapList = {};
    std::vector<std::vector<std::string>> mapListStringMap = {};

    bool refilterIsNeeded = false;
    bool resortIsNeeded = false;

    std::array<char, 128> filterText = {0};

    float minimapZoom = 1;
    bool mapCanBeOpened = true;
    bool isWmoMap = false;
    bool adtMinimapFilled = false;

    float prevMinimapZoom = 1;
    float prevZoomedSize = 0;

    bool drawZoneLights = false;
    bool drawAreaLights = false;

    std::array<std::array<HGSamplableTexture, 64>, 64> adtSelectionMinimapTextures;
    std::array<std::array<std::shared_ptr<IUIMaterial>, 64>, 64> adtSelectionMinimapMaterials;
    std::vector<mapInnerZoneLightRecord> m_zoneLights = {};
    std::vector<LightResult> m_mapLights = {};
    int limitZoneLight = -1;

    MapRecord prevMapRec;

    float worldPosX = 0;
    float worldPosY = 0;
    float worldPosZ = 0;

    void emptyMinimap() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                adtSelectionMinimapTextures[i][j] = nullptr;
                adtSelectionMinimapMaterials[i][j] = nullptr;
            }
        }
    }

    void getAdtSelectionMinimap(int wdtFileDataId);
    void getAdtSelectionMinimap(std::string wdtFilePath);
    void getMapList(std::vector<MapRecord> &mapList);

    void filterMapList(const std::string &text);
    void fillMapListStrings();
    bool fillAdtSelectionminimap(bool &isWMOMap, bool &wdtFileExists);
};


#endif //AWEBWOWVIEWERCPP_MAPSELECTDIALOG_H
