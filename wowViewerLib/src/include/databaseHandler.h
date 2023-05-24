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
    virtual bool getMapById(int mapId, MapRecord &mapRecord) = 0;
    virtual AreaRecord getArea(int areaId) = 0;
    virtual AreaRecord getWmoArea(int wmoId, int nameId, int groupId) = 0;
    virtual void getLightById(int lightId, int time, LightResult &lightResult) = 0;
    virtual void getEnvInfo(int mapId, float x, float y, float z, int time, std::vector<LightResult> &lightResults) = 0;
    virtual void getLiquidObjectData(int liquidObjectId, int fallbackliquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) = 0;
    virtual void getLiquidTypeData(int liquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) = 0;
    virtual void getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) = 0;
};

#endif //AWEBWOWVIEWERCPP_DATABASEHANDLER_H
