//
// Created by deamon on 24.12.19.
//

#include <sqlite3.h>
#include "CSqliteDB.h"
#include <cmath>
#include <algorithm>

CSqliteDB::CSqliteDB(std::string dbFileName) :
    m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getWmoAreaAreaName(m_sqliteDatabase,
        "select wat.AreaName_lang as wmoAreaName, at.AreaName_lang from WMOAreaTable wat "
        "left join AreaTable at on at.id = wat.AreaTableID "
        "where wat.WMOID = ? and wat.NameSetID = ? and (wat.WMOGroupID = -1 or wat.WMOGroupID = ?) ORDER BY wat.WMOGroupID DESC"),
    getAreaNameStatement(m_sqliteDatabase,
        "select at.AreaName_lang from AreaTable at where at.ID = ?"),

    getLightStatement(m_sqliteDatabase,
        "select l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd, LightParamsID_0 from Light l"
        " where "
        "  (l.ContinentID = ?) and (("
        "    abs(l.GameCoords_0 - ?) < l.GameFalloffEnd and\n"
        "    abs(l.GameCoords_1 - ?) < l.GameFalloffEnd and\n"
        "    abs(l.GameCoords_2 - ?) < l.GameFalloffEnd\n"
        "  ) or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0 )) ORDER BY ID desc "),
    getLightData(m_sqliteDatabase,
        "select ld.AmbientColor, ld.DirectColor, ld.Time from LightData ld where ld.LightParamID = ? ORDER BY Time ASC"
        )

{
    char *sErrMsg = "";
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);

}

void CSqliteDB::getMapArray(std::vector<MapRecord> &mapRecords) {
    SQLite::Statement getMapList(m_sqliteDatabase, "select m.ID, m.Directory, m.WdtFileDataID, m.MapType from Map m where m.WdtFileDataID > 0");

    while (getMapList.executeStep())
    {
        MapRecord mapRecord;

        // Demonstrate how to get some typed column value
        mapRecord.ID = getMapList.getColumn(0);
        mapRecord.MapName = std::string((const char*) getMapList.getColumn(1));
        mapRecord.WdtFileID = getMapList.getColumn(2);
        mapRecord.MapType = getMapList.getColumn(3);

        mapRecords.push_back(mapRecord);
    }

}

std::string CSqliteDB::getAreaName(int areaId) {
    getAreaNameStatement.reset();

    getAreaNameStatement.bind(1, areaId);
    while (getAreaNameStatement.executeStep()) {
        std::string areaName = getAreaNameStatement.getColumn(0);

        return areaName;
    }


    return std::string();
}

std::string CSqliteDB::getWmoAreaName(int wmoId, int nameId, int groupId) {
    getWmoAreaAreaName.reset();

    getWmoAreaAreaName.bind(1, wmoId);
    getWmoAreaAreaName.bind(2, nameId);
    getWmoAreaAreaName.bind(3, groupId);

    while (getWmoAreaAreaName.executeStep()) {
        std::string wmoAreaName = getWmoAreaAreaName.getColumn(0);
        std::string areaName = getWmoAreaAreaName.getColumn(1);

        if (wmoAreaName == "") {
            return areaName;
        } else {
            return wmoAreaName;
        }
    }

    return "";
}

template <int T>
float getFloatFromInt(int value) {
    if (T == 0) {
        return (value & 0xFF) / 255.0f;
    }
    if (T == 1) {
        return ((value >> 8) & 0xFF) / 255.0f;
    }
    if (T == 2) {
        return ((value >> 16) & 0xFF) / 255.0f;
    }
}


