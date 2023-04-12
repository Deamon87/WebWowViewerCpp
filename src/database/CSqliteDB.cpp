//
// Created by deamon on 24.12.19.
//

#include <sqlite3.h>
#include "CSqliteDB.h"
#include <cmath>
#include <algorithm>
#include <iostream>

CSqliteDB::StatementFieldHolder::StatementFieldHolder(SQLite::Database &database, const std::string &query) :
    m_query(database, query) {

    for (int i = 0; i < m_query.getColumnCount(); i++) {
        fieldToIndex[CalculateFNV(m_query.getColumnName(i))] = i;
    }
}

void CSqliteDB::StatementFieldHolder::setInputs() {
    this->m_query.reset();
}

template<class... Ts>
void CSqliteDB::StatementFieldHolder::setInputs(Ts &&... inputs) {
    int i = 1;
    this->m_query.reset();
    ((this->m_query.bind(i++, inputs)), ...);
}

bool CSqliteDB::StatementFieldHolder::execute() {
    return m_query.executeStep();
}
SQLite::Column CSqliteDB::StatementFieldHolder::getField(const HashedString fieldName) {
    int index = getFieldIndex(fieldName);

    if (index >= 0) {
        return m_query.getColumn(index);
    } else {
        throw SQLite::Exception("Field was not found in query");
    }
}


const std::string getMapListSQL =
        "select m.ID, m.Directory, m.MapName_lang, m.WdtFileDataID, m.MapType from Map m where m.WdtFileDataID > 0";

const std::string getMapListSQL_classic =
        "select m.ID, m.Directory, m.MapName_lang, m.MapType from Map m";

const std::string getMapByIDSQL =
        "select m.ID, m.Directory, m.MapName_lang, m.WdtFileDataID, m.MapType from Map m where m.ID = ?";

const std::string getMapByIDSQL_classic =
        "select m.ID, m.Directory, m.MapName_lang, m.MapType from Map m";

const std::string getWmoAreaAreaNameSQL = R"===(
        select wat.AreaName_lang as wmoAreaName, at.AreaName_lang, at.ID, at.ParentAreaID, at.Ambient_multiplier from WMOAreaTable wat
        left join AreaTable at on at.id = wat.AreaTableID
        where wat.WMOID = ? and wat.NameSetID = ? and (wat.WMOGroupID = -1 or wat.WMOGroupID = ?) ORDER BY wat.WMOGroupID DESC
        )===";

const std::string getLightByIdSQL = R"===(
        select
        l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd, l.LightParamsID_0,
        IFNULL(ls.SkyboxFileDataID, 0) as SkyboxFileDataID, IFNULL(lp.LightSkyboxID, 0) as LightSkyboxID,
        lp.Glow, IFNULL(ls.Flags, 0) as SkyboxFlags
        from Light l
        left join LightParams lp on lp.ID = l.LightParamsID_0
        left join LightSkybox ls on ls.ID = lp.LightSkyboxID
        where l.ID = ?
)===";

const std::string getLightSQL = R"===(
        select
        l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd, l.LightParamsID_0,
        IFNULL(ls.SkyboxFileDataID, 0) as SkyboxFileDataID, IFNULL(lp.LightSkyboxID, 0) as LightSkyboxID,
        lp.Glow, IFNULL(ls.Flags, 0) as SkyboxFlags
        from Light l
            left join LightParams lp on lp.ID = l.LightParamsID_0
            left join LightSkybox ls on ls.ID = lp.LightSkyboxID
         where
           ((l.ContinentID = ?) and ((
            abs(l.GameCoords_0 - ?) < l.GameFalloffEnd and
            abs(l.GameCoords_1 - ?) < l.GameFalloffEnd and
            abs(l.GameCoords_2 - ?) < l.GameFalloffEnd
          ) or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0)))
            or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0 and l.ContinentID = 0)
        ORDER BY l.ID desc
)===";

