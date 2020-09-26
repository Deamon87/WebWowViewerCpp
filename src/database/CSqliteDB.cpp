//
// Created by deamon on 24.12.19.
//

#include <sqlite3.h>
#include "CSqliteDB.h"
#include <cmath>
#include <algorithm>
#include <iostream>

CSqliteDB::CSqliteDB(std::string dbFileName) :
    m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getWmoAreaAreaName(m_sqliteDatabase,
        "select wat.AreaName_lang as wmoAreaName, at.AreaName_lang, at.ID, at.ParentAreaID, at.Ambient_multiplier from WMOAreaTable wat "
        "left join AreaTable at on at.id = wat.AreaTableID "
        "where wat.WMOID = ? and wat.NameSetID = ? and (wat.WMOGroupID = -1 or wat.WMOGroupID = ?) ORDER BY wat.WMOGroupID DESC"),
    getAreaNameStatement(m_sqliteDatabase,
        "select at.AreaName_lang, at.ID, at.ParentAreaID, at.Ambient_multiplier from AreaTable at where at.ID = ?"),

    getLightStatement(m_sqliteDatabase,
        "select l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd, l.LightParamsID_0, IFNULL(ls.SkyboxFileDataID, 0), IFNULL(lp.LightSkyboxID, 0), lp.Glow, IFNULL(ls.Flags, 0) from Light l "
        "    left join LightParams lp on lp.ID = l.LightParamsID_0 "
        "    left join LightSkybox ls on ls.ID = lp.LightSkyboxID "
        " where  "
        "   ((l.ContinentID = ?) and (( "
        "    abs(l.GameCoords_0 - ?) < l.GameFalloffEnd and "
        "    abs(l.GameCoords_1 - ?) < l.GameFalloffEnd and "
        "    abs(l.GameCoords_2 - ?) < l.GameFalloffEnd "
        "  ) or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0))) "
        "    or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0 and l.ContinentID = 0)  "
        "ORDER BY l.ID desc"),
    getLightByIdStatement(m_sqliteDatabase,
        "select l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd, l.LightParamsID_0, IFNULL(ls.SkyboxFileDataID, 0), IFNULL(lp.LightSkyboxID, 0), lp.Glow, IFNULL(ls.Flags, 0) from Light l "
        "    left join LightParams lp on lp.ID = l.LightParamsID_0 "
        "    left join LightSkybox ls on ls.ID = lp.LightSkyboxID "
        " where l.ID = ?"),
    getLightData(m_sqliteDatabase,
        "select ld.AmbientColor, ld.HorizonAmbientColor, ld.GroundAmbientColor, ld.DirectColor, ld.RiverCloseColor, "
        "ld.SkyTopColor, ld.SkyMiddleColor, ld.SkyBand1Color, ld.SkyBand2Color, ld.SkySmogColor, ld.SkyFogColor, "
        "ld.FogEnd, ld.FogScaler, ld.FogDensity, ld.FogHeight, ld.FogHeightScaler, ld.FogHeightDensity, ld.SunFogAngle, "
        "ld.EndFogColor, ld.EndFogColorDistance, ld.SunFogColor, ld.SunFogStrength, ld.FogHeightColor, "
        "ld.FogHeightCoefficients_0, ld.FogHeightCoefficients_1, ld.FogHeightCoefficients_2, ld.FogHeightCoefficients_3, "
        "ld.Time from LightData ld "
        "where ld.LightParamID = ? ORDER BY Time ASC"
        ),
    getLiquidObjectInfo(m_sqliteDatabase,
        "select ltxt.FileDataID, lm.LVF, ltxt.OrderIndex, lt.Color_0 from LiquidObject lo "
        "left join LiquidTypeXTexture ltxt on ltxt.LiquidTypeID = lo.LiquidTypeID "
        "left join LiquidType lt on lt.ID = lo.LiquidTypeID "
        "left join LiquidMaterial lm on lt.MaterialID = lm.ID "
        "where lo.ID = ? "
    ),
    getLiquidTypeInfo(m_sqliteDatabase,
        "select ltxt.FileDataID from LiquidTypeXTexture ltxt "
        "where ltxt.LiquidTypeID = ? order by ltxt.OrderIndex"
    ),
    getZoneLightInfo(m_sqliteDatabase,
        "select ID, Name, LightID, Zmin, Zmax from ZoneLight where MapID = ?"
    ),
    getZoneLightPointsInfo(m_sqliteDatabase,
        "select Pos_0, Pos_1 from ZoneLightPoint where ZoneLightID = ? order by PointOrder;"
    )


