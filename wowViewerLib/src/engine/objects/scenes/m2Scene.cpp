//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../algorithms/mathHelper.h"

void M2Scene::checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);

    m_drawModel = m_m2Object->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
}

void M2Scene::draw() {
    if (!m_drawModel) return;

    this->m_api->activateM2Shader();
    m_m2Object->draw(false);
    m_m2Object->draw(true);
    this->m_api->deactivateM2Shader();
}

void M2Scene::update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) {
    m_m2Object->update(deltaTime, cameraVec3, lookAtMat);
}

mathfu::vec4 M2Scene::getAmbientColor() {
    return mathfu::vec4(0,0,0,0);
    return m_m2Object->getAmbientLight();
}

bool M2Scene::getCameraSettings(M2CameraResult &result) {
    if (m_cameraView > -1 && m_m2Object->getGetIsLoaded()) {
        result = m_m2Object->updateCamera(0, m_cameraView);
        return true;
    }
    return false;
}
