//
// Created by Deamon on 4/3/2024.
//

#include "CWmoNewLight.h"
#include "../../algorithms/mathHelper.h"

float unpackPackedFloat_light(uint16_t a1)
{
  unsigned int v1; // eax
  uint16_t v2; // edx
  unsigned int v3; // ecx
  int v4; // eax
  uint32_t result; // xmm0_4

  v1 = a1;
  v2 = a1 & 0x3FF;
  v3 = (a1 << 16) & 0x80000000;
  v4 = (v1 >> 10) & 0x1F;
  if ( v4 == 31 )
  {
    result = v3 | ((v2 | 0x3FC00) << 13);
  }
  else if ( v4 )
  {
    result = v3 | ((v2 | ((v4 + 112) << 10)) << 13);
  }
  else
  {
    if ( v2 )
    {
      v4 = 113;
      do
      {
        v2 = 2 * v2;
        --v4;
      }
      while ( (v2 & 0x400) == 0 );
      v2 &= 0x3FFu;
    }
    result = v3 | ((v2 | (v4 << 10)) << 13);
  }
  return *(float *)&result;
}

CWmoNewLight::CWmoNewLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef) {
    assert(newLightDef.type <= 1);

    isPointLight = newLightDef.type == 0;
    isSpotLight = newLightDef.type == 1;

    isWmoNewLight = true;

    flags_raw = 0;

    m_innerColor = newLightDef.innerColor;
    m_pos = (modelMatrix * mathfu::vec4(mathfu::vec3(newLightDef.position), 1.0)).xyz();
    m_rotation = newLightDef.rotation;
    m_attenuationStart = newLightDef.attenStart;
    m_attenuationEnd = newLightDef.attenEnd;

    float unpackedIntensityMult = unpackPackedFloat_light(newLightDef.packedIntesityMultiplier);
    if (feq(unpackedIntensityMult, 0.0f))
        unpackedIntensityMult = 1.0f;
    if ((newLightDef.flags & 0x2) == 0)
        unpackedIntensityMult = 1.0f;

    m_intensity = newLightDef.intensity * unpackedIntensityMult ;
    m_outerColor = newLightDef.outerColor;
    m_falloffStart = newLightDef.falloffStart;
    if (!isSpotLight) {
        m_falloff = 1.0f;//newLightDef.falloff;
    } else {
        m_falloff = newLightDef.spotlightRadius;
    }
    m_spotlightRadius = newLightDef.spotlightRadius;
    m_innerAngle = newLightDef.innerAngle;
    m_outerAngle = newLightDef.outerAngle;

    lightModelMat =
        modelMatrix *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(newLightDef.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat4::RotationZ(m_rotation.z) *
            mathfu::mat4::RotationY(m_rotation.y) *
            mathfu::mat4::RotationX(m_rotation.x)
        );

    calcedLightDir = ((lightModelMat).Inverse().Transpose()*mathfu::vec3(0,0,1));
}

CWmoNewLight::CWmoNewLight(const mathfu::mat4 &modelMatrix, const WdtLightFile::MapSpotLight &mapSpotLight ) {
    isPointLight = false;
    isSpotLight = true;
    isWmoNewLight = false;

    flags_raw = 0;

    m_spotlightRadius = mapSpotLight.spotlightRadius;
    m_innerAngle = mapSpotLight.innerAngle;
    m_outerAngle = mapSpotLight.outerAngle;
    m_innerColor = mapSpotLight.color;
    m_outerColor = mapSpotLight.color;
    m_pos = (modelMatrix * mathfu::vec4(mathfu::vec3(mapSpotLight.position), 1.0f)).xyz();
    m_attenuationStart = mapSpotLight.attenuationStart;
    m_attenuationEnd = mapSpotLight.attenuationEnd;
    m_intensity = mapSpotLight.intensity;
    m_rotation = mapSpotLight.rotation;

    lightModelMat =
        modelMatrix *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(mapSpotLight.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat3::RotationZ(m_rotation.z) *
            mathfu::mat3::RotationY(m_rotation.y) *
            mathfu::mat3::RotationX(m_rotation.x)
        );

    calcedLightDir = ((lightModelMat).Inverse().Transpose()*mathfu::vec3(0,0,1)).Normalized();
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

        spotLight.lightModelMat = lightModelMat;

        spotLight.spotLightLen = mathfu::vec4(
            tanf(m_outerAngle * 0.5f) * m_attenuationEnd + 0.8f,0,0,0
        );
        float l_falloffStart = 0.0f;
        if (flags.FLAG_OUTERCOLOR_IS_MAIN) {
            spotLight.colorAndFalloff = mathfu::vec4(
                ImVectorToVec4(m_outerColor).xyz() * m_intensity, m_falloff
            );
            l_falloffStart = m_falloffStart;
        } else {
            spotLight.colorAndFalloff = mathfu::vec4(
                ImVectorToVec4(m_innerColor).xyz() * m_intensity, m_falloff
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
            calcedLightDir,
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
