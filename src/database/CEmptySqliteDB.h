//
// Created by Deamon on 8/1/2021.
//

#ifndef AWEBWOWVIEWERCPP_CEMPTYSQLITEDB_H
#define AWEBWOWVIEWERCPP_CEMPTYSQLITEDB_H


#include "../../wowViewerLib/src/include/databaseHandler.h"

class CEmptySqliteDB : public IClientDatabase {
public:
    void getMapArray(std::vector<MapRecord> &mapRecords) override {};
    bool getMapById(int mapId, MapRecord &mapRecord) override {return false;};
    AreaRecord getArea(int areaId) override { return {}; };
    bool getWmoArea(int wmoId, int nameId, int groupId, AreaRecord &result) override {return false;};
    void getLightById(int lightId, int time, LightResult &lightResult, float zFar) override {};
    void getEnvInfo(int mapId, float x, float y, float z, int time, std::vector<LightResult> &lightResults, float zFar) override {};
    void getLiquidObjectData(int liquidObjectId, int fallbackliquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) override {};
    void getLiquidTypeData(int liquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) override {};
    void getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) override  {};
};


#endif //AWEBWOWVIEWERCPP_CEMPTYSQLITEDB_H
