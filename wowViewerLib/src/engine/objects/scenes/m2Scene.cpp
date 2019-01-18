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
//    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);

    m_drawModel = m_m2Object->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
}

void M2Scene::draw(WoWFrameData *frameData) {
    if (!m_drawModel) return;

    m_api->getDevice()->drawMeshes(frameData->renderedThisFrame);
}

extern "C" {
    extern void supplyPointer(int *availablePointer, int length);
}

void M2Scene::doPostLoad(WoWFrameData *frameData) {
    if (m_m2Object->doPostLoad()) {

        CAaBox aabb = m_m2Object->getColissionAABB();
        if ((mathfu::vec3(aabb.max) - mathfu::vec3(aabb.min)).LengthSquared() < 0.001 ) {
            aabb = m_m2Object->getAABB();
        }

        auto max = aabb.max;
        auto min = aabb.min;

        if ((mathfu::vec3(aabb.max) - mathfu::vec3(aabb.min)).LengthSquared() < 20000) {

            mathfu::vec3 modelCenter = mathfu::vec3(
                ((max.x + min.x) / 2.0f),
                ((max.y + min.y) / 2.0f),
                ((max.z + min.z) / 2.0f)
            );

            if ((max.z - modelCenter.z) > (max.y - modelCenter.y)) {
                m_api->setCameraPosition((max.z - modelCenter.z) / tan(M_PI * 19.0f / 180.0f), 0, 0);
            } else {
                m_api->setCameraPosition((max.y - modelCenter.y) / tan(M_PI * 19.0f / 180.0f), 0, 0);
            }
            m_api->setCameraOffset(modelCenter.x, modelCenter.y, modelCenter.z);
        } else {
            m_api->setCameraPosition(1.0,0,0);
            m_api->setCameraOffset(0,0,0);
        }
        std::vector <int> availableAnimations;
        m_m2Object->getAvailableAnimation(availableAnimations);
#ifdef __EMSCRIPTEN__
        supplyPointer(&availableAnimations[0], availableAnimations.size());
#endif
    }
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

void M2Scene::collectMeshes(WoWFrameData * frameData) {
    frameData->renderedThisFrame = std::vector<HGMesh>();

    m_m2Object->collectMeshes(frameData->renderedThisFrame, 0);
    m_m2Object->drawParticles(frameData->renderedThisFrame, 0);

    std::sort(frameData->renderedThisFrame.begin(),
              frameData->renderedThisFrame.end(),
              IDevice::sortMeshes
    );

}

void M2Scene::setReplaceTextureArray(std::vector<int> &replaceTextureArray) {
    //std::cout << "replaceTextureArray.size == " << replaceTextureArray.size() << std::endl;
    //std::cout << "m_m2Object == " << m_m2Object << std::endl;
    if (m_m2Object == nullptr) return;

    auto textureCache = m_api->getTextureCache();
    std::vector<HBlpTexture> replaceTextures;
    replaceTextures.reserve(replaceTextureArray.size());

    for (int i = 0; i < replaceTextureArray.size(); i++) {
        if (replaceTextureArray[i] == 0) {
            replaceTextures.push_back(nullptr);
        } else {
            //std::cout << "replaceTextureArray[i] == " << replaceTextureArray[i] << std::endl;
            //std::cout << "i == " << i << std::endl;
            replaceTextures.push_back(textureCache->getFileId(replaceTextureArray[i]));
        }
    }

    m_m2Object->setReplaceTextures(replaceTextures);
}
