//
// Created by deamon on 24.12.19.
//

#include <array>

#ifndef AWEBWOWVIEWERCPP_DBSTRUCTS_H
#define AWEBWOWVIEWERCPP_DBSTRUCTS_H
struct MapRecord {
    int ID;
    std::string MapDirectory;
    std::string MapName;
    int WdtFileID;
    int MapType;
};

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

    std::array<float, 3> SkyTopColor;
    float SkyMiddleColor[3];
    float SkyBand1Color[3];
    float SkyBand2Color[3];
    float SkySmogColor[3];
    std::array<float, 3> SkyFogColor;

    //Fog
    float FogEnd;
    float FogScaler;
    float FogDensity;
    float FogHeight;
    float FogHeightScaler;
    float FogHeightDensity;
    float SunFogAngle;

    std::array<float, 3> EndFogColor;
    float EndFogColorDistance;
    std::array<float, 3> SunFogColor;
    float SunFogStrength;
    std::array<float, 3> FogHeightColor;
    float FogHeightCoefficients[4];


    std::string skyBoxName;
    int skyBoxFdid;
    int skyBoxFlags;
    int lightSkyboxId;
    int lightParamId;
    float glow = 0;
    float blendCoef;
    bool isDefault = false;
};

struct LiquidMat {
    int FileDataId;
    std::string texture0Pattern;
    int LVF;
    int OrderIndex;
    float color1[3];
    float color2[3];
    int flags;
    std::array<float,3> minimapStaticCol;
};

struct LiquidTypeData {
    int FileDataId;
    std::string texture0Pattern;
    float color1[3];
    float color2[3];
    int flags;
    std::array<float, 3> minimapStaticCol;
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
