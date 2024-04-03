//
// Created by Deamon on 4/3/2024.
//

#include "CWmoNewLight.h"

CWmoNewLight::CWmoNewLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef) {
    isPointLight = newLightDef.type == 0;
    isSpotLight = newLightDef.type == 1;

    m_innerColor = newLightDef.innerColor;
    m_pos = (modelMatrix * mathfu::vec4(mathfu::vec3(newLightDef.position), 1.0)).xyz();
    m_rotation = newLightDef.rotation;
    m_attenuationStart = newLightDef.attenStart;
    m_attenuationEnd = newLightDef.attenEnd;
    m_intensity = newLightDef.intensity;
    m_outerColor = newLightDef.outerColor;
    m_spotLightAttenuationStart = newLightDef.spotLightAttenuationStart;
    m_spotLightAttenuationEnd = newLightDef.spotLightAttenuationEnd;
    m_spotlightRadius = newLightDef.spotlightRadius;
    m_innerAngle = newLightDef.innerAngle;
    m_outerAngle = newLightDef.outerAngle;
}

void CWmoNewLight::collectLight(std::vector<LocalLight> &pointLights, std::vector<Spotlight> &spotLights) {
    if (isPointLight) {
        auto &pointLight = pointLights.emplace_back();
        //TODO: implement flickering animation
        pointLight.innerColor = mathfu::vec4(
            ((float)m_innerColor.r)/255.0f,
            ((float)m_innerColor.g)/255.0f,
            ((float)m_innerColor.b)/255.0f,
        0);
        pointLight.outerColor = mathfu::vec4(
            ((float)m_outerColor.r)/255.0f,
            ((float)m_outerColor.g)/255.0f,
            ((float)m_outerColor.b)/255.0f,
        0);;
        pointLight.blendParams.x = m_attenuationStart;
        pointLight.blendParams.y = m_attenuationEnd;

        mathfu::vec4 attenVec = mathfu::vec4(m_attenuationStart, m_intensity, m_attenuationEnd, 0);
        pointLight.attenuation = attenVec;
        pointLight.position = mathfu::vec4(mathfu::vec3(m_pos), 1.0);

    } else if (isSpotLight) {

    }
}
