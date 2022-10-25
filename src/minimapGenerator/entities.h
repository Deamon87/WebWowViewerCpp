//
// Created by Deamon on 1/22/2021.
//

#ifndef AWEBWOWVIEWERCPP_ENTITIES_H
#define AWEBWOWVIEWERCPP_ENTITIES_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <array>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <mathfu/glsl_mappings.h>
#include "../../wowViewerLib/src/include/config.h"

enum class ScenarioOrientation {
    soTopDownOrtho,
    so45DegreeTick0,
    so45DegreeTick1,
    so45DegreeTick2,
    so45DegreeTick3,
};
enum class EMGMode {
    eNone,
    eBoundingBoxCalculation,
    eScreenshotGeneration,
    ePreview,
};

struct MapRenderDef {
    int id = -1;

    int mapId;

    double deltaX;
    double deltaY;
    double deltaZ;

    HADTRenderConfigDataHolder adtConfigHolder = std::make_shared<ADTRenderConfigData>();
};

struct ScenarioDef {
    std::vector<MapRenderDef> maps;

    int id = -1;
    std::string name;
    ScenarioOrientation orientation = ScenarioOrientation::so45DegreeTick0;

    std::unordered_map<int, std::vector<int>> activatePhasePerMap;

    mathfu::vec4 closeOceanColor;

    mathfu::vec2 minWowWorldCoord;
    mathfu::vec2 maxWowWorldCoord;

    int imageHeight;

    int imageWidth;

    float zoom = 1.0f;

    std::string folderToSave;
};


#endif //AWEBWOWVIEWERCPP_ENTITIES_H
