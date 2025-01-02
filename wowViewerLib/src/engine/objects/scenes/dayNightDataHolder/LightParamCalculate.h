//
// Created by Deamon on 12/28/2024.
//

#ifndef AWEBWOWVIEWERCPP_LIGHTPARAMCALCULATE_H
#define AWEBWOWVIEWERCPP_LIGHTPARAMCALCULATE_H

#include <string>
#include <vector>
#include "../../../persistance/header/commonFileStructs.h"
#include "../../../../include/databaseHandler.h"
#include "../../../../renderer/mapScene/FrameDependentData.h"
#include "../../../algorithms/mathHelper.h"

struct mapInnerZoneLightRecord {
    int ID;
    std::string name;
    int LightID;
    CAaBox aabb;
    std::vector<mathfu::vec2> points;
    std::vector<mathfu::vec2> lines;
};

inline float clampF(float val, float min, float max) {
    return std::min<float>(
        std::max<float>(min, val),
        max
    );
}

inline
std::vector<IdAndBlendAndPriority> calculateLightParamBlends(
    const std::shared_ptr<IClientDatabase> &databaseHandler,
    int mapId,
    const mathfu::vec3 &cameraVec3,
    StateForConditions *stateForConditions,
    const std::vector<mapInnerZoneLightRecord> &zoneLights,
    int currentLightParamIdIndex
) {
    //Get light from DB
    std::vector<LightResult> lightResults;
    databaseHandler->getEnvInfo(mapId,
                                cameraVec3.x,
                                cameraVec3.y,
                                cameraVec3.z,
                                lightResults
    );
    std::sort(lightResults.begin(), lightResults.end(), [](const LightResult &a, const LightResult &b) -> bool {
        return a.blendAlpha < b.blendAlpha;
    });

    int defaultLightParamId = -1;
    int defaultLightId = -1;
    bool selectedDefault = false;
    bool selectedDefaultMap = false;
    {
        for (auto it = lightResults.begin(); it != lightResults.end();) {
            if (feq(it->pos[0], 0.0) && feq(it->pos[1], 0.0) && feq(it->pos[2], 0.0)) {
                //This is default record. If zoneLight was selected -> skip it.
                if (!selectedDefaultMap) {
                    if (it->continentId == mapId) {
                        selectedDefaultMap = true;
                        defaultLightParamId = it->lightParamId[currentLightParamIdIndex]; defaultLightId = it->id;
                    } else if (!selectedDefault && it->continentId == 0) {
                        selectedDefault = true;
                        defaultLightParamId = it->lightParamId[currentLightParamIdIndex]; defaultLightId = it->id;
                    }
                }
                it = lightResults.erase(it);
            } else {
                it++;
            }
        }
    }


    struct FoundZoneLights {
        int LightId = -1;
        float dist;
    };

    std::vector<FoundZoneLights> foundZoneLights;
    std::vector<IdAndBlendAndPriority> paramsBlend;

    const float zoneBlendDistStart = 50.0f;

    for (const auto &zoneLight : zoneLights) {
        CAaBox laabb = zoneLight.aabb;

        bool isInsideConvex2 = false;

        bool isInsideConvex = MathHelper::isPointInsideNonConvex(cameraVec3, zoneLight.aabb, zoneLight.points);
        float distToBorder = MathHelper::findLeastDistanceToBorder(cameraVec3, zoneLight.points, isInsideConvex2);

        float diffMin = cameraVec3.z-zoneLight.aabb.min.z;
        float diffMax = zoneLight.aabb.max.z - cameraVec3.z;

        bool isInsideZMinMax = diffMin > 0.0f && diffMax > 0.0f;
        float distToZminMax = std::min<float>(fabs(diffMin), fabs(diffMax));

        if (isInsideConvex) distToBorder = -distToBorder;
//        if (isInsideConvex2) distToBorder = -distToBorder;

        if (isInsideZMinMax) distToZminMax = -distToZminMax;

        float blendDist = distToBorder - 50.0f;
        float blendDistZMinMax = distToZminMax - 50.0f;

        if (blendDist < 0.0f && blendDistZMinMax < 0.0f) {
            float finalBlendDist = fmax(blendDist, blendDistZMinMax);

            if (stateForConditions != nullptr) {
                stateForConditions->currentZoneLights.push_back({zoneLight.ID, finalBlendDist});
            }
            foundZoneLights.emplace_back() = {zoneLight.LightID, finalBlendDist};
        }
    }

    if (foundZoneLights.empty()) {
        if (stateForConditions != nullptr && defaultLightParamId > 0 && defaultLightId > 0) {
            stateForConditions->currentLightParams.push_back({defaultLightParamId, 1.0f});
            stateForConditions->currentLightIds.push_back({defaultLightId, 1.0f});
        }
        paramsBlend.push_back({defaultLightParamId, 1.0f, 0});
    }

    bool applyFirstAsDefault = !selectedDefaultMap;
    std::sort(foundZoneLights.begin(), foundZoneLights.end(), [](const FoundZoneLights &a, const FoundZoneLights &b) {
        return a.LightId > b.LightId;
    });
    for (auto it = foundZoneLights.begin(); it != foundZoneLights.end(); it++) {
        LightResult zoneLightResult;


        float blendFactor =
            clampF((-it->dist) / (2 * zoneBlendDistStart), 0.0f, 1.0f);
//            (-it->dist) / (2 * zoneBlendDistStart);


        databaseHandler->getLightById(it->LightId, zoneLightResult);

        paramsBlend.push_back({zoneLightResult.lightParamId[currentLightParamIdIndex], blendFactor, 1});

        if (stateForConditions != nullptr) {
            stateForConditions->currentLightParams.push_back({zoneLightResult.lightParamId[currentLightParamIdIndex], blendFactor});
            stateForConditions->currentLightIds.push_back({it->LightId, blendFactor});
        }
    }

    std::sort(lightResults.begin(), lightResults.end(), [](const LightResult &a, const LightResult &b) {
        return a.blendAlpha > b.blendAlpha;
    });
    for (auto it = lightResults.begin(); it != lightResults.end(); it++) {
        if (it->blendAlpha > 0) {
            paramsBlend.push_back({it->lightParamId[currentLightParamIdIndex], it->blendAlpha, 2});
        }

        if (stateForConditions != nullptr) {
            stateForConditions->currentLightParams.push_back({it->lightParamId[currentLightParamIdIndex], it->blendAlpha});
            stateForConditions->currentLightIds.push_back({it->id, it->blendAlpha});
        }
    }

    return paramsBlend;
}

inline std::vector<mapInnerZoneLightRecord> loadZoneLightRecs(const std::shared_ptr<IClientDatabase> &databaseHandler, int mapId) {
    std::vector<mapInnerZoneLightRecord> results;
    if (databaseHandler != nullptr) {
        std::vector<ZoneLight> zoneLights;
        databaseHandler->getZoneLightsForMap(mapId, zoneLights);

        for (const auto &zoneLight : zoneLights) {
            mapInnerZoneLightRecord innerZoneLightRecord;
            innerZoneLightRecord.ID = zoneLight.ID;
            innerZoneLightRecord.name = zoneLight.name;
            innerZoneLightRecord.LightID = zoneLight.LightID;
//            innerZoneLightRecord.Zmin = zoneLight.Zmin;
//            innerZoneLightRecord.Zmax = zoneLight.Zmax;

            float minX = 9999; float maxX = -9999;
            float minY = 9999; float maxY = -9999;

            auto &points = innerZoneLightRecord.points;
            for (auto &zonePoint : zoneLight.points) {
                minX = std::min<float>(zonePoint.x, minX); minY = std::min<float>(zonePoint.y, minY);
                maxX = std::max<float>(zonePoint.x, maxX); maxY = std::max<float>(zonePoint.y, maxY);

                points.push_back(mathfu::vec2(zonePoint.x, zonePoint.y));
            }

            innerZoneLightRecord.aabb = CAaBox(
                C3Vector(mathfu::vec3(minX, minY, zoneLight.Zmin)),
                C3Vector(mathfu::vec3(maxX, maxY, zoneLight.Zmax))
            );

            auto &lines = innerZoneLightRecord.lines;
            for (int i = 0; i < (points.size() - 1); i++) {
                lines.push_back(points[i + 1] - points[i]);
            }
            lines.push_back( points[0] - points[points.size() - 1] );

            results.push_back(innerZoneLightRecord);
        }
    }

    return results;
}

#endif //AWEBWOWVIEWERCPP_LIGHTPARAMCALCULATE_H
