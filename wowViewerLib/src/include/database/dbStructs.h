//
// Created by deamon on 24.12.19.
//

#include <array>
#include <string>
#include <vector>
#include <cstdint>

#ifndef AWEBWOWVIEWERCPP_DBSTRUCTS_H
#define AWEBWOWVIEWERCPP_DBSTRUCTS_H
struct MapRecord {
    int ID;
    std::string MapDirectory;
    std::string MapName;
    int WdtFileID;
    int MapType;
    int overrideTime;
    uint32_t flags0;
    uint32_t flags1;
    uint32_t flags2;
};

struct LightResult {
    int id;
    float pos[3];
    int continentId;
    float fallbackStart;
    float fallbackEnd;
    float lightDistSQR = 0;
    float blendAlpha = 0;
    int lightParamId[8];
};

struct LightTimedData {
    int time = 0;

    int ambientLight = 0 ;
    int horizontAmbientColor = 0;
    int groundAmbientColor = 0;

    int directColor = 0;

    int closeRiverColor = 0;
    int farRiverColor = 0;
    int closeOceanColor = 0;
    int farOceanColor = 0;

    int SkyTopColor = 0;
    int SkyMiddleColor = 0;
    int SkyBand1Color = 0;
    int SkyBand2Color = 0;
    int SkySmogColor = 0;
    int SkyFogColor = 0;

    float FogEnd = 0.0f;
    float FogScaler = 0.0f;
    float FogDensity = 0.0f;
    float FogHeight = 0.0f;
    float FogHeightScaler = 0.0f;
    float FogHeightDensity = 0.0f;
    float FogZScalar = 0.00001f;
    float MainFogStartDist = 0.00001f;
    float MainFogEndDist = 0.00001f;
    float SunFogAngle = 0.0f;
    int EndFogColor = 0;
    float EndFogColorDistance = 0.0f;
    float FogStartOffset = 0.00001f;
    int SunFogColor = 0;
    float SunFogStrength = 0.0f;
    int FogHeightColor = 0;
    int EndFogHeightColor = 0;
    std::array<float, 4> FogHeightCoefficients = {0.f, 0.f, 0.f, 0.f};
    std::array<float, 4> MainFogCoefficients = {0.f, 0.f, 0.f, 0.f};
    std::array<float, 4> HeightDensityFogCoeff = {0.f, 0.f, 0.f, 0.f};
};

struct SkyBoxInfo {
    int id = -1;
    std::string skyBoxName;
    int skyBoxFdid = 0;
    int celectialSkyBoxFdid = 0;
    int skyBoxFlags = 0;
};

struct LightParamData {
    int lightSkyBoxId = 0;
    float glow = 1.0f;
    float waterShallowAlpha = 1.0f;
    float waterDeepAlpha= 1.0f;
    float oceanShallowAlpha = 1.0f;
    float oceanDeepAlpha = 1.0f;
    int lightParamFlags = 0;

    std::array<float, 3> celestialBodyOverride;
    std::array<float, 3> celestialBodyOverride2;

    SkyBoxInfo skyboxInfo;

    std::array<LightTimedData, 2> lightTimedData;
};

/*
struct LightResult {
    int id;
    float ambientColor[3];
    float horizontAmbientColor[3];
    float groundAmbientColor[3];
    float directColor[3];
    float closeRiverColor[3];
    float farRiverColor[3];
    float closeOceanColor[3];
    float farOceanColor[3];

    //Sky
    std::array<float, 3> SkyTopColor;
    float SkyMiddleColor[3];
    float SkyBand1Color[3];
    float SkyBand2Color[3];
    float SkySmogColor[3];
    std::array<float, 3> SkyFogColor;

    //Fog
    float FogStart = 0.0;
    float FogEnd = 0.0;
    float FogScaler = 0.0;
    float FogDensity = 100000;
    float FogHeight = 0.0;
    float FogHeightScaler = 0.0;
    float FogHeightDensity = 100000;
    float SunFogAngle = 0.0; //Blended Separately
    float SunAngleBlend = 0.0f; //Blended Separately

    std::array<float, 3> EndFogColor;
    float EndFogColorDistance;
    std::array<float, 3> SunFogColor;
    float SunFogStrength; //Blended Separately
    std::array<float, 3> FogHeightColor;
    std::array<float, 4> FogHeightCoefficients;
    std::array<float, 4> MainFogCoefficients;
    std::array<float, 4> HeightDensityFogCoefficients;
    float FogZScalar = 0;
    float LegacyFogScalar = 0; //calculated separately
    float MainFogStartDist = 0;
    float MainFogEndDist = 0;
    float FogBlendAlpha = 0.0f; //Set manually in map.cpp //TODO BTW
    std::array<float, 3> HeightEndFogColor;
    float FogStartOffset = 0;

    std::string skyBoxName;
    int skyBoxFdid;
    int skyBoxFlags;
    int lightSkyboxId;
    int lightParamId;
    float glow = 0;
    float blendCoef;
    bool isDefault = false;
};
 */

struct LiquidTextureData {
    int fileDataId;
    int type;
};

struct LiquidTypeAndMat {
    int liquidTypeId;
    std::string name;
    std::array<std::string, 6> texture;
    uint16_t flags;
//    uint8_t soundBank;
//    uint32_t SoundID;
    uint32_t spellID;
//    float maxDarkenDepth;
//    float fogDarkenIntensity;
//    float ambDarkenIntensity;
//    float dirDarkenIntensity;
    uint16_t lightID;
//    float particleScale;
//    uint8_t particleMovement;
//    uint8_t particleTexSlots;
    uint8_t materialID;
    std::array<float,3> minimapStaticCol;
    std::array<uint8_t, 6> frameCountTexture;
    std::array<float,3> color1;
    std::array<float,3> color2;
    std::array<float, 18> m_floats = {0};
    std::array<uint32_t, 4> m_int = {0};
    std::array<float, 4> coefficient;

    uint8_t matFlag;
    uint8_t matLVF;

    float flowSpeed = 0.0f;
    float flowDirection = 0.0f;
    bool fishable = false;
    bool reflection = false;
};

struct vec2 {
    float x; float y;
};
struct ZoneLight {
    int ID;
    std::string name;
    int LightID;
    float Zmin;
    float Zmax;
    std::vector<vec2> points;
};



struct AreaRecord {
    std::string areaName = "";
    int areaId = 0;
    int parentAreaId = 0;
    float ambientMultiplier = 1;
};
#endif //AWEBWOWVIEWERCPP_DBSTRUCTS_H
