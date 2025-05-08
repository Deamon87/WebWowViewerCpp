//
// Created by Deamon on 4/1/2024.
//

#include "CPointLight.h"

CPointLight::CPointLight() {

}

CPointLight::CPointLight(const mathfu::mat4 &modelMat, const map_object_point_light &lightRecord) {
    const auto &pointLightRec = lightRecord;

    mathfu::vec4 attenVec = mathfu::vec4(
        pointLightRec.attenuationStart, pointLightRec.attenuationEnd, 1.0f / (pointLightRec.attenuationEnd - pointLightRec.attenuationStart),
        0
    );

    m_localLight.attenuation = attenVec;

    m_localLight.innerColor = ImVectorToVec4(lightRecord.color) * lightRecord.intensity;

    m_localLight.outerColor = m_localLight.innerColor;
    m_localLight.position = mathfu::vec4((modelMat * mathfu::vec4(mathfu::vec3(pointLightRec.position), 1.0)).xyz(), 1.0);
    m_localLight.blendParams = mathfu::vec4(
        pointLightRec.attenuationStart,
        pointLightRec.attenuationEnd,
        0,
        0
    );
}
