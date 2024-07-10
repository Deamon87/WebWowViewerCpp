//
// Created by Deamon on 7/5/2024.
//

#include "DayNightLightHolder.h"
#include "../../../../include/database/dbStructs.h"
#include "../../../algorithms/mathHelper.h"


DayNightLightHolder::DayNightLightHolder(const HApiContainer &api, int mapId) : m_api(api), m_mapId(mapId) {

    MapRecord mapRecord;
    if (m_api) {
        api->databaseHandler->getMapById(mapId, mapRecord);
        m_mapFlag2_0x2 = (mapRecord.flags2 & 0x2) > 0;
        m_useWeightedBlend = (mapRecord.flags0 & 0x4) > 0;
        m_mapHasFlag_0x200000 = (mapRecord.flags0 & 0x200000) > 0;
        m_mapHasFlag_0x10000 = (mapRecord.flags0 & 0x10000) > 0;
    }
}

void DayNightLightHolder::loadZoneLights() {
    if (m_api && m_api->databaseHandler != nullptr) {
        std::vector<ZoneLight> zoneLights;
        m_api->databaseHandler->getZoneLightsForMap(m_mapId, zoneLights);

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
            lines.push_back( points[0] - points[points.size() - 1]);

            m_zoneLights.push_back(innerZoneLightRecord);
        }
    }
}


static inline mathfu::vec4 mix(const mathfu::vec4 &a, const mathfu::vec4 &b, float alpha) {
    return (b - a) * alpha + a;
}
static inline mathfu::vec3 mix(const mathfu::vec3 &a, const mathfu::vec3 &b, float alpha) {
    return (b - a) * alpha + a;
}
static inline float mix(const float &a, const float &b, float alpha) {
    return (b - a) * alpha + a;
}

template <typename T>
void mixFogMember(FogResult& a, FogResult& b, T FogResult::*member, float blendTimeCoeff) {
    a.*member = mix(a.*member, b.*member, blendTimeCoeff);
}

void fogMixAndSet(FogResult& a, FogResult& b, float blendCoeff) {
    mixFogMember(a, b, &FogResult::FogEnd                       , blendCoeff);
    mixFogMember(a, b, &FogResult::FogScaler                    , blendCoeff);
    mixFogMember(a, b, &FogResult::FogDensity                   , blendCoeff);
    mixFogMember(a, b, &FogResult::FogHeight                    , blendCoeff);
    mixFogMember(a, b, &FogResult::FogHeightScaler              , blendCoeff);
    mixFogMember(a, b, &FogResult::FogHeightDensity             , blendCoeff);
    mixFogMember(a, b, &FogResult::SunFogAngle                  , blendCoeff);
    mixFogMember(a, b, &FogResult::FogColor                     , blendCoeff);
    mixFogMember(a, b, &FogResult::EndFogColor                  , blendCoeff);
    mixFogMember(a, b, &FogResult::EndFogColorDistance          , blendCoeff);
    mixFogMember(a, b, &FogResult::SunFogColor                  , blendCoeff);
    mixFogMember(a, b, &FogResult::SunFogStrength               , blendCoeff);
    mixFogMember(a, b, &FogResult::FogHeightColor               , blendCoeff);
    mixFogMember(a, b, &FogResult::FogHeightCoefficients        , blendCoeff);
    mixFogMember(a, b, &FogResult::MainFogCoefficients          , blendCoeff);
    mixFogMember(a, b, &FogResult::HeightDensityFogCoefficients , blendCoeff);
    mixFogMember(a, b, &FogResult::FogZScalar                   , blendCoeff);
    mixFogMember(a, b, &FogResult::LegacyFogScalar              , blendCoeff);
    mixFogMember(a, b, &FogResult::MainFogStartDist             , blendCoeff);
    mixFogMember(a, b, &FogResult::MainFogEndDist               , blendCoeff);
    mixFogMember(a, b, &FogResult::FogBlendAlpha                , blendCoeff);
    mixFogMember(a, b, &FogResult::HeightEndFogColor            , blendCoeff);
    mixFogMember(a, b, &FogResult::FogStartOffset               , blendCoeff);
    mixFogMember(a, b, &FogResult::SunAngleBlend                , blendCoeff);
}