void CSqliteDB::getEnvInfo(int mapId, float x, float y, float z, int ptime, LightResult &lightResult) {
    getLightStatement.reset();

    getLightStatement.bind(1, mapId);
    getLightStatement.bind(2, x);
    getLightStatement.bind(3, y);
    getLightStatement.bind(4, z);

struct InnerLightResult {
    float pos[3];
    float fallbackStart;
    float fallbackEnd;
    float blendAlpha = 0;
    int paramId;
};

    std::vector<InnerLightResult> innerResults;

    float totalBlend = 0;
    while (getLightStatement.executeStep()) {
        InnerLightResult ilr;
        ilr.pos[0] = getLightStatement.getColumn(0).getDouble();
        ilr.pos[1] = getLightStatement.getColumn(1).getDouble();
        ilr.pos[2] = getLightStatement.getColumn(2).getDouble();
        ilr.fallbackStart = getLightStatement.getColumn(3).getDouble();
        ilr.fallbackEnd = getLightStatement.getColumn(4).getDouble();
        ilr.paramId = getLightStatement.getColumn(5).getDouble();

        bool defaultRec = false;
        if (ilr.pos[0] == 0 && ilr.pos[1] == 0 && ilr.pos[2] == 0 ) {
            if (totalBlend > 1) continue;
            defaultRec = true;
        }

        if (!defaultRec) {
            float distanceToLight =
                std::sqrt(
                    (ilr.pos[0] - x) * (ilr.pos[0] - x) +
                    (ilr.pos[1] - y) * (ilr.pos[1] - y) +
                    (ilr.pos[2] - z) * (ilr.pos[2] - z));

            ilr.blendAlpha = 1.0f - (distanceToLight - ilr.fallbackStart) / (ilr.fallbackEnd - ilr.fallbackStart);
            if (ilr.blendAlpha <= 0) continue;

            totalBlend += ilr.blendAlpha;
        } else {
            ilr.blendAlpha = 1.0f - totalBlend;
        }



        innerResults.push_back(ilr);
    }

    //From lowest to highest
    std::sort(innerResults.begin(), innerResults.end(), [](const InnerLightResult &a, const InnerLightResult &b) {
        return a.blendAlpha - b.blendAlpha;
    });

    struct InnerLightDataRes {
        int ambientLight;
        int directLight;
        int time;
    };

    lightResult.ambientColor[0] = 0;
    lightResult.ambientColor[1] = 0;
    lightResult.ambientColor[2] = 0;

    lightResult.directColor[0] = 0;
    lightResult.directColor[1] = 0;
    lightResult.directColor[2] = 0;


    float totalSummator = 0.0f;
    for (int i = 0; i < innerResults.size() && totalSummator < 1.0f; i++) {
        auto &innerResult = innerResults[i];

        getLightData.reset();

        getLightData.bind(1, innerResult.paramId);

        InnerLightDataRes lastLdRes = {0, 0, -1};
        bool assigned = false;
        float innerAlpha = innerResult.blendAlpha < 1.0 ? innerResult.blendAlpha : 1.0;
        if (totalSummator + innerAlpha > 1.0f) {
            innerAlpha = 1.0f - totalSummator;
        }

        while (getLightData.executeStep()) {
            InnerLightDataRes currLdRes;
            currLdRes.ambientLight = getLightData.getColumn(0);
            currLdRes.directLight = getLightData.getColumn(1);
            currLdRes.time = getLightData.getColumn(2);
            if (currLdRes.time > ptime) {
                assigned = true;



                if (lastLdRes.time == -1) {
                    //Set as is
                    lightResult.ambientColor[0] += getFloatFromInt<0>(currLdRes.ambientLight) * innerAlpha;
                    lightResult.ambientColor[1] += getFloatFromInt<1>(currLdRes.ambientLight) * innerAlpha;
                    lightResult.ambientColor[2] += getFloatFromInt<2>(currLdRes.ambientLight) * innerAlpha;

                    lightResult.directColor[0] += getFloatFromInt<0>(currLdRes.directLight) * innerAlpha;
                    lightResult.directColor[1] += getFloatFromInt<1>(currLdRes.directLight) * innerAlpha;
                    lightResult.directColor[2] += getFloatFromInt<2>(currLdRes.directLight) * innerAlpha;
                } else {
                    //Blend using time as alpha
                    float timeAlphaBlend = 1.0f - (((float)currLdRes.time - (float)ptime) / ((float)currLdRes.time - (float)lastLdRes.time));

                    lightResult.ambientColor[0] += (getFloatFromInt<0>(currLdRes.ambientLight) * timeAlphaBlend + getFloatFromInt<0>(lastLdRes.ambientLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.ambientColor[1] += (getFloatFromInt<1>(currLdRes.ambientLight) * timeAlphaBlend + getFloatFromInt<1>(lastLdRes.ambientLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.ambientColor[2] += (getFloatFromInt<2>(currLdRes.ambientLight) * timeAlphaBlend + getFloatFromInt<2>(lastLdRes.ambientLight)*(1.0 - timeAlphaBlend)) * innerAlpha;

                    lightResult.directColor[0] += (getFloatFromInt<0>(currLdRes.directLight) * timeAlphaBlend + getFloatFromInt<0>(lastLdRes.directLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.directColor[1] += (getFloatFromInt<1>(currLdRes.directLight) * timeAlphaBlend + getFloatFromInt<1>(lastLdRes.directLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.directColor[2] += (getFloatFromInt<2>(currLdRes.directLight) * timeAlphaBlend + getFloatFromInt<2>(lastLdRes.directLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                }
                break;
            }
            lastLdRes = currLdRes;
        }

        if (!assigned) {
            lightResult.ambientColor[0] += getFloatFromInt<0>(lastLdRes.ambientLight) * innerAlpha;
            lightResult.ambientColor[1] += getFloatFromInt<1>(lastLdRes.ambientLight) * innerAlpha;
            lightResult.ambientColor[2] += getFloatFromInt<2>(lastLdRes.ambientLight) * innerAlpha;

            lightResult.directColor[0] += getFloatFromInt<0>(lastLdRes.directLight) * innerAlpha;
            lightResult.directColor[1] += getFloatFromInt<1>(lastLdRes.directLight) * innerAlpha;
            lightResult.directColor[2] += getFloatFromInt<2>(lastLdRes.directLight) * innerAlpha;
        }

        totalSummator+= innerResult.blendAlpha;
    }

}
