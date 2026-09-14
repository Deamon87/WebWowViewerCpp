//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"
#include <algorithm>
#include <vector>

void WmoScene::getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                                    const mathfu::vec4 &cameraPos,
                                    const HMapRenderPlan &mapRenderPlan,
                                    M2ObjectListContainer &potentialM2,
                                    WMOListContainer &potentialWmo) {
    potentialWmo.addCand(this->m_wmoObject);
}

void WmoScene::getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                     const mathfu::vec4 &cameraPos,
                                     const HMapRenderPlan &mapRenderPlan,
                                     M2ObjectListContainer &m2ObjectsCandidates,
                                     WMOListContainer &wmoCandidates) {

    wmoCandidates.addCand(this->m_wmoObject);
};

void WmoScene::updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                        StateForConditions &stateForConditions, const AreaRecord &areaRecord) {
    Config* config = this->m_api->getConfig();

    Map::updateLightAndSkyboxData(mapRenderPlan, frustumData, stateForConditions, areaRecord);

    mathfu::vec4 exteriorAmbient = mathfu::vec4(1.0,1.0,1.0,1.0);

    auto frameDependantData = mapRenderPlan->frameDependentData;

    if (m_api->getConfig()->globalLighting == EParameterSource::eDatabase) {
        frameDependantData->colors.exteriorAmbientColor         = mathfu::vec3(exteriorAmbient.x, exteriorAmbient.y, exteriorAmbient.z);
        frameDependantData->colors.exteriorHorizontAmbientColor = mathfu::vec3(exteriorAmbient.x, exteriorAmbient.y, exteriorAmbient.z);
        frameDependantData->colors.exteriorGroundAmbientColor   = mathfu::vec3(exteriorAmbient.x, exteriorAmbient.y, exteriorAmbient.z);
        frameDependantData->colors.exteriorDirectColor          = mathfu::vec3(0.5, 0.5, 0.5);
        frameDependantData->colors.exteriorSpecularColor        = mathfu::vec3(0, 0, 0);
    } else if (config->globalLighting == EParameterSource::eConfig) {
        auto fdd = mapRenderPlan->frameDependentData;

        fdd->colors.exteriorAmbientColor = config->exteriorColors.exteriorAmbientColor;
        fdd->colors.exteriorGroundAmbientColor = config->exteriorColors.exteriorGroundAmbientColor;
        fdd->colors.exteriorHorizontAmbientColor = config->exteriorColors.exteriorHorizontAmbientColor;
        fdd->colors.exteriorDirectColor = config->exteriorColors.exteriorDirectColor;
        fdd->colors.exteriorSpecularColor = config->exteriorColors.exteriorSpecularColor;
    }

    //Fog: WmoScene has no real area/DB context (it's a fake mapObjDef at a hardcoded position), so
    //DB-sourced fog makes no sense here. Use the WMO file's own MFOG fog, but only when the camera
    //is inside a non-exterior group of the WMO, ramping in from the nearest portal — the same
    //gating/blending the world path applies in DayNightLightHolder::updateLightAndSkyboxData.
    {
        auto frameDependantData = mapRenderPlan->frameDependentData;

        //Map::makeFramePlan sets m_currentWMO/m_currentWmoGroup when the camera is inside the WMO
        int currentGroup = (mapRenderPlan->m_currentWMO != emptyWMO) ? mapRenderPlan->m_currentWmoGroup : -1;

        WmoObject::WmoFogBlendResult wmoFogBlendResult;
        if (m_wmoObject->isLoaded()) {
            m_wmoObject->checkFog(frustumData.cameraPos, currentGroup, wmoFogBlendResult);
        }

        if (wmoFogBlendResult.fogFound && wmoFogBlendResult.insideInterior && !frameDependantData->fogResults.empty()) {
            //Same conversion the world path uses: it encodes the WMO fog as a legacy exponential
            //fog (LegacyFogScalar = 1). Writing raw WMO fog fields into a default FogResult leaves
            //LegacyFogScalar/MainFog* zeroed, which selects the shader's artistic-fog path — and
            //with all-zero curve data that path evaluates to full fog color at any distance.
            FogResult wmoFog = m_dayNightLightHolder.wmoFogDataToFogResult(wmoFogBlendResult.fog.fog, config->farPlane);

            //Ramp the fog in from the portal (full fog 25+ units into the interior), like the world
            //path — except here it ramps from "no fog" instead of the DB fog. The base keeps the
            //WMO fog's colors so only the strength ramps, not the tint.
            float wmoBlend = std::min<float>(std::max<float>(wmoFogBlendResult.distToExit * 0.04f, 0.0f), 1.0f);

            FogResult noFog = wmoFog;
            noFog.FogScaler = 0.0f;
            noFog.FogDensity = 0.0f;
            noFog.FogHeightDensity = 0.0f;

            FogResult fogResult = noFog;
            DayNightLightHolder::blendWmoFogIntoFogResult(fogResult, wmoFog, wmoBlend, false);

            frameDependantData->fogResults[0] = fogResult;
            frameDependantData->FogDataFound = true;
        } else {
            frameDependantData->FogDataFound = false;
        }
    }
}