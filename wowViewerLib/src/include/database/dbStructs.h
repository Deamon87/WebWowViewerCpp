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
    float directColor[3];
    float closeRiverColor[3];
    std::string skyBoxName;
    int skyBoxFdid;
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
#endif //AWEBWOWVIEWERCPP_DBSTRUCTS_H