void DayNightLightHolder::updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan,
                                                   MathHelper::FrustumCullingData &frustumData,
                                                   StateForConditions &stateForConditions,
                                                   const AreaRecord &areaRecord) {

    ZoneScoped ;
    if(!m_api) return;

    Config* config = this->m_api->getConfig();

    bool fogRecordWasFound = false;
    mathfu::vec3 endFogColor = mathfu::vec3(0.0, 0.0, 0.0);

    std::vector<SMOFog_Data> wmoFogData = {};
    if (mapRenderPlan->m_currentWMO != emptyWMO) {
        auto l_currentWmoObject = wmoFactory.getObjectById<0>(mapRenderPlan->m_currentWMO);
        if (l_currentWmoObject != nullptr) {
            l_currentWmoObject->checkFog(frustumData.cameraPos, wmoFogData);
        }
    }

    FogResult exteriorFogResult;

    auto fdd = mapRenderPlan->frameDependentData;

    std::vector<LightResult> lightResults;
    if ((m_api->databaseHandler != nullptr)) {
        //Check zoneLight
        SkyColors skyColors;
        ExteriorColors exteriorColors;

        LiquidColors liquidColors;
        SkyBodyData skyBodyData;

        getLightResultsFromDB(frustumData.cameraPos, config, skyColors, skyBodyData, exteriorColors, exteriorFogResult, liquidColors, &stateForConditions);

        //TODO: restore skyboxes
        /*
        //Delete skyboxes that are not in light array
        std::unordered_map<int, std::shared_ptr<M2Object>> perFdidMap;
        auto modelIt = m_exteriorSkyBoxes.begin();
        while (modelIt != m_exteriorSkyBoxes.end()) {
            bool found = false;
            for (auto &_light : lightResults) {
                if (_light.skyBoxFdid == (*modelIt)->getModelFileId()) {
                    if (!drawDefaultSkybox && _light.isDefault) continue;

                    found = true;
                    break;
                }
            }

            if (found) {
                perFdidMap[(*modelIt)->getModelFileId()] = *modelIt;
                modelIt++;
            } else {
                modelIt = m_exteriorSkyBoxes.erase(modelIt);
            }
        }

        m_skyConeAlpha = 1.0;



        for (auto &_light : lightResults) {
            if (_light.skyBoxFdid == 0 || _light.lightSkyboxId == 0) continue;

            stateForConditions.currentSkyboxIds.push_back(_light.lightSkyboxId);
            std::shared_ptr<M2Object> skyBox = nullptr;
            if (perFdidMap[_light.skyBoxFdid] == nullptr) {
                skyBox = m2Factory.createObject(m_api, true);
                skyBox->setLoadParams(0, {}, {});

                skyBox->setModelFileId(_light.skyBoxFdid);

                skyBox->createPlacementMatrix(mathfu::vec3(0, 0, 0), 0, mathfu::vec3(1, 1, 1), nullptr);
                skyBox->calcWorldPosition();
                m_exteriorSkyBoxes.push_back(skyBox);
            } else {
                skyBox = perFdidMap[_light.skyBoxFdid];
            }

            skyBox->setAlpha(_light.blendCoef);
            if ((_light.skyBoxFlags & 4) > 0 ) {
                //In this case conus is still rendered been, but all values are final fog values.
                auto fdd = mapRenderPlan->frameDependentData;
                fdd->overrideValuesWithFinalFog = true;
            }

            if ((_light.skyBoxFlags & 2) == 0) {
//                m_skyConeAlpha -= _light.blendCoef;
                m_skyConeAlpha -= _light.blendCoef;
            }

            if (_light.skyBoxFlags & 1) {
                skyBox->setOverrideAnimationPerc(config->currentTime / 2880.0, true);
            }
        }
        */

        {
            mathfu::vec3 sunPlanetPosVec3 = MathHelper::calcSunPlanetPos(
                mapRenderPlan->renderingMatrices->lookAtMat,
                m_api->getConfig()->currentTime
            ) + frustumData.cameraPos;

            if (skyBodyData.celestialBodyOverride2.LengthSquared() > 0.0f) {
                sunPlanetPosVec3 = mathfu::vec3(
                    skyBodyData.celestialBodyOverride2[0],
                    skyBodyData.celestialBodyOverride2[1],
                    skyBodyData.celestialBodyOverride2[2]);
            }
            mathfu::vec4 sunPlanetPos = mathfu::vec4((sunPlanetPosVec3 - frustumData.cameraPos).Normalized(), 0.0f);
            fdd->sunDirection = (frustumData.viewMat.Inverse().Transpose() * sunPlanetPos).xyz().Normalized();
        }

        float ambientMult = areaRecord.ambientMultiplier * 2.0f + 1;

        if (config->glowSource == EParameterSource::eDatabase) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->currentGlow = 1.0;//currentGlow;
        } else if (config->glowSource == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->currentGlow = config->currentGlow;
        }


        if (config->globalLighting == EParameterSource::eDatabase) {
            auto fdd = mapRenderPlan->frameDependentData;

            fdd->colors = exteriorColors;

            auto extDir = MathHelper::calcExteriorColorDir(
                frustumData.viewMat,
                m_api->getConfig()->currentTime
            );
            fdd->exteriorDirectColorDir = { extDir.x, extDir.y, extDir.z };
        } else if (config->globalLighting == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;

            fdd->colors.exteriorAmbientColor = config->exteriorColors.exteriorAmbientColor;
            fdd->colors.exteriorGroundAmbientColor = config->exteriorColors.exteriorGroundAmbientColor;
            fdd->colors.exteriorHorizontAmbientColor = config->exteriorColors.exteriorHorizontAmbientColor;
            fdd->colors.exteriorDirectColor = config->exteriorColors.exteriorDirectColor;
            auto extDir = MathHelper::calcExteriorColorDir(
                frustumData.viewMat,
                m_api->getConfig()->currentTime
            );
            fdd->exteriorDirectColorDir = { extDir.x, extDir.y, extDir.z };
        }

        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->useMinimapWaterColor = config->useMinimapWaterColor;
            fdd->useCloseRiverColorForDB = config->useCloseRiverColorForDB;
        }
        if (config->waterColorParams == EParameterSource::eDatabase)
        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->liquidColors = liquidColors;
        } else if (config->waterColorParams == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->liquidColors.closeRiverColor_shallowAlpha = config->closeRiverColor;
            fdd->liquidColors.farRiverColor_deepAlpha = config->farRiverColor;
            fdd->liquidColors.closeOceanColor_shallowAlpha = config->closeOceanColor;
            fdd->liquidColors.farOceanColor_deepAlpha = config->farOceanColor;
        }
        if (config->skyParams == EParameterSource::eDatabase) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->skyColors = skyColors;
        }
    }

    //Handle fog
    {
        std::vector<LightResult> combinedResults = {};
        float totalSummator = 0.0;

        //Apply fog from WMO
//        {
//            for (auto &wmoFog : wmoFogData) {
//                auto &lightResult = combinedResults.emplace_back();
//                auto farPlaneClamped = std::min<float>(config->farPlane, wmoFog.end);
//
//                std::array<float, 3> colorConverted;
//                ImVectorToArrBGR(colorConverted, wmoFog.color);
//
//                lightResult.FogEnd = farPlaneClamped;
//                lightResult.FogStart = farPlaneClamped * wmoFog.start_scalar;
//                lightResult.SkyFogColor = colorConverted;
//                lightResult.FogDensity = 1.0;
//                lightResult.FogHeightColor = colorConverted;
//                lightResult.EndFogColor = colorConverted;
//                lightResult.SunFogColor = colorConverted;
//                lightResult.HeightEndFogColor = colorConverted;
//
//                if (farPlaneClamped < 30.f) {
//                    lightResult.FogEnd = farPlaneClamped;
//                    farPlaneClamped = 30.f;
//                }
//
//                bool mapHasWeightedBlendFlag = false;
//                if (!mapHasWeightedBlendFlag) {
//                    float difference = farPlaneClamped - lightResult.FogStart;
//                    float farPlaneClamped2 = std::min<float>(config->farPlane, 700.0f) - 200.0f;
//                    if ((difference > farPlaneClamped2) || (farPlaneClamped2 <= 0.0f)) {
//                        lightResult.FogDensity = 1.5;
//                    } else {
//                        lightResult.FogDensity = ((1.0 - (difference / farPlaneClamped2)) * 5.5) + 1.5;
//                    }
//                    lightResult.FogEnd = config->farPlane;
//                    if (lightResult.FogStart < 0.0f)
//                        lightResult.FogStart = 0.0;
//                }
//
//                lightResult.FogHeightDensity = lightResult.FogDensity;
//                lightResult.FogStartOffset = 0;
//                lightResult.FogHeightScaler = 1.0;
//                lightResult.FogZScalar = 0;
//                lightResult.FogHeight = -10000.0;
//                lightResult.LegacyFogScalar = 1.0;
//                lightResult.EndFogColorDistance = 10000.0;
//            }
//        }

        //In case of no data -> disable the fog
        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->FogDataFound = !combinedResults.empty();

            auto &fogResult = fdd->fogResults.emplace_back();
            fogResult = exteriorFogResult;

            fdd->FogDataFound = true;

        }
    }
}

template <int T>
inline float getFloatFromInt(int value) {
    if constexpr (T == 0) {
        return (value & 0xFF) / 255.0f;
    }
    if constexpr (T == 1) {
        return ((value >> 8) & 0xFF) / 255.0f;
    }
    if constexpr (T == 2) {
        return ((value >> 16) & 0xFF) / 255.0f;
    }

    return 0.0f;
}

inline mathfu::vec3 intToColor3(int a) {
    //BGR
    return mathfu::vec3(
        getFloatFromInt<2>(a),
        getFloatFromInt<1>(a),
        getFloatFromInt<0>(a)
    );
}
inline mathfu::vec4 intToColor4(int a) {
    //BGRA
    return mathfu::vec4(
        getFloatFromInt<2>(a),
        getFloatFromInt<1>(a),
        getFloatFromInt<0>(a),
        getFloatFromInt<3>(a)
    );
}
inline mathfu::vec4 floatArr(std::array<float, 4> a) {
    //BGRA
    return mathfu::vec4(
        a[3],
        a[2],
        a[1],
        a[0]
    );
}

template <int C, typename T>
decltype(auto) mixMembers(LightParamData& data, T LightTimedData::*member, float blendTimeCoeff) {
    if constexpr (C == 3) {
        static_assert(std::is_same<T, int>::value, "the type must be int for vector component");
        return mix(intToColor3(data.lightTimedData[0].*member), intToColor3(data.lightTimedData[1].*member), blendTimeCoeff);
    }
    if constexpr (C == 4 && std::is_same<T, std::array<float, 4>>::value) {
        return mix(floatArr(data.lightTimedData[0].*member), floatArr(data.lightTimedData[1].*member), blendTimeCoeff);
    } else if constexpr (C == 4) {
        static_assert(std::is_same<T, int>::value, "the type must be int for vector component");
        return mix(intToColor4(data.lightTimedData[0].*member), intToColor4(data.lightTimedData[1].*member), blendTimeCoeff);
    }
    if constexpr (C == 1) {
        static_assert(std::is_same<T, float>::value, "the type must be float for one component");
        return mix(data.lightTimedData[0].*member, data.lightTimedData[1].*member, blendTimeCoeff);
    }
}

bool vec3EqZero(const mathfu::vec3 &a) {
    return feq(a.x, 0.0f) && feq(a.y, 0.0f) && feq(a.z, 0.0f);
}

float maxFarClip(float farClip) {
     return std::max<float>(std::min<float>(farClip, 50000.0), 1000.0);
}

float DayNightLightHolder::getClampedFarClip(float farClip) {
    int someflag = 0;
    float multiplier = 1.0f;

    if ((m_mapHasFlag_0x10000) != 0 && farClip >= 4400.0f)
        farClip = 4400.0;

    return maxFarClip(fmaxf(fmaxf(fmaxf(m_minFogDist1, farClip), m_minFogDist3), m_minFogDist2));
}

void DayNightLightHolder::fixLightTimedData(LightTimedData &data, float farClip, float &fogScalarOverride) {
    if (data.EndFogColor == 0) {
        data.EndFogColor = data.SkyFogColor;
    }

    if (data.FogHeightColor == 0) {
        data.FogHeightColor = data.SkyFogColor;
    }

    if (data.EndFogHeightColor == 0) {
        data.EndFogHeightColor = data.EndFogColor;
    }

    //Clamp into (-1.0f, 1.0f)
    data.FogScaler = std::max<float>(std::min<float>(data.FogScaler, 1.0f), -1.0f);
    data.FogEnd = std::max<float>(data.FogEnd, 10.0f);
    data.FogHeight = std::max<float>(data.FogHeight, -10000.0f);
    data.FogHeightScaler = std::max<float>(std::min<float>(data.FogHeightScaler, 1.0f), -1.0f);

    if (data.SunFogColor == 0)
        data.SunFogAngle = 1.0f;

    if (data.EndFogColorDistance <= 0.0f)
        data.EndFogColorDistance = getClampedFarClip(farClip);

    if (data.FogHeight > 10000.0f)
        data.FogHeight = 0.0f;

    if (data.FogDensity <= 0.0f) {
        float farPlaneClamped = std::min<float>(farClip, 700.0f) - 200.0f;

        float difference = data.FogEnd - (float)(data.FogEnd * data.FogScaler);
        if (difference > farPlaneClamped || difference <= 0.0f) {
            data.FogDensity = 1.5f;
        } else {
            data.FogDensity = ((1.0f - (difference / farPlaneClamped)) * 5.5f) + 1.5f;
        }
    } else {
        fogScalarOverride = std::min(fogScalarOverride, -0.2f);
    }

    if ( data.FogHeightScaler == 0.0f )
        data.FogHeightDensity = data.FogDensity;
}

void DayNightLightHolder::getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config,
                                SkyColors &skyColors,
                                SkyBodyData &skyBodyData,
                                ExteriorColors &exteriorColors,
                                FogResult &fogResult,
                                LiquidColors &liquidColors,
                                StateForConditions *stateForConditions) {
    if (!m_api || !m_api->databaseHandler)
        return ;

    LightResult zoneLightResult;

    bool zoneLightFound = false;
    int LightId;
    for (const auto &zoneLight : m_zoneLights) {
        CAaBox laabb = zoneLight.aabb;
        auto const vec50 = mathfu::vec3(50.0f,50.0f,0);
        laabb.min = (mathfu::vec3(laabb.min) - vec50);
        laabb.max = (mathfu::vec3(laabb.max) + vec50);
        if (MathHelper::isPointInsideNonConvex(cameraVec3, zoneLight.aabb, zoneLight.points)) {
            zoneLightFound = true;

            if (stateForConditions != nullptr) {
                stateForConditions->currentZoneLights.push_back(zoneLight.ID);
            }
            LightId = zoneLight.LightID;
            break;
        }
    }

    uint8_t currentLightParamIdIndex = 0;

    int selectedLightParam = 0;
    int selectedLightId = 0;

    if (zoneLightFound) {
        selectedLightId = LightId;
        m_api->databaseHandler->getLightById(LightId, config->currentTime, zoneLightResult);
        if (stateForConditions != nullptr) {
            selectedLightParam = zoneLightResult.lightParamId[currentLightParamIdIndex];
            stateForConditions->currentZoneLights.push_back(zoneLightResult.lightParamId[currentLightParamIdIndex]);
        }
    }

    //Get light from DB
    std::vector<LightResult> lightResults;
    m_api->databaseHandler->getEnvInfo(m_mapId,
                                       cameraVec3.x,
                                       cameraVec3.y,
                                       cameraVec3.z,
                                       lightResults
    );
    std::sort(lightResults.begin(), lightResults.end(), [](const LightResult &a, const LightResult &b) -> bool {
        return a.blendAlpha > b.blendAlpha;
    });


    for (auto it = lightResults.begin(); it != lightResults.end(); it++) {
        if (feq(it->pos[0], 0.0) && feq(it->pos[1], 0.0) && feq(it->pos[2], 0.0)) {
            //This is default record. If zoneLight was selected -> skip it.
            if (!zoneLightFound) {
                selectedLightParam = it->lightParamId[currentLightParamIdIndex];
                selectedLightId = it->id;
            }
        } else {
            selectedLightParam = it->lightParamId[currentLightParamIdIndex];
            selectedLightId = it->id;
            break;
        }
    }

    if (stateForConditions != nullptr) {
        stateForConditions->currentZoneLights.push_back(selectedLightParam);
        stateForConditions->currentLightIds.push_back(selectedLightId);
    }

    LightParamData lightParamData;
    if (m_api->databaseHandler->getLightParamData(selectedLightParam, config->currentTime, lightParamData)) {

        float blendTimeCoeff = (config->currentTime - lightParamData.lightTimedData[0].time) / (float)(lightParamData.lightTimedData[1].time - lightParamData.lightTimedData[0].time);
        blendTimeCoeff = std::min<float>(std::max<float>(blendTimeCoeff, 0.0f), 1.0f);

        skyBodyData.celestialBodyOverride2 = mathfu::vec3(
            lightParamData.celestialBodyOverride2[0],
            lightParamData.celestialBodyOverride2[1],
            lightParamData.celestialBodyOverride2[2]
        );

        auto &dataA = lightParamData.lightTimedData[0];
        auto &dataB = lightParamData.lightTimedData[1];

        //Blend two times using certain rules
        float fogScalarOverride = 0.0f;
        fixLightTimedData(dataA, config->farPlane, fogScalarOverride);
        fixLightTimedData(dataB, config->farPlane, fogScalarOverride);

        //Ambient lights
        exteriorColors.exteriorAmbientColor =         mixMembers<4>(lightParamData, &LightTimedData::ambientLight, blendTimeCoeff);
        exteriorColors.exteriorGroundAmbientColor =   mixMembers<4>(lightParamData, &LightTimedData::groundAmbientColor, blendTimeCoeff);
        if (vec3EqZero(exteriorColors.exteriorGroundAmbientColor.xyz()))
            exteriorColors.exteriorGroundAmbientColor = exteriorColors.exteriorAmbientColor;

        exteriorColors.exteriorHorizontAmbientColor = mixMembers<4>(lightParamData, &LightTimedData::horizontAmbientColor, blendTimeCoeff);
        if (vec3EqZero(exteriorColors.exteriorHorizontAmbientColor.xyz()))
            exteriorColors.exteriorHorizontAmbientColor = exteriorColors.exteriorAmbientColor;

        exteriorColors.exteriorDirectColor =          mixMembers<4>(lightParamData, &LightTimedData::directColor, blendTimeCoeff);

        //Liquid colors
        liquidColors.closeOceanColor_shallowAlpha = mixMembers<4>(lightParamData, &LightTimedData::closeOceanColor, blendTimeCoeff);
        liquidColors.farOceanColor_deepAlpha =      mixMembers<4>(lightParamData, &LightTimedData::farOceanColor, blendTimeCoeff);
        liquidColors.closeRiverColor_shallowAlpha = mixMembers<4>(lightParamData, &LightTimedData::closeRiverColor, blendTimeCoeff);
        liquidColors.farRiverColor_deepAlpha =      mixMembers<4>(lightParamData, &LightTimedData::farRiverColor, blendTimeCoeff);

        liquidColors.closeOceanColor_shallowAlpha.w = lightParamData.oceanShallowAlpha;
        liquidColors.farOceanColor_deepAlpha.w =      lightParamData.oceanDeepAlpha;
        liquidColors.closeRiverColor_shallowAlpha.w = lightParamData.waterShallowAlpha;
        liquidColors.farRiverColor_deepAlpha.w =      lightParamData.waterDeepAlpha;

        //SkyColors
        skyColors.SkyTopColor =    mixMembers<4>(lightParamData, &LightTimedData::SkyTopColor, blendTimeCoeff);
        skyColors.SkyMiddleColor = mixMembers<4>(lightParamData, &LightTimedData::SkyMiddleColor, blendTimeCoeff);
        skyColors.SkyBand1Color =  mixMembers<4>(lightParamData, &LightTimedData::SkyBand1Color, blendTimeCoeff);
        skyColors.SkyBand2Color =  mixMembers<4>(lightParamData, &LightTimedData::SkyBand2Color, blendTimeCoeff);
        skyColors.SkySmogColor =   mixMembers<4>(lightParamData, &LightTimedData::SkySmogColor, blendTimeCoeff);
        skyColors.SkyFogColor =    mixMembers<4>(lightParamData, &LightTimedData::SkyFogColor, blendTimeCoeff);

        //Fog!
        fogResult.FogEnd =           mixMembers<1>(lightParamData, &LightTimedData::FogEnd, blendTimeCoeff);
        fogResult.FogScaler =        mixMembers<1>(lightParamData, &LightTimedData::FogScaler, blendTimeCoeff);
        fogResult.FogDensity =       mixMembers<1>(lightParamData, &LightTimedData::FogDensity, blendTimeCoeff);
        fogResult.FogHeight =        mixMembers<1>(lightParamData, &LightTimedData::FogHeight, blendTimeCoeff);
        fogResult.FogHeightScaler =  mixMembers<1>(lightParamData, &LightTimedData::FogHeightScaler, blendTimeCoeff);
        fogResult.FogHeightDensity = mixMembers<1>(lightParamData, &LightTimedData::FogHeightDensity, blendTimeCoeff);
        //Custom blend for Sun
        {
            float SunFogAngle1 = lightParamData.lightTimedData[0].SunFogAngle;
            float SunFogAngle2 = lightParamData.lightTimedData[1].SunFogAngle;
            if (SunFogAngle1 >= 1.0f) {
                if (SunFogAngle2 >= 1.0f) {
                    fogResult.SunAngleBlend = 0.0f;
                    fogResult.SunFogStrength = 0.0f;
                    fogResult.SunAngleBlend = 1.0f;
                } else if (SunFogAngle2 < 1.0f) {
                    fogResult.SunAngleBlend = blendTimeCoeff;
                    fogResult.SunFogAngle = SunFogAngle2;
                    fogResult.SunFogStrength = lightParamData.lightTimedData[1].SunFogStrength;
                }
            } else if (SunFogAngle1 < 1.0f) {
                if (SunFogAngle2 < 1.0f) {
                    fogResult.SunAngleBlend = 1.0f;
                    fogResult.SunFogAngle =      mixMembers<1>(lightParamData, &LightTimedData::SunFogAngle, blendTimeCoeff);
                    fogResult.SunFogStrength =   mixMembers<1>(lightParamData, &LightTimedData::SunFogStrength, blendTimeCoeff);
                } else if (SunFogAngle2 >= 1.0f) {
                    fogResult.SunFogStrength = lightParamData.lightTimedData[0].SunFogStrength;
                    fogResult.SunFogAngle = SunFogAngle1;
                    fogResult.SunAngleBlend = 1.0f - blendTimeCoeff;
                }
            }
        }

        if (false) {//fdd->overrideValuesWithFinalFog) {
            fogResult.FogColor = mixMembers<3>(lightParamData, &LightTimedData::EndFogColor, blendTimeCoeff);
        } else {
            fogResult.FogColor = mixMembers<3>(lightParamData, &LightTimedData::SkyFogColor, blendTimeCoeff);
        }

        fogResult.EndFogColor =           mixMembers<3>(lightParamData, &LightTimedData::EndFogColor, blendTimeCoeff);
        fogResult.EndFogColorDistance =   mixMembers<1>(lightParamData, &LightTimedData::EndFogColorDistance, blendTimeCoeff);
        fogResult.SunFogColor =           mixMembers<3>(lightParamData, &LightTimedData::SunFogColor, blendTimeCoeff);
//        fogResult.SunFogColor = mathfu::vec3(0,0,0);
        fogResult.FogHeightColor =        mixMembers<3>(lightParamData, &LightTimedData::FogHeightColor, blendTimeCoeff);
        fogResult.FogHeightCoefficients = mixMembers<4>(lightParamData, &LightTimedData::FogHeightCoefficients, blendTimeCoeff);
        fogResult.MainFogCoefficients =   mixMembers<4>(lightParamData, &LightTimedData::MainFogCoefficients, blendTimeCoeff);
        fogResult.HeightDensityFogCoefficients = mixMembers<4>(lightParamData, &LightTimedData::MainFogCoefficients, blendTimeCoeff);

        fogResult.FogZScalar =        mixMembers<1>(lightParamData, &LightTimedData::FogZScalar, blendTimeCoeff);
        fogResult.MainFogStartDist =  mixMembers<1>(lightParamData, &LightTimedData::MainFogStartDist, blendTimeCoeff);
        fogResult.MainFogEndDist =    mixMembers<1>(lightParamData, &LightTimedData::MainFogEndDist, blendTimeCoeff);
        fogResult.HeightEndFogColor = mixMembers<3>(lightParamData, &LightTimedData::EndFogHeightColor, blendTimeCoeff);
        fogResult.FogStartOffset =    mixMembers<1>(lightParamData, &LightTimedData::FogStartOffset, blendTimeCoeff);

        if (fogResult.FogHeightCoefficients.LengthSquared() <= 0.00000011920929f ){
            fogResult.FogHeightCoefficients = mathfu::vec4(0,0,1,0);
        }

        if (
            (fogResult.MainFogCoefficients.LengthSquared()) > 0.00000011920929f ||
            (fogResult.HeightDensityFogCoefficients.LengthSquared()) > 0.00000011920929f
            ) {
            fogResult.LegacyFogScalar = 0.0f;
        } else {
            fogResult.LegacyFogScalar = 1.0f;
        }

        fogResult.FogDensity = fmaxf(fogResult.FogDensity, 0.89999998f);
        if ( m_mapFlag2_0x2 )
        {
            fogResult.FogDensity = 1.0f;
        }
        else if ( fogScalarOverride > fogResult.FogScaler )
        {
            fogResult.FogScaler = fogScalarOverride;
        }
    }
}

void DayNightLightHolder::createMinFogDistances() {
    m_minFogDist1 = maxFarClip(0.0f);
    m_minFogDist2 = maxFarClip(0.0f);

    switch ( m_mapId )
    {
        case 2695:
            m_minFogDist1 = maxFarClip(7000.0);
            break;
        case 1492:
            m_minFogDist1 = maxFarClip(7000.0);
            break;
        case 1718:
            m_minFogDist1 = maxFarClip(7000.0);
            break;
        case 571:
            m_minFogDist1 = maxFarClip(30000.0);
            break;
    }

    if (m_mapHasFlag_0x200000) {
        m_minFogDist2 = maxFarClip(30000.0f);
    }
}
