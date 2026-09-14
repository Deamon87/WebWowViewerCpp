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
#include "../../../../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"
#include "LightParamCalculate.h"


class DayNightLightHolder {
public:
    explicit DayNightLightHolder(const HApiContainer &api, int mapId);

private:

    struct SkyBoxCollector {
        public:
            SkyBoxCollector(const HApiContainer& api, std::vector<std::shared_ptr<M2Object>> &existingSkyBoxes) : m_existingSkyBoxes(existingSkyBoxes), m_api(api) {};

            void addSkyBox(StateForConditions &stateForConditions, const SkyBoxInfo &skyBoxInfo, float alpha);
            const std::vector<std::shared_ptr<M2Object>> &getNewSkyBoxes() const {
                return m_newSkyBoxes;
            }
            bool getOverrideValuesWithFinalFog() const  { return m_overrideValuesWithFinalFog; }

        private:
            HApiContainer m_api;

            bool m_overrideValuesWithFinalFog = false;
            std::vector<std::shared_ptr<M2Object>> &m_existingSkyBoxes;
            std::vector<std::shared_ptr<M2Object>> m_newSkyBoxes;
    };

    std::vector<mapInnerZoneLightRecord> m_zoneLights;
    std::vector<std::shared_ptr<M2Object>> m_exteriorSkyBoxes;


    HApiContainer m_api;
    int m_mapId = -1;
public:
    void loadZoneLights();
    void updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                   StateForConditions &stateForConditions, const AreaRecord &areaRecord);

    void getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config,
                                float &glow,
                                SkyColors &skyColors,
                                SkyBodyData &skyBodyData,
                                ExteriorColors &exteriorColors,
                                FogResult &fogResult,
                                FogResult &underWaterFogResult,
                                LiquidColors &liquidColors,
                                SkyBoxCollector &skyBoxCollector,
                                StateForConditions *stateForConditions);

    const std::vector<std::shared_ptr<M2Object>> &getExteriorSkyBoxes() const {
        return m_exteriorSkyBoxes;
    }

    FogResult wmoFogDataToFogResult(const SMOFog_Data &wmoFogData, float farClip) const;
    static void blendWmoFogIntoFogResult(FogResult &fogResult, const FogResult &wmoFog, float wmoBlend, bool underwater);

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

    void
    inline calcLightParamResult(int lightParamId, const Config *config,
                                float &glow,
                                SkyBodyData &skyBodyData,
                                ExteriorColors &exteriorColors,
                                FogResult &fogResult, LiquidColors &liquidColors,
                                SkyColors &skyColors);

    void processSkyBoxes(const HMapRenderPlan &mapRenderPlan, const StateForConditions &stateForConditions,
                         const Config *config) const;

    std::array<mathfu::vec3, 4> calcPlanetPositions(const mathfu::vec3 &cameraVec3);

    // Computes per-frame planet (sun/moon discs) and stars render data into frameDependentData
    void updatePlanetsAndStars(const mathfu::vec3 &cameraPos, const SkyBodyData &skyBodyData,
                               const HFrameDependantData &fdd);

    mathfu::vec3 calcSunPosition(
        const SkyBodyData &skyBody,
        const mathfu::vec3 &cameraVec3,
        const mathfu::mat4 &viewMat,
        float &sunAttenuationStart,
        float &sunAttenuationEnd,
        std::array<mathfu::vec3, 4> &planetPositions
    );
    mathfu::vec3 calcDirectColorDir(const SkyBodyData &skyBody, const mathfu::mat4 &invTranspViewMat );
    mathfu::vec3 calcSunDirForFog(
        const SkyBodyData &skyBody,
        const mathfu::vec3 &sunPositionInView,
        const mathfu::vec3 &cameraVec3,
        const mathfu::mat4 &invTranspViewMat,
        std::array<mathfu::vec3, 4> &planetPositions
    );
};


#endif //AWEBWOWVIEWERCPP_DAYNIGHTLIGHTHOLDER_H
