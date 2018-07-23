//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../../gapi/GM2Mesh.h"
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

    std::vector<HGM2Mesh> renderedThisFrame;

    m_m2Object->fillBuffersAndArray(renderedThisFrame);

    std::sort(renderedThisFrame.begin(),
              renderedThisFrame.end(),
              [&](HGM2Mesh& a, HGM2Mesh& b) -> const bool {
                  if (a->m_priorityPlane != b->m_priorityPlane) {
                      return b->m_priorityPlane < a->m_priorityPlane;
                  }

                  if (a->m_sortDistance > b->m_sortDistance) {
                      return false;
                  }
                  if (a->m_sortDistance < b->m_sortDistance) {
                      return true;
                  }

                  if (a->m_m2Object > b->m_m2Object) {
                      return false;
                  }
                  if (a->m_m2Object < b->m_m2Object) {
                      return true;
                  }

                  return b->m_layer > a->m_layer;
              }
    );

    m_api->getDevice()->drawM2Meshes(renderedThisFrame);


}

void M2Scene::update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) {
    m_m2Object->update(deltaTime, cameraVec3, lookAtMat);
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
