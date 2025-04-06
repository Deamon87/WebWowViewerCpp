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

    int mapId = -1;

    double deltaX = 0.0f;
    double deltaY = 0.0f;
    double deltaZ = 0.0f;

    HADTRenderConfigDataHolder adtConfigHolder = std::make_shared<ADTRenderConfigData>();
};

struct ScenarioDef {

    int id = -1;
    std::string name;
    ScenarioOrientation orientation = ScenarioOrientation::so45DegreeTick0;

    mathfu::vec4 closeOceanColor;

    mathfu::vec2 minWowWorldCoord;
    mathfu::vec2 maxWowWorldCoord;

    //TODO: add check
    int imageHeight = 1024;
    int imageWidth = 1024;

    float zoom = 1.0f;

    int time = 0;

    std::string folderToSave;

    std::unordered_map<int, std::vector<int>> activatePhasePerMap;
    std::vector<MapRenderDef> maps;
public:
    int getOrientation() const {return (int) orientation;}
    void setOrientation(int value) { orientation = static_cast<ScenarioOrientation>(value); };

    float getCloseOceanColor_R() const {return closeOceanColor[0];}
    float getCloseOceanColor_G() const {return closeOceanColor[1];}
    float getCloseOceanColor_B() const {return closeOceanColor[2];}

    void setCloseOceanColor_R(float value) {closeOceanColor[0] = value;}
    void setCloseOceanColor_G(float value) {closeOceanColor[1] = value;}
    void setCloseOceanColor_B(float value) {closeOceanColor[2] = value;}

    float getMinWowWorldCoordX() const {return minWowWorldCoord.x;}
    float getMinWowWorldCoordY() const {return minWowWorldCoord.y;}
    float getMaxWowWorldCoordX() const {return maxWowWorldCoord.x;}
    float getMaxWowWorldCoordY() const {return maxWowWorldCoord.y;}

    void setMinWowWorldCoordX(float value){minWowWorldCoord.x = value;}
    void setMinWowWorldCoordY(float value){minWowWorldCoord.y = value;}
    void setMaxWowWorldCoordX(float value){maxWowWorldCoord.x = value;}
    void setMaxWowWorldCoordY(float value){maxWowWorldCoord.y = value;}
};


#endif //AWEBWOWVIEWERCPP_ENTITIES_H
