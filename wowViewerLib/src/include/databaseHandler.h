//
// Created by Deamon on 12/30/2019.
//

#ifndef AWEBWOWVIEWERCPP_DATABASEHANDLER_H
#define AWEBWOWVIEWERCPP_DATABASEHANDLER_H

#include <vector>
#include <string>
#include "database/dbStructs.h"

class IClientDatabase {
public:
    virtual void getMapArray(std::vector<MapRecord> &mapRecords) = 0;
    virtual std::string getAreaName(int areaId) = 0;
    virtual std::string getWmoAreaName(int wmoId, int nameId, int groupId) = 0;
    virtual void getEnvInfo(int mapId, float x, float y, float z, int time, LightResult &lightResult) = 0;
    virtual void getLiquidObjectData(int liquidObjectId, std::vector<LiquidMat> &loData) = 0;
    virtual void getLiquidTypeData(int liquidTypeId, std::vector<int > &fileDataIds) = 0;
};

#endif //AWEBWOWVIEWERCPP_DATABASEHANDLER_H
