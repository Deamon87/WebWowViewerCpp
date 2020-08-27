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
    AreaRecord getArea(int areaId) override;
    AreaRecord getWmoArea(int wmoId, int nameId, int groupId) override;

    void getEnvInfo(int mapId, float x, float y, float z, int time, std::vector<LightResult> &lightResults) override;
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
        int lightSkyboxId;
        float glow;
        int skyBoxFlags ;
        bool isDefault = false;
    };

    struct InnerLightDataRes {
        int ambientLight;
        int horizontAmbientColor;
        int groundAmbientColor;

        int directLight;
        int closeRiverColor;

        int SkyTopColor;
        int SkyMiddleColor;
        int SkyBand1Color;
        int SkyBand2Color;
        int SkySmogColor;
        int SkyFogColor;

        float FogEnd;
        float FogScaler;
        float FogDensity;
        float FogHeight;
        float FogHeightScaler;
        float FogHeightDensity;
        float SunFogAngle;
        int EndFogColor;
        float EndFogColorDistance;
        int SunFogColor;
        float SunFogStrength;
        int FogHeightColor;
        float FogHeightCoefficients[4];

        int time;
    };

    void convertInnerResultsToPublic(int ptime, std::vector<LightResult> &lightResults, std::vector<InnerLightResult> &innerResults);

    void
    addOnlyOne(LightResult &lightResult, const CSqliteDB::InnerLightDataRes &currLdRes,
               float innerAlpha) const;

    void
    blendTwoAndAdd(LightResult &lightResult, const InnerLightDataRes &lastLdRes,
                   const InnerLightDataRes &currLdRes,
                   float timeAlphaBlend, float innerAlpha) const;
};


#endif //AWEBWOWVIEWERCPP_CSQLITEDB_H
