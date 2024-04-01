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
static const std::vector<CSpotLight> emptySpotLights = {};

const std::vector<CPointLight> &WdtLightsObject::getPointLights(uint8_t tileX, uint8_t tileY) {
    if (m_wdtLightFile->getStatus() != FileStatus::FSLoaded)
        return emptyPointLights;

    if (!m_lightsCreated) {
        createLightArray();
        m_lightsCreated = true;
    }

    return m_pointLights[tileX][tileY];
}

const std::vector<CSpotLight> &WdtLightsObject::getSpotLights(uint8_t tileX, uint8_t tileY) {
    if (m_wdtLightFile->getStatus() != FileStatus::FSLoaded)
        return emptySpotLights;

    if (!m_lightsCreated) {
        createLightArray();
        m_lightsCreated = true;
    }

    return m_spotLights[tileX][tileY];
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
}

