//
// Created by Deamon on 4/3/2024.
//

#include "CEngineLight.h"
#include "../../algorithms/mathHelper.h"
#include "../../managers/particles/CRndSeed.h"

CEngineLight::CEngineLight(const mathfu::mat4 &modelMatrix, const map_object_pointlight_anim &mapPointLightAnim) {
    isPointLight = true;
    isSpotLight = false;
    isWmoNewLight = false;

    auto &lightRecord = mapPointLightAnim.pointLight;

    m_lightId = lightRecord.lightId;

    m_innerColor = lightRecord.color;
    m_outerColor = lightRecord.color;
    m_pos = (modelMatrix * mathfu::vec4(mathfu::vec3(lightRecord.position), 1.0f)).xyz();
    m_attenuationStart = lightRecord.attenuationStart;
    m_attenuationEnd = lightRecord.attenuationEnd;
    m_intensity = lightRecord.intensity;
    m_rotation = lightRecord.rotation;

    lightAnimation = mapPointLightAnim.lightTextureAnimation;

    flags_raw = 0;
    flags.FLAG_USE_TEXTURE     = mapPointLightAnim.lightUnkRecord.lightTextureFileDataId > 0;

    lightModelMat =
        modelMatrix *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(lightRecord.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat3::RotationZ(m_rotation.z) *
            mathfu::mat3::RotationY(m_rotation.y) *
            mathfu::mat3::RotationX(m_rotation.x)
        );
    invLightModelMat = (lightModelMat).Inverse();

    calcedLightDir = (invLightModelMat.Transpose()*mathfu::vec3(0,0,1)).Normalized();
}

CEngineLight::CEngineLight(const mathfu::mat4 &modelMatrix, const mapobject_new_light_def &newLightDef) {
    assert(newLightDef.type <= 1);

    isPointLight = newLightDef.type == 0;
    isSpotLight = newLightDef.type == 1;

    isWmoNewLight = true;

    flags_raw = 0;

    flags.FLAG_ANIMATE_OUTER_COLOR     = (newLightDef.flags & 0x1) > 0 ? 1 : 0;
    flags.FLAG_IS_SHADOWED             = (newLightDef.flags & 0x2) > 0 ? 1 : 0;
    flags.FLAG_4                       = (newLightDef.flags & 0x4) > 0 ? 1 : 0;
    flags.FLAG_USE_TEXTURE             = (newLightDef.flags & 0x8) > 0 ? 1 : 0; //Requires FLAG_IS_SHADOWED to be off to be active

    m_lightId = newLightDef.lightIndex;

    m_innerColor = newLightDef.innerColor;
    m_pos = (modelMatrix * mathfu::vec4(mathfu::vec3(newLightDef.position), 1.0)).xyz();
    m_rotation = newLightDef.rotation;
    m_attenuationStart = newLightDef.attenStart;
    m_attenuationEnd = newLightDef.attenEnd;

    float unpackedIntensityMult = halfToFloat(newLightDef.f16_intesityMultiplier);
    if (feq(unpackedIntensityMult, 0.0f))
        unpackedIntensityMult = 1.0f;
    if (flags.FLAG_IS_SHADOWED == 0)
        unpackedIntensityMult = 1.0f;

    if (flags.FLAG_IS_SHADOWED)
        m_intensity = newLightDef.intensity * unpackedIntensityMult;
    else
        m_intensity = newLightDef.intensity;

    m_outerColor = newLightDef.outerColor;

    m_blendStart = newLightDef.blendStart;
    m_blendEnd = newLightDef.blendEnd;

    m_falloff = newLightDef.falloff;

    m_innerAngle = newLightDef.innerAngle;
    m_outerAngle = newLightDef.outerAngle;

    if (newLightDef.lightTextureAnimation.flickerMode != 0) {
        lightAnimation = newLightDef.lightTextureAnimation;
    }

    lightModelMat =
        modelMatrix *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(newLightDef.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat4::RotationZ(m_rotation.z) *
            mathfu::mat4::RotationY(m_rotation.y) *
            mathfu::mat4::RotationX(m_rotation.x)
        );
    invLightModelMat = (lightModelMat).Inverse();

    calcedLightDir = (invLightModelMat.Transpose()*mathfu::vec3(0,0,1));
}

