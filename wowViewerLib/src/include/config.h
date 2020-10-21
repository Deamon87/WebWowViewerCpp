//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H

#include <string>
#include <mathfu/glsl_mappings.h>

enum class EParameterSource: char {
    eConfig,
    eDatabase,
    eM2
};

class Config {

public:
    bool renderAdt = true;
    bool renderWMO = true;
    bool renderM2 = true;
    bool renderBSP = false;
    bool renderPortals = false;
    bool usePortalCulling = true;
    bool useInstancing = false;

    bool disableFog = false;
    bool fogDataFound = false;

    bool drawWmoBB = false;
    bool drawM2BB = true;
    bool secondCamera = false;
    bool doubleCameraDebug = false;

    bool BCLightHack = false;

    bool drawDepthBuffer = false;
    int cameraM2 = -1; // this will be sceneNumber of object

    int wmoMinBatch = 0;
    int wmoMaxBatch = 9999;

    int m2MinBatch = 0;
    int m2MaxBatch = 9999;

    int minParticle = 0;
    int maxParticle = 9999;

    int threadCount = 4;
    int quickSortCutoff = 100;

    int currentTime = 0;

    bool useWotlkLogic = false;

    float nearPlane = 1;
    float farPlane = 1000;
    float farPlaneForCulling = 400;

    bool useGaussBlur = true;

    bool pauseAnimation = false;

    bool useTimedGloabalLight = true;
    bool useM2AmbientLight = false;

    mathfu::vec4 clearColor = {0.117647, 0.207843, 0.392157, 0};

    EParameterSource globalLighting = EParameterSource::eDatabase;
    mathfu::vec4 exteriorAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorHorizontAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorGroundAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorDirectColor = {0.3,0.3,0.3, 0.3};
    mathfu::vec3 exteriorDirectColorDir;

    mathfu::vec4 interiorAmbientColor;
    mathfu::vec4 interiorSunColor;
    mathfu::vec3 interiorSunDir;

    mathfu::vec4 closeRiverColor = {1,1,1,1};

    EParameterSource skyParams = EParameterSource::eDatabase;
    mathfu::vec4 SkyTopColor;
    mathfu::vec4 SkyMiddleColor;
    mathfu::vec4 SkyBand1Color;
    mathfu::vec4 SkyBand2Color;
    mathfu::vec4 SkySmogColor;
    mathfu::vec4 SkyFogColor;

    EParameterSource globalFog = EParameterSource::eDatabase;
    mathfu::vec4 actualFogColor;
    float FogEnd;
    float FogScaler;
    float FogDensity;
    float FogHeight;
    float FogHeightScaler;
    float FogHeightDensity;
    float SunFogAngle;
    mathfu::vec3 FogColor;
    mathfu::vec3 EndFogColor;
    float EndFogColorDistance;
    mathfu::vec3 SunFogColor;
    float SunFogStrength;
    mathfu::vec3 FogHeightColor;
    mathfu::vec4 FogHeightCoefficients;

    std::string areaName;

    int diffuseColorHack = 0;

    EParameterSource glowSource = EParameterSource::eDatabase;
    float currentGlow = 0;

};


#endif //WOWVIEWERLIB_CONFIG_H
