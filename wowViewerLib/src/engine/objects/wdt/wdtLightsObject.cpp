//
// Created by Deamon on 3/31/2024.
//

#include "wdtLightsObject.h"

WdtLightsObject::WdtLightsObject(HApiContainer api, std::string &wdtLgtFileName) {
    m_api = api;
    m_wdtLightFile = m_api->cacheStorage->getWdtLightFileCache()->get(wdtLgtFileName);
}

WdtLightsObject::WdtLightsObject(HApiContainer api, int wdtLgtFileDataId) {
    m_api = api;
    m_wdtLightFile = m_api->cacheStorage->getWdtLightFileCache()->getFileId(wdtLgtFileDataId);
}

static const std::vector<CPointLight> emptyPointLights = {};
static const std::vector<CWmoNewLight> emptySpotLights = {};

const std::vector<CPointLight> &WdtLightsObject::getPointLights(uint8_t tileX, uint8_t tileY) {
    if (m_wdtLightFile->getStatus() != FileStatus::FSLoaded)
        return emptyPointLights;

    if (!m_lightsCreated) {
        createLightArray();
        m_lightsCreated = true;
    }

    return m_pointLights[tileX][tileY];
}

void WdtLightsObject::collectSpotLights(uint8_t tileX, uint8_t tileY, std::vector<SpotLight> &spotLights) {
    if (m_wdtLightFile->getStatus() != FileStatus::FSLoaded)
        return;

    if (!m_lightsCreated) {
        createLightArray();
        m_lightsCreated = true;
    }

    std::vector<LocalLight> dummyPointLight;
    auto &lights = m_spotLights[tileX][tileY];
    for (auto &spotLight : lights) {
        spotLight.collectLight(dummyPointLight, spotLights);
    }
}

void WdtLightsObject::createLightArray() {
    if (m_wdtLightFile->getStatus() != FileStatus::FSLoaded)
        return;

    //Create Point Lights
    {
//    std::unordered_map<uint32_t, WdtLightFile::MapPointLight3*> processedLightIds;
        for (int i = 0; i < m_wdtLightFile->mapPointLights3Len; i++) {
            auto &pointLight3 = m_wdtLightFile->mapPointLights3[i];

//        {
//            auto it = processedLightIds.find(pointLight3.lightIndex);
//            bool found = it != processedLightIds.end();
//            if (found) {
//                auto duplicateRec = it->second;
//                std::cout << "Found duploicate" << std::endl;
//            }
//        }


            m_pointLights[pointLight3.tileX][pointLight3.tileY].emplace_back() =
                CPointLight(pointLight3);

//        processedLightIds[pointLight3.lightIndex] = &pointLight3;
        }
    }

    //Create Spot Lights
    {
        mathfu::mat4 m = mathfu::mat4::Identity();
//    std::unordered_map<uint32_t, WdtLightFile::MapPointLight3*> processedLightIds;
        for (int i = 0; i < m_wdtLightFile->mapSpotLightLen; i++) {
            auto &spotLight = m_wdtLightFile->mapSpotLights[i];

//        {
//            auto it = processedLightIds.find(pointLight3.lightIndex);
//            bool found = it != processedLightIds.end();
//            if (found) {
//                auto duplicateRec = it->second;
//                std::cout << "Found duploicate" << std::endl;
//            }
//        }


            m_spotLights[spotLight.tileX][spotLight.tileY].emplace_back() =
                CWmoNewLight(m, spotLight);

//        processedLightIds[pointLight3.lightIndex] = &pointLight3;
        }
    }
}