{
    char *sErrMsg = "";
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA temp_store = MEMORY", NULL, NULL, &sErrMsg);
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA schema.journal_mode = MEMORY", NULL, NULL, &sErrMsg);

}

void CSqliteDB::getMapArray(std::vector<MapRecord> &mapRecords) {
    SQLite::Statement getMapList(m_sqliteDatabase, "select m.ID, m.Directory, m.MapName_lang, m.WdtFileDataID, m.MapType from Map m where m.WdtFileDataID > 0");

    while (getMapList.executeStep())
    {
        MapRecord mapRecord;

        // Demonstrate how to get some typed column value
        mapRecord.ID = getMapList.getColumn(0);
        mapRecord.MapDirectory = std::string((const char*) getMapList.getColumn(1));
        mapRecord.MapName = std::string((const char*) getMapList.getColumn(2));
        mapRecord.WdtFileID = getMapList.getColumn(3);
        mapRecord.MapType = getMapList.getColumn(4);

        mapRecords.push_back(mapRecord);
    }

}

AreaRecord CSqliteDB::getArea(int areaId) {
    AreaRecord areaRecord;
    getAreaNameStatement.reset();

    getAreaNameStatement.bind(1, areaId);
    while (getAreaNameStatement.executeStep()) {
        areaRecord.areaName = getAreaNameStatement.getColumn(0).getString();
        areaRecord.areaId = getAreaNameStatement.getColumn(1).getInt();
        areaRecord.parentAreaId = getAreaNameStatement.getColumn(2).getInt();
        areaRecord.ambientMultiplier= getAreaNameStatement.getColumn(3).getDouble();

        break;
    }


    return areaRecord;
}