CEngineLight::CEngineLight(const MapSpotLight &mapSpotLight, const MapLightTextureAnimation * mltaRec ) {
    isPointLight = false;
    isSpotLight = true;
    isWmoNewLight = false;

    flags_raw = 0;

    m_lightId = mapSpotLight.id;

    m_falloff = mapSpotLight.falloff;
    m_innerAngle = mapSpotLight.innerAngle;
    m_outerAngle = mapSpotLight.outerAngle;
    m_innerColor = mapSpotLight.color;
    m_outerColor = mapSpotLight.color;
    m_pos = (mathfu::vec4(mathfu::vec3(mapSpotLight.position), 1.0f)).xyz();
    m_attenuationStart = mapSpotLight.attenuationStart;
    m_attenuationEnd = mapSpotLight.attenuationEnd;
    m_intensity = mapSpotLight.intensity;
    m_rotation = mapSpotLight.rotation;

    if (mltaRec) {
        lightAnimation = mltaRec->textureAnimation;
    }

    lightModelMat =
        mathfu::mat4::FromTranslationVector(mathfu::vec3(mapSpotLight.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat3::RotationZ(m_rotation.z) *
            mathfu::mat3::RotationY(m_rotation.y) *
            mathfu::mat3::RotationX(m_rotation.x)
        );
    invLightModelMat = (lightModelMat).Inverse();

    calcedLightDir = (invLightModelMat.Transpose()*mathfu::vec3(0,0,1)).Normalized();
}

CEngineLight::CEngineLight(const MapPointLight3 &lightRecord, const MapLightTextureAnimation * mltaRec) {
    isPointLight = true;
    isSpotLight = false;
    isWmoNewLight = false;

    m_lightId = lightRecord.lightIndex;

    m_innerColor = lightRecord.color;
    m_outerColor = lightRecord.color;
    m_pos = (mathfu::vec4(mathfu::vec3(lightRecord.position), 1.0f)).xyz();
    m_attenuationStart = lightRecord.attenuationStart;
    m_attenuationEnd = lightRecord.attenuationEnd;
    m_intensity = lightRecord.intensity;
    m_rotation = lightRecord.rotation;

    flags_raw = 0;

    //Remap the flags from MPL3. They are shifted by 1
    flags.FLAG_IS_SHADOWED     = (lightRecord.flags & 0x1) > 0 ? 1 : 0;
    flags.FLAG_4               = (lightRecord.flags & 0x2) > 0 ? 1 : 0;
    flags.FLAG_USE_TEXTURE     = (lightRecord.flags & 0x4) > 0 ? 1 : 0;
    flags.FLAG_10              = (lightRecord.flags & 0x8) > 0 ? 1 : 0;

    if (mltaRec) {
        lightAnimation = mltaRec->textureAnimation;
    }

    lightModelMat =
        mathfu::mat4::FromTranslationVector(mathfu::vec3(lightRecord.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat3::RotationZ(m_rotation.z) *
            mathfu::mat3::RotationY(m_rotation.y) *
            mathfu::mat3::RotationX(m_rotation.x)
        );
    invLightModelMat = (lightModelMat).Inverse();

    calcedLightDir = (invLightModelMat.Transpose()*mathfu::vec3(0,0,1)).Normalized();
}
CEngineLight::CEngineLight(const MapPointLight2 &lightRecord, const MapLightTextureAnimation * mltaRec ) {
    isPointLight = true;
    isSpotLight = false;
    isWmoNewLight = false;

    m_lightId = lightRecord.lightIndex;

    m_innerColor = lightRecord.color;
    m_outerColor = lightRecord.color;
    m_pos = (mathfu::vec4(mathfu::vec3(lightRecord.position), 1.0f)).xyz();
    m_attenuationStart = lightRecord.attenuationStart;
    m_attenuationEnd = lightRecord.attenuationEnd;
    m_intensity = lightRecord.intensity;
    m_rotation = lightRecord.rotation;

    flags_raw = 0;

    if (mltaRec) {
        lightAnimation = mltaRec->textureAnimation;
    }

    lightModelMat =
        mathfu::mat4::FromTranslationVector(mathfu::vec3(lightRecord.position)) *
        mathfu::mat4::FromRotationMatrix(
            mathfu::mat3::RotationZ(m_rotation.z) *
            mathfu::mat3::RotationY(m_rotation.y) *
            mathfu::mat3::RotationX(m_rotation.x)
        );
    invLightModelMat = (lightModelMat).Inverse();

    calcedLightDir = (invLightModelMat.Transpose()*mathfu::vec3(0,0,1)).Normalized();
}

mathfu::vec3 animateFlicker(
    float time,
    int flickerMode,
    mathfu::vec3 color,
    float flickerSpeed,
    float flickerIntensity,
    float staticRandomCookie)
{
    // Mode 0: No flicker, just copy color
    if (flickerMode == 0) {
        return color;
    }

    float scaledIntensity = flickerIntensity * 0.01f;
    float scaledSpeed = flickerSpeed * 0.1f;

    unsigned int timeMs = time;
    float timeOffset = (float) (timeMs % 60000) * 0.001f + staticRandomCookie;

    mathfu::vec3 outputColor(0,0,0);

    switch (flickerMode) {
        case 1: // Sine wave flicker
        {
            float sineTime = (float) (timeMs % 62831) * 0.001f + staticRandomCookie;
            float flickerAmount = sinf(sineTime * scaledSpeed) * scaledIntensity * 0.5f;
            outputColor.x = fmaxf(color.x + flickerAmount, 0.0f);
            outputColor.y = fmaxf(color.y + flickerAmount, 0.0f);
            outputColor.z = fmaxf(color.z + flickerAmount, 0.0f);
            break;
        }

        case 2: // Noise-based flicker
        {
            auto hsv = MathHelper::rgb2hsv(color);

            float noiseValue = CRndSeed::noise_(timeOffset * scaledSpeed);
            float newBrightness = hsv.v + (noiseValue - 0.5f) * scaledIntensity;
            hsv.v = fmaxf(0.0f, fminf(newBrightness, 1.0f));

            outputColor = MathHelper::hsv2rgb(hsv);
            break;
        }

        case 3: // Binary on/off flicker
        {
            auto hsv = MathHelper::rgb2hsv(color);

            float noiseValue = CRndSeed::noise_(timeOffset * scaledSpeed);
            float brightness = (noiseValue >= 0.5f) ? scaledIntensity : 0.0f;
            hsv.v = fmaxf(0.0f, fminf(brightness, 1.0f));

            outputColor = MathHelper::hsv2rgb(hsv);
            break;
        }
    }

    return outputColor;
}

inline mathfu::vec4 CEngineLight::animateColor(animTime_t currentTime, const mathfu::vec4 &color) {
    float innerColorA = color.w;

    auto colorAnimated = animateFlicker(
        currentTime, lightAnimation.flickerMode,
        color.xyz(),
        lightAnimation.flickerSpeed,
        lightAnimation.flickerIntensity,
         (float)(int)(
            16777619 * ((m_lightId >> 24) ^ (
            16777619 * ((m_lightId >> 16) & 0xFF ^ (
            16777619 * (((m_lightId >> 8) & 0xFF) ^ (
            16777619 * ((m_lightId & 0xFF) ^ 0x811C9DC5)))))))
         ) * 2.3283064e-10
    );

    return mathfu::vec4(colorAnimated, innerColorA);
}

void CEngineLight::collectLight(mathfu::vec3 camera, animTime_t currentTime, std::vector<LocalLight> &pointLights, std::vector<SpotLight> &spotLights, std::vector<SpotLight> &insideSpotLights) {
    if (isPointLight) {
        auto &pointLight = pointLights.emplace_back();

        pointLight.innerColor = animateColor(currentTime, ImVectorToVec4(m_innerColor)) * m_intensity;

        float attenuationStart = 0;
        float blendEnd = 0;

        if (flags.FLAG_ANIMATE_OUTER_COLOR) {
            pointLight.outerColor = animateColor(currentTime, ImVectorToVec4(m_outerColor)) * m_intensity;

            attenuationStart = m_blendStart;
            blendEnd = m_blendEnd;
        } else {
            pointLight.outerColor = pointLight.innerColor;

            attenuationStart = m_attenuationStart;
            blendEnd = m_attenuationEnd;
        }

        pointLight.blendParams.x = attenuationStart;
        pointLight.blendParams.y = blendEnd;
        pointLight.blendParams.z = flags.FLAG_IS_SHADOWED ? 1.0 : 0.0 ;

        mathfu::vec4 attenVec = mathfu::vec4(
            attenuationStart, m_attenuationEnd, 1.0f / (m_attenuationEnd - attenuationStart),
            flags.FLAG_USE_TEXTURE ? 0.0f : 1.0f)
        ;
        pointLight.attenuation = attenVec;
        pointLight.position = mathfu::vec4(mathfu::vec3(m_pos), 1.0);

    } else if (isSpotLight) {
        auto localCamera = invLightModelMat * mathfu::vec4(camera, 1);
        auto localDir = (localCamera.xyz() - mathfu::vec3(0,0,-1));

        //This dot product in local space is simplified to just taking z component:
        //float ld = mathfu::dot(localDir, mathfu::vec3(0,0,-1));
        float ld = localDir.z;
        float ld_norm = localDir.Normalized().z;

        float localAtten = 1.0 - ((ld - m_attenuationStart) / (m_attenuationEnd - m_attenuationStart));

        bool isCameraInsideSpotlight =
            // localAtten > 0.0f &&
            ld_norm >= cosf(m_outerAngle * 0.5f);

        auto &spotLight = isCameraInsideSpotlight ? insideSpotLights.emplace_back() : spotLights.emplace_back();

        spotLight.lightModelMat = lightModelMat;

        //1. Animate Outer color
        float blendStart = 0;
        float blendEnd = 0;

        mathfu::vec3 innerColor = animateColor(currentTime, ImVectorToVec4(m_innerColor)).xyz() * m_intensity;
        mathfu::vec3 outerColor;
        if (flags.FLAG_ANIMATE_OUTER_COLOR) {
            outerColor = animateColor(currentTime, ImVectorToVec4(m_outerColor)).xyz() * m_intensity;

            blendStart = m_blendStart;
            blendEnd = m_blendEnd;
        } else {
            outerColor = innerColor;

            blendStart = m_attenuationStart;
            blendEnd = m_attenuationEnd;
        }

        spotLight.innerColorAndBlendStart = mathfu::vec4(
            innerColor,
            blendStart
        );
        spotLight.outerColorAndBlendEnd = mathfu::vec4(
            outerColor,
            blendEnd
        );

        spotLight.positionAndCosInnerAngle = mathfu::vec4(
            mathfu::vec3(m_pos), cosf(m_innerAngle * 0.5f)
        );
        spotLight.attenuationAndCosOuterAngle = mathfu::vec4(
            m_attenuationStart, m_attenuationEnd, 1.0f / (m_attenuationEnd - m_attenuationStart),
            cosf(m_outerAngle * 0.5f)
        );
        spotLight.directionAndCosAngleDiff = mathfu::vec4(
            calcedLightDir,
            1.0f/fmaxf(cosf(m_innerAngle * 0.5f) - cosf(m_outerAngle * 0.5f), 0.0099999f)
        );
        spotLight.interiorAndSpotLightLenAndFallOff = mathfu::vec4(
            isWmoNewLight ? 0.0f : 1.0f,
            tanf(m_outerAngle * 0.5f) * m_attenuationEnd + 0.8f,
            m_falloff,
            0.0f
        );
    }
}
