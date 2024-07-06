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
        "select m.ID, m.Directory, m.MapName_lang, m.WdtFileDataID, m.MapType, m.TimeOfDayOverride, m.Flags_0, m.Flags_1, m.Flags_2 from Map m where m.WdtFileDataID > 0";

const std::string getMapListSQL_classic =
        "select m.ID, m.Directory, m.MapName_lang, m.MapType, m.TimeOfDayOverride, m.Flags_0, m.Flags_1, m.Flags_2 from Map m";

const std::string getMapByIDSQL =
        "select m.ID, m.Directory, m.MapName_lang, m.WdtFileDataID, m.MapType, m.TimeOfDayOverride, m.Flags_0, m.Flags_1, m.Flags_2 from Map m where m.ID = ?";

const std::string getMapByIDSQL_classic =
        "select m.ID, m.Directory, m.MapName_lang, m.MapType, m.TimeOfDayOverride, m.Flags_0, m.Flags_1, m.Flags_2  from Map m where m.ID = ?";

const std::string getWmoAreaAreaNameSQL = R"===(
        select wat.AreaName_lang as wmoAreaName, at.AreaName_lang as areaName, at.ID as ID, at.ParentAreaID as ParentAreaID, at.Ambient_multiplier as Ambient_multiplier from WMOAreaTable wat
        left join AreaTable at on at.id = wat.AreaTableID
        where wat.WMOID = ? and wat.NameSetID = ? and (wat.WMOGroupID = -1 or wat.WMOGroupID = ?) ORDER BY wat.WMOGroupID DESC
        )===";

const std::string getLightByIdSQL = R"===(
        select
        l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd,
        l.LightParamsID_0, l.LightParamsID_1, l.LightParamsID_2, l.LightParamsID_3, l.LightParamsID_4,
        l.LightParamsID_5, l.LightParamsID_6, l.LightParamsID_7,
        IFNULL(ls.SkyboxFileDataID, 0) as SkyboxFileDataID,
        IFNULL(ls.Flags, 0) as SkyboxFlags,
        IFNULL(lp.LightSkyboxID, 0) as LightSkyboxID,
        lp.Glow
        from Light l
        left join LightParams lp on lp.ID = l.LightParamsID_0
        left join LightSkybox ls on ls.ID = lp.LightSkyboxID
        where l.ID = ?
)===";

const std::string getLightSQL = R"===(
        select
        l.id as LightId, l.GameCoords_0, l.GameCoords_1, l.GameCoords_2,
        l.GameFalloffStart, l.GameFalloffEnd,
        l.ContinentID,

        l.LightParamsID_0, l.LightParamsID_1, l.LightParamsID_2, l.LightParamsID_3, l.LightParamsID_4,
        l.LightParamsID_5, l.LightParamsID_6, l.LightParamsID_7,

        (abs(l.GameCoords_0 - ?1) * abs(l.GameCoords_0 - ?1) +
         abs(l.GameCoords_1 - ?2) * abs(l.GameCoords_1 - ?2) +
         abs(l.GameCoords_2 - ?3) * abs(l.GameCoords_2 - ?3)) as lightDistSQR
    from Light l
    where
        ((l.ContinentID = ?4) and
        (
             (lightDistSQR < (l.GameFalloffEnd * l.GameFalloffEnd)) or
             (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0)
        )) or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0 and l.ContinentID = 0)
    ORDER BY l.ID desc
)===";

const std::string liquidObjectInfoSQL = R"===(
    select lo.LiquidTypeID, lo.FlowDirection, lo.FlowSpeed, lo.Reflection from LiquidObject lo
    where lo.ID = ?;
        )===";

const std::string liquidTypeSQL =   R"===(
        select
            lt.Texture_0, lt.Texture_1, lt.Texture_2, lt.Texture_3, lt.Texture_4, lt.Texture_5,
            lt.Flags, lt.SpellID, lt.LightID,
            lt.MaterialID,
            lt.MinimapStaticCol,
            lt.FrameCountTexture_0, lt.FrameCountTexture_1, lt.FrameCountTexture_2, lt.FrameCountTexture_3,
            lt.FrameCountTexture_4, lt.FrameCountTexture_5, lt.Color_0, lt.Color_1,
            lt.Float_0, lt.Float_1, lt.Float_2, lt.Float_3, lt.Float_4, lt.Float_5, lt.Float_6, lt.Float_7,
            lt.Float_8, lt.Float_9, lt.Float_10, lt.Float_11, lt.Float_12, lt.Float_13, lt.Float_14,
            lt.Float_15, lt.Float_16, lt.Float_17,
            lt.Int_0, lt.Int_1, lt.Int_2, lt.Int_3, lt.Int_3,
            lt.Coefficient_0, lt.Coefficient_1, lt.Coefficient_2, lt.Coefficient_3,
            lm.Flags as MatFlag,
            lm.LVF as MatLVF
        from LiquidType lt
        left join LiquidMaterial lm on lm.ID = lt.MaterialID
        where lt.ID = ?;
    )===";

const std::string liquidTextureFileDataIdsSQL = R"===(
    select ltxt.FileDataID, ltxt.Type
    from LiquidTypeXTexture ltxt
    where ltxt.LiquidTypeID = ?
    order by ltxt.OrderIndex;
)===";

CSqliteDB::CSqliteDB(std::string dbFileName) :
    m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getWmoAreaAreaName(m_sqliteDatabase, getWmoAreaAreaNameSQL),
    getAreaNameStatement(m_sqliteDatabase,
        "select at.AreaName_lang, at.ID, at.ParentAreaID, at.Ambient_multiplier from AreaTable at where at.ID = ?"),

    getLightStatement(m_sqliteDatabase, getLightSQL),
    getLightByIdStatement(m_sqliteDatabase, getLightByIdSQL),

    getLightData(m_sqliteDatabase, generateSimpleSelectSQL("LightData",
                                                           {"LightParamID", "ID"},
                                                           " where LightParamID = ? ORDER BY Time ASC")),
    getLightParamDataStatement(m_sqliteDatabase, generateSimpleSelectSQL("LightParams",
                                                                         {},
                                                                       "where ID = ?")),
    getLiquidObjectInfo(m_sqliteDatabase,liquidObjectInfoSQL),
    getLiquidTypeInfo(m_sqliteDatabase, liquidTypeSQL),
    getLiquidTextureFileDataIds(m_sqliteDatabase, getHasLiquidTypeXTexture(m_sqliteDatabase) ? liquidTextureFileDataIdsSQL : "select 1 from Map;"),
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
        mapRecord.overrideTime = getMapList.getField("TimeOfDayOverride").getInt();
        mapRecord.flags0 = getMapList.getField("Flags_0").getInt();
        mapRecord.flags1 = getMapList.getField("Flags_1").getInt();
        mapRecord.flags2 = getMapList.getField("Flags_2").getInt();
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
        mapRecord.overrideTime = getMapByIdStatement.getField("TimeOfDayOverride").getInt();
        mapRecord.flags0 = getMapByIdStatement.getField("Flags_0").getInt();
        mapRecord.flags1 = getMapByIdStatement.getField("Flags_1").getInt();
        mapRecord.flags2 = getMapByIdStatement.getField("Flags_2").getInt();

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