AreaRecord CSqliteDB::getWmoArea(int wmoId, int nameId, int groupId) {
    AreaRecord areaRecord;

    getWmoAreaAreaName.reset();

    getWmoAreaAreaName.bind(1, wmoId);
    getWmoAreaAreaName.bind(2, nameId);
    getWmoAreaAreaName.bind(3, groupId);

    while (getWmoAreaAreaName.executeStep()) {
        std::string wmoAreaName = getWmoAreaAreaName.getColumn(0);
        std::string areaName = getWmoAreaAreaName.getColumn(1);


        if (wmoAreaName == "") {
            areaRecord.areaName = areaName;
        } else {
            areaRecord.areaName = wmoAreaName;
        }
        areaRecord.areaId = getWmoAreaAreaName.getColumn(2).getInt();
        areaRecord.parentAreaId = getWmoAreaAreaName.getColumn(3).getInt();
        areaRecord.ambientMultiplier= getWmoAreaAreaName.getColumn(4).getDouble();

        break;
    }

    return areaRecord;
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

void initWithZeros(float *colorF) {
    colorF[0] = 0;
    colorF[1] = 0;
    colorF[2] = 0;
}
void initWithZeros(std::array<float, 3> &colorF) {
    colorF[0] = 0;
    colorF[1] = 0;
    colorF[2] = 0;
}
void blendTwoAndAdd(float *colorF, int currLdRes, int lastLdRes, float timeAlphaBlend, float innerAlpha) {
    colorF[0] += (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<0>(lastLdRes) *
                                    (1.0 - timeAlphaBlend)) * innerAlpha;
    colorF[1] += (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<1>(lastLdRes) *
                                    (1.0 - timeAlphaBlend)) * innerAlpha;
    colorF[2] += (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<2>(lastLdRes) *
                                    (1.0 - timeAlphaBlend)) * innerAlpha;
}
void blendTwoAndAdd(std::array<float, 3> &colorF, int currLdRes, int lastLdRes, float timeAlphaBlend, float innerAlpha) {
    colorF[0] += (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<0>(lastLdRes) *
                  (1.0 - timeAlphaBlend)) * innerAlpha;
    colorF[1] += (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<1>(lastLdRes) *
                  (1.0 - timeAlphaBlend)) * innerAlpha;
    colorF[2] += (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<2>(lastLdRes) *
                  (1.0 - timeAlphaBlend)) * innerAlpha;
}
void blendTwoAndAdd(float &colorF, float currLdRes, float lastLdRes, float timeAlphaBlend, float innerAlpha) {
    colorF += (currLdRes * timeAlphaBlend + lastLdRes * (1.0 - timeAlphaBlend)) * innerAlpha;
}

void addOnlyOne(float *colorF, int currLdRes, float innerAlpha) {
    colorF[0] += getFloatFromInt<0>(currLdRes) * innerAlpha;
    colorF[1] += getFloatFromInt<1>(currLdRes) * innerAlpha;
    colorF[2] += getFloatFromInt<2>(currLdRes) * innerAlpha;
}

void addOnlyOne(std::array<float, 3> &colorF, int currLdRes, float innerAlpha) {
    colorF[0] += getFloatFromInt<0>(currLdRes) * innerAlpha;
    colorF[1] += getFloatFromInt<1>(currLdRes) * innerAlpha;
    colorF[2] += getFloatFromInt<2>(currLdRes) * innerAlpha;
}

void addOnlyOne(float &colorF, float currLdRes, float innerAlpha) {
    colorF += currLdRes * innerAlpha;
}

void CSqliteDB::getLightById(int lightId, int time, LightResult &lightResult) {
    getLightByIdStatement.reset();

    getLightByIdStatement.bind(1, lightId);

    std::vector<InnerLightResult> innerResults;


    while (getLightByIdStatement.executeStep()) {
        InnerLightResult ilr;
        ilr.pos[0] = getLightByIdStatement.getColumn(0).getDouble();
        ilr.pos[1] = getLightByIdStatement.getColumn(1).getDouble();
        ilr.pos[2] = getLightByIdStatement.getColumn(2).getDouble();
        ilr.fallbackStart = getLightByIdStatement.getColumn(3).getDouble();
        ilr.fallbackEnd = getLightByIdStatement.getColumn(4).getDouble();
        ilr.paramId = getLightByIdStatement.getColumn(5).getInt();
        ilr.skyBoxFileId = getLightByIdStatement.getColumn(6).getInt();
        ilr.lightSkyboxId = getLightByIdStatement.getColumn(7).getInt();
        ilr.glow = getLightByIdStatement.getColumn(8).getDouble();
        ilr.skyBoxFlags = getLightByIdStatement.getColumn(9).getInt();

        ilr.blendAlpha = 1.0f;

        innerResults.push_back(ilr);
    }
    std::vector<LightResult> lightResults;
    convertInnerResultsToPublic(time, lightResults, innerResults);
    if (lightResults.size() > 0) {
        lightResult = lightResults[0];
    }

};
void CSqliteDB::getEnvInfo(int mapId, float x, float y, float z, int ptime, std::vector<LightResult> &lightResults) {
    getLightStatement.reset();

    getLightStatement.bind(1, mapId);
    getLightStatement.bind(2, x);
    getLightStatement.bind(3, y);
    getLightStatement.bind(4, z);

    std::vector<InnerLightResult> innerResults;

    float totalBlend = 0;
    while (getLightStatement.executeStep()) {
        InnerLightResult ilr;
        ilr.pos[0] = getLightStatement.getColumn(0).getDouble();
        ilr.pos[1] = getLightStatement.getColumn(1).getDouble();
        ilr.pos[2] = getLightStatement.getColumn(2).getDouble();
        ilr.fallbackStart = getLightStatement.getColumn(3).getDouble();
        ilr.fallbackEnd = getLightStatement.getColumn(4).getDouble();
        ilr.paramId = getLightStatement.getColumn(5).getInt();
        ilr.skyBoxFileId = getLightStatement.getColumn(6).getInt();
        ilr.lightSkyboxId = getLightStatement.getColumn(7).getInt();
        ilr.glow = getLightStatement.getColumn(8).getDouble();
        ilr.skyBoxFlags = getLightStatement.getColumn(9).getInt();

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
            ilr.isDefault = true;
        }



        innerResults.push_back(ilr);
    }
    convertInnerResultsToPublic(ptime, lightResults, innerResults);


}
void CSqliteDB::addOnlyOne(LightResult &lightResult,
                           const CSqliteDB::InnerLightDataRes &currLdRes,
                           float innerAlpha) const {//Set as is
    ::addOnlyOne(lightResult.ambientColor, currLdRes.ambientLight, innerAlpha);
    ::addOnlyOne(lightResult.horizontAmbientColor, currLdRes.horizontAmbientColor, innerAlpha);
    ::addOnlyOne(lightResult.groundAmbientColor, currLdRes.groundAmbientColor, innerAlpha);

    ::addOnlyOne(lightResult.directColor, currLdRes.directLight, innerAlpha);
    ::addOnlyOne(lightResult.closeRiverColor, currLdRes.closeRiverColor, innerAlpha);

    ::addOnlyOne(lightResult.SkyTopColor, currLdRes.SkyTopColor, innerAlpha);
    ::addOnlyOne(lightResult.SkyMiddleColor, currLdRes.SkyMiddleColor, innerAlpha);
    ::addOnlyOne(lightResult.SkyBand1Color, currLdRes.SkyBand1Color, innerAlpha);
    ::addOnlyOne(lightResult.SkyBand2Color, currLdRes.SkyBand2Color, innerAlpha);
    ::addOnlyOne(lightResult.SkySmogColor, currLdRes.SkySmogColor, innerAlpha);
    ::addOnlyOne(lightResult.SkyFogColor, currLdRes.SkyFogColor, innerAlpha);

    ::addOnlyOne(lightResult.FogEnd, currLdRes.FogEnd, innerAlpha);
    ::addOnlyOne(lightResult.FogScaler, currLdRes.FogScaler, innerAlpha);
    ::addOnlyOne(lightResult.FogDensity, currLdRes.FogDensity, innerAlpha);
    ::addOnlyOne(lightResult.FogHeight, currLdRes.FogHeight, innerAlpha);
    ::addOnlyOne(lightResult.FogHeightScaler, currLdRes.FogHeightScaler, innerAlpha);
    ::addOnlyOne(lightResult.FogHeightDensity, currLdRes.FogHeightDensity, innerAlpha);
    ::addOnlyOne(lightResult.SunFogAngle, currLdRes.SunFogAngle, innerAlpha);
    ::addOnlyOne(lightResult.EndFogColor, currLdRes.EndFogColor, innerAlpha);
    ::addOnlyOne(lightResult.EndFogColorDistance, currLdRes.EndFogColorDistance, innerAlpha);
    ::addOnlyOne(lightResult.SunFogColor, currLdRes.SunFogColor, innerAlpha);
    ::addOnlyOne(lightResult.SunFogStrength, currLdRes.SunFogStrength, innerAlpha);
    ::addOnlyOne(lightResult.FogHeightColor, currLdRes.FogHeightColor, innerAlpha);
    ::addOnlyOne(lightResult.FogHeightCoefficients[0], currLdRes.FogHeightCoefficients[0], innerAlpha);
    ::addOnlyOne(lightResult.FogHeightCoefficients[1], currLdRes.FogHeightCoefficients[1], innerAlpha);
    ::addOnlyOne(lightResult.FogHeightCoefficients[2], currLdRes.FogHeightCoefficients[2], innerAlpha);
    ::addOnlyOne(lightResult.FogHeightCoefficients[3], currLdRes.FogHeightCoefficients[3], innerAlpha);
}
void CSqliteDB::blendTwoAndAdd(LightResult &lightResult, const CSqliteDB::InnerLightDataRes &lastLdRes,
                               const CSqliteDB::InnerLightDataRes &currLdRes, float timeAlphaBlend,
                               float innerAlpha) const {
    ::blendTwoAndAdd(lightResult.ambientColor,
                     currLdRes.ambientLight, lastLdRes.ambientLight,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.horizontAmbientColor,
                     currLdRes.horizontAmbientColor, lastLdRes.horizontAmbientColor,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.groundAmbientColor,
                     currLdRes.groundAmbientColor, lastLdRes.groundAmbientColor,
                     timeAlphaBlend, innerAlpha);

    ::blendTwoAndAdd(lightResult.directColor,
                     currLdRes.directLight, lastLdRes.directLight,
                     timeAlphaBlend, innerAlpha);

    ::blendTwoAndAdd(lightResult.closeRiverColor,
                     currLdRes.closeRiverColor, lastLdRes.closeRiverColor,
                     timeAlphaBlend, innerAlpha);

    ::blendTwoAndAdd(lightResult.SkyTopColor,
                     currLdRes.SkyTopColor, lastLdRes.SkyTopColor,
                     timeAlphaBlend, innerAlpha);

    ::blendTwoAndAdd(lightResult.SkyMiddleColor,
                     currLdRes.SkyMiddleColor, lastLdRes.SkyMiddleColor,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SkyBand1Color,
                     currLdRes.SkyBand1Color, lastLdRes.SkyBand1Color,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SkyBand2Color,
                     currLdRes.SkyBand2Color, lastLdRes.SkyBand2Color,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SkySmogColor,
                     currLdRes.SkySmogColor, lastLdRes.SkySmogColor,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SkyFogColor,
                     currLdRes.SkyFogColor, lastLdRes.SkyFogColor,
                     timeAlphaBlend, innerAlpha);

    ::blendTwoAndAdd(lightResult.FogEnd, currLdRes.FogEnd, lastLdRes.FogEnd, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogScaler, currLdRes.FogScaler, lastLdRes.FogScaler, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogDensity, currLdRes.FogDensity, lastLdRes.FogDensity, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeight, currLdRes.FogHeight, lastLdRes.FogHeight, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightScaler, currLdRes.FogHeightScaler, lastLdRes.FogHeightScaler, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightDensity, currLdRes.FogHeightDensity, lastLdRes.FogHeightDensity, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SunFogAngle, currLdRes.SunFogAngle, lastLdRes.SunFogAngle, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.EndFogColor, currLdRes.EndFogColor, lastLdRes.EndFogColor, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.EndFogColorDistance, currLdRes.EndFogColorDistance, lastLdRes.EndFogColorDistance, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SunFogColor, currLdRes.SunFogColor, lastLdRes.SunFogColor, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.SunFogStrength, currLdRes.SunFogStrength, lastLdRes.SunFogStrength, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightColor, currLdRes.FogHeightColor, lastLdRes.FogHeightColor, timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[0], currLdRes.FogHeightCoefficients[0], lastLdRes.FogHeightCoefficients[0], timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[1], currLdRes.FogHeightCoefficients[1], lastLdRes.FogHeightCoefficients[1], timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[2], currLdRes.FogHeightCoefficients[2], lastLdRes.FogHeightCoefficients[2], timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[3], currLdRes.FogHeightCoefficients[3], lastLdRes.FogHeightCoefficients[3], timeAlphaBlend, innerAlpha);
}

