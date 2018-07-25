//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../../gapi/meshes/GM2Mesh.h"
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

    std::vector<HGMesh> renderedThisFrame;

    m_m2Object->fillBuffersAndArray(renderedThisFrame);
    m_m2Object->drawParticles(renderedThisFrame);

    std::sort(renderedThisFrame.begin(),
              renderedThisFrame.end(),
              [&](HGMesh& a, HGMesh& b) -> const bool {
                  if (a->getIsTransparent() > b-> getIsTransparent()) {
                      return false;
                  }
                  if (a->getIsTransparent() < b->getIsTransparent()) {
                      return true;
                  }

                  if (a->getMeshType() > b->getMeshType()) {
                      return false;
                  }
                  if (a->getMeshType() < b->getMeshType()) {
                      return true;
                  }

                  if (a->getMeshType() == MeshType::eM2Mesh) {
                      HGM2Mesh a1 = std::static_pointer_cast<GM2Mesh>(a);
                      HGM2Mesh b1 = std::static_pointer_cast<GM2Mesh>(b);
                      if (a1->m_priorityPlane != b1->m_priorityPlane) {
                          return b1->m_priorityPlane > a1->m_priorityPlane;
                      }

                      if (a1->m_sortDistance > b1->m_sortDistance) {
                          return true;
                      }
                      if (a1->m_sortDistance < b1->m_sortDistance) {
                          return false;
                      }

                      if (a1->m_m2Object > b1->m_m2Object) {
                          return true;
                      }
                      if (a1->m_m2Object < b1->m_m2Object) {
                          return false;
                      }

                      return b1->m_layer < a1->m_layer;
                  }

                  return a > b;
              }
    );

    m_api->getDevice()->drawMeshes(renderedThisFrame);


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