bool CSqliteDB::getWmoArea(int wmoId, int nameId, int groupId, AreaRecord &result) {

    getWmoAreaAreaName.setInputs( wmoId, nameId, groupId);

    while (getWmoAreaAreaName.execute()) {
        std::string wmoAreaName = getWmoAreaAreaName.getField("wmoAreaName").getString();
        std::string areaName = getWmoAreaAreaName.getField("areaName").getString();


        if (wmoAreaName == "") {
            result.areaName = areaName;
        } else {
            result.areaName = wmoAreaName;
        }
        result.areaId = getWmoAreaAreaName.getField("ID").getInt();
        result.parentAreaId = getWmoAreaAreaName.getField("ParentAreaID").getInt();
        result.ambientMultiplier= getWmoAreaAreaName.getField("Ambient_multiplier").getDouble();

        return true;
    }

    return false;
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
void initWithZeros(std::array<float, 4> &colorF) {
    colorF[0] = 0;
    colorF[1] = 0;
    colorF[2] = 0;
    colorF[3] = 0;
}
void blendTwoAndAdd(float *colorF, int currLdRes, int lastLdRes, float timeAlphaBlend) {
    colorF[0] = (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<0>(lastLdRes) *
                                    (1.0f - timeAlphaBlend));
    colorF[1] = (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<1>(lastLdRes) *
                                    (1.0f - timeAlphaBlend));
    colorF[2] = (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<2>(lastLdRes) *
                                    (1.0f - timeAlphaBlend));
}
inline void blendTwoAndAdd(std::array<float, 3> &colorF, int currLdRes, int lastLdRes, float timeAlphaBlend) {
    colorF[0] = (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<0>(lastLdRes) *
                  (1.0f - timeAlphaBlend));
    colorF[1] = (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<1>(lastLdRes) *
                  (1.0f - timeAlphaBlend));
    colorF[2] = (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<2>(lastLdRes) *
                  (1.0f - timeAlphaBlend));
}

inline void blendTwoAndAdd(float &colorF, float currLdRes, float lastLdRes, float timeAlphaBlend) {
    colorF += (currLdRes * timeAlphaBlend + lastLdRes * (1.0f - timeAlphaBlend));
}

inline void addOnlyOne(float *colorF, int currLdRes) {
    colorF[0] = getFloatFromInt<0>(currLdRes);
    colorF[1] = getFloatFromInt<1>(currLdRes);
    colorF[2] = getFloatFromInt<2>(currLdRes);
}

inline void addOnlyOne(std::array<float, 3> &colorF, int currLdRes) {
    colorF[0] = getFloatFromInt<0>(currLdRes);
    colorF[1] = getFloatFromInt<1>(currLdRes);
    colorF[2] = getFloatFromInt<2>(currLdRes);
}

inline void addOnlyOne(float &colorF, float currLdRes) {
    colorF = currLdRes;
}
void CSqliteDB::getLightById(int lightId, int time, LightResult &lightResult) {
    getLightByIdStatement.setInputs( lightId );

    while (getLightByIdStatement.execute()) {
        auto &ilr = lightResult;

        ilr.pos[0] = getLightByIdStatement.getField("GameCoords_0").getDouble();
        ilr.pos[1] = getLightByIdStatement.getField("GameCoords_1").getDouble();
        ilr.pos[2] = getLightByIdStatement.getField("GameCoords_2").getDouble();
        ilr.fallbackStart = getLightByIdStatement.getField("GameFalloffStart").getDouble();
        ilr.fallbackEnd = getLightByIdStatement.getField("GameFalloffEnd").getDouble();
        ilr.lightParamId[0] = getLightByIdStatement.getField("LightParamsID_0").getInt();
        ilr.lightParamId[1] = getLightByIdStatement.getField("LightParamsID_1").getInt();
        ilr.lightParamId[2] = getLightByIdStatement.getField("LightParamsID_2").getInt();
        ilr.lightParamId[3] = getLightByIdStatement.getField("LightParamsID_3").getInt();
        ilr.lightParamId[4] = getLightByIdStatement.getField("LightParamsID_4").getInt();
        ilr.lightParamId[5] = getLightByIdStatement.getField("LightParamsID_5").getInt();
        ilr.lightParamId[6] = getLightByIdStatement.getField("LightParamsID_6").getInt();
        ilr.lightParamId[7] = getLightByIdStatement.getField("LightParamsID_7").getInt();

        ilr.id = lightId;

        return;
    }
};
void CSqliteDB::getEnvInfo(int mapId, float x, float y, float z, std::vector<LightResult> &lightResults) {
    getLightStatement.setInputs(x, y, z, mapId );

    bool defaultRecordSet = false;

    while (getLightStatement.execute()) {
        auto &ilr = lightResults.emplace_back();
        ilr.id = getLightStatement.getField("LightId").getInt();
        ilr.pos[0] = getLightStatement.getField("GameCoords_0").getDouble();
        ilr.pos[1] = getLightStatement.getField("GameCoords_1").getDouble();
        ilr.pos[2] = getLightStatement.getField("GameCoords_2").getDouble();
        ilr.fallbackStart = getLightStatement.getField("GameFalloffStart").getDouble();
        ilr.fallbackEnd = getLightStatement.getField("GameFalloffEnd").getDouble();
        ilr.lightParamId[0] = getLightStatement.getField("LightParamsID_0").getInt();
        ilr.lightParamId[1] = getLightStatement.getField("LightParamsID_1").getInt();
        ilr.lightParamId[2] = getLightStatement.getField("LightParamsID_2").getInt();
        ilr.lightParamId[3] = getLightStatement.getField("LightParamsID_3").getInt();
        ilr.lightParamId[4] = getLightStatement.getField("LightParamsID_4").getInt();
        ilr.lightParamId[5] = getLightStatement.getField("LightParamsID_5").getInt();
        ilr.lightParamId[6] = getLightStatement.getField("LightParamsID_6").getInt();
        ilr.lightParamId[7] = getLightStatement.getField("LightParamsID_7").getInt();


        ilr.lightDistSQR = getLightStatement.getField("lightDistSQR").getDouble();

        ilr.blendAlpha =
            sqrtf(ilr.lightDistSQR) > ilr.fallbackStart ?
            std::max<float>( ((ilr.fallbackStart - sqrtf(ilr.lightDistSQR)) / (ilr.fallbackEnd - ilr.fallbackStart)) + 1.0f, 0.0f) :
            1.0f;
    }
}
bool CSqliteDB::getLightParamData(int lightParamId, int time, LightParamData &lightParamData) {

    getLightParamDataStatement.setInputs(lightParamId);

    if (getLightParamDataStatement.execute()) {
        lightParamData.glow = getLightParamDataStatement.getField("Glow").getDouble();
        lightParamData.lightSkyBoxId = getLightParamDataStatement.getField("LightSkyboxID").getInt();
        lightParamData.waterShallowAlpha = getLightParamDataStatement.getField("WaterShallowAlpha").getDouble();
        lightParamData.waterDeepAlpha = getLightParamDataStatement.getField("WaterDeepAlpha").getDouble();
        lightParamData.oceanShallowAlpha = getLightParamDataStatement.getField("OceanShallowAlpha").getDouble();
        lightParamData.oceanDeepAlpha = getLightParamDataStatement.getField("OceanDeepAlpha").getDouble();
        lightParamData.lightParamFlags = getLightParamDataStatement.getField("Flags").getInt();
    } else {
        //Record not found
        return false;
    }

    getTimedLightParamData(lightParamId, time, lightParamData);

    return true;
}

void CSqliteDB::getTimedLightParamData(int lightParamId, int time, LightParamData &lightParamData) {
    getLightData.setInputs(lightParamId );

    bool hasHorizonAmbientColor = getLightData.getFieldIndex("HorizonAmbientColor") >= 0;
    bool hasGroundAmbientColor = getLightData.getFieldIndex("GroundAmbientColor") >= 0;
    bool hasFogDensity = getLightData.getFieldIndex("FogDensity") >= 0;
    bool hasFogHeight = getLightData.getFieldIndex("FogHeight") >= 0;
    bool hasFogHeightScaler = getLightData.getFieldIndex("FogHeightScaler") >= 0;
    bool hasFogHeightDensity = getLightData.getFieldIndex("FogHeightDensity") >= 0;
    bool hasSunFogAngle = getLightData.getFieldIndex("SunFogAngle") >= 0;
    bool hasEndFogColor = getLightData.getFieldIndex("EndFogColor") >= 0;
    bool hasEndFogColorDistance = getLightData.getFieldIndex("EndFogColorDistance") >= 0;
    bool hasSunFogColor = getLightData.getFieldIndex("SunFogColor") >= 0;
    bool hasSunFogStrength = getLightData.getFieldIndex("SunFogStrength") >= 0;
    bool hasFogHeightColor = getLightData.getFieldIndex("FogHeightColor") >= 0;
    bool hasFogHeightCoefficients = getLightData.getFieldIndex("FogHeightCoefficients_0") >= 0;
    bool hasFogZScalar = getLightData.getFieldIndex("FogZScalar") >= 0;
    bool hasMainFogStartDist = getLightData.getFieldIndex("MainFogStartDist") >= 0;
    bool hasMainFogEndDist = getLightData.getFieldIndex("MainFogEndDist") >= 0;
    bool hasFogStartOffset = getLightData.getFieldIndex("FogStartOffset") >= 0;
    bool hasEndFogHeightColor = getLightData.getFieldIndex("EndFogHeightColor") >= 0;
    bool hasMainFogCoefficients = getLightData.getFieldIndex("MainFogCoefficients_0") >= 0;
    bool hasHeightDensityFogCoeff = getLightData.getFieldIndex("HeightDensityFogCoeff_0") >= 0;

    int timeIndex = 0;
    std::array<bool, 2> slotWritten = {false, false};
    while (getLightData.execute()) {
        int thisTime = getLightData.getField("Time").getInt();
        if (thisTime > time) {
            //Increase timeIndex if the time of current record is bigger than time
            timeIndex++;
        }

        if (timeIndex > 1) return;

        auto &currLdRes = lightParamData.lightTimedData[timeIndex];
        currLdRes.time = thisTime;

        currLdRes.ambientLight = getLightData.getField("AmbientColor");
        currLdRes.horizontAmbientColor = hasHorizonAmbientColor ? getLightData.getField("HorizonAmbientColor") : 0;
        currLdRes.groundAmbientColor = hasGroundAmbientColor ? getLightData.getField("GroundAmbientColor") : 0;

        currLdRes.directColor = getLightData.getField("DirectColor");

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
        currLdRes.FogDensity = hasFogDensity ? getLightData.getField("FogDensity").getDouble() : 0.000001f;
        currLdRes.FogHeight = hasFogHeight ? getLightData.getField("FogHeight").getDouble() : -10000.0;
        currLdRes.FogHeightScaler = hasFogHeightScaler ? getLightData.getField("FogHeightScaler").getDouble() : -1.0f;
        currLdRes.FogHeightDensity = hasFogHeightDensity ? getLightData.getField("FogHeightDensity").getDouble() : 0.0f;
        currLdRes.FogZScalar = hasFogZScalar ? getLightData.getField("FogZScalar").getDouble() : 0.0f;
        currLdRes.MainFogStartDist = hasMainFogStartDist ? getLightData.getField("MainFogStartDist").getDouble() : 0.0f;
        currLdRes.MainFogEndDist = hasMainFogEndDist ? getLightData.getField("MainFogEndDist").getDouble() : 0.0f;
        currLdRes.SunFogAngle = hasSunFogAngle ? getLightData.getField("SunFogAngle").getDouble() : 0.f;
        currLdRes.EndFogColor = hasEndFogColor ? getLightData.getField("EndFogColor").getInt() : 0;
        currLdRes.EndFogColorDistance = hasEndFogColorDistance ? getLightData.getField("EndFogColorDistance").getDouble() : 0;
        currLdRes.FogStartOffset = hasFogStartOffset ? getLightData.getField("FogStartOffset").getDouble() : 0;
        currLdRes.SunFogColor = hasSunFogColor ? getLightData.getField("SunFogColor").getInt() : 0;
        currLdRes.SunFogStrength = hasSunFogStrength ? getLightData.getField("SunFogStrength").getDouble() : 0.0f;
        currLdRes.FogHeightColor = hasFogHeightColor ? getLightData.getField("FogHeightColor").getInt() : 0.0f;
        currLdRes.EndFogHeightColor = hasEndFogHeightColor ? getLightData.getField("EndFogHeightColor").getInt() : 0;
        currLdRes.FogHeightCoefficients[0] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_0").getDouble() : 0.0f;
        currLdRes.FogHeightCoefficients[1] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_1").getDouble() : 0.0f;
        currLdRes.FogHeightCoefficients[2] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_2").getDouble() : 0.0f;
        currLdRes.FogHeightCoefficients[3] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_3").getDouble() : 0.0f;
        currLdRes.MainFogCoefficients[0] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_0").getDouble() : 0.0f;
        currLdRes.MainFogCoefficients[1] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_1").getDouble() : 0.0f;
        currLdRes.MainFogCoefficients[2] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_2").getDouble() : 0.0f;
        currLdRes.MainFogCoefficients[3] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_3").getDouble() : 0.0f;
        currLdRes.HeightDensityFogCoeff[0] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_0").getDouble() : 0.0f;
        currLdRes.HeightDensityFogCoeff[1] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_1").getDouble() : 0.0f;
        currLdRes.HeightDensityFogCoeff[2] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_2").getDouble() : 0.0f;
        currLdRes.HeightDensityFogCoeff[3] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_3").getDouble() : 0.0f;

        slotWritten[timeIndex] = true;
    }

    //Found only one result. Let's copy it to the second slot
    if (slotWritten[0] && !slotWritten[1]) lightParamData.lightTimedData[1] = lightParamData.lightTimedData[0];
    if (slotWritten[1] && !slotWritten[0]) lightParamData.lightTimedData[0] = lightParamData.lightTimedData[1];

}


void CSqliteDB::getLiquidObjectData(int liquidObjectId, int fallbackliquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) {
    getLiquidObjectInfo.setInputs( liquidObjectId );

    if (getLiquidObjectInfo.execute()) {

        loData.liquidTypeId = getLiquidObjectInfo.getField("LiquidTypeID").getInt();
        loData.flowDirection = getLiquidObjectInfo.getField("FlowDirection").getDouble();
        loData.flowSpeed = getLiquidObjectInfo.getField("FlowSpeed").getDouble();
        loData.reflection = getLiquidObjectInfo.getField("Reflection").getInt() > 0;

        getLiquidTypeData(loData.liquidTypeId, loData, textures);

        return;
    } else {
        loData.liquidTypeId = fallbackliquidTypeId;
        getLiquidTypeData(fallbackliquidTypeId, loData, textures);

    }
};
void CSqliteDB::getLiquidTypeData(int liquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) {
    getLiquidTypeInfo.setInputs(liquidTypeId);
    while (getLiquidTypeInfo.execute()) {

        for (int i = 0; i < loData.texture.size(); i++) {
            HashedString fieldHash = HashedString(("Texture_" + std::to_string(i)).c_str());
            loData.texture[i] = getLiquidTypeInfo.getField(fieldHash).getString();
        }
        loData.flags = getLiquidTypeInfo.getField("Flags").getUInt();
        loData.spellID = getLiquidTypeInfo.getField("SpellID").getUInt();
        loData.lightID = getLiquidTypeInfo.getField("SpellID").getUInt();
        loData.materialID = getLiquidTypeInfo.getField("MaterialID").getUInt();

        int minimapStaticCol = getLiquidTypeInfo.getField("MinimapStaticCol").getInt();
        loData.minimapStaticCol[0] = getFloatFromInt<0>(minimapStaticCol);
        loData.minimapStaticCol[1] = getFloatFromInt<1>(minimapStaticCol);
        loData.minimapStaticCol[2] = getFloatFromInt<2>(minimapStaticCol);

        for (int i = 0; i < loData.frameCountTexture.size(); i++) {
            HashedString fieldHash = HashedString(("FrameCountTexture_" + std::to_string(i)).c_str());
            loData.frameCountTexture[i] = getLiquidTypeInfo.getField(fieldHash).getInt();
        }

        int color1 = getLiquidTypeInfo.getField("Color_0").getInt();
        loData.color1[0] = getFloatFromInt<0>(color1);
        loData.color1[1] = getFloatFromInt<1>(color1);
        loData.color1[2] = getFloatFromInt<2>(color1);
        int color2 = getLiquidTypeInfo.getField("Color_1").getInt();
        loData.color2[0] = getFloatFromInt<0>(color2);
        loData.color2[1] = getFloatFromInt<1>(color2);
        loData.color2[2] = getFloatFromInt<2>(color2);

        for (int i = 0; i < loData.m_floats.size(); i++) {
            HashedString fieldHash = HashedString(("Float_" + std::to_string(i)).c_str());
            loData.m_floats[i] = getLiquidTypeInfo.getField(fieldHash).getDouble();
        }
        for (int i = 0; i < loData.m_int.size(); i++) {
            HashedString fieldHash = HashedString(("Int_" + std::to_string(i)).c_str());
            loData.m_int[i] = getLiquidTypeInfo.getField(fieldHash).getInt();
        }

        for (int i = 0; i < loData.coefficient.size(); i++) {
            HashedString fieldHash = HashedString(("Coefficient_" + std::to_string(i)).c_str());
            loData.coefficient[i] = getLiquidTypeInfo.getField(fieldHash).getDouble();
        }

        loData.matFlag = getLiquidTypeInfo.getField("MatFlag").getUInt();
        loData.matLVF = getLiquidTypeInfo.getField("MatLVF").getUInt();

        getLiquidTexture(liquidTypeId, textures);
        return;
    }
};

void CSqliteDB::getLiquidTexture(int liquidTypeId, std::vector<LiquidTextureData> &textures) {
    if (!getHasLiquidTypeXTexture(m_sqliteDatabase)) return;

    getLiquidTextureFileDataIds.setInputs(liquidTypeId);

    while (getLiquidTextureFileDataIds.execute()) {
        auto &ltd = textures.emplace_back();
        ltd.fileDataId = getLiquidTextureFileDataIds.getField("FileDataID").getInt();
        ltd.type = getLiquidTextureFileDataIds.getField("Type").getInt();
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

std::string
CSqliteDB::generateSimpleSelectSQL(const std::string &tableName, const std::vector<std::string> &skipColumnNames,
                                   const std::string &whereClause) {
    std::string query = "SELECT ";
    std::vector<std::string> fieldList = getTableFields(m_sqliteDatabase, tableName);

    bool fieldWasAdded = false;
    for (auto const &fieldName : fieldList) {
        if (std::find(skipColumnNames.begin(), skipColumnNames.end(), fieldName) == std::end(skipColumnNames)) {
            query += fieldName + ", ";
            fieldWasAdded = true;
        }
    }
    query = query.substr(0, query.size()-2);
    query += " FROM "+tableName+" "+whereClause;
    return query;
}
