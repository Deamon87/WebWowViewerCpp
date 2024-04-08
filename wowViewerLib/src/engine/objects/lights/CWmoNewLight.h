//
// Created by Deamon on 4/3/2024.
//

#ifndef AWEBWOWVIEWERCPP_CWMONEWLIGHT_H
#define AWEBWOWVIEWERCPP_CWMONEWLIGHT_H

#include <vector>
#include "../../persistance/header/wmoFileHeader.h"
#include "../../persistance/wdtLightFile.h"
#include "../../../gapi/UniformBufferStructures.h"


class CWmoNewLight {
public:
    CWmoNewLight(){};
    CWmoNewLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef );
    CWmoNewLight(const WdtLightFile::MapSpotLight &mapSpotLight );

    void collectLight(std::vector<LocalLight> &pointLights, std::vector<SpotLight> &spotLights);

private:
    bool isPointLight;
    bool isSpotLight;
    bool isWmoNewLight;
    union {
        uint32_t flags_raw;
        struct
        {
            uint32_t FLAG_OUTERCOLOR_IS_MAIN : 1;
            uint32_t FLAG_2 : 1;
            uint32_t FLAG_4 : 1;
            uint32_t FLAG_8 : 1;
            uint32_t FLAG_10 : 1;
            uint32_t FLAG_20 : 1;
            uint32_t FLAG_40 : 1;
        } flags;
    };

    CImVector m_innerColor;
    C3Vector m_pos;
    C3Vector m_rotation;
    float m_attenuationStart;
    float m_attenuationEnd;
    float m_intensity = 1.0f;
    CImVector m_outerColor;
    float m_falloffStart;
    float m_falloff;
    float m_spotlightRadius;
    float m_innerAngle;
    float m_outerAngle;

    mathfu::mat4 rot_mat;
    mathfu::vec4 calcedLightDir;

};

#endif //AWEBWOWVIEWERCPP_CWMONEWLIGHT_H
