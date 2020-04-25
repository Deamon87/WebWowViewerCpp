//
// Created by deamon on 24.12.19.
//

#ifndef AWEBWOWVIEWERCPP_CSQLITEDB_H
#define AWEBWOWVIEWERCPP_CSQLITEDB_H

#include <vector>
#include <SQLiteCpp/Database.h>
#include "../../wowViewerLib/src/include/databaseHandler.h"


class CSqliteDB : public IClientDatabase {
public:
    CSqliteDB(std::string dbFileName);
    void getMapArray(std::vector<MapRecord> &mapRecords) override;
    std::string getAreaName(int areaId) override;
    std::string getWmoAreaName(int wmoId, int nameId, int groupId) override;

    void getEnvInfo(int mapId, float x, float y, float z, int time, LightResult &lightResult) override;
    void getLightById(int lightId, int time, LightResult &lightResult) override;
    void getLiquidObjectData(int liquidObjectId, std::vector<LiquidMat> &loData) override;
    void getLiquidTypeData(int liquidTypeId, std::vector<int > &fileDataIds) override;
    void getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) override;
private:
    SQLite::Database m_sqliteDatabase;

    SQLite::Statement getWmoAreaAreaName;
    SQLite::Statement getAreaNameStatement;
    SQLite::Statement getLightStatement;
    SQLite::Statement getLightByIdStatement;
    SQLite::Statement getLightData;
    SQLite::Statement getLiquidObjectInfo;
    SQLite::Statement getLiquidTypeInfo;

    SQLite::Statement getZoneLightInfo;
    SQLite::Statement getZoneLightPointsInfo;


    struct InnerLightResult {
        float pos[3];
        float fallbackStart;
        float fallbackEnd;
        float blendAlpha = 0;
        int paramId;
        int skyBoxFileId;
    };

    void convertInnerResultsToPublic(int ptime, LightResult &lightResult, std::vector<InnerLightResult> &innerResults);
};


#endif //AWEBWOWVIEWERCPP_CSQLITEDB_H
