//
// Created by Deamon on 3/31/2024.
//

#ifndef AWEBWOWVIEWERCPP_WDTLIGHTSOBJECT_H
#define AWEBWOWVIEWERCPP_WDTLIGHTSOBJECT_H


#include "../../ApiContainer.h"
#include "../lights/CPointLight.h"
#include "../lights/CSpotLight.h"

class WdtLightsObject {
public:
    explicit WdtLightsObject(HApiContainer api, std::string &wdtLgtFileName);
    explicit WdtLightsObject(HApiContainer api, int wdtLgtFileDataId);

    const std::vector<CPointLight> &getPointLights(uint8_t tileX, uint8_t tileY);
    const std::vector<CSpotLight> &getSpotLights(uint8_t tileX, uint8_t tileY);
private:
    void createLightArray();
private:
    HApiContainer m_api;
    HWdtLightFile m_wdtLightFile;

    bool m_lightsCreated = false;

    std::array<std::array<std::vector<CPointLight>, 64>, 64> m_pointLights = {};
    std::array<std::array<std::vector<CSpotLight>, 64>, 64> m_spotLights = {};
};


#endif //AWEBWOWVIEWERCPP_WDTLIGHTSOBJECT_H
