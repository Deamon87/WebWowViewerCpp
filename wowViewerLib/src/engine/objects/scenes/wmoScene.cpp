//
// Created by Deamon on 10/10/2017.
//

#include "wmoScene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"

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
    config->globalFog = EParameterSource::eNone;

    Map::updateLightAndSkyboxData(mapRenderPlan, frustumData, stateForConditions, areaRecord);

    mathfu::vec4 exteriorAmbient = mathfu::vec4(1.0,1.0,1.0,1.0);

    auto frameDependantData = mapRenderPlan->frameDependentData;
    frameDependantData->exteriorDirectColorDir = MathHelper::calcExteriorColorDir(
        mapRenderPlan->renderingMatrices->lookAtMat,
        m_api->getConfig()->currentTime
    );

    if (m_api->getConfig()->globalLighting == EParameterSource::eDatabase) {
        frameDependantData->colors.exteriorAmbientColor = mathfu::vec4(exteriorAmbient.x, exteriorAmbient.y, exteriorAmbient.z, 1.0);
        frameDependantData->colors.exteriorHorizontAmbientColor = mathfu::vec4(exteriorAmbient.x, exteriorAmbient.y, exteriorAmbient.z, 1.0);
        frameDependantData->colors.exteriorGroundAmbientColor = mathfu::vec4(exteriorAmbient.x, exteriorAmbient.y, exteriorAmbient.z, 1.0);
        frameDependantData->colors.exteriorDirectColor = mathfu::vec4(0.5, 0.5, 0.5, 0.5);
    } else if (config->globalLighting == EParameterSource::eConfig) {
        auto fdd = mapRenderPlan->frameDependentData;

        fdd->colors.exteriorAmbientColor = config->exteriorColors.exteriorAmbientColor;
        fdd->colors.exteriorGroundAmbientColor = config->exteriorColors.exteriorGroundAmbientColor;
        fdd->colors.exteriorHorizontAmbientColor = config->exteriorColors.exteriorHorizontAmbientColor;
        fdd->colors.exteriorDirectColor = config->exteriorColors.exteriorDirectColor;
    }
}