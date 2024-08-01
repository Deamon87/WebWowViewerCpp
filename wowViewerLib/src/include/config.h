//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mathfu/glsl_mappings.h>
#include "../renderer/mapScene/FrameDependentData.h"

struct RiverColorOverride {
    int liquidObjectId;
    int liquidType;
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
        m_hardwareThreadCount = std::max<int>((int)std::thread::hardware_concurrency()-3, 1);
    }

public:
    auto hardwareThreadCount() const -> int { return m_hardwareThreadCount;}
    bool renderAdt = true;
    bool renderWMO = true;
    bool renderM2 = true;
    bool renderSkyDom = true;
    bool renderSkyScene = true;
    bool renderLiquid = true;
    bool renderBSP = false;
    bool renderPortals = false;
    bool renderAntiPortals = false;
    bool renderPortalsIgnoreDepth = false;
    bool usePortalCulling = true;
    bool discardInvisibleMeshes = true;

    bool ignoreADTHoles = false;


    bool disableFog = false;


    bool drawWmoBB = false;
    bool drawM2BB = false;

    bool doubleCameraDebug = false;
    bool controlSecondCamera = false;
    bool swapMainAndDebug = false;

    bool BCLightHack = false;

    bool enableLightBuffer = true;

    bool drawDepthBuffer = false;
    int cameraM2 = -1; // this will be sceneNumber of object

    int wmoMinBatch = 0;
    int wmoMaxBatch = 9999;

    int m2MinBatch = 0;
    int m2MaxBatch = 9999;

    int minParticle = 0;
    int maxParticle = 9999;

    uint16_t currentTime = 0;

    bool useWotlkLogic = false;

    float nearPlane = 1;
    float farPlane = 1000;
    float farPlaneForCulling = 400;

    float fogDensityIncreaser = 0;

    bool disableGlow = false;

    bool pauseAnimation = false;

    mathfu::vec4 clearColor = {0.117647, 0.207843, 0.392157, 0};

    EParameterSource globalLighting = EParameterSource::eDatabase;
    ExteriorColors exteriorColors;
    mathfu::vec3 exteriorDirectColorDir;

    float adtSpecMult = 1.0;

    bool useMinimapWaterColor = false;
    bool useCloseRiverColorForDB = false;
    EParameterSource waterColorParams = EParameterSource::eDatabase;
    mathfu::vec4 closeRiverColor = {1,1,1,1};
    mathfu::vec4 farRiverColor = {1,1,1,1};
    mathfu::vec4 closeOceanColor = {1,1,1,1};
    mathfu::vec4 farOceanColor = {1,1,1,1};

    EParameterSource skyParams = EParameterSource::eDatabase;
    SkyColors skyColors;

    EParameterSource globalFog = EParameterSource::eDatabase;
    FogResult fogResult;

    int diffuseColorHack = 0;

    EParameterSource glowSource = EParameterSource::eDatabase;
    float currentGlow = 0;

    //Culling preferences
    double adtTTLWithoutUpdate = 10000; //10 secs by default
    double adtFTLWithoutUpdate = 6; //25 frames by default
    EFreeStrategy adtFreeStrategy = EFreeStrategy::eTimeBased;

    HRiverColorOverrideHolder colorOverrideHolder = nullptr;
private:
    int m_hardwareThreadCount;
};

//ADT STUFF FOR MAP GENERATION
typedef std::array<int,2> AdtCell;

struct AdtCellCompare
{
    bool operator()(const AdtCell &a, const AdtCell &b) const {
        return a[0] == b[0] && a[1] == b[1];
    }
};

struct AdtCellHasher {
    std::size_t operator()(const AdtCell &k) const {
        using std::hash;
        return hash<int>{}(k[0]) ^ (hash<int>{}(k[1]) << 16);
    };
};

struct ADTRenderConfigData {
    ADTRenderConfigData() {
        std::array<mathfu::vec3, 64> defaultMax = {};
        std::array<mathfu::vec3, 64> defaultMin = {};
        defaultMax.fill({-200000, -200000, -200000});
        defaultMin.fill({200000, 200000, 200000});

        adtMax.fill(defaultMax);
        adtMin.fill(defaultMin);
    }
    std::array<std::array<mathfu::vec3, 64>, 64> adtMin;
    std::array<std::array<mathfu::vec3, 64>, 64> adtMax;
    std::unordered_set<AdtCell, AdtCellHasher, AdtCellCompare> excludedADTs;
    std::unordered_map<AdtCell, std::unordered_set<AdtCell, AdtCellHasher, AdtCellCompare>, AdtCellHasher> excludedChunksPerADTs;
};

typedef std::shared_ptr<ADTRenderConfigData> HADTRenderConfigDataHolder;



#endif //WOWVIEWERLIB_CONFIG_H
