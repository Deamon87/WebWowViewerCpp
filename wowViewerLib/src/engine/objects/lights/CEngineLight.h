//
// Created by Deamon on 4/3/2024.
//

#ifndef AWEBWOWVIEWERCPP_CWMONEWLIGHT_H
#define AWEBWOWVIEWERCPP_CWMONEWLIGHT_H

#include <vector>
#include "../../persistance/header/wmoFileHeader.h"
#include "../../persistance/wdtLightFile.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../../include/iostuff.h"

class CEngineLight {
public:
    CEngineLight(){};
    CEngineLight(const mathfu::mat4 &modelMatrix, const map_object_pointlight_anim &mapPointLightAnim);
    CEngineLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef );
    CEngineLight(const MapSpotLight &mapSpotLight, const MapLightTextureAnimation * mltaRec );
    CEngineLight(const MapPointLight3 &lightRecord, const MapLightTextureAnimation * mltaRec );
    CEngineLight(const MapPointLight2 &lightRecord, const MapLightTextureAnimation * mltaRec );

    void collectLight(mathfu::vec3 camera, animTime_t currentTime, std::vector<LocalLight> &pointLights, std::vector<SpotLight> &spotLights, std::vector<SpotLight> &insideSpotLights);
private:
    inline mathfu::vec4 animateColor(animTime_t currentTime, const mathfu::vec4 &color);

private:
    mathfu::mat4 invLightModelMat;
    mathfu::mat4 lightModelMat;

    bool isPointLight;
    bool isSpotLight;
    bool isWmoNewLight;

    int m_lightId;

    union {
        uint32_t flags_raw;
        struct
        {
            uint32_t FLAG_ANIMATE_OUTER_COLOR : 1;
            uint32_t FLAG_IS_SHADOWED : 1;
            uint32_t FLAG_4 : 1;
            uint32_t FLAG_USE_TEXTURE : 1;
            uint32_t FLAG_10 : 1;
            uint32_t FLAG_20 : 1;
            uint32_t FLAG_40 : 1;
        } flags;
    };

    CImVector m_innerColor;
    C3Vector m_pos;
    C3Vector m_rotation;

    float m_blendStart = 0.0f;
    float m_blendEnd = 0.0f;

    float m_attenuationStart;
    float m_attenuationEnd;
    float m_intensity = 1.0f;
    CImVector m_outerColor;
    float m_falloff;

    float m_innerAngle;
    float m_outerAngle;

    mathfu::vec3 calcedLightDir;

    LightTextureAnimation lightAnimation = {0.0, 0.0, 0};

};

#endif //AWEBWOWVIEWERCPP_CWMONEWLIGHT_H
