//
// Created by Deamon on 7/5/2024.
//

#include "DayNightLightHolder.h"
#include "../../../../include/database/dbStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LightParamCalculate.h"


DayNightLightHolder::DayNightLightHolder(const HApiContainer &api, int mapId) : m_api(api), m_mapId(mapId) {

    MapRecord mapRecord;
    if (m_api && m_mapId > -1) {
        api->databaseHandler->getMapById(mapId, mapRecord);
        m_mapFlag2_0x2 = (mapRecord.flags2 & 0x2) > 0;
        m_useWeightedBlend = (mapRecord.flags0 & 0x4) > 0;
        m_mapHasFlag_0x200000 = (mapRecord.flags0 & 0x200000) > 0;
        m_mapHasFlag_0x10000 = (mapRecord.flags0 & 0x10000) > 0;
    }

    createMinFogDistances();
}

void DayNightLightHolder::loadZoneLights() {
    if (m_api && m_api->databaseHandler != nullptr && m_mapId > -1) {
        m_zoneLights = loadZoneLightRecs(m_api->databaseHandler, m_mapId);
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

static inline mathfu::vec4 mad(const mathfu::vec4 &a, const mathfu::vec4 &b, float alpha) {
    return a + b * alpha;
}
static inline mathfu::vec3 mad(const mathfu::vec3 &a, const mathfu::vec3 &b, float alpha) {
    return a + b * alpha;
}
static inline float mad(const float &a, const float &b, float alpha) {
    return a + b * alpha;
}

template <typename T, typename S>
void mixStructOffset(T& a, T& b, S T::*member, float blendTimeCoeff) {
    a.*member = mix(a.*member, b.*member, blendTimeCoeff);
}

template <typename T, typename S>
void madStructOffset(T& a, T& b, S T::*member, float blendTimeCoeff) {
    a.*member = mad(a.*member, b.*member, blendTimeCoeff);
}

template <typename T>
void mixStructure(T &a, T &b, float blendCoeff);

template <>
void mixStructure<FogResult>(FogResult& a, FogResult& b, float blendCoeff) {
    mixStructOffset(a, b, &FogResult::FogEnd                       , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogScaler                    , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogDensity                   , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogHeight                    , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogHeightScaler              , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogHeightDensity             , blendCoeff);
    mixStructOffset(a, b, &FogResult::SunFogAngle                  , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogColor                     , blendCoeff);
    mixStructOffset(a, b, &FogResult::EndFogColor                  , blendCoeff);
    mixStructOffset(a, b, &FogResult::EndFogColorDistance          , blendCoeff);
    mixStructOffset(a, b, &FogResult::SunFogColor                  , blendCoeff);
    mixStructOffset(a, b, &FogResult::SunFogStrength               , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogHeightColor               , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogHeightCoefficients        , blendCoeff);
    mixStructOffset(a, b, &FogResult::MainFogCoefficients          , blendCoeff);
    mixStructOffset(a, b, &FogResult::HeightDensityFogCoefficients , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogZScalar                   , blendCoeff);
    mixStructOffset(a, b, &FogResult::LegacyFogScalar              , blendCoeff);
    mixStructOffset(a, b, &FogResult::MainFogStartDist             , blendCoeff);
    mixStructOffset(a, b, &FogResult::MainFogEndDist               , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogBlendAlpha                , blendCoeff);
    mixStructOffset(a, b, &FogResult::HeightEndFogColor            , blendCoeff);
    mixStructOffset(a, b, &FogResult::FogStartOffset               , blendCoeff);
    mixStructOffset(a, b, &FogResult::SunAngleBlend                , blendCoeff);
}

template <>
void mixStructure<SkyColors>(SkyColors& a, SkyColors& b, float blendCoeff) {
    mixStructOffset(a, b, &SkyColors::SkyTopColor        , blendCoeff);
    mixStructOffset(a, b, &SkyColors::SkyMiddleColor     , blendCoeff);
    mixStructOffset(a, b, &SkyColors::SkyBand1Color      , blendCoeff);
    mixStructOffset(a, b, &SkyColors::SkyBand2Color      , blendCoeff);
    mixStructOffset(a, b, &SkyColors::SkySmogColor       , blendCoeff);
    mixStructOffset(a, b, &SkyColors::SkyFogColor        , blendCoeff);
}

template <>
void mixStructure<ExteriorColors>(ExteriorColors& a, ExteriorColors& b, float blendCoeff) {
    mixStructOffset(a, b, &ExteriorColors::exteriorAmbientColor          , blendCoeff);
    mixStructOffset(a, b, &ExteriorColors::exteriorHorizontAmbientColor  , blendCoeff);
    mixStructOffset(a, b, &ExteriorColors::exteriorGroundAmbientColor    , blendCoeff);
    mixStructOffset(a, b, &ExteriorColors::exteriorDirectColor           , blendCoeff);
    mixStructOffset(a, b, &ExteriorColors::exteriorSpecularColor           , blendCoeff);
}
template <>
void mixStructure<LiquidColors>(LiquidColors& a, LiquidColors& b, float blendCoeff) {
    mixStructOffset(a, b, &LiquidColors::closeRiverColor  , blendCoeff);
    mixStructOffset(a, b, &LiquidColors::farRiverColor    , blendCoeff);
    mixStructOffset(a, b, &LiquidColors::closeOceanColor  , blendCoeff);
    mixStructOffset(a, b, &LiquidColors::farOceanColor    , blendCoeff);

    mixStructOffset(a, b, &LiquidColors::riverShallowAlpha , blendCoeff);
    mixStructOffset(a, b, &LiquidColors::riverDeepAlpha    , blendCoeff);
    mixStructOffset(a, b, &LiquidColors::oceanShallowAlpha , blendCoeff);
    mixStructOffset(a, b, &LiquidColors::oceanDeepAlpha    , blendCoeff);
}
template <>
void mixStructure<SkyBodyData>(SkyBodyData& a, SkyBodyData& b, float blendCoeff) {
    mixStructOffset(a, b, &SkyBodyData::celestialBodyOverride  , blendCoeff);
    mixStructOffset(a, b, &SkyBodyData::sunPlanetHideBlend     , blendCoeff);
    mixStructOffset(a, b, &SkyBodyData::moonPlanetHideBlend    , blendCoeff);
    mixStructOffset(a, b, &SkyBodyData::starsHideBlend         , blendCoeff);

    if (b.hasSunPositionOverride) {
        if (a.hasSunPositionOverride) {
            mixStructOffset(a, b, &SkyBodyData::sunPositionOverride , blendCoeff);
            mixStructOffset(a, b, &SkyBodyData::sunAttenuationStart , blendCoeff);
            mixStructOffset(a, b, &SkyBodyData::sunAttenuationEnd , blendCoeff);
            mixStructOffset(a, b, &SkyBodyData::sunPositionBlend , blendCoeff);
        } else {
            mixStructOffset(a, b, &SkyBodyData::sunPositionOverride, 1.0f);
            mixStructOffset(a, b, &SkyBodyData::sunAttenuationStart, 1.0f);
            mixStructOffset(a, b, &SkyBodyData::sunAttenuationEnd, 1.0f);
            mixStructOffset(a, b, &SkyBodyData::sunPositionBlend, 1.0f);
        }
        a.hasSunPositionOverride = true;
    }

    if (b.hasSunDirectionOverride) {
        if (a.hasSunDirectionOverride) {
            mixStructOffset(a, b, &SkyBodyData::sunDirPolar , blendCoeff);
            mixStructOffset(a, b, &SkyBodyData::sunDirAzimuth , blendCoeff);
            mixStructOffset(a, b, &SkyBodyData::sunDirectionBlend , blendCoeff);
        } else {
            mixStructOffset(a, b, &SkyBodyData::sunDirPolar , 1.0f);
            mixStructOffset(a, b, &SkyBodyData::sunDirAzimuth , 1.0f);
            mixStructOffset(a, b, &SkyBodyData::sunDirectionBlend , 1.0f);
        }
        a.hasSunDirectionOverride = true;
    }
}

template <typename T>
//Multiply and Add
void madStructure(T &a, T &b, float blendCoeff);


template <>
void madStructure<FogResult>(FogResult& a, FogResult& b, float blendCoeff) {
    madStructOffset(a, b, &FogResult::FogEnd                       , blendCoeff);
    madStructOffset(a, b, &FogResult::FogScaler                    , blendCoeff);
    madStructOffset(a, b, &FogResult::FogDensity                   , blendCoeff);
    madStructOffset(a, b, &FogResult::FogHeight                    , blendCoeff);
    madStructOffset(a, b, &FogResult::FogHeightScaler              , blendCoeff);
    madStructOffset(a, b, &FogResult::FogHeightDensity             , blendCoeff);
    madStructOffset(a, b, &FogResult::SunFogAngle                  , blendCoeff);
    madStructOffset(a, b, &FogResult::FogColor                     , blendCoeff);
    madStructOffset(a, b, &FogResult::EndFogColor                  , blendCoeff);
    madStructOffset(a, b, &FogResult::EndFogColorDistance          , blendCoeff);
    madStructOffset(a, b, &FogResult::SunFogColor                  , blendCoeff);
    madStructOffset(a, b, &FogResult::SunFogStrength               , blendCoeff);
    madStructOffset(a, b, &FogResult::FogHeightColor               , blendCoeff);
    madStructOffset(a, b, &FogResult::FogHeightCoefficients        , blendCoeff);
    madStructOffset(a, b, &FogResult::MainFogCoefficients          , blendCoeff);
    madStructOffset(a, b, &FogResult::HeightDensityFogCoefficients , blendCoeff);
    madStructOffset(a, b, &FogResult::FogZScalar                   , blendCoeff);
    madStructOffset(a, b, &FogResult::LegacyFogScalar              , blendCoeff);
    madStructOffset(a, b, &FogResult::MainFogStartDist             , blendCoeff);
    madStructOffset(a, b, &FogResult::MainFogEndDist               , blendCoeff);
    madStructOffset(a, b, &FogResult::FogBlendAlpha                , blendCoeff);
    madStructOffset(a, b, &FogResult::HeightEndFogColor            , blendCoeff);
    madStructOffset(a, b, &FogResult::FogStartOffset               , blendCoeff);
    madStructOffset(a, b, &FogResult::SunAngleBlend                , blendCoeff);
}

template <>
void madStructure<SkyColors>(SkyColors& a, SkyColors& b, float blendCoeff) {
    madStructOffset(a, b, &SkyColors::SkyTopColor        , blendCoeff);
    madStructOffset(a, b, &SkyColors::SkyMiddleColor     , blendCoeff);
    madStructOffset(a, b, &SkyColors::SkyBand1Color      , blendCoeff);
    madStructOffset(a, b, &SkyColors::SkyBand2Color      , blendCoeff);
    madStructOffset(a, b, &SkyColors::SkySmogColor       , blendCoeff);
    madStructOffset(a, b, &SkyColors::SkyFogColor        , blendCoeff);
}

template <>
void madStructure<ExteriorColors>(ExteriorColors& a, ExteriorColors& b, float blendCoeff) {
    madStructOffset(a, b, &ExteriorColors::exteriorAmbientColor          , blendCoeff);
    madStructOffset(a, b, &ExteriorColors::exteriorHorizontAmbientColor  , blendCoeff);
    madStructOffset(a, b, &ExteriorColors::exteriorGroundAmbientColor    , blendCoeff);
    madStructOffset(a, b, &ExteriorColors::exteriorDirectColor           , blendCoeff);
    madStructOffset(a, b, &ExteriorColors::exteriorSpecularColor         , blendCoeff);
}
template <>
void madStructure<LiquidColors>(LiquidColors& a, LiquidColors& b, float blendCoeff) {
    madStructOffset(a, b, &LiquidColors::closeRiverColor  , blendCoeff);
    madStructOffset(a, b, &LiquidColors::farRiverColor    , blendCoeff);
    madStructOffset(a, b, &LiquidColors::closeOceanColor  , blendCoeff);
    madStructOffset(a, b, &LiquidColors::farOceanColor    , blendCoeff);

    madStructOffset(a, b, &LiquidColors::riverShallowAlpha , blendCoeff);
    madStructOffset(a, b, &LiquidColors::riverDeepAlpha    , blendCoeff);
    madStructOffset(a, b, &LiquidColors::oceanShallowAlpha , blendCoeff);
    madStructOffset(a, b, &LiquidColors::oceanDeepAlpha    , blendCoeff);
}
// template <>
// void madStructure<SkyBodyData>(SkyBodyData& a, SkyBodyData& b, float blendCoeff) {
//     madStructOffset(a, b, &SkyBodyData::celestialBodyOverride  , blendCoeff);
//     madStructOffset(a, b, &SkyBodyData::celestialBodyOverride2 , blendCoeff);
// }


std::array<mathfu::vec3, 4> DayNightLightHolder::calcPlanetPositions(const mathfu::vec3 &cameraVec3) {
    std::array<mathfu::vec3, 4> planetPositions;

    const auto time = m_api->getConfig()->currentTime;
    {
        float posPhi = 0, posTheta = 0;
        MathHelper::calcSunPlanetPos(time, posPhi, posTheta);
        planetPositions[0] = MathHelper::polarToCartesian(posPhi, posTheta) + cameraVec3;
    }
    {
        float posPhi = 0, posTheta = 0;
        MathHelper::calcMoon1PlanetPos(time, posPhi, posTheta);
        planetPositions[1] = MathHelper::polarToCartesian(posPhi, posTheta) + cameraVec3;
    }
    {
        float posPhi = 0, posTheta = 0;
        MathHelper::calcMoon2PlanetPos(time, posPhi, posTheta);
        planetPositions[2] = MathHelper::polarToCartesian(posPhi, posTheta) + cameraVec3;
    }
    planetPositions[3] = planetPositions[1];

    return planetPositions;
}

mathfu::vec3 DayNightLightHolder::calcSunPosition(
    const SkyBodyData &skyBodyData,
    const mathfu::vec3 &cameraVec3,
    const mathfu::mat4 &viewMat,
    float &sunAttenuationStart,
    float &sunAttenuationEnd,
    std::array<mathfu::vec3, 4> &planetPositions
) {
    float dayNightProgress = m_api->getConfig()->currentTime / 2880.0f;
    bool isDaytime = (dayNightProgress >= 0.25f && dayNightProgress <= 0.89583331f);

    auto shinyPlanetPos = isDaytime ? planetPositions[0] : planetPositions[1]; //Sun or Moon

    auto sunDir = shinyPlanetPos - cameraVec3;

    sunAttenuationStart = 0.f;
    sunAttenuationEnd = 0.f;

    if (skyBodyData.hasSunPositionOverride) {
        auto overrideSunDir = skyBodyData.sunPositionOverride - cameraVec3;
        auto overrideSunDist = overrideSunDir.Length();

        auto scaledSunDir = sunDir.Normalized() * overrideSunDist;

        sunDir = mix(scaledSunDir, overrideSunDir, skyBodyData.sunPositionBlend);

        //Set back the sun position
        planetPositions[0] = cameraVec3 + sunDir.Normalized() * 12.0f;

        //Blend the sun attenuation
        float horizontalDistToSun = overrideSunDir.xy().Length();
        float horizonStart = m_api->getConfig()->farPlane; // Viewer doesn't use Horizon as of now
        float usualAtten = horizonStart + horizontalDistToSun;

        sunAttenuationStart = mix(usualAtten, skyBodyData.sunAttenuationStart, skyBodyData.sunPositionBlend) ;
        sunAttenuationEnd = mix(usualAtten, skyBodyData.sunAttenuationEnd, skyBodyData.sunPositionBlend);
    }

    auto viewSpaceSunPos = viewMat * mathfu::vec4(cameraVec3 + sunDir, 1.0f);
    return viewSpaceSunPos.xyz();
}

mathfu::vec3 DayNightLightHolder::calcDirectColorDir(const SkyBodyData &skyBodyData, const mathfu::mat4 &invTranspViewMat) {
    float sunPosPhi = 0, sunPosTheta = 0;
    MathHelper::calcExteriorDirectColorDir(m_api->getConfig()->currentTime, sunPosPhi, sunPosTheta);

    if (skyBodyData.hasSunDirectionOverride) {
        float blend = skyBodyData.sunDirectionBlend;
        float invBlend = 1.0f - skyBodyData.sunDirectionBlend;

        sunPosPhi   = skyBodyData.sunDirPolar   * blend + sunPosPhi   * invBlend;
        sunPosTheta = skyBodyData.sunDirAzimuth * blend + sunPosTheta * invBlend;
    }

    mathfu::vec4 sunDir = mathfu::vec4(MathHelper::polarToCartesian(sunPosPhi, sunPosTheta), 0.0f);
    auto exteriorDirectColorDir = (invTranspViewMat * sunDir).xyz().Normalized();

    return exteriorDirectColorDir;
}

mathfu::vec3 DayNightLightHolder::calcSunDirForFog(
    const SkyBodyData &skyBody,
    const mathfu::vec3 &sunPositionInView,
    const mathfu::vec3 &cameraVec3,
    const mathfu::mat4 &invTranspViewMat,
    std::array<mathfu::vec3, 4> &planetPositions
) {

    mathfu::vec3 normalizedSunDir;
    if (skyBody.hasSunDirectionOverride) {
        normalizedSunDir = sunPositionInView.Normalized();
    } else {
        normalizedSunDir = (
            invTranspViewMat *
            mathfu::vec4((planetPositions[0] - cameraVec3).Normalized(), 0.0f)
        ).xyz();
    }

    return normalizedSunDir;
}

void DayNightLightHolder::updatePlanetsAndStars(const mathfu::vec3 &cameraPos, const SkyBodyData &skyBodyData,
                                                const HFrameDependantData &fdd) {
    if (!m_api || !fdd) return;
    Config* config = m_api->getConfig();
    const int time = config->currentTime;

    struct PlanetDef {
        void (*posFn)(int, float&, float&);
        float loadScale;
    };
    static const PlanetDef planetDefs[3] = {
        { &MathHelper::calcSunPlanetPos,   1.0f }, // sun   (fdid 186220)
        { &MathHelper::calcMoon1PlanetPos, 2.2f }, // moon1 (fdid 4629581)
        { &MathHelper::calcMoon2PlanetPos, 1.2f }, // moon2 (fdid 4629582)
    };

    // The client hides the planet discs when a custom sun position is active (planet alpha = 0)
    float planetAlpha = skyBodyData.hasSunPositionOverride ? 0.0f : 1.0f;

    for (int i = 0; i < 3; i++) {
        auto &out = fdd->planets[i];

        float posPhi = 0.0f, posTheta = 0.0f;
        planetDefs[i].posFn(time, posPhi, posTheta);

        // Planet discs sit on the sky sphere at radius 12 around the camera
        out.worldPos = cameraPos + MathHelper::polarToCartesian(posPhi, posTheta).Normalized() * 12.0f;

        out.scale = (i == 0 ? MathHelper::calcSunPlanetScale(time) : MathHelper::calcMoonPlanetScale(time))
                    * planetDefs[i].loadScale;
        out.color = fdd->colors.exteriorSpecularColor;
        out.alpha = planetAlpha;

        float hideBlend = (i == 0) ? skyBodyData.sunPlanetHideBlend : skyBodyData.moonPlanetHideBlend;
        out.visible = hideBlend < 0.5f && out.alpha > 0.0f;
    }

    // Stars: own brightness curve (0 at day, 1 at night) scaled by (1 - starsHideBlend)
    float starByte = MathHelper::calcStarsBrightness(time) * 254.0f + 1.0f;
    fdd->stars.alpha = (starByte / 255.0f) * (1.0f - skyBodyData.starsHideBlend);
    fdd->stars.enabled = starByte >= 2.0f && fdd->stars.alpha > 0.0f;
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

    WmoObject::WmoFogBlendResult wmoFogBlendResult;
    if (mapRenderPlan->m_currentWMO != emptyWMO) {
        auto l_currentWmoObject = wmoFactory->getObjectById<0>(mapRenderPlan->m_currentWMO);
        if (l_currentWmoObject != nullptr) {
            l_currentWmoObject->checkFog(frustumData.cameraPos, mapRenderPlan->m_currentWmoGroup, wmoFogBlendResult);
        }
    }

    FogResult exteriorFogResult;
    FogResult exteriorUnderWaterFogResult;

    auto fdd = mapRenderPlan->frameDependentData;
    if ((m_api->databaseHandler != nullptr)) {
        //Check zoneLight
        SkyColors skyColors;
        ExteriorColors exteriorColors;
        float currentGlow = 0.0f;
        mathfu::vec3 exteriorDirectColorDir = mathfu::vec3(0.0, 0.0, 0.0);
        mathfu::vec3 sunDirForFog = mathfu::vec3(0.0, 0.0, 0.0);
        mathfu::vec3 sunPosViewSpace = mathfu::vec3(0.0, 0.0, 0.0);

        LiquidColors liquidColors;
        SkyBodyData skyBodyData;

        SkyBoxCollector skyBoxCollector(m_api, m_exteriorSkyBoxes);

        getLightResultsFromDB(frustumData.cameraPos, config,
                              currentGlow,
                              skyColors, skyBodyData, exteriorColors,
                              exteriorFogResult, exteriorUnderWaterFogResult, liquidColors, skyBoxCollector, &stateForConditions);

        m_exteriorSkyBoxes = skyBoxCollector.getNewSkyBoxes();
        mapRenderPlan->frameDependentData->overrideValuesWithFinalFog = skyBoxCollector.getOverrideValuesWithFinalFog();


        {
            auto invTranspViewMat = frustumData.viewMat.Inverse().Transpose();

            auto planets = calcPlanetPositions(frustumData.cameraPos);

            sunPosViewSpace = calcSunPosition(
                skyBodyData,
                frustumData.cameraPos, frustumData.viewMat,
                fdd->sunAttentuationStart, fdd->sunAttentuationEnd,
                planets
            );
            exteriorDirectColorDir = calcDirectColorDir(skyBodyData, invTranspViewMat);
            sunDirForFog = calcSunDirForFog(
                skyBodyData,
                sunPosViewSpace,
                frustumData.cameraPos,
                invTranspViewMat, planets
            );

            fdd->useSunAttenuation = skyBodyData.hasSunPositionOverride;
        }

        float ambientMult = areaRecord.ambientMultiplier * 2.0f + 1;

        if (config->glowSource == EParameterSource::eDatabase) {
            fdd->currentGlow = currentGlow;
        } else if (config->glowSource == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->currentGlow = config->currentGlow; //copy from config to FDD
        }
        fdd->sunPos = sunPosViewSpace;
        fdd->sunDirection = sunDirForFog;

        if (config->globalLighting == EParameterSource::eDatabase) {
                        fdd->colors = exteriorColors;

            fdd->exteriorDirectColorDir = exteriorDirectColorDir;
        } else if (config->globalLighting == EParameterSource::eConfig) {
            fdd->colors = config->exteriorColors;
            fdd->exteriorDirectColorDir = exteriorDirectColorDir;
        }

        {
            fdd->useMinimapWaterColor = config->useMinimapWaterColor;
            fdd->useCloseRiverColorForDB = config->useCloseRiverColorForDB;
        }
        if (config->waterColorParams == EParameterSource::eDatabase)
        {
            fdd->liquidColors = liquidColors;
        } else if (config->waterColorParams == EParameterSource::eConfig) {
            fdd->liquidColors = config->liquidColors;
        }
        if (config->skyParams == EParameterSource::eDatabase) {
            fdd->skyColors = skyColors;
        } else if (config->skyParams == EParameterSource::eConfig) {
            fdd->skyColors = config->skyColors;
        }

        updatePlanetsAndStars(frustumData.cameraPos, skyBodyData, fdd);
    }

    //Mix the WMO fog into the DB fogs
    //The blend weight is the distance from the camera to the nearest portal of the current
    //interior WMO group: full DB fog at the portal, full WMO fog 25+ units into the interior.
    //Note: the client additionally gates this on the liquid type at the camera (WMO fog is
    //suppressed for most liquids); that refinement is not implemented here.
    if (wmoFogBlendResult.fogFound && wmoFogBlendResult.insideInterior &&
        m_api->databaseHandler != nullptr && config->globalFog == EParameterSource::eDatabase) {

        float wmoBlend = std::min<float>(std::max<float>(wmoFogBlendResult.distToExit * 0.04f, 0.0f), 1.0f);

        FogResult wmoExteriorFog = wmoFogDataToFogResult(wmoFogBlendResult.fog.fog, config->farPlane);
        blendWmoFogIntoFogResult(exteriorFogResult, wmoExteriorFog, wmoBlend, false);

        FogResult wmoUnderwaterFog = wmoFogDataToFogResult(wmoFogBlendResult.fog.underwater_fog, config->farPlane);
        blendWmoFogIntoFogResult(exteriorUnderWaterFogResult, wmoUnderwaterFog, wmoBlend, true);
    }

    //Handle fog
    {
        std::vector<LightResult> combinedResults = {};
        float totalSummator = 0.0;

        //In case of no data -> disable the fog
        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->FogDataFound = !combinedResults.empty();

            auto &fogResult = fdd->fogResults.emplace_back();
            if (config->globalFog == EParameterSource::eDatabase) {
                fogResult = exteriorFogResult;
                fdd->underWaterFogResult = exteriorUnderWaterFogResult;
            } else if (config->globalFog == EParameterSource::eConfig){
                fogResult = config->fogResult;
                fdd->underWaterFogResult = FogResult();
            }

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

// Density heuristic shared with fixLightTimedData
static float calcFogDensityFromStartEnd(float fogEnd, float fogStart, float farClip) {
    float farPlaneClamped = std::min<float>(farClip, 700.0f) - 200.0f;

    float difference = fogEnd - fogStart;
    if (difference > farPlaneClamped || farPlaneClamped <= 0.0f) {
        return 1.5f;
    }
    return ((1.0f - (difference / farPlaneClamped)) * 5.5f) + 1.5f;
}

// Converts a blended WMO fog record into the FogResult form
// WMO fogs carry only end/start_scalar/color; the rest of the fields are filled with
// the neutral values the client uses so that blendFogs keeps the DB fog's height-fog
// and main-fog behaviour.
FogResult DayNightLightHolder::wmoFogDataToFogResult(const SMOFog_Data &wmoFogData, float farClip) const {
    FogResult result;

    float fogEnd = std::min<float>(farClip, wmoFogData.end);
    fogEnd = std::max<float>(fogEnd, 30.0f);
    float fogStart = fogEnd * wmoFogData.start_scalar;

    int colorInt = 0;
    static_assert(sizeof(wmoFogData.color) == sizeof(colorInt), "CImVector must be 4 bytes");
    memcpy(&colorInt, &wmoFogData.color, sizeof(colorInt));
    mathfu::vec3 fogColor = intToColor3(colorInt);

    // The client assigns the WMO fog color to every color slot
    result.FogColor = fogColor;
    result.EndFogColor = fogColor;
    result.SunFogColor = fogColor;
    result.FogHeightColor = fogColor;
    result.HeightEndFogColor = fogColor;

    // Camera-not-in-liquid path of the client converter: density is derived from
    // end/start, fog end becomes the far clip and fog start is clamped to >= 0
    result.FogDensity = calcFogDensityFromStartEnd(fogEnd, fogStart, farClip);
    result.FogEnd = farClip;
    fogStart = std::max<float>(fogStart, 0.0f);
    // MapSceneRenderer derives the fog start as min(farClip, 3000) * FogScaler
    result.FogScaler = fogStart / std::min<float>(farClip, 3000.0f);

    result.FogHeightDensity = result.FogDensity;

    // Neutral values the client converter writes (they make blendFogs keep the DB fog's
    // height fog / main fog parameters)
    result.FogHeight = -10000.0f;
    result.FogHeightScaler = 1.0f;
    result.FogZScalar = 0.0f;
    result.FogStartOffset = 0.0f;

    result.LegacyFogScalar = 1.0f;
    result.EndFogColorDistance = 10000.0f;

    // sunPercentage is not produced by the WMO fog converter
    result.SunFogStrength = 0.0f;

    return result;
}

// Mixes a WMO fog into a DB fog
// fogResult = lerp(dbFog, wmoFog, wmoBlend) with per-field rules.
void DayNightLightHolder::blendWmoFogIntoFogResult(FogResult &fogResult, const FogResult &wmoFog, float wmoBlend, bool underwater) {
    // Colors (the client blends them per-byte; float lerp is equivalent within rounding)
    fogResult.FogColor = mix(fogResult.FogColor, wmoFog.FogColor, wmoBlend);
    fogResult.EndFogColor = mix(fogResult.EndFogColor, wmoFog.EndFogColor, wmoBlend);
    fogResult.SunFogColor = mix(fogResult.SunFogColor, wmoFog.SunFogColor, wmoBlend);
    fogResult.FogHeightColor = mix(fogResult.FogHeightColor, wmoFog.FogHeightColor, wmoBlend);

    // Fog start/end and density
    fogResult.FogEnd = mix(fogResult.FogEnd, wmoFog.FogEnd, wmoBlend);
    fogResult.FogScaler = mix(fogResult.FogScaler, wmoFog.FogScaler, wmoBlend);
    fogResult.FogDensity = mix(fogResult.FogDensity, wmoFog.FogDensity, wmoBlend);

    // The WMO fog carries no sun fog angle of its own (stays 0 in the converter), so the
    // DB value blends toward 0: sun fog fades out the deeper the camera goes into the WMO
    fogResult.SunFogAngle = mix(fogResult.SunFogAngle, 0.0f, wmoBlend);

    if (underwater || wmoFog.FogHeight >= -9999.0f) {
        // Underwater fogs (or WMO fogs defining their own height plane) blend the
        // height fog too; the height plane distance uses a cubic blend factor
        float cubicBlend = wmoBlend * wmoBlend * wmoBlend;
        fogResult.FogHeight = mix(fogResult.FogHeight, wmoFog.FogHeight, cubicBlend);
        fogResult.FogHeightScaler = mix(fogResult.FogHeightScaler, wmoFog.FogHeightScaler, wmoBlend);
        fogResult.FogHeightDensity = mix(fogResult.FogHeightDensity, wmoFog.FogHeightDensity, wmoBlend);
        fogResult.FogZScalar = mix(fogResult.FogZScalar, wmoFog.FogZScalar, wmoBlend);
        fogResult.FogHeightCoefficients = mix(fogResult.FogHeightCoefficients, wmoFog.FogHeightCoefficients, wmoBlend);
    } else {
        // WMO fog has no height fog of its own: the DB height plane/rate/coefficients are kept,
        // height density blends toward the WMO density and FogZScalar fades out
        fogResult.FogHeightDensity = mix(fogResult.FogHeightDensity, wmoFog.FogDensity, wmoBlend);
        fogResult.FogZScalar = fogResult.FogZScalar * (1.0f - wmoBlend);
    }

    // FogStartOffset, MainFogStartDist, MainFogEndDist, MainFogCoefficients and
    // HeightDensityFogCoefficients are always kept from the DB fog (blendFogs copies fog1)

    fogResult.EndFogColorDistance = mix(fogResult.EndFogColorDistance, wmoFog.EndFogColorDistance, wmoBlend);
    fogResult.LegacyFogScalar = mix(fogResult.LegacyFogScalar, 1.0f, wmoBlend);

    // sunPercentage: the WMO fog contributes none, so it fades out with the blend
    fogResult.SunFogStrength = fogResult.SunFogStrength * (1.0f - wmoBlend);
}

// Day-progress curve for the sun fog strength
// 0 at night, ramps up 06:30->07:00, 1.0 during the day, ramps down 18:00->18:30.
static float sunFogStrengthDayCurve(float dayProgress) {
    static const float rampUpStart   = 0.2708333f; // 06:30
    static const float rampUpEnd     = 0.2916667f; // 07:00
    static const float rampDownStart = 0.75f;      // 18:00
    static const float rampDownEnd   = 0.7708333f; // 18:30

    if (dayProgress <= rampUpStart || dayProgress >= rampDownEnd)
        return 0.0f;
    if (dayProgress < rampUpEnd)
        return (dayProgress - rampUpStart) / (rampUpEnd - rampUpStart);
    if (dayProgress <= rampDownStart)
        return 1.0f;
    return 1.0f - (dayProgress - rampDownStart) / (rampDownEnd - rampDownStart);
}

void DayNightLightHolder::getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config,
                                float &glow,
                                SkyColors &skyColors,
                                SkyBodyData &skyBodyData,
                                ExteriorColors &exteriorColors,
                                FogResult &fogResult,
                                FogResult &underWaterFogResult,
                                LiquidColors &liquidColors,
                                SkyBoxCollector &skyBoxCollector,
                                StateForConditions *stateForConditions) {
    if (!m_api || !m_api->databaseHandler || m_mapId == -1)
        return ;

    auto blendResults = calculateLightParamBlends(
        m_api->databaseHandler,
        m_mapId,
        cameraVec3,
        stateForConditions,
        m_zoneLights
    );
    auto &paramsBlend = blendResults.params;


    if (!paramsBlend.empty()) {
        // assign zero values
        skyColors.assignZeros();
        exteriorColors.assignZeros();
        liquidColors.assignZeros();
        skyBodyData.assignZeros();
        //fogResult.assignZeros();
    }

    for (auto it = paramsBlend.begin(); it != paramsBlend.end(); it++) {
        SkyColors tmp_skyColors;
        ExteriorColors tmp_exteriorColors;

        LiquidColors tmp_liquidColors;
        SkyBodyData tmp_skyBodyData;
        FogResult tmp_fogResult;

        float tmp_glow = 0.0;

        calcLightParamResult(it->id, config,
                             tmp_glow,
                             tmp_skyBodyData, tmp_exteriorColors, tmp_fogResult, tmp_liquidColors, tmp_skyColors);

        mixStructure(skyColors,      tmp_skyColors,      it->blend);
        mixStructure(exteriorColors, tmp_exteriorColors, it->blend);
        mixStructure(liquidColors,   tmp_liquidColors,   it->blend);
        mixStructure(skyBodyData,    tmp_skyBodyData,    it->blend);
        mixStructure(fogResult,      tmp_fogResult,      it->blend);
        glow =   mix(glow,           tmp_glow,           it->blend);

        if (tmp_skyBodyData.skyBoxInfo.id > 0 && (stateForConditions != nullptr)) {
            skyBoxCollector.addSkyBox(*stateForConditions, tmp_skyBodyData.skyBoxInfo, it->blend);
        }
    }

    // Underwater fog (Light param slot 1) — from the same blend computation, no re-query
    for (auto it = blendResults.underwaterParams.begin(); it != blendResults.underwaterParams.end(); it++) {
        SkyColors tmp_skyColors;
        ExteriorColors tmp_exteriorColors;
        LiquidColors tmp_liquidColors;
        SkyBodyData tmp_skyBodyData;
        FogResult tmp_underWaterFogResult;

        float tmp_glow = 0.0;

        calcLightParamResult(it->id, config,
                             tmp_glow,
                             tmp_skyBodyData, tmp_exteriorColors, tmp_underWaterFogResult, tmp_liquidColors, tmp_skyColors);

        mixStructure(underWaterFogResult, tmp_underWaterFogResult, it->blend);
    }


    float blendCoeff = fmaxf(fminf(getClampedFarClip(config->farPlane) / fogResult.EndFogColorDistance, 1.0f), 0.0f);
    skyColors.SkyFogColor = mix(skyColors.SkyFogColor, fogResult.EndFogColor, blendCoeff);

    // The client scales the sun fog blend factor by a day-progress curve (SetPlanets),
    // which is what actually turns the sun halo off at night
    fogResult.SunAngleBlend *= sunFogStrengthDayCurve(config->currentTime / 2880.0f);

    stateForConditions->currentLightParams = paramsBlend;
}

void DayNightLightHolder::calcLightParamResult(int lightParamId, const Config *config,
                                               float &glow,
                                               SkyBodyData &skyBodyData,
                                               ExteriorColors &exteriorColors, FogResult &fogResult,
                                               LiquidColors &liquidColors,
                                               SkyColors &skyColors) {
    LightParamData lightParamData;
    if (m_api->databaseHandler->getLightParamData(lightParamId, config->currentTime, lightParamData)) {

        bool resultTimeIsTheSame = lightParamData.lightTimedData[1].time == lightParamData.lightTimedData[0].time;
        float blendTimeCoeff = 0.0f;
        if (!resultTimeIsTheSame)
            blendTimeCoeff =
                (float)(config->currentTime - lightParamData.lightTimedData[0].time) /
                (float)(lightParamData.lightTimedData[1].time - lightParamData.lightTimedData[0].time);

        blendTimeCoeff = std::min<float>(std::max<float>(blendTimeCoeff, 0.0f), 1.0f);

        skyBodyData.skyBoxInfo = lightParamData.skyboxInfo;

        skyBodyData.hasSunPositionOverride = lightParamData.lightParamFlags & 0x100;
        skyBodyData.hasSunDirectionOverride = lightParamData.lightParamFlags & 0x200;

        // LightParams flags as blendable floats
        skyBodyData.sunPlanetHideBlend = (lightParamData.lightParamFlags & 0x4) ? 1.0f : 0.0f;
        skyBodyData.moonPlanetHideBlend = (lightParamData.lightParamFlags & 0x8) ? 1.0f : 0.0f;
        skyBodyData.starsHideBlend = (lightParamData.lightParamFlags & 0x10) ? 1.0f : 0.0f;

        if (skyBodyData.hasSunPositionOverride) {
            skyBodyData.sunPositionOverride = mathfu::vec3(
                lightParamData.celestialBodyOverride2[0],
                lightParamData.celestialBodyOverride2[1],
                lightParamData.celestialBodyOverride2[2]
            );
            skyBodyData.sunAttenuationStart = lightParamData.sunAttenuationStart;
            skyBodyData.sunAttenuationEnd = lightParamData.sunAttenuationEnd;

            skyBodyData.sunPositionBlend = 1.0f;
        }

        if (skyBodyData.hasSunDirectionOverride) {
            skyBodyData.sunDirAzimuth = lightParamData.sunAzimuth * ( M_PI / 180.0f);
            skyBodyData.sunDirPolar = lightParamData.sunPolar * ( M_PI / 180.0f);
            skyBodyData.sunDirectionBlend = 1.0f;
        }


        glow = lightParamData.glow;

        auto &dataA = lightParamData.lightTimedData[0];
        auto &dataB = lightParamData.lightTimedData[1];

        //Blend two times using certain rules
        float fogScalarOverride = 0.0f;
        fixLightTimedData(dataA, config->farPlane, fogScalarOverride);
        fixLightTimedData(dataB, config->farPlane, fogScalarOverride);


        //Ambient lights
        exteriorColors.exteriorAmbientColor =         mixMembers<3>(lightParamData, &LightTimedData::ambientLight, blendTimeCoeff);
        exteriorColors.exteriorGroundAmbientColor =   mixMembers<3>(lightParamData, &LightTimedData::groundAmbientColor, blendTimeCoeff);
        if (vec3EqZero(exteriorColors.exteriorGroundAmbientColor))
            exteriorColors.exteriorGroundAmbientColor = exteriorColors.exteriorAmbientColor;

        exteriorColors.exteriorHorizontAmbientColor = mixMembers<3>(lightParamData, &LightTimedData::horizontAmbientColor, blendTimeCoeff);
        if (vec3EqZero(exteriorColors.exteriorHorizontAmbientColor))
            exteriorColors.exteriorHorizontAmbientColor = exteriorColors.exteriorAmbientColor;

        exteriorColors.exteriorDirectColor   = mixMembers<3>(lightParamData, &LightTimedData::directColor, blendTimeCoeff);
        exteriorColors.exteriorSpecularColor = mixMembers<3>(lightParamData, &LightTimedData::SunColor, blendTimeCoeff);

        //Liquid colors
        liquidColors.closeOceanColor = mixMembers<3>(lightParamData, &LightTimedData::closeOceanColor, blendTimeCoeff);
        liquidColors.farOceanColor =   mixMembers<3>(lightParamData, &LightTimedData::farOceanColor, blendTimeCoeff);
        liquidColors.closeRiverColor = mixMembers<3>(lightParamData, &LightTimedData::closeRiverColor, blendTimeCoeff);
        liquidColors.farRiverColor =   mixMembers<3>(lightParamData, &LightTimedData::farRiverColor, blendTimeCoeff);

        liquidColors.oceanShallowAlpha = lightParamData.oceanShallowAlpha;
        liquidColors.oceanDeepAlpha    = lightParamData.oceanDeepAlpha;
        liquidColors.riverShallowAlpha = lightParamData.waterShallowAlpha;
        liquidColors.riverDeepAlpha    = lightParamData.waterDeepAlpha;

        //SkyColors
        skyColors.SkyTopColor =    mixMembers<3>(lightParamData, &LightTimedData::SkyTopColor, blendTimeCoeff);
        skyColors.SkyMiddleColor = mixMembers<3>(lightParamData, &LightTimedData::SkyMiddleColor, blendTimeCoeff);
        skyColors.SkyBand1Color =  mixMembers<3>(lightParamData, &LightTimedData::SkyBand1Color, blendTimeCoeff);
        skyColors.SkyBand2Color =  mixMembers<3>(lightParamData, &LightTimedData::SkyBand2Color, blendTimeCoeff);
        skyColors.SkySmogColor =   mixMembers<3>(lightParamData, &LightTimedData::SkySmogColor, blendTimeCoeff);
        skyColors.SkyFogColor =    mixMembers<3>(lightParamData, &LightTimedData::SkyFogColor, blendTimeCoeff);

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

        // LightParams flag 0x4 disables the sun fog by forcing the angle to 1.1
        if ((lightParamData.lightParamFlags & 0x4) != 0) {
            fogResult.SunFogAngle = 1.1f;
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
        fogResult.HeightDensityFogCoefficients = mixMembers<4>(lightParamData, &LightTimedData::HeightDensityFogCoeff, blendTimeCoeff);

        fogResult.FogZScalar =        mixMembers<1>(lightParamData, &LightTimedData::FogZScalar, blendTimeCoeff);
        fogResult.MainFogStartDist =  mixMembers<1>(lightParamData, &LightTimedData::MainFogStartDist, blendTimeCoeff);
        fogResult.MainFogEndDist =    mixMembers<1>(lightParamData, &LightTimedData::MainFogEndDist, blendTimeCoeff);
        fogResult.HeightEndFogColor = mixMembers<3>(lightParamData, &LightTimedData::EndFogHeightColor, blendTimeCoeff);
        fogResult.FogStartOffset =    mixMembers<1>(lightParamData, &LightTimedData::FogStartOffset, blendTimeCoeff);

        if (fogResult.FogHeightCoefficients.LengthSquared() <= 0.00000011920929f ){
            //Client falls back to DB-ordered (0,0,0,1); stored reversed for the shader here
            fogResult.FogHeightCoefficients = mathfu::vec4(1,0,0,0);
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
        if (m_mapFlag2_0x2)
        {
            fogResult.FogDensity = 1.0f;
        }
        else if ( fogScalarOverride > fogResult.FogScaler )
        {
            fogResult.FogScaler = fogScalarOverride;
        }
    } else {
        // lightParamData.lightTimedData[0].time = 0;
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

void DayNightLightHolder::SkyBoxCollector::addSkyBox(StateForConditions &stateForConditions, const SkyBoxInfo &skyBoxInfo, float alpha) {
    if (skyBoxInfo.skyBoxFdid == 0) return;

    std::shared_ptr<M2Object> skyBoxModel = nullptr;

    //1. Find in added skyboxes
    for (const auto &model : m_newSkyBoxes) {
        if (skyBoxInfo.skyBoxFdid == model->getModelFileId()) {
            skyBoxModel = model;

            float currentAlpha = model->getAlpha();
            alpha = std::max<float>(currentAlpha, alpha);

            model->setAlpha(alpha);
            break;
        }
    }

    //2. Otherwise, find in old skyboxes
    if (!skyBoxModel) {
        for (const auto &model: m_existingSkyBoxes) {
            if (skyBoxInfo.skyBoxFdid == model->getModelFileId()) {
                skyBoxModel = model;

                model->setAlpha(alpha);
                m_newSkyBoxes.push_back(model);
                break;
            }
        }
    }

    stateForConditions.currentSkyboxIds.push_back({skyBoxInfo.id, alpha});

    //3. Otherwise create
    if (skyBoxModel == nullptr) {
        skyBoxModel = m2Factory->createObject(m_api, true);
        skyBoxModel->setLoadParams(0, {}, {});

        skyBoxModel->setModelFileId(skyBoxInfo.skyBoxFdid);

        skyBoxModel->createPlacementMatrix(mathfu::vec3(0, 0, 0), 0, mathfu::vec3(1, 1, 1), nullptr);
        skyBoxModel->calcWorldPosition();
        m_newSkyBoxes.push_back(skyBoxModel);
    }

    //4. Decrease alpha of all previous skyboxes except the current one
    for (const auto &model : m_newSkyBoxes) {
        if (model == skyBoxModel) continue;

        float currentAlpha = model->getAlpha();
        model->setAlpha( currentAlpha * (1.0f - alpha));
    }

    if ((skyBoxInfo.skyBoxFlags & 4) > 0 ) {
        //In this case cone is still rendered been, but all values are final fog values.
        m_overrideValuesWithFinalFog = true;
    }

    if (skyBoxInfo.skyBoxFlags & 1) {
        skyBoxModel->setOverrideAnimationPerc(m_api->getConfig()->currentTime / 2880.0, true);
    }
}
