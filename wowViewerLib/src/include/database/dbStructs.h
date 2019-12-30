//
// Created by deamon on 24.12.19.
//

#ifndef AWEBWOWVIEWERCPP_DBSTRUCTS_H
#define AWEBWOWVIEWERCPP_DBSTRUCTS_H
struct MapRecord {
    int ID;
    std::string MapName;
    int WdtFileID;
    int MapType;
};

struct LightResult {
    float ambientColor[3];
    float directColor[3];
    std::string skyBoxName;
    int skyBoxFdid;
};
#endif //AWEBWOWVIEWERCPP_DBSTRUCTS_H
