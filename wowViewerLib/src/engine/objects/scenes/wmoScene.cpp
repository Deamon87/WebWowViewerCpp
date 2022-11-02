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
                                    HCullStage &cullStage,
                                    M2ObjectListContainer &potentialM2,
                                    WMOListContainer &potentialWmo) {
    potentialWmo.addCand(this->m_wmoObject);
}

void WmoScene::getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                     const mathfu::vec4 &cameraPos,
                                     HCullStage &cullStage,
                                     M2ObjectListContainer &m2ObjectsCandidates,
                                     WMOListContainer &wmoCandidates) {

    wmoCandidates.addCand(this->m_wmoObject);
};

void WmoScene::updateLightAndSkyboxData(const HCullStage &cullStage, mathfu::vec3 &cameraVec3,
                                       StateForConditions &stateForConditions, const AreaRecord &areaRecord) {
    Config* config = this->m_api->getConfig();
    config->globalFog = EParameterSource::eNone;

    Map::updateLightAndSkyboxData(cullStage, cameraVec3, stateForConditions, areaRecord);

    mathfu::vec4 ambient = mathfu::vec4(1.0,1.0,1.0,1.0);

    auto frameDepedantData = cullStage->frameDepedantData;

    frameDepedantData->exteriorAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
    frameDepedantData->exteriorHorizontAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
    frameDepedantData->exteriorGroundAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
    frameDepedantData->exteriorDirectColor = mathfu::vec4(0.3,0.30,0.3,0.3);
    frameDepedantData->exteriorDirectColorDir = MathHelper::calcExteriorColorDir(
        cullStage->matricesForCulling->lookAtMat,
        m_api->getConfig()->currentTime
    );
}