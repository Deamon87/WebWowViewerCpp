//
// Created by Deamon on 4/3/2024.
//

#include "CWmoNewLight.h"
#include "../../algorithms/mathHelper.h"

CWmoNewLight::CWmoNewLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef) {
    isPointLight = newLightDef.type == 0;
    isSpotLight = newLightDef.type == 1;
    isWmoNewLight = true;

    flags_raw = 0;

    m_innerColor = newLightDef.innerColor;
    m_pos = (modelMatrix * mathfu::vec4(mathfu::vec3(newLightDef.position), 1.0)).xyz();
    m_rotation = newLightDef.rotation;
    m_attenuationStart = newLightDef.attenStart;
    m_attenuationEnd = newLightDef.attenEnd;
    m_intensity = newLightDef.intensity;
    m_outerColor = newLightDef.outerColor;
    m_falloffStart = newLightDef.falloffStart;
    m_falloff = newLightDef.falloff;
    m_spotlightRadius = newLightDef.spotlightRadius;
    m_innerAngle = newLightDef.innerAngle;
    m_outerAngle = newLightDef.outerAngle;

    rot_mat =
        MathHelper::RotationZ((m_rotation.z)) *
        MathHelper::RotationY((m_rotation.y)) *
        MathHelper::RotationX((m_rotation.x));
    calcedLightDir = ((modelMatrix * rot_mat).Inverse().Transpose()*mathfu::vec4(0,0,-1,0));
}

CWmoNewLight::CWmoNewLight(const WdtLightFile::MapSpotLight &mapSpotLight ) {
    isPointLight = false;
    isSpotLight = true;
    isWmoNewLight = false;

    flags_raw = 0;

    m_spotlightRadius = mapSpotLight.spotlightRadius;
    m_innerAngle = mapSpotLight.innerAngle;
    m_outerAngle = mapSpotLight.outerAngle;
    m_innerColor = mapSpotLight.color;
    m_outerColor = mapSpotLight.color;
    m_pos = mapSpotLight.position;
    m_attenuationStart = mapSpotLight.attenuationStart;
    m_attenuationEnd = mapSpotLight.attenuationEnd;
    m_intensity = mapSpotLight.intensity;
    m_rotation = mapSpotLight.rotation;

    rot_mat =
        MathHelper::RotationZ((m_rotation.z)) *
        MathHelper::RotationY((m_rotation.y)) *
        MathHelper::RotationX((m_rotation.x));
    calcedLightDir = (rot_mat).Inverse().Transpose()*mathfu::vec4(0,0,-1,0);
}

void CWmoNewLight::collectLight(std::vector<LocalLight> &pointLights, std::vector<SpotLight> &spotLights) {
    if (isPointLight) {
        auto &pointLight = pointLights.emplace_back();
        //TODO: implement flickering animation
        pointLight.innerColor = ImVectorToVec4(m_innerColor) * m_intensity;
        pointLight.outerColor = ImVectorToVec4(m_outerColor) * m_intensity;
        pointLight.blendParams.x = m_attenuationStart;
        pointLight.blendParams.y = m_attenuationEnd;

        mathfu::vec4 attenVec = mathfu::vec4(m_attenuationStart, m_intensity, m_attenuationEnd, 0);
        pointLight.attenuation = attenVec;
        pointLight.position = mathfu::vec4(mathfu::vec3(m_pos), 1.0);

    } else if (isSpotLight) {
        auto &spotLight = spotLights.emplace_back();
//        auto rotQuat = mathfu::quat::FromEulerAngles(m_rotation.x, m_rotation.y, m_rotation.z);

        if (true) {
            auto rotQuatD = mathfu::quat::FromMatrix(
            rot_mat
            );

//            assert(rotQuatD.vector() == rotQuat.vector() && rotQuatD.scalar() == rotQuat.scalar());
//            rotQuat = rotQuatD;
        }

        spotLight.rotMat = rot_mat;
//        spotLight.rotQuaternion = mathfu::vec4(
//            rotQuat[1], rotQuat[2],rotQuat[3], rotQuat[0]
//        );
        spotLight.spotLightLen = mathfu::vec4(
            tanf(m_outerAngle * 0.5f) * m_attenuationEnd + 0.8,0,0,0
        );
        float l_falloffStart = 0.0f;
        if (flags.FLAG_OUTERCOLOR_IS_MAIN) {
            spotLight.colorAndFalloff = mathfu::vec4(
                ImVectorToVec4(m_outerColor).xyz() * m_intensity, m_falloff
            );
            l_falloffStart = m_falloffStart;
        } else {
            spotLight.colorAndFalloff = mathfu::vec4(
                ImVectorToVec4(m_innerColor).xyz() * m_intensity, m_attenuationEnd
            );
            l_falloffStart = m_attenuationStart;
        }
        spotLight.positionAndcosInnerAngle = mathfu::vec4(
            mathfu::vec3(m_pos), cosf(m_innerAngle * 0.5f)
        );
        spotLight.attenuationAndcosOuterAngle = mathfu::vec4(
            m_attenuationStart, m_attenuationEnd, 1.0f / (m_attenuationEnd - m_attenuationStart),
            cosf(m_outerAngle * 0.5f)
        );
        spotLight.directionAndcosAngleDiff = mathfu::vec4(
            calcedLightDir.xyz(),
            1.0f/fmaxf(cosf(m_innerAngle * 0.5f) - cosf(m_outerAngle * 0.5f), 0.0099999f)
        );
        spotLight.interior = mathfu::vec4(
            isWmoNewLight ? 0.0f : 1.0f,
            l_falloffStart,
            0.0f,
            0.0f
        );

    }
}
