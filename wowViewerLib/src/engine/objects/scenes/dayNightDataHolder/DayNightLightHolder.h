//
// Created by Deamon on 7/5/2024.
//

#ifndef AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H
#define AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H

#include <string>
#include <vector>
#include "../../../persistance/header/commonFileStructs.h"
#include "../../../ApiContainer.h"
#include "../../../../renderer/mapScene/MapScenePlan.h"

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
                                SkyBodyData &skyBodyData,
                                ExteriorColors &exteriorColors,
                                FogResult &fogResult,
                                LiquidColors &liquidColors,
                                StateForConditions *stateForConditions);

private:
    bool m_mapFlag2_0x2 = false;
    bool m_useWeightedBlend = false;
    bool m_mapHasFlag_0x200000 = false;
    bool m_mapHasFlag_0x10000 = false;

    float m_minFogDist1 = 0.0;
    float m_minFogDist2 = 0.0;
    float m_minFogDist3 = 0.0;

    void fixLightTimedData(LightTimedData &data, float farClip, float &fogScalarOverride);
    float getClampedFarClip(float farClip);
    void createMinFogDistances();
};


#endif //AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H
