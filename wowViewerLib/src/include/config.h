//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H

#include <string>
#include <thread>
#include <mathfu/glsl_mappings.h>

enum class EParameterSource: char {
    eConfig,
    eDatabase,
    eM2
};

enum class EFreeStrategy : char {
    eTimeBased,
    eFrameBase
};

class Config {
public:
    Config() {
        threadCount = std::max<int>((int)std::thread::hardware_concurrency()-2, 1);
    }

public:
    bool renderAdt = true;
    bool renderWMO = true;
    bool renderM2 = true;
    bool renderBSP = false;
    bool renderPortals = false;
    bool usePortalCulling = true;
    bool useInstancing = false;

    bool disableFog = false;
    bool renderSkyDom = true;

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

    bool useMinimapWaterColor = false;
    bool useCloseRiverColorForDB = false;
    EParameterSource waterColorParams = EParameterSource::eDatabase;
    mathfu::vec4 closeRiverColor = {1,1,1,1};
    mathfu::vec4 farRiverColor = {1,1,1,1};
    mathfu::vec4 closeOceanColor = {1,1,1,1};
    mathfu::vec4 farOceanColor = {1,1,1,1};

    EParameterSource skyParams = EParameterSource::eDatabase;
    mathfu::vec4 SkyTopColor;
    mathfu::vec4 SkyMiddleColor;
    mathfu::vec4 SkyBand1Color;
    mathfu::vec4 SkyBand2Color;
    mathfu::vec4 SkySmogColor;
    mathfu::vec4 SkyFogColor;

    EParameterSource globalFog = EParameterSource::eDatabase;
    mathfu::vec4 actualFogColor = mathfu::vec4(1,1,1, 1);
    float FogEnd = 0;
    float FogScaler = 0;
    float FogDensity = 0;
    float FogHeight = 0;
    float FogHeightScaler = 0;
    float FogHeightDensity = 0;
    float SunFogAngle = 0;
    mathfu::vec3 FogColor = mathfu::vec3(0,0,0);
    mathfu::vec3 EndFogColor = mathfu::vec3(0,0,0);
    float EndFogColorDistance = 0;
    mathfu::vec3 SunFogColor = mathfu::vec3(0,0,0);
    float SunFogStrength = 0;
    mathfu::vec3 FogHeightColor = mathfu::vec3(0,0,0);
    mathfu::vec4 FogHeightCoefficients = mathfu::vec4(0,0,0,0);

    std::string areaName;

    int diffuseColorHack = 0;

    EParameterSource glowSource = EParameterSource::eDatabase;
    float currentGlow = 0;

    //Culling preferences
    double adtTTLWithoutUpdate = 10000; //10 secs by default
    double adtFTLWithoutUpdate = 4; //25 frames by default
    EFreeStrategy adtFreeStrategy = EFreeStrategy::eTimeBased;


    //Stuff to display in UI
    double cullingTimePerFrame = 0;
    double updateTimePerFrame = 0;
    double m2UpdateTime = 0;

    double singleUpdateCNT = 0;
    double meshesCollectCNT = 0;
    double updateBuffersCNT = 0;
    double updateBuffersDeviceCNT = 0;
    double postLoadCNT = 0;
    double textureUploadCNT = 0;
    double drawStageAndDepsCNT = 0;
    double endUpdateCNT = 0;
};


#endif //WOWVIEWERLIB_CONFIG_H
