//
// Created by Deamon on 4/3/2024.
//

#ifndef AWEBWOWVIEWERCPP_CWMONEWLIGHT_H
#define AWEBWOWVIEWERCPP_CWMONEWLIGHT_H

#include <vector>
#include "../../persistance/header/wmoFileHeader.h"
#include "../../../gapi/UniformBufferStructures.h"


class CWmoNewLight {
public:
    CWmoNewLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef );

    void collectLight(std::vector<LocalLight> &pointLights, std::vector<Spotlight> &spotLights);

private:
    bool isPointLight;
    bool isSpotLight;
    CImVector m_innerColor;
    C3Vector m_pos;
    C3Vector m_rotation;
    float m_attenuationStart;
    float m_attenuationEnd;
    float m_intensity;
    CImVector m_outerColor;
    float m_spotLightAttenuationStart;
    float m_spotLightAttenuationEnd;
    float m_spotlightRadius;
    float m_innerAngle;
    float m_outerAngle;
};

#endif //AWEBWOWVIEWERCPP_CWMONEWLIGHT_H