const std::string lightDataSQL = R"===(
        select ld.AmbientColor, ld.HorizonAmbientColor, ld.GroundAmbientColor, ld.DirectColor,
        ld.RiverCloseColor, ld.RiverFarColor, ld.OceanCloseColor, ld.OceanFarColor,
        ld.SkyTopColor, ld.SkyMiddleColor, ld.SkyBand1Color, ld.SkyBand2Color, ld.SkySmogColor, ld.SkyFogColor,
        ld.FogEnd, ld.FogScaler, ld.FogDensity, ld.FogHeight, ld.FogHeightScaler, ld.FogHeightDensity, ld.SunFogAngle,
        ld.EndFogColor, ld.EndFogColorDistance, ld.SunFogColor, ld.SunFogStrength, ld.FogHeightColor,
        ld.FogHeightCoefficients_0, ld.FogHeightCoefficients_1, ld.FogHeightCoefficients_2, ld.FogHeightCoefficients_3,
        ld.Time from LightData ld
        where ld.LightParamID = ? ORDER BY Time ASC
        )===";

const std::string liquidObjectInfoSQL = R"===(
        select ltxt.FileDataID, lm.LVF, ltxt.OrderIndex, lt.Color_0, lt.Color_1, lt.Flags, lt.MinimapStaticCol, lo.FlowSpeed, lt.MaterialID from LiquidObject lo
        left join LiquidTypeXTexture ltxt on ltxt.LiquidTypeID = lo.LiquidTypeID
        left join LiquidType lt on lt.ID = lo.LiquidTypeID
        left join LiquidMaterial lm on lt.MaterialID = lm.ID
        where lo.ID = ?
        )===";

const std::string liquidObjectInfoSQL_classic = R"===(
        select lt.Texture_0, lm.LVF, lt.Color_0, lt.Color_1, lt.Flags, lt.MinimapStaticCol, lo.FlowSpeed, lt.MaterialID from LiquidObject lo
        left join LiquidType lt on lt.ID = lo.LiquidTypeID
        left join LiquidMaterial lm on lt.MaterialID = lm.ID
        where lo.ID = ?
        )===";

const std::string liquidTypeSQL =   R"===(
        select ltxt.FileDataID, lt.Color_0, lt.Color_1, lt.Flags, lt.MinimapStaticCol, lm.LVF, lt.MaterialID from LiquidType lt
        left join LiquidTypeXTexture ltxt on ltxt.LiquidTypeID = lt.ID
        left join LiquidMaterial lm on lt.MaterialID = lm.ID
        where lt.ID = ? order by ltxt.OrderIndex
        )===";
const std::string liquidTypeSQL_classic =   R"===(
        select lt.Texture_0, lt.Color_0, lt.Color_1, lt.Flags, lt.MinimapStaticCol, lm.LVF, lt.MaterialID from LiquidType lt
        left join LiquidMaterial lm on lt.MaterialID = lm.ID
        where lt.ID = ?
        )===";

