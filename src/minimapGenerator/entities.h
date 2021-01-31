//
// Created by Deamon on 1/22/2021.
//

#ifndef AWEBWOWVIEWERCPP_ENTITIES_H
#define AWEBWOWVIEWERCPP_ENTITIES_H

#include <string>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <mathfu/glsl_mappings.h>

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

struct ScenarioDef {
    int id;
    int mapId;
    std::string name;

    mathfu::vec4 closeOceanColor;

    mathfu::vec2 minWowWorldCoord;
    mathfu::vec2 maxWowWorldCoord;

    int imageHeight;
    int imageWidth;

    float zoom = 1.0f;

    ScenarioOrientation orientation = ScenarioOrientation::so45DegreeTick0;

    std::string folderToSave;
};


#endif //AWEBWOWVIEWERCPP_ENTITIES_H
