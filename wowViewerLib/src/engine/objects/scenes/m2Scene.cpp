//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"

void M2Scene::checkCulling(HCullStage cullStage) {
    mathfu::vec4 cameraPos = cullStage->matricesForCulling->cameraPos;
    mathfu::mat4 &frustumMat = cullStage->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = cullStage->matricesForCulling->lookAtMat;


    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
//    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);

    m_drawModel = m_m2Object->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
}

extern "C" {
    extern void supplyPointer(int *availablePointer, int length);
}

void M2Scene::doPostLoad(HCullStage cullStage) {
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
//                m_api->setCameraPosition((max.z - modelCenter.z) / tan(M_PI * 19.0f / 180.0f), 0, 0);
            } else {
//                m_api->setCameraPosition((max.y - modelCenter.y) / tan(M_PI * 19.0f / 180.0f), 0, 0);
            }
//            m_api->setCameraOffset(modelCenter.x, modelCenter.y, modelCenter.z);
        } else {
//            m_api->setCameraPosition(1.0,0,0);
//            m_api->setCameraOffset(0,0,0);
        }
        std::vector <int> availableAnimations;
        m_m2Object->getAvailableAnimation(availableAnimations);
#ifdef __EMSCRIPTEN__
        supplyPointer(&availableAnimations[0], availableAnimations.size());
#endif
    }
}

void M2Scene::update(HUpdateStage updateStage) {
    auto cameraVec3 = updateStage->cameraMatrices->cameraPos.xyz();
    m_m2Object->update(updateStage->delta, cameraVec3, updateStage->cameraMatrices->lookAtMat);
    m_m2Object->uploadGeneratorBuffers(updateStage->cameraMatrices->lookAtMat);
}

//mathfu::vec4 M2Scene::getAmbientColor() {
//    if (doOverride) {
//        return m_ambientColorOverride;
//    } else {
//        return m_m2Object->getAmbientLight();
//    }
//}
//
//bool M2Scene::getCameraSettings(M2CameraResult &result) {
//    if (m_cameraView > -1 && m_m2Object->getGetIsLoaded()) {
//        result = m_m2Object->updateCamera(0, m_cameraView);
//        return true;
//    }
//    return false;
//}
//
//void M2Scene::setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) {
//    doOverride = override;
//    m_ambientColorOverride = ambientColor;
//
//    m_m2Object->setAmbientColorOverride(ambientColor, override);
//}


void M2Scene::setReplaceTextureArray(std::vector<int> &replaceTextureArray) {
    //std::cout << "replaceTextureArray.size == " << replaceTextureArray.size() << std::endl;
    //std::cout << "m_m2Object == " << m_m2Object << std::endl;
    if (m_m2Object == nullptr) return;

    auto textureCache = m_api->cacheStorage->getTextureCache();
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

void M2Scene::updateBuffers(HCullStage cullStage) {

}

void M2Scene::produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage) {
    if (!m_drawModel) return;

    resultDrawStage->meshesToRender = std::make_shared<MeshesToRender>();

    m_m2Object->collectMeshes(resultDrawStage->meshesToRender->meshes, 0);
    m_m2Object->drawParticles(resultDrawStage->meshesToRender->meshes, 0);

    std::sort(resultDrawStage->meshesToRender->meshes.begin(),
              resultDrawStage->meshesToRender->meshes.end(),
              IDevice::sortMeshes
    );
}