CSqliteDB::CSqliteDB(std::string dbFileName) :
    m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getWmoAreaAreaName(m_sqliteDatabase, getWmoAreaAreaNameSQL),
    getAreaNameStatement(m_sqliteDatabase,
        "select at.AreaName_lang, at.ID, at.ParentAreaID, at.Ambient_multiplier from AreaTable at where at.ID = ?"),

    getLightStatement(m_sqliteDatabase, getLightSQL),
    getLightByIdStatement(m_sqliteDatabase, getLightByIdSQL),

    getLightData(m_sqliteDatabase, lightDataSQL),
    getLiquidObjectInfo(m_sqliteDatabase,
        getHasLiquidTypeXTexture(m_sqliteDatabase) ? liquidObjectInfoSQL : liquidObjectInfoSQL_classic
    ),
    getLiquidTypeInfo(m_sqliteDatabase,
        getHasLiquidTypeXTexture(m_sqliteDatabase) ? liquidTypeSQL : liquidTypeSQL_classic
    ),
    getZoneLightInfo(m_sqliteDatabase,
        "select ID, Name, LightID, Zmin, Zmax from ZoneLight where MapID = ?"
    ),
    getZoneLightPointsInfo(m_sqliteDatabase,
        "select Pos_0, Pos_1 from ZoneLightPoint where ZoneLightID = ? order by PointOrder DESC;"
    ),
    getMapList(m_sqliteDatabase, getHasWDTId(m_sqliteDatabase) ? getMapListSQL : getMapListSQL_classic),
    getMapByIdStatement(m_sqliteDatabase, getHasWDTId(m_sqliteDatabase) ? getMapByIDSQL : getMapByIDSQL_classic)
{
    char *sErrMsg = "";
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA temp_store = MEMORY", NULL, NULL, &sErrMsg);
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA schema.journal_mode = MEMORY", NULL, NULL, &sErrMsg);
}

void CSqliteDB::getMapArray(std::vector<MapRecord> &mapRecords) {
    getMapList.setInputs();

    bool hasWDTId = getMapList.getFieldIndex("WdtFileDataID") >= 0;

    while (getMapList.execute())
    {
        MapRecord &mapRecord = mapRecords.emplace_back();

        // Demonstrate how to get some typed column value
        mapRecord.ID = getMapList.getField("ID").getInt();
        mapRecord.MapDirectory = getMapList.getField("Directory").getString();
        mapRecord.MapName = getMapList.getField("MapName_lang").getString();
        if (hasWDTId) {
            mapRecord.WdtFileID = getMapList.getField("WdtFileDataID").getInt();
        } else {
            mapRecord.WdtFileID = -1;
        }
        mapRecord.MapType = getMapList.getField("MapType").getInt();
    }

}

bool CSqliteDB::getMapById(int mapId, MapRecord &mapRecord) {
    getMapByIdStatement.setInputs( mapId );

    bool hasWDTId = getMapByIdStatement.getFieldIndex("WdtFileDataID") >= 0;

    while (getMapByIdStatement.execute())
    {
        mapRecord.ID = getMapByIdStatement.getField("ID").getInt();
        mapRecord.MapDirectory = getMapByIdStatement.getField("Directory").getString();
        mapRecord.MapName = getMapByIdStatement.getField("MapName_lang").getString();

        if (hasWDTId) {
            mapRecord.WdtFileID = getMapByIdStatement.getField("WdtFileDataID").getInt();
        } else {
            mapRecord.WdtFileID = -1;
        }
        mapRecord.MapType = getMapByIdStatement.getField("MapType").getInt();

        return true;
    }
    return false;
}

AreaRecord CSqliteDB::getArea(int areaId) {
    AreaRecord areaRecord;

    getAreaNameStatement.setInputs( areaId );

    while (getAreaNameStatement.execute()) {
        areaRecord.areaName = getAreaNameStatement.getField("AreaName_lang").getString();
        areaRecord.areaId = getAreaNameStatement.getField("ID").getInt();
        areaRecord.parentAreaId = getAreaNameStatement.getField("ParentAreaID").getInt();
        areaRecord.ambientMultiplier= getAreaNameStatement.getField("Ambient_multiplier").getDouble();

        break;
    }

    return areaRecord;
}

AreaRecord CSqliteDB::getWmoArea(int wmoId, int nameId, int groupId) {
    AreaRecord areaRecord;

    getWmoAreaAreaName.setInputs( wmoId, nameId, groupId);

    while (getWmoAreaAreaName.execute()) {
        std::string wmoAreaName = getWmoAreaAreaName.getField("wmoAreaName").getString();
        std::string areaName = getWmoAreaAreaName.getField("AreaName_lang").getString();


        if (wmoAreaName == "") {
            areaRecord.areaName = areaName;
        } else {
            areaRecord.areaName = wmoAreaName;
        }
        areaRecord.areaId = getWmoAreaAreaName.getField("ID").getInt();
        areaRecord.parentAreaId = getWmoAreaAreaName.getField("ParentAreaID").getInt();
        areaRecord.ambientMultiplier= getWmoAreaAreaName.getField("Ambient_multiplier").getDouble();

        break;
    }

    return areaRecord;
}

