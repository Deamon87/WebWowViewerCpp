//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../camera/m2TiedCamera.h"

void M2Scene::getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                                   const mathfu::vec4 &cameraPos,
                                   HCullStage &cullStage,
                                   M2ObjectListContainer &potentialM2,
                                   WMOListContainer &potentialWmo)  {
    potentialM2.addCandidate(m_m2Object);
}

void M2Scene::getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                    const mathfu::vec4 &cameraPos,
                                    HCullStage &cullStage,
                                    M2ObjectListContainer &m2ObjectsCandidates,
                                    WMOListContainer &wmoCandidates) {
    m2ObjectsCandidates.addCandidate(m_m2Object);
}

void M2Scene::updateLightAndSkyboxData(const HCullStage &cullStage, mathfu::vec3 &cameraVec3,
                              StateForConditions &stateForConditions, const AreaRecord &areaRecord) {
    Config* config = this->m_api->getConfig();
    Map::updateLightAndSkyboxData(cullStage, cameraVec3, stateForConditions, areaRecord);
    if (config->globalLighting == EParameterSource::eM2) {
        auto ambient = m_m2Object->getM2SceneAmbientLight();

        if (ambient.Length() < 0.0001)
            ambient = mathfu::vec4(1.0,1.0,1.0,1.0);

        auto frameDepedantData = cullStage->frameDepedantData;

        frameDepedantData->exteriorAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        frameDepedantData->exteriorHorizontAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        frameDepedantData->exteriorGroundAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        frameDepedantData->exteriorDirectColor = mathfu::vec4(0.0,0.0,0.0,0.0);
        frameDepedantData->exteriorDirectColorDir = mathfu::vec3(0.0,0.0,0.0);
    }
    auto frameDepedantData = cullStage->frameDepedantData;
    frameDepedantData->FogDataFound = false;
}

extern "C" {
    extern void supplyAnimationList(int *availablePointer, int length);
    extern void supplyMeshIds(int *availablePointer, int length);
    extern void offerFileAsDownload(std::string filename, std::string mime);
}

void M2Scene::doPostLoad(HCullStage cullStage) {
    Map::doPostLoad(cullStage);
}

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

void M2Scene::setMeshIdArray(std::vector<uint8_t> &meshIds) {
    m_m2Object->setMeshIds(meshIds);
}

int M2Scene::getCameraNum() {
    return m_m2Object->getCameraNum();
}

std::shared_ptr<ICamera> M2Scene::createCamera(int cameraNum) {
    if (cameraNum < 0 || cameraNum >= getCameraNum()) {
        return nullptr;
    }

    return std::make_shared<m2TiedCamera>(m_m2Object, cameraNum);
}

void updateCameraPosOnLoad(const HApiContainer &m_api, const std::shared_ptr<M2Object> &m2Object) {
    if (m2Object->isMainDataLoaded()) {
        CAaBox aabb = m2Object->getColissionAABB();
        if ((mathfu::vec3(aabb.max) - mathfu::vec3(aabb.min)).LengthSquared() < 0.001 ) {
            aabb = m2Object->getAABB();
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
                m_api->camera->setCameraPos((max.z - modelCenter.z) / tan(M_PI * 19.0f / 180.0f), 0, 0);

            } else {
                m_api->camera->setCameraPos((max.y - modelCenter.y) / tan(M_PI * 19.0f / 180.0f), 0, 0);
            }
            m_api->camera->setCameraOffset(modelCenter.x, modelCenter.y, modelCenter.z);
        } else {
            m_api->camera->setCameraPos(1.0,0,0);
            m_api->camera->setCameraOffset(0,0,0);
        }
#ifdef __EMSCRIPTEN__
        std::vector <int> availableAnimations;
        m2Object->getAvailableAnimation(availableAnimations);

        supplyAnimationList(&availableAnimations[0], availableAnimations.size());

        std::vector<int> meshIds;
        m2Object->getMeshIds(meshIds);

        supplyMeshIds(&meshIds[0], meshIds.size());

#endif
    }
}

M2Scene::M2Scene(HApiContainer api, std::string m2Model, int cameraView) {
    m_api = api; m_m2Model = m2Model; m_cameraView = cameraView;
    m_sceneMode = SceneMode::smM2;
    m_suppressDrawingSky = true;

    auto  m2Object = std::make_shared<M2Object>(m_api);
    std::vector<HBlpTexture> replaceTextures = {};
    std::vector<uint8_t> meshIds = {};
    m2Object->setLoadParams(0, meshIds, replaceTextures);
    m2Object->setModelFileName(m_m2Model);
    m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);

    m2Object->calcWorldPosition();

    m_m2Object = m2Object;
    auto l_api = m_api;
    m_m2Object->addPostLoadEvent([m2Object, l_api]() {
        updateCameraPosOnLoad(l_api, m2Object);
    });

    api->getConfig()->globalFog = EParameterSource::eConfig;
}

M2Scene::M2Scene(HApiContainer api, int fileDataId, int cameraView) {
    m_api = api; m_cameraView = cameraView;
    m_sceneMode = SceneMode::smM2;
    m_suppressDrawingSky = true;

    auto m2Object = std::make_shared<M2Object>(m_api);
    std::vector<HBlpTexture> replaceTextures = {};
    std::vector<uint8_t> meshIds = {};
    m2Object->setLoadParams(0, meshIds, replaceTextures);
    m2Object->setModelFileId(fileDataId);
    m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
    m2Object->calcWorldPosition();

    m_m2Object = m2Object;
    auto l_api = m_api;
    m_m2Object->addPostLoadEvent([m2Object, l_api]() {
        updateCameraPosOnLoad(l_api, m2Object);
    });

    api->getConfig()->globalFog = EParameterSource::eConfig;
}

void M2Scene::setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {
    m_m2Object->setReplaceParticleColors(particleColorReplacement);
}

void M2Scene::resetReplaceParticleColor() {
    m_m2Object->resetReplaceParticleColor();
}

void M2Scene::exportScene(IExporter* exporter) {
    exporter->addM2Object(m_m2Object);
}
