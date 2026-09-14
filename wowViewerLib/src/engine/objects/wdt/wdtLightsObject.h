//
// Created by Deamon on 3/31/2024.
//

#ifndef AWEBWOWVIEWERCPP_WDTLIGHTSOBJECT_H
#define AWEBWOWVIEWERCPP_WDTLIGHTSOBJECT_H


#include "../../ApiContainer.h"
#include "../lights/CPointLight.h"
#include "../lights/CEngineLight.h"

class WdtLightsObject {
public:
    explicit WdtLightsObject(HApiContainer api, std::string &wdtLgtFileName);
    explicit WdtLightsObject(HApiContainer api, int wdtLgtFileDataId);

    void collectLights(mathfu::vec3 camera,
                       animTime_t sceneTime,
                       uint8_t tileX, uint8_t tileY,
                       std::vector<LocalLight> &pointLights,
                       std::vector<SpotLight> &spotLights, std::vector<SpotLight> &insideSpotLights);
private:
    void createLightArray();
private:
    HApiContainer m_api;
    HWdtLightFile m_wdtLightFile;

    bool m_lightsCreated = false;

    std::array<std::array<std::vector<CEngineLight>, 64>, 64> m_lights = {};
};


#endif //AWEBWOWVIEWERCPP_WDTLIGHTSOBJECT_H
