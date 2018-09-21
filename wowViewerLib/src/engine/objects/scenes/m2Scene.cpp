//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"

void M2Scene::checkCulling(WoWFrameData *frameData) {
    mathfu::vec4 cameraPos = mathfu::vec4(frameData->m_cameraVec3, 1.0);
    mathfu::mat4 &frustumMat = frameData->m_perspectiveMatrixForCulling;
    mathfu::mat4 &lookAtMat4 = frameData->m_lookAtMat4;


    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);

    m_drawModel = m_m2Object->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
}

void M2Scene::draw(WoWFrameData *frameData) {
    if (!m_drawModel) return;

    std::vector<HGMesh> renderedThisFrame;

    m_m2Object->collectMeshes(renderedThisFrame, 0);
    m_m2Object->drawParticles(renderedThisFrame, 0);

    std::sort(renderedThisFrame.begin(),
        renderedThisFrame.end(),
        IDevice::sortMeshes
    );

    m_api->getDevice()->drawMeshes(renderedThisFrame);


}

void M2Scene::doPostLoad(WoWFrameData *frameData) {
    m_m2Object->doPostLoad();
}

void M2Scene::update(WoWFrameData *frameData) {
    m_m2Object->update(frameData->deltaTime, frameData->m_cameraVec3, frameData->m_lookAtMat4);
}

mathfu::vec4 M2Scene::getAmbientColor() {
    if (doOverride) {
        return m_ambientColorOverride;
    } else {
        return m_m2Object->getAmbientLight();
    }
}

bool M2Scene::getCameraSettings(M2CameraResult &result) {
    if (m_cameraView > -1 && m_m2Object->getGetIsLoaded()) {
        result = m_m2Object->updateCamera(0, m_cameraView);
        return true;
    }
    return false;
}

void M2Scene::setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) {
    doOverride = override;
    m_ambientColorOverride = ambientColor;

    m_m2Object->setAmbientColorOverride(ambientColor, override);
}