template <int T>
float getFloatFromInt(int value) {
    if constexpr (T == 0) {
        return (value & 0xFF) / 255.0f;
    }
    if constexpr (T == 1) {
        return ((value >> 8) & 0xFF) / 255.0f;
    }
    if constexpr (T == 2) {
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
                                    (1.0f - timeAlphaBlend)) * innerAlpha;
    colorF[1] += (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<1>(lastLdRes) *
                                    (1.0f - timeAlphaBlend)) * innerAlpha;
    colorF[2] += (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<2>(lastLdRes) *
                                    (1.0f - timeAlphaBlend)) * innerAlpha;
}
inline void blendTwoAndAdd(std::array<float, 3> &colorF, int currLdRes, int lastLdRes, float timeAlphaBlend, float innerAlpha) {
    colorF[0] += (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<0>(lastLdRes) *
                  (1.0f - timeAlphaBlend)) * innerAlpha;
    colorF[1] += (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<1>(lastLdRes) *
                  (1.0f - timeAlphaBlend)) * innerAlpha;
    colorF[2] += (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<2>(lastLdRes) *
                  (1.0f - timeAlphaBlend)) * innerAlpha;
}

inline void blendTwoAndAdd(float &colorF, float currLdRes, float lastLdRes, float timeAlphaBlend, float innerAlpha) {
    colorF += (currLdRes * timeAlphaBlend + lastLdRes * (1.0f - timeAlphaBlend)) * innerAlpha;
}

inline void addOnlyOne(float *colorF, int currLdRes, float innerAlpha) {
    colorF[0] += getFloatFromInt<0>(currLdRes) * innerAlpha;
    colorF[1] += getFloatFromInt<1>(currLdRes) * innerAlpha;
    colorF[2] += getFloatFromInt<2>(currLdRes) * innerAlpha;
}

inline void addOnlyOne(std::array<float, 3> &colorF, int currLdRes, float innerAlpha) {
    colorF[0] += getFloatFromInt<0>(currLdRes) * innerAlpha;
    colorF[1] += getFloatFromInt<1>(currLdRes) * innerAlpha;
    colorF[2] += getFloatFromInt<2>(currLdRes) * innerAlpha;
}

