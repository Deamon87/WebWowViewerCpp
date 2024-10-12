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
    mathfu::vec4 SkyTopColor;
    mathfu::vec4 SkyMiddleColor;
    mathfu::vec4 SkyBand1Color;
    mathfu::vec4 SkyBand2Color;
    mathfu::vec4 SkySmogColor;
    mathfu::vec4 SkyFogColor;
};

struct SkyBodyData {
    mathfu::vec3 celestialBodyOverride;
    mathfu::vec3 celestialBodyOverride2;

    SkyBoxInfo skyBoxInfo;
};

struct LiquidColors {
    mathfu::vec4 closeRiverColor_shallowAlpha = mathfu::vec4(0,0,0,0);
    mathfu::vec4 farRiverColor_deepAlpha = mathfu::vec4(0,0,0,0);

    mathfu::vec4 closeOceanColor_shallowAlpha = mathfu::vec4(0,0,0,0);
    mathfu::vec4 farOceanColor_deepAlpha = mathfu::vec4(0,0,0,0);
};
struct ExteriorColors {
    mathfu::vec4 exteriorAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorHorizontAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorGroundAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorDirectColor = {0.3,0.3,0.3, 0.3};
};

struct IdAndBlend{
    int id;
    float blend;
};

struct StateForConditions {
    int currentAreaId = 0;
    int currentParentAreaId = 0;
    std::vector<IdAndBlend> currentSkyboxIds = {};
    std::vector<IdAndBlend> currentLightIds = {};
    std::vector<IdAndBlend> currentLightParams = {};
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
