//
// Created by Deamon on 4/1/2024.
//

#include "CPointLight.h"

CPointLight::CPointLight() {

}

CPointLight::CPointLight(WdtLightFile::MapPointLight3 &lightRecord) {

    mathfu::vec4 attenVec = mathfu::vec4(lightRecord.attenuationStart, lightRecord.intensity, lightRecord.attenuationEnd, 0);

    m_localLight.attenuation = attenVec;

    m_localLight.innerColor = mathfu::vec4(
        ((float)lightRecord.color.r)/255.0f,
        ((float)lightRecord.color.g)/255.0f,
        ((float)lightRecord.color.b)/255.0f,
        0);

    m_localLight.outerColor = m_localLight.innerColor;
    m_localLight.position = mathfu::vec4(mathfu::vec3(lightRecord.position), 0.0);
    m_localLight.blendParams = mathfu::vec4(0.0);
}