inline void addOnlyOne(float &colorF, float currLdRes, float innerAlpha) {
    colorF += currLdRes * innerAlpha;
}
void CSqliteDB::getLightById(int lightId, int time, LightResult &lightResult) {
    getLightByIdStatement.setInputs( lightId );

    std::vector<InnerLightResult> innerResults;
    while (getLightByIdStatement.execute()) {
        InnerLightResult &ilr = innerResults.emplace_back();
        ilr.pos[0] = getLightByIdStatement.getField("GameCoords_0").getDouble();
        ilr.pos[1] = getLightByIdStatement.getField("GameCoords_1").getDouble();
        ilr.pos[2] = getLightByIdStatement.getField("GameCoords_2").getDouble();
        ilr.fallbackStart = getLightByIdStatement.getField("GameFalloffStart").getDouble();
        ilr.fallbackEnd = getLightByIdStatement.getField("GameFalloffEnd").getDouble();
        ilr.paramId = getLightByIdStatement.getField("LightParamsID_0").getInt();
        ilr.skyBoxFileId = getLightByIdStatement.getField("SkyboxFileDataID").getInt();
        ilr.lightSkyboxId = getLightByIdStatement.getField("LightSkyboxID").getInt();
        ilr.glow = getLightByIdStatement.getField("Glow").getDouble();
        ilr.skyBoxFlags = getLightByIdStatement.getField("SkyboxFlags").getInt();

        ilr.blendAlpha = 1.0f;
    }
    std::vector<LightResult> lightResults;
    convertInnerResultsToPublic(time, lightResults, innerResults);
    if (lightResults.size() > 0) {
        lightResult = lightResults[0];
    }

};
void CSqliteDB::getEnvInfo(int mapId, float x, float y, float z, int ptime, std::vector<LightResult> &lightResults) {
    getLightStatement.setInputs( mapId, x, y, z );

    std::vector<InnerLightResult> innerResults;

    float totalBlend = 0;
    while (getLightStatement.execute()) {
        InnerLightResult &ilr = innerResults.emplace_back();
        ilr.pos[0] = getLightStatement.getField("GameCoords_0").getDouble();
        ilr.pos[1] = getLightStatement.getField("GameCoords_1").getDouble();
        ilr.pos[2] = getLightStatement.getField("GameCoords_2").getDouble();
        ilr.fallbackStart = getLightStatement.getField("GameFalloffStart").getDouble();
        ilr.fallbackEnd = getLightStatement.getField("GameFalloffEnd").getDouble();
        ilr.paramId = getLightStatement.getField("LightParamsID_0").getInt();
        ilr.skyBoxFileId = getLightStatement.getField("SkyboxFileDataID").getInt();
        ilr.lightSkyboxId = getLightStatement.getField("LightSkyboxID").getInt();
        ilr.glow = getLightStatement.getField("Glow").getDouble();
        ilr.skyBoxFlags = getLightStatement.getField("SkyboxFlags").getInt();

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
    ::addOnlyOne(lightResult.farRiverColor, currLdRes.farRiverColor, innerAlpha);
    ::addOnlyOne(lightResult.closeOceanColor, currLdRes.closeOceanColor, innerAlpha);
    ::addOnlyOne(lightResult.farOceanColor, currLdRes.farOceanColor, innerAlpha);

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
    ::blendTwoAndAdd(lightResult.farRiverColor,
                     currLdRes.farRiverColor, lastLdRes.farRiverColor,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.closeOceanColor,
                     currLdRes.closeOceanColor, lastLdRes.closeOceanColor,
                     timeAlphaBlend, innerAlpha);
    ::blendTwoAndAdd(lightResult.farOceanColor,
                     currLdRes.farOceanColor, lastLdRes.farOceanColor,
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
        initWithZeros(lightResult.farRiverColor);
        initWithZeros(lightResult.closeOceanColor);
        initWithZeros(lightResult.farOceanColor);

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
        lightResult.lightParamId = 0;

        lightResult.FogEnd = 0.0;
        lightResult.FogScaler = 0.0;
        lightResult.FogDensity = 0.0;
        lightResult.FogHeight = 0.0;
        lightResult.FogHeightScaler = 0.0;
        lightResult.FogHeightDensity = 0.0;
        lightResult.SunFogAngle = 0.0;
        lightResult.EndFogColorDistance = 0.0;
        lightResult.SunFogStrength = 0.0;



        auto &innerResult = innerResults[i];
        lightResult.isDefault = innerResult.isDefault;
        lightResult.lightParamId = innerResult.paramId;

        getLightData.setInputs( innerResult.paramId );

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

        while (getLightData.execute()) {
            InnerLightDataRes currLdRes;
            currLdRes.ambientLight = getLightData.getField("AmbientColor");
            currLdRes.horizontAmbientColor = getLightData.getField("HorizonAmbientColor");
            if (currLdRes.horizontAmbientColor == 0) {
                currLdRes.horizontAmbientColor = currLdRes.ambientLight;
            }
            currLdRes.groundAmbientColor = getLightData.getField("GroundAmbientColor");
            if (currLdRes.groundAmbientColor == 0) {
                currLdRes.groundAmbientColor = currLdRes.ambientLight;
            }

            currLdRes.directLight = getLightData.getField("DirectColor");

            currLdRes.closeRiverColor = getLightData.getField("RiverCloseColor");
            currLdRes.farRiverColor = getLightData.getField("RiverFarColor");
            currLdRes.closeOceanColor = getLightData.getField("OceanCloseColor");
            currLdRes.farOceanColor = getLightData.getField("OceanFarColor");

            currLdRes.SkyTopColor = getLightData.getField("SkyTopColor");
            currLdRes.SkyMiddleColor = getLightData.getField("SkyMiddleColor");
            currLdRes.SkyBand1Color = getLightData.getField("SkyBand1Color");
            currLdRes.SkyBand2Color = getLightData.getField("SkyBand2Color");
            currLdRes.SkySmogColor = getLightData.getField("SkySmogColor");
            currLdRes.SkyFogColor = getLightData.getField("SkyFogColor");

            currLdRes.FogEnd = getLightData.getField("FogEnd").getDouble();
            currLdRes.FogScaler = getLightData.getField("FogScaler").getDouble();
            currLdRes.FogDensity = getLightData.getField("FogDensity").getDouble();
            currLdRes.FogHeight = getLightData.getField("FogHeight").getDouble();
            currLdRes.FogHeightScaler = getLightData.getField("FogHeightScaler").getDouble();
            currLdRes.FogHeightDensity = getLightData.getField("FogHeightDensity").getDouble();
            currLdRes.SunFogAngle = getLightData.getField("SunFogAngle").getDouble();
            currLdRes.EndFogColor = getLightData.getField("EndFogColor").getInt();
            currLdRes.EndFogColorDistance = getLightData.getField("EndFogColorDistance").getDouble();
            currLdRes.SunFogColor = getLightData.getField("SunFogColor").getInt();
            currLdRes.SunFogStrength = getLightData.getField("SunFogStrength").getDouble();
            currLdRes.FogHeightColor = getLightData.getField("FogHeightColor").getInt();
            currLdRes.FogHeightCoefficients[0] = getLightData.getField("FogHeightCoefficients_0").getDouble();
            currLdRes.FogHeightCoefficients[1] = getLightData.getField("FogHeightCoefficients_1").getDouble();
            currLdRes.FogHeightCoefficients[2] = getLightData.getField("FogHeightCoefficients_2").getDouble();
            currLdRes.FogHeightCoefficients[3] = getLightData.getField("FogHeightCoefficients_3").getDouble();

            currLdRes.time = getLightData.getField("Time").getInt();

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


        lightResults.push_back(lightResult);

        totalSummator += innerResult.blendAlpha;
    }
}

void CSqliteDB::getLiquidObjectData(int liquidObjectId, std::vector<LiquidMat> &loData) {
    getLiquidObjectInfo.setInputs( liquidObjectId );

    bool hasFileDataId = getLiquidObjectInfo.getFieldIndex("FileDataID") >= 0;

    while (getLiquidObjectInfo.execute()) {
        LiquidMat &lm = loData.emplace_back();

        if (hasFileDataId) {
            lm.FileDataId = getLiquidObjectInfo.getField("FileDataID").getInt();
            lm.OrderIndex = getLiquidObjectInfo.getField("OrderIndex").getInt();
        } else {
            lm.texture0Pattern = getLiquidObjectInfo.getField("Texture_0").getInt();
        }

        lm.LVF = getLiquidObjectInfo.getField("LVF").getInt();
        lm.flowSpeed = getLiquidObjectInfo.getField("FlowSpeed").getDouble();
        lm.materialId = getLiquidObjectInfo.getField("MaterialID").getInt();
        int color1 = getLiquidObjectInfo.getField("Color_0").getInt();
        lm.color1[0] = getFloatFromInt<0>(color1);
        lm.color1[1] = getFloatFromInt<1>(color1);
        lm.color1[2] = getFloatFromInt<2>(color1);
        int color2 = getLiquidObjectInfo.getField("Color_1").getInt();
        lm.color2[0] = getFloatFromInt<0>(color2);
        lm.color2[1] = getFloatFromInt<1>(color2);
        lm.color2[2] = getFloatFromInt<2>(color2);
        lm.flags = getLiquidObjectInfo.getField("Flags").getInt();
        int minimapStaticCol = getLiquidObjectInfo.getField("MinimapStaticCol").getInt();
        lm.minimapStaticCol[0] = getFloatFromInt<0>(minimapStaticCol);
        lm.minimapStaticCol[1] = getFloatFromInt<1>(minimapStaticCol);
        lm.minimapStaticCol[2] = getFloatFromInt<2>(minimapStaticCol);
    }
}
void CSqliteDB::getLiquidTypeData(int liquidTypeId, std::vector<LiquidTypeData > &liquidTypeData) {
    getLiquidTypeInfo.setInputs( liquidTypeId );

    bool hasFileDataId = getLiquidObjectInfo.getFieldIndex("FileDataID") >= 0;

    while (getLiquidTypeInfo.execute()) {
        LiquidTypeData &ltd = liquidTypeData.emplace_back();

        if (hasFileDataId) {
            ltd.FileDataId = getLiquidTypeInfo.getField("FileDataID").getInt();
        } else {
            ltd.texture0Pattern = getLiquidTypeInfo.getField("Texture_0").getInt();
        }
        ltd.materialId = getLiquidTypeInfo.getField("MaterialID").getInt();

        int color1 = getLiquidTypeInfo.getField("Color_0").getInt();
        ltd.color1[0] = getFloatFromInt<0>(color1);
        ltd.color1[1] = getFloatFromInt<1>(color1);
        ltd.color1[2] = getFloatFromInt<2>(color1);
        int color2 = getLiquidTypeInfo.getField("Color_1").getInt();
        ltd.color2[0] = getFloatFromInt<0>(color2);
        ltd.color2[1] = getFloatFromInt<1>(color2);
        ltd.color2[2] = getFloatFromInt<2>(color2);
        ltd.flags = getLiquidTypeInfo.getField("Flags").getInt();
        int minimapStaticCol = getLiquidTypeInfo.getField("MinimapStaticCol").getInt();
        ltd.minimapStaticCol[0] = getFloatFromInt<0>(minimapStaticCol);
        ltd.minimapStaticCol[1] = getFloatFromInt<1>(minimapStaticCol);
        ltd.minimapStaticCol[2] = getFloatFromInt<2>(minimapStaticCol);

        ltd.LVF = getLiquidTypeInfo.getField("LVF").getInt();
    }

}

void CSqliteDB::getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) {

    zoneLights.clear();
    getZoneLightInfo.setInputs( mapId );
    while (getZoneLightInfo.execute()) {
        ZoneLight &zoneLight = zoneLights.emplace_back();

        zoneLight.ID = getZoneLightInfo.getField("ID").getInt();
        zoneLight.name = getZoneLightInfo.getField("Name").getName();
        zoneLight.LightID = getZoneLightInfo.getField("LightID").getInt();
        zoneLight.Zmin = getZoneLightInfo.getField("Zmin").getDouble();
        zoneLight.Zmax = getZoneLightInfo.getField("Zmax").getDouble();
    }

    for (auto &zoneLight : zoneLights) {
        getZoneLightPointsInfo.setInputs( zoneLight.ID );

        while (getZoneLightPointsInfo.execute()) {
            vec2 &pt = zoneLight.points.emplace_back();

            pt.x = getZoneLightPointsInfo.getField("Pos_0").getDouble();
            pt.y = getZoneLightPointsInfo.getField("Pos_1").getDouble();
        }
    }
}