void CSqliteDB::convertInnerResultsToPublic(int ptime, std::vector<LightResult> &lightResults,
                                            std::vector<CSqliteDB::InnerLightResult> &innerResults)  {

    //From lowest to highest
    std::sort(innerResults.begin(), innerResults.end(), [](const InnerLightResult &a, const InnerLightResult &b) {
        return a.blendAlpha > b.blendAlpha;
    });


    float totalSummator = 0.0f;
    for (int i = 0; i < innerResults.size() && totalSummator < 1.0f; i++) {
        LightResult lightResult;
        initWithZeros(lightResult.ambientColor);
        initWithZeros(lightResult.horizontAmbientColor);
        initWithZeros(lightResult.groundAmbientColor);
        initWithZeros(lightResult.directColor);
        initWithZeros(lightResult.closeRiverColor);

        initWithZeros(lightResult.SkyTopColor);
        initWithZeros(lightResult.SkyMiddleColor);
        initWithZeros(lightResult.SkyBand1Color);
        initWithZeros(lightResult.SkyBand2Color);
        initWithZeros(lightResult.SkySmogColor);
        initWithZeros(lightResult.SkyFogColor);

        initWithZeros(lightResult.EndFogColor);
        initWithZeros(lightResult.SunFogColor);
        initWithZeros(lightResult.FogHeightColor);
        initWithZeros(lightResult.FogHeightCoefficients);
        lightResult.FogHeightCoefficients[3] = 0;

        lightResult.skyBoxFdid = 0;
        lightResult.lightSkyboxId = 0;


        auto &innerResult = innerResults[i];
        lightResult.isDefault = innerResult.isDefault;

        getLightData.reset();

        getLightData.bind(1, innerResult.paramId);

//        std::cout << "innerResult.paramId = " << innerResult.paramId << std::endl;

        InnerLightDataRes lastLdRes = {0, 0, -1};
        bool assigned = false;
        float innerAlpha = innerResult.blendAlpha < 1.0 ? innerResult.blendAlpha : 1.0;
        if (totalSummator + innerAlpha > 1.0f) {
            innerAlpha = 1.0f - totalSummator;
        }
        lightResult.blendCoef = innerAlpha;

        lightResult.skyBoxFdid = innerResult.skyBoxFileId;
        lightResult.skyBoxFlags = innerResult.skyBoxFlags;
        lightResult.lightSkyboxId = innerResult.lightSkyboxId;
        lightResult.glow = innerResult.glow;

        while (getLightData.executeStep()) {
            InnerLightDataRes currLdRes;
            currLdRes.ambientLight = getLightData.getColumn(0);
            currLdRes.horizontAmbientColor = getLightData.getColumn(1);
            if (currLdRes.horizontAmbientColor == 0) {
                currLdRes.horizontAmbientColor = currLdRes.ambientLight;
            }
            currLdRes.groundAmbientColor = getLightData.getColumn(2);
            if (currLdRes.groundAmbientColor == 0) {
                currLdRes.groundAmbientColor = currLdRes.ambientLight;
            }

            currLdRes.directLight = getLightData.getColumn(3);
            currLdRes.closeRiverColor = getLightData.getColumn(4);

            currLdRes.SkyTopColor = getLightData.getColumn(5);
            currLdRes.SkyMiddleColor = getLightData.getColumn(6);
            currLdRes.SkyBand1Color = getLightData.getColumn(7);
            currLdRes.SkyBand2Color = getLightData.getColumn(8);
            currLdRes.SkySmogColor = getLightData.getColumn(9);
            currLdRes.SkyFogColor = getLightData.getColumn(10);

            currLdRes.FogEnd = getLightData.getColumn(11).getDouble();
            currLdRes.FogScaler = getLightData.getColumn(12).getDouble();
            currLdRes.FogDensity = getLightData.getColumn(13).getDouble();
            currLdRes.FogHeight = getLightData.getColumn(14).getDouble();
            currLdRes.FogHeightScaler = getLightData.getColumn(15).getDouble();
            currLdRes.FogHeightDensity = getLightData.getColumn(16).getDouble();
            currLdRes.SunFogAngle = getLightData.getColumn(17).getDouble();
            currLdRes.EndFogColor = getLightData.getColumn(18);
            currLdRes.EndFogColorDistance = getLightData.getColumn(19).getDouble();
            currLdRes.SunFogColor = getLightData.getColumn(20);
            currLdRes.SunFogStrength = getLightData.getColumn(21).getDouble();
            currLdRes.FogHeightColor = getLightData.getColumn(22);
            currLdRes.FogHeightCoefficients[0] = getLightData.getColumn(23).getDouble();
            currLdRes.FogHeightCoefficients[1] = getLightData.getColumn(24).getDouble();
            currLdRes.FogHeightCoefficients[2] = getLightData.getColumn(25).getDouble();
            currLdRes.FogHeightCoefficients[3] = getLightData.getColumn(26).getDouble();

            currLdRes.time = getLightData.getColumn(27);

            if (!currLdRes.EndFogColor) {
                currLdRes.EndFogColor = currLdRes.SkyFogColor;
            }
            if (!currLdRes.FogHeightColor) {
                currLdRes.FogHeightColor = currLdRes.SkyFogColor;
            }
            if (currLdRes.FogHeightScaler == 0.0) {
                currLdRes.FogHeightDensity = currLdRes.FogDensity;
            }

            if (currLdRes.time > ptime) {
                assigned = true;

                if (lastLdRes.time == -1) {
                    addOnlyOne(lightResult, currLdRes, innerAlpha);

                } else {
                    //Blend using time as alpha
                    float timeAlphaBlend = 1.0f - (((float) currLdRes.time - (float) ptime) /
                                                   ((float) currLdRes.time - (float) lastLdRes.time));

                    blendTwoAndAdd(lightResult, lastLdRes, currLdRes, timeAlphaBlend, innerAlpha);
                }
                break;
            }
            lastLdRes = currLdRes;
        }

        if (!assigned) {
            addOnlyOne(lightResult, lastLdRes, innerAlpha);
        }

        lightResult.FogColor = lightResult.SkyFogColor;
        lightResults.push_back(lightResult);

        totalSummator += innerResult.blendAlpha;
    }
}




