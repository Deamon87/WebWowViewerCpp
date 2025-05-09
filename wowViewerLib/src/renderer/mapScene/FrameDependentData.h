//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRAMEDEPENDENTDATA_H
#define AWEBWOWVIEWERCPP_FRAMEDEPENDENTDATA_H

#include <array>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "mathfu/glsl_mappings.h"

#include "../../include/database/dbStructs.h"

struct FogResult {
    float FogEnd = 0;
    float FogScaler = 0;
    float FogDensity = 0;
    float FogHeight = 0;
    float FogHeightScaler = 0;
    float FogHeightDensity = 0;
    float SunFogAngle = 0;
    mathfu::vec3 FogColor = mathfu::vec3(0, 0, 0);
    mathfu::vec3 EndFogColor = mathfu::vec3(0, 0, 0);
    float EndFogColorDistance = 0;
    mathfu::vec3 SunFogColor = mathfu::vec3(0, 0, 0);
    float SunFogStrength = 0;
    mathfu::vec3 FogHeightColor = mathfu::vec3(0, 0, 0);
    mathfu::vec4 FogHeightCoefficients = mathfu::vec4(0, 0, 0, 0);
    mathfu::vec4 MainFogCoefficients = mathfu::vec4(0, 0, 0, 0);
    mathfu::vec4 HeightDensityFogCoefficients = mathfu::vec4(0, 0, 0, 0);
    float FogZScalar = 0.00001f;
    float LegacyFogScalar = 0.00001f;
    float MainFogStartDist = 0.00001f;
    float MainFogEndDist = 0.00001f;
    float FogBlendAlpha = 0.00001f;
    mathfu::vec3 HeightEndFogColor = mathfu::vec3(0, 0, 0);
    float FogStartOffset = 0.00001f;

    float SunAngleBlend = 1.0f;
};

struct SkyColors {
    mathfu::vec3 SkyTopColor;
    mathfu::vec3 SkyMiddleColor;
    mathfu::vec3 SkyBand1Color;
    mathfu::vec3 SkyBand2Color;
    mathfu::vec3 SkySmogColor;
    mathfu::vec3 SkyFogColor;

    void assignZeros() {
        SkyTopColor     = {0,0,0};
        SkyMiddleColor  = {0,0,0};
        SkyBand1Color   = {0,0,0};
        SkyBand2Color   = {0,0,0};
        SkySmogColor    = {0,0,0};
        SkyFogColor     = {0,0,0};
    }
};

struct SkyBodyData {
    mathfu::vec3 celestialBodyOverride;
    mathfu::vec3 celestialBodyOverride2;

    SkyBoxInfo skyBoxInfo;

    void assignZeros() {
        celestialBodyOverride = {0,0,0};
        celestialBodyOverride2 = {0,0,0};
    }
};

struct LiquidColors {
    mathfu::vec3 closeRiverColor = mathfu::vec3(0,0,0);
    mathfu::vec3 farRiverColor = mathfu::vec3(0,0,0);
    float riverShallowAlpha = 0;
    float riverDeepAlpha = 0;

    mathfu::vec3 closeOceanColor = mathfu::vec3(0,0,0);
    mathfu::vec3 farOceanColor = mathfu::vec3(0,0,0);
    float oceanShallowAlpha = 0;
    float oceanDeepAlpha = 0;

    void assignZeros() {
        closeRiverColor = {0, 0, 0};
        farRiverColor =  {0, 0, 0};
        riverShallowAlpha = 0;
        riverDeepAlpha = 0;

        closeOceanColor =  {0, 0, 0};
        farOceanColor =  {0, 0, 0};
        oceanShallowAlpha = 0;
        oceanDeepAlpha = 0;
    }
};
struct ExteriorColors {
    mathfu::vec3 exteriorAmbientColor =         {1, 1, 1};
    mathfu::vec3 exteriorHorizontAmbientColor = {1, 1, 1};
    mathfu::vec3 exteriorGroundAmbientColor =   {1, 1, 1};
    mathfu::vec3 exteriorDirectColor =          {0.3f,0.3f,0.3f};

    void assignZeros() {
        exteriorAmbientColor =          {0, 0, 0};
        exteriorHorizontAmbientColor =  {0, 0, 0};
        exteriorGroundAmbientColor =    {0, 0, 0};
        exteriorDirectColor =           {0, 0, 0};
    }
};

struct IdAndBlend{
    int id;
    float blend;
};

struct IdAndBlendAndPriority{
    int id;
    float blend;
    int priority ;
};


struct StateForConditions {
    int currentAreaId = 0;
    int currentParentAreaId = 0;
    std::vector<IdAndBlend> currentSkyboxIds = {};
    std::vector<IdAndBlend> currentLightIds = {};
    std::vector<IdAndBlendAndPriority> currentLightParams = {};
    std::vector<IdAndBlend> currentZoneLights = {};
};


struct FrameDependantData {
    //Glow
    float currentGlow;

    //Ambient
    ExteriorColors colors;
    mathfu::vec3 exteriorDirectColorDir;

    StateForConditions stateForConditions;

//Sky params
    bool overrideValuesWithFinalFog = false;
    SkyColors skyColors;

//Planet data
    mathfu::vec3 sunDirection;


//Fog params
    bool FogDataFound = false;
    std::vector<FogResult> fogResults;

//Water params
    bool useMinimapWaterColor;
    bool useCloseRiverColorForDB;

    LiquidColors liquidColors;
};
typedef std::shared_ptr<FrameDependantData> HFrameDependantData;

#endif //AWEBWOWVIEWERCPP_FRAMEDEPENDENTDATA_H
