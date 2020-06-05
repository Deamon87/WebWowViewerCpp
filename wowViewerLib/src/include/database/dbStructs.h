//
// Created by deamon on 24.12.19.
//

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
    float ambientColor[3];
    float horizontAmbientColor[3];
    float groundAmbientColor[3];
    float directColor[3];
    float closeRiverColor[3];

    float SkyTopColor[3];
    float SkyMiddleColor[3];
    float SkyBand1Color[3];
    float SkyBand2Color[3];
    float SkySmogColor[3];
    float SkyFogColor[3];


    std::string skyBoxName;
    int skyBoxFdid;
    int lightSkyboxId;
    float glow;
    float blendCoef;
    bool isDefault = false;
};

struct LiquidMat {
    int FileDataId;
    int LVF;
    int OrderIndex;
    float color1[3];
    float color2[3];
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