void CSqliteDB::getLiquidObjectData(int liquidObjectId, std::vector<LiquidMat> &loData) {
    getLiquidObjectInfo.reset();

    getLiquidObjectInfo.bind(1, liquidObjectId);

    while (getLiquidObjectInfo.executeStep()) {
        LiquidMat lm = {};
        lm.FileDataId = getLiquidObjectInfo.getColumn(0).getInt();
        lm.LVF = getLiquidObjectInfo.getColumn(1).getInt();
        lm.OrderIndex = getLiquidObjectInfo.getColumn(2).getInt();
        int color1 = getLiquidObjectInfo.getColumn(3).getInt();
        lm.color1[0] = getFloatFromInt<0>(color1);
        lm.color1[1] = getFloatFromInt<1>(color1);
        lm.color1[2] = getFloatFromInt<2>(color1);

        loData.push_back(lm);
    }
}

void CSqliteDB::getLiquidTypeData(int liquidTypeId, std::vector<int > &fileDataIds) {
    getLiquidTypeInfo.reset();

    getLiquidTypeInfo.bind(1, liquidTypeId);
    while (getLiquidTypeInfo.executeStep()) {
        fileDataIds.push_back(getLiquidTypeInfo.getColumn(0).getInt());
    }

}
void CSqliteDB::getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) {
// "select ID, Name, LightID, Zmin, Zmax from ZoneLight where MapID = ?"
    zoneLights = {};
    getZoneLightInfo.reset();

    getZoneLightInfo.bind(1, mapId);
    while (getZoneLightInfo.executeStep()) {
        ZoneLight zoneLight;

        zoneLight.ID = getZoneLightInfo.getColumn(0).getInt();
        zoneLight.name = getZoneLightInfo.getColumn(1).getName();
        zoneLight.LightID = getZoneLightInfo.getColumn(2).getInt();
        zoneLight.Zmin = getZoneLightInfo.getColumn(3).getDouble();
        zoneLight.Zmax = getZoneLightInfo.getColumn(4).getDouble();

        zoneLights.push_back(zoneLight);
    }

    for (auto &zoneLight : zoneLights) {
        getZoneLightPointsInfo.reset();
        getZoneLightPointsInfo.bind(1, zoneLight.ID);

        while (getZoneLightPointsInfo.executeStep()) {
            vec2 pt;
            pt.x = getZoneLightPointsInfo.getColumn(0).getDouble();
            pt.y = getZoneLightPointsInfo.getColumn(1).getDouble();

            zoneLight.points.push_back(pt);
        }
    }
}