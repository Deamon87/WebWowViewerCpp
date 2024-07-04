//
// Created by Deamon on 7/5/2024.
//

#ifndef AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H
#define AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H

#include <string>
#include <vector>
#include "../../../persistance/header/commonFileStructs.h"
#include "../../../ApiContainer.h"

class DayNightLightHolder {
public:
    DayNightLightHolder(const HApiContainer &api, int mapId);

private:
    struct mapInnerZoneLightRecord {
        int ID;
        std::string name;
        int LightID;
        CAaBox aabb;
        std::vector<mathfu::vec2> points;
        std::vector<mathfu::vec2> lines;
    };
    std::vector<mapInnerZoneLightRecord> m_zoneLights;

    HApiContainer m_api;
    int m_mapId = -1;
public:
    void loadZoneLights();
    void updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                   StateForConditions &stateForConditions, const AreaRecord &areaRecord);

    void getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config,
                                SkyColors &skyColors,
                                ExteriorColors &exteriorColors,
                                FogResult &fogResult,
                                LiquidColors &liquidColors,
                                StateForConditions *stateForConditions);
};


#endif //AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H
