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
    AreaRecord getWmoArea(int wmoId, int nameId, int groupId) override { return {}; };
    void getLightById(int lightId, int time, LightResult &lightResult) override {};
    void getEnvInfo(int mapId, float x, float y, float z, int time, std::vector<LightResult> &lightResults) override {};
    void getLiquidObjectData(int liquidObjectId, std::vector<LiquidMat> &loData) override {};
    void getLiquidTypeData(int liquidTypeId,  std::vector<LiquidTypeData > &liquidTypeData) override {};
    void getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) override  {};
};


#endif //AWEBWOWVIEWERCPP_CEMPTYSQLITEDB_H
