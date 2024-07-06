//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../camera/m2TiedCamera.h"

M2Scene::~M2Scene() {
//    std::cout << "M2Scene destroyed" << std::endl;
}


void M2Scene::getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                                   const mathfu::vec4 &cameraPos,
                                   const HMapRenderPlan &mapRenderPlan,
                                   M2ObjectListContainer &potentialM2,
                                   WMOListContainer &potentialWmo)  {
    potentialM2.addCandidate(m_m2Object);
}

void M2Scene::getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                    const mathfu::vec4 &cameraPos,
                                    const HMapRenderPlan &mapRenderPlan,
                                    M2ObjectListContainer &m2ObjectsCandidates,
                                    WMOListContainer &wmoCandidates) {
    m2ObjectsCandidates.addCandidate(m_m2Object);
}

void M2Scene::updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan,
                                       MathHelper::FrustumCullingData &frustumData,
                                       StateForConditions &stateForConditions, const AreaRecord &areaRecord) {
    const Config* config = this->m_api->getConfig();
    Map::updateLightAndSkyboxData(mapRenderPlan, frustumData, stateForConditions, areaRecord);
    if (config->globalLighting == EParameterSource::eM2) {
        auto ambient = m_m2Object->getM2SceneAmbientLight();

        if (ambient.Length() < 0.0001)
            ambient = mathfu::vec4(1.0,1.0,1.0,1.0);

        auto frameDependantData = mapRenderPlan->frameDependentData;

        frameDependantData->colors.exteriorAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        frameDependantData->colors.exteriorHorizontAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        frameDependantData->colors.exteriorGroundAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        frameDependantData->colors.exteriorDirectColor = mathfu::vec4(0.0, 0.0, 0.0, 0.0);
        frameDependantData->exteriorDirectColorDir = mathfu::vec3(0.0, 0.0, 0.0);
    }
    auto frameDepedantData = mapRenderPlan->frameDependentData;
    frameDepedantData->FogDataFound = false;
}

extern "C" {
    extern void supplyAnimationList(int *availablePointer, int length);
    extern void supplyMeshIds(int *availablePointer, int length);
    extern void offerFileAsDownload(std::string filename, std::string mime);
}

void M2Scene::setReplaceTextureArray(const HMapSceneBufferCreate &sceneRenderer, const std::vector<int> &replaceTextureArray) {
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

    m_m2Object->setReplaceTextures(sceneRenderer, replaceTextures);
}

void M2Scene::setMeshIdArray(const HMapSceneBufferCreate &sceneRenderer, const std::vector<uint8_t> &meshIds) {
    m_m2Object->setMeshIds(sceneRenderer, meshIds);
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



M2Scene::M2Scene(const HApiContainer &api, const std::string &m2Model) {
    m_api = api; m_m2Model = m2Model;
    m_sceneMode = SceneMode::smM2;
    m_suppressDrawingSky = true;

    auto  m2Object = m2Factory.createObject(m_api);
    std::vector<HBlpTexture> replaceTextures = {};
    std::vector<uint8_t> meshIds = {};
    m2Object->setLoadParams(0, meshIds, replaceTextures);
    m2Object->setModelFileName(m_m2Model);
    m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);

    m2Object->calcWorldPosition();

    m_m2Object = m2Object;

    api->getConfig()->globalFog = EParameterSource::eConfig;
}

M2Scene::M2Scene(const HApiContainer &api, int fileDataId)  {
    m_api = api;
    m_sceneMode = SceneMode::smM2;
    m_suppressDrawingSky = true;

    auto m2Object = m2Factory.createObject(m_api);
    std::vector<HBlpTexture> replaceTextures = {};
    std::vector<uint8_t> meshIds = {};
    m2Object->setLoadParams(0, meshIds, replaceTextures);
    m2Object->setModelFileId(fileDataId);
    m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
    m2Object->calcWorldPosition();

    m_m2Object = m2Object;

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

std::shared_ptr<M2Object> M2Scene::getSceneM2() {
    return m_m2Object;
}

