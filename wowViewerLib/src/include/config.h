//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H

#include <string>
#include <thread>
#include <mathfu/glsl_mappings.h>

struct RiverColorOverride {
    int areaId = -1;
    mathfu::vec4 color = {0,0,0,0};
};

typedef std::vector<RiverColorOverride> RiverColorOverrideHolder;
typedef std::shared_ptr<RiverColorOverrideHolder> HRiverColorOverrideHolder;

enum class EParameterSource: char {
    eNone,
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
    bool renderPortalsIgnoreDepth = false;
    bool usePortalCulling = true;
    bool useInstancing = false;

    bool disableFog = false;
    bool renderSkyDom = true;

    bool drawWmoBB = false;
    bool drawM2BB = false;

    bool doubleCameraDebug = false;
    bool controlSecondCamera = false;
    bool swapMainAndDebug = false;

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

    bool disableGlow = false;

    bool pauseAnimation = false;

    mathfu::vec4 clearColor = {0.117647, 0.207843, 0.392157, 0};

    EParameterSource globalLighting = EParameterSource::eDatabase;
    mathfu::vec4 exteriorAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorHorizontAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorGroundAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorDirectColor = {0.3,0.3,0.3, 0.3};
    mathfu::vec3 exteriorDirectColorDir;

    float adtSpecMult = 1.0;

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
    double adtFTLWithoutUpdate = 6; //25 frames by default
    EFreeStrategy adtFreeStrategy = EFreeStrategy::eTimeBased;


    //Stuff to display in UI
    double cullingTimePerFrame = 0;
    double updateTimePerFrame = 0;
    double mapUpdateTime = 0;
    double m2UpdateTime = 0;
    double wmoGroupUpdateTime = 0;
    double adtUpdateTime = 0;
    double m2calcDistanceTime = 0;
    double adtCleanupTime = 0;

    double mapProduceUpdateTime = 0;
    double interiorViewCollectMeshTime = 0;
    double exteriorViewCollectMeshTime = 0;
    double m2CollectMeshTime = 0;
    double sortMeshTime = 0;
    double collectBuffersTime = 0;
    double sortBuffersTime = 0;

    double startUpdateForNexFrame = 0;
    double singleUpdateCNT = 0;
    double produceDrawStage = 0;
    double meshesCollectCNT = 0;
    double updateBuffersCNT = 0;
    double updateBuffersDeviceCNT = 0;
    double postLoadCNT = 0;
    double textureUploadCNT = 0;
    double drawStageAndDepsCNT = 0;
    double endUpdateCNT = 0;

    double cullCreateVarsCounter = 0;
    double cullGetCurrentWMOCounter = 0;
    double cullGetCurrentZoneCounter = 0;
    double cullUpdateLightsFromDBCounter = 0;
    double cullExterior = 0;
    double cullExteriorSetDecl = 0;
    double cullExteriorWDLCull = 0;
    double cullExteriorGetCands = 0;
    double cullExterioFrustumWMO = 0;
    double cullExterioFrustumM2 = 0;
    double cullSkyDoms = 0;
    double cullCombineAllObjects = 0;

    HRiverColorOverrideHolder colorOverrideHolder = nullptr;
};


#endif //WOWVIEWERLIB_CONFIG_H
