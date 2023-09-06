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
        lp.Glow, IFNULL(ls.Flags, 0) as SkyboxFlags, l.ContinentID,
        (abs(l.GameCoords_0 - ?1) * abs(l.GameCoords_0 - ?1) +
         abs(l.GameCoords_1 - ?2) * abs(l.GameCoords_1 - ?2) +
         abs(l.GameCoords_2 - ?3) * abs(l.GameCoords_2 - ?3)) as lightDistSQR
    from Light l
             left join LightParams lp on lp.ID = l.LightParamsID_0
             left join LightSkybox ls on ls.ID = lp.LightSkyboxID
    where
        ((l.ContinentID = ?4) and
        (
             (lightDistSQR < (l.GameFalloffEnd * l.GameFalloffEnd)) or
             (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0)
        )) or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0 and l.ContinentID = 0)
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

//    getLightData(m_sqliteDatabase, lightDataSQL),
    getLightData(m_sqliteDatabase, generateSimpleSelectSQL("LightData",
                                                           {"LightParamID", "ID"},
                                                           " where LightParamID = ? ORDER BY Time ASC")),
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
void initWithZeros(std::array<float, 4> &colorF) {
    colorF[0] = 0;
    colorF[1] = 0;
    colorF[2] = 0;
    colorF[3] = 0;
}
void blendTwoAndAdd(float *colorF, int currLdRes, int lastLdRes, float timeAlphaBlend) {
    colorF[0] += (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<0>(lastLdRes) *
                                    (1.0f - timeAlphaBlend));
    colorF[1] += (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<1>(lastLdRes) *
                                    (1.0f - timeAlphaBlend));
    colorF[2] += (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                                    getFloatFromInt<2>(lastLdRes) *
                                    (1.0f - timeAlphaBlend));
}
inline void blendTwoAndAdd(std::array<float, 3> &colorF, int currLdRes, int lastLdRes, float timeAlphaBlend) {
    colorF[0] += (getFloatFromInt<0>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<0>(lastLdRes) *
                  (1.0f - timeAlphaBlend));
    colorF[1] += (getFloatFromInt<1>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<1>(lastLdRes) *
                  (1.0f - timeAlphaBlend));
    colorF[2] += (getFloatFromInt<2>(currLdRes) * timeAlphaBlend +
                  getFloatFromInt<2>(lastLdRes) *
                  (1.0f - timeAlphaBlend));
}

inline void blendTwoAndAdd(float &colorF, float currLdRes, float lastLdRes, float timeAlphaBlend) {
    colorF += (currLdRes * timeAlphaBlend + lastLdRes * (1.0f - timeAlphaBlend));
}

inline void addOnlyOne(float *colorF, int currLdRes) {
    colorF[0] += getFloatFromInt<0>(currLdRes);
    colorF[1] += getFloatFromInt<1>(currLdRes);
    colorF[2] += getFloatFromInt<2>(currLdRes);
}

inline void addOnlyOne(std::array<float, 3> &colorF, int currLdRes) {
    colorF[0] += getFloatFromInt<0>(currLdRes);
    colorF[1] += getFloatFromInt<1>(currLdRes);
    colorF[2] += getFloatFromInt<2>(currLdRes);
}

inline void addOnlyOne(float &colorF, float currLdRes) {
    colorF += currLdRes;
}
void CSqliteDB::getLightById(int lightId, int time, LightResult &lightResult, float farClip) {
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
    }
    std::vector<LightResult> lightResults;
    convertInnerResultsToPublic(time, lightResults, innerResults, farClip);
    if (lightResults.size() > 0) {
        lightResult = lightResults[0];
    }

};
void CSqliteDB::getEnvInfo(int mapId, float x, float y, float z, int ptime, std::vector<LightResult> &lightResults, float farClip) {
    getLightStatement.setInputs(x, y, z, mapId );

    std::vector<InnerLightResult> innerResults;
    bool defaultRecordSet = false;
    InnerLightResult defaultRecord;

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

        ilr.lightDistSQR = getLightStatement.getField("lightDistSQR").getDouble();

        ilr.blendAlpha =
            sqrtf(ilr.lightDistSQR) > ilr.fallbackStart ?
            std::max<float>( ((ilr.fallbackStart - sqrtf(ilr.lightDistSQR)) / (ilr.fallbackEnd - ilr.fallbackStart)) + 1.0f, 0.0f) :
            1.0f;

        if (ilr.pos[0] == 0 && ilr.pos[1] == 0 && ilr.pos[2] == 0 ) {
            if (!defaultRecordSet || mapId == getLightStatement.getField("ContinentID").getInt()){
                defaultRecord = ilr;
                defaultRecord.isDefault = true;
                defaultRecord.blendAlpha = 1.0f;
                defaultRecordSet = true;
            }
            innerResults.resize(std::max<int>(innerResults.size() - 1, 0));
        }
    }
    if (defaultRecordSet) innerResults.push_back(defaultRecord);

    convertInnerResultsToPublic(ptime, lightResults, innerResults, farClip);
}
void CSqliteDB::addOnlyOne(LightResult &lightResult,
                           const CSqliteDB::InnerLightDataRes &currLdRes) const {//Set as is
    ::addOnlyOne(lightResult.ambientColor, currLdRes.ambientLight);
    ::addOnlyOne(lightResult.horizontAmbientColor, currLdRes.horizontAmbientColor);
    ::addOnlyOne(lightResult.groundAmbientColor, currLdRes.groundAmbientColor);

    ::addOnlyOne(lightResult.directColor, currLdRes.directLight);

    ::addOnlyOne(lightResult.closeRiverColor, currLdRes.closeRiverColor);
    ::addOnlyOne(lightResult.farRiverColor, currLdRes.farRiverColor);
    ::addOnlyOne(lightResult.closeOceanColor, currLdRes.closeOceanColor);
    ::addOnlyOne(lightResult.farOceanColor, currLdRes.farOceanColor);

    ::addOnlyOne(lightResult.SkyTopColor, currLdRes.SkyTopColor);
    ::addOnlyOne(lightResult.SkyMiddleColor, currLdRes.SkyMiddleColor);
    ::addOnlyOne(lightResult.SkyBand1Color, currLdRes.SkyBand1Color);
    ::addOnlyOne(lightResult.SkyBand2Color, currLdRes.SkyBand2Color);
    ::addOnlyOne(lightResult.SkySmogColor, currLdRes.SkySmogColor);
    ::addOnlyOne(lightResult.SkyFogColor, currLdRes.SkyFogColor);

    ::addOnlyOne(lightResult.FogEnd, currLdRes.FogEnd);
    ::addOnlyOne(lightResult.FogScaler, currLdRes.FogScaler);
    ::addOnlyOne(lightResult.FogDensity, currLdRes.FogDensity);
    ::addOnlyOne(lightResult.FogHeight, currLdRes.FogHeight);
    ::addOnlyOne(lightResult.FogHeightScaler, currLdRes.FogHeightScaler);
    ::addOnlyOne(lightResult.FogHeightDensity, currLdRes.FogHeightDensity);

    ::addOnlyOne(lightResult.EndFogColor, currLdRes.EndFogColor);
    ::addOnlyOne(lightResult.EndFogColorDistance, currLdRes.EndFogColorDistance);
    ::addOnlyOne(lightResult.SunFogColor, currLdRes.SunFogColor);

    ::addOnlyOne(lightResult.FogHeightColor, currLdRes.FogHeightColor);
    ::addOnlyOne(lightResult.FogHeightCoefficients[0], currLdRes.FogHeightCoefficients[0]);
    ::addOnlyOne(lightResult.FogHeightCoefficients[1], currLdRes.FogHeightCoefficients[1]);
    ::addOnlyOne(lightResult.FogHeightCoefficients[2], currLdRes.FogHeightCoefficients[2]);
    ::addOnlyOne(lightResult.FogHeightCoefficients[3], currLdRes.FogHeightCoefficients[3]);

    ::addOnlyOne(lightResult.MainFogCoefficients[0], currLdRes.MainFogCoefficients[0]);
    ::addOnlyOne(lightResult.MainFogCoefficients[1], currLdRes.MainFogCoefficients[1]);
    ::addOnlyOne(lightResult.MainFogCoefficients[2], currLdRes.MainFogCoefficients[2]);
    ::addOnlyOne(lightResult.MainFogCoefficients[3], currLdRes.MainFogCoefficients[3]);

    ::addOnlyOne(lightResult.HeightDensityFogCoefficients[0], currLdRes.HeightDensityFogCoeff[0]);
    ::addOnlyOne(lightResult.HeightDensityFogCoefficients[1], currLdRes.HeightDensityFogCoeff[1]);
    ::addOnlyOne(lightResult.HeightDensityFogCoefficients[2], currLdRes.HeightDensityFogCoeff[2]);
    ::addOnlyOne(lightResult.HeightDensityFogCoefficients[3], currLdRes.HeightDensityFogCoeff[3]);

    ::addOnlyOne(lightResult.FogZScalar, currLdRes.FogZScalar);
    ::addOnlyOne(lightResult.MainFogStartDist, currLdRes.MainFogStartDist);
    ::addOnlyOne(lightResult.MainFogEndDist, currLdRes.MainFogEndDist);
    ::addOnlyOne(lightResult.HeightEndFogColor, currLdRes.EndFogHeightColor);
    ::addOnlyOne(lightResult.FogStartOffset, currLdRes.FogStartOffset);
}

void CSqliteDB::blendTwoAndAdd(LightResult &lightResult, const CSqliteDB::InnerLightDataRes &lastLdRes,
                               const CSqliteDB::InnerLightDataRes &currLdRes, float timeAlphaBlend) const {
    ::blendTwoAndAdd(lightResult.ambientColor,
                     currLdRes.ambientLight, lastLdRes.ambientLight,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.horizontAmbientColor,
                     currLdRes.horizontAmbientColor, lastLdRes.horizontAmbientColor,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.groundAmbientColor,
                     currLdRes.groundAmbientColor, lastLdRes.groundAmbientColor,
                     timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.directColor,
                     currLdRes.directLight, lastLdRes.directLight,
                     timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.closeRiverColor,
                     currLdRes.closeRiverColor, lastLdRes.closeRiverColor,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.farRiverColor,
                     currLdRes.farRiverColor, lastLdRes.farRiverColor,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.closeOceanColor,
                     currLdRes.closeOceanColor, lastLdRes.closeOceanColor,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.farOceanColor,
                     currLdRes.farOceanColor, lastLdRes.farOceanColor,
                     timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.SkyTopColor,
                     currLdRes.SkyTopColor, lastLdRes.SkyTopColor,
                     timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.SkyMiddleColor,
                     currLdRes.SkyMiddleColor, lastLdRes.SkyMiddleColor,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SkyBand1Color,
                     currLdRes.SkyBand1Color, lastLdRes.SkyBand1Color,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SkyBand2Color,
                     currLdRes.SkyBand2Color, lastLdRes.SkyBand2Color,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SkySmogColor,
                     currLdRes.SkySmogColor, lastLdRes.SkySmogColor,
                     timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SkyFogColor,
                     currLdRes.SkyFogColor, lastLdRes.SkyFogColor,
                     timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.FogEnd, currLdRes.FogEnd, lastLdRes.FogEnd, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogScaler, currLdRes.FogScaler, lastLdRes.FogScaler, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogDensity, currLdRes.FogDensity, lastLdRes.FogDensity, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeight, currLdRes.FogHeight, lastLdRes.FogHeight, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightScaler, currLdRes.FogHeightScaler, lastLdRes.FogHeightScaler, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightDensity, currLdRes.FogHeightDensity, lastLdRes.FogHeightDensity, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SunFogAngle, currLdRes.SunFogAngle, lastLdRes.SunFogAngle, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.EndFogColor, currLdRes.EndFogColor, lastLdRes.EndFogColor, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.EndFogColorDistance, currLdRes.EndFogColorDistance, lastLdRes.EndFogColorDistance, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SunFogColor, currLdRes.SunFogColor, lastLdRes.SunFogColor, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.SunFogStrength, currLdRes.SunFogStrength, lastLdRes.SunFogStrength, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightColor, currLdRes.FogHeightColor, lastLdRes.FogHeightColor, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[0], currLdRes.FogHeightCoefficients[0], lastLdRes.FogHeightCoefficients[0], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[1], currLdRes.FogHeightCoefficients[1], lastLdRes.FogHeightCoefficients[1], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[2], currLdRes.FogHeightCoefficients[2], lastLdRes.FogHeightCoefficients[2], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogHeightCoefficients[3], currLdRes.FogHeightCoefficients[3], lastLdRes.FogHeightCoefficients[3], timeAlphaBlend);


    ::blendTwoAndAdd(lightResult.MainFogCoefficients[0], currLdRes.MainFogCoefficients[0], lastLdRes.MainFogCoefficients[0], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.MainFogCoefficients[1], currLdRes.MainFogCoefficients[1], lastLdRes.MainFogCoefficients[1], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.MainFogCoefficients[2], currLdRes.MainFogCoefficients[2], lastLdRes.MainFogCoefficients[2], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.MainFogCoefficients[3], currLdRes.MainFogCoefficients[3], lastLdRes.MainFogCoefficients[3], timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.HeightDensityFogCoefficients[0], currLdRes.HeightDensityFogCoeff[0], lastLdRes.HeightDensityFogCoeff[0], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.HeightDensityFogCoefficients[1], currLdRes.HeightDensityFogCoeff[1], lastLdRes.HeightDensityFogCoeff[1], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.HeightDensityFogCoefficients[2], currLdRes.HeightDensityFogCoeff[2], lastLdRes.HeightDensityFogCoeff[2], timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.HeightDensityFogCoefficients[3], currLdRes.HeightDensityFogCoeff[3], lastLdRes.HeightDensityFogCoeff[3], timeAlphaBlend);

    ::blendTwoAndAdd(lightResult.FogZScalar,        currLdRes.FogZScalar,       lastLdRes.FogZScalar, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.MainFogStartDist,  currLdRes.MainFogStartDist, lastLdRes.MainFogStartDist, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.MainFogEndDist,    currLdRes.MainFogEndDist,   lastLdRes.MainFogEndDist, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.HeightEndFogColor, currLdRes.EndFogHeightColor,lastLdRes.EndFogHeightColor, timeAlphaBlend);
    ::blendTwoAndAdd(lightResult.FogStartOffset,    currLdRes.FogStartOffset,   lastLdRes.FogStartOffset, timeAlphaBlend);
}

float arr4SqrLength(const std::array<float,4> &arr) {
    return
        arr[0] * arr[0] +
        arr[1] * arr[1] +
        arr[2] * arr[2] +
        arr[3] * arr[3];
}

void CSqliteDB::convertInnerResultsToPublic(int ptime, std::vector<LightResult> &lightResults,
                                            std::vector<CSqliteDB::InnerLightResult> &innerResults,
                                            float farClip)  {

    //From lowest to highest
    std::sort(innerResults.begin(), innerResults.end(), +[](const InnerLightResult &a, const InnerLightResult &b) -> bool {
        if (a.isDefault) return true;
        if (b.isDefault) return false;

        float distanceSqr =
            (a.pos[0] - b.pos[0]) * (a.pos[0] - b.pos[0]) +
            (a.pos[1] - b.pos[1]) * (a.pos[1] - b.pos[1]) +
            (a.pos[2] - b.pos[2]) * (a.pos[2] - b.pos[2]);

        static const float tolerance = 0.33333334f;
        if (sqrtf(distanceSqr) < tolerance) {
            return a.fallbackStart < b.fallbackStart;
        } else {
            return a.lightDistSQR < b.lightDistSQR;
        }

        return false;
    });

    for (int i = 0; i < innerResults.size(); i++) {
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
        initWithZeros(lightResult.MainFogCoefficients);
        initWithZeros(lightResult.HeightDensityFogCoefficients);
        initWithZeros(lightResult.HeightEndFogColor);


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
        lightResult.FogZScalar = 0.0;
        lightResult.LegacyFogScalar = 0.0;
        lightResult.MainFogStartDist = 0.0;
        lightResult.MainFogEndDist = 0.0;
        lightResult.FogBlendAlpha = 0.0;
        lightResult.FogStartOffset = 0.0;



        auto &innerResult = innerResults[i];
        lightResult.isDefault = innerResult.isDefault;
        lightResult.lightParamId = innerResult.paramId;

        getLightData.setInputs( innerResult.paramId );

//        std::cout << "innerResult.paramId = " << innerResult.paramId << std::endl;

        InnerLightDataRes lastLdRes = {0, 0, -1};
        bool assigned = false;

        lightResult.blendCoef = innerResult.blendAlpha;


        lightResult.skyBoxFdid = innerResult.skyBoxFileId;
        lightResult.skyBoxFlags = innerResult.skyBoxFlags;
        lightResult.lightSkyboxId = innerResult.lightSkyboxId;
        lightResult.glow = innerResult.glow;

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

        while (getLightData.execute()) {
            InnerLightDataRes currLdRes;
            currLdRes.ambientLight = getLightData.getField("AmbientColor");
            currLdRes.horizontAmbientColor = hasHorizonAmbientColor ? getLightData.getField("HorizonAmbientColor") : 0;

            if (currLdRes.horizontAmbientColor == 0) {
                currLdRes.horizontAmbientColor = currLdRes.ambientLight;
            }
            currLdRes.groundAmbientColor = hasGroundAmbientColor ? getLightData.getField("GroundAmbientColor") : 0;
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
            currLdRes.FogDensity = hasFogDensity ? getLightData.getField("FogDensity").getDouble() : 0.000001f;
            currLdRes.FogHeight =   hasFogHeight ? getLightData.getField("FogHeight").getDouble() : -10000.0;
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
            currLdRes.FogHeightCoefficients[1] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_1").getDouble(): 0.0f;
            currLdRes.FogHeightCoefficients[2] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_2").getDouble(): 0.0f;
            currLdRes.FogHeightCoefficients[3] = hasFogHeightCoefficients ? getLightData.getField("FogHeightCoefficients_3").getDouble(): 0.0f;
            currLdRes.MainFogCoefficients[0] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_0").getDouble() : 0.0f;
            currLdRes.MainFogCoefficients[1] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_1").getDouble(): 0.0f;
            currLdRes.MainFogCoefficients[2] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_2").getDouble(): 0.0f;
            currLdRes.MainFogCoefficients[3] = hasMainFogCoefficients ? getLightData.getField("MainFogCoefficients_3").getDouble(): 0.0f;
            currLdRes.HeightDensityFogCoeff[0] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_0").getDouble() : 0.0f;
            currLdRes.HeightDensityFogCoeff[1] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_1").getDouble(): 0.0f;
            currLdRes.HeightDensityFogCoeff[2] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_2").getDouble(): 0.0f;
            currLdRes.HeightDensityFogCoeff[3] = hasHeightDensityFogCoeff ? getLightData.getField("HeightDensityFogCoeff_3").getDouble(): 0.0f;

            currLdRes.time = getLightData.getField("Time").getInt();

            //Hacks for loading of fog information
            if (!currLdRes.EndFogColor) {
                currLdRes.EndFogColor = currLdRes.SkyFogColor;
            }
            if (!currLdRes.FogHeightColor) {
                currLdRes.FogHeightColor = currLdRes.SkyFogColor;
            }
            if (!currLdRes.EndFogHeightColor) {
                currLdRes.EndFogHeightColor = currLdRes.EndFogColor;
            }
            currLdRes.FogScaler = std::clamp(currLdRes.FogScaler, -1.0f, 1.0f);
            currLdRes.FogEnd = std::max<float>(currLdRes.FogEnd, 10.0f);
            currLdRes.FogHeight = std::max<float>(currLdRes.FogHeight, -10000.0f);
            currLdRes.FogHeightScaler = std::clamp(currLdRes.FogHeightScaler, -1.0f, 1.0f);
            if (!currLdRes.SunFogColor)
                currLdRes.SunFogAngle = 1.0f;

            if (currLdRes.FogHeight > 10000.0f)
                currLdRes.FogHeight = 0.0f;

            if (currLdRes.FogDensity <= 0.0f) {
                float a = std::min<float>(farClip, 700.0) - 200.0;
                float b = currLdRes.FogEnd - (float)(currLdRes.FogEnd * lastLdRes.FogScaler);
                if ( b > a || a <= 0.0 )
                    currLdRes.FogDensity = 1.5;
                else
                    currLdRes.FogDensity = (float)((float)(1.0f - (float)(b / a)) * 5.5f) + 1.5f;
            }

//            if (currLdRes.FogHeightScaler == 0.0) {
//                currLdRes.FogHeightDensity = currLdRes.FogDensity;
//            }

            if (currLdRes.time > ptime) {
                assigned = true;
                //Blend using time as alpha
                float timeAlphaBlend = 1.0f - (((float) currLdRes.time - (float) ptime) /
                                               ((float) currLdRes.time - (float) lastLdRes.time));

                //Hack for SunFogAngle
                {
                    auto SunFogAngle2 = currLdRes.SunFogAngle;
                    auto SunFogAngle1 = lastLdRes.SunFogAngle;
                    if (SunFogAngle2 >= 1.0f && SunFogAngle1 >= 1.0f) {
                        lightResult.SunAngleBlend = 0.0f;
                        lightResult.SunFogStrength = 0.0f;
                        lightResult.SunFogAngle = 1.0f;
                    } else
                    if (SunFogAngle1 < 1.0 && SunFogAngle2 < 1.0) {
                            lightResult.SunAngleBlend = 1.0f;
                            ::blendTwoAndAdd(lightResult.SunFogStrength,
                                             currLdRes.SunFogStrength, lastLdRes.SunFogStrength,
                                             timeAlphaBlend);
                            ::blendTwoAndAdd(lightResult.SunFogAngle,
                                             currLdRes.SunFogAngle, lastLdRes.SunFogAngle,
                                             timeAlphaBlend);
                            lightResult.SunFogAngle = 1.0f;
                    } else
                    if ((SunFogAngle2 >= 1.0f && SunFogAngle2 >= 1.0) || (SunFogAngle1 < 1.0 && SunFogAngle2 >= 1.0))
                    {
                        lightResult.SunAngleBlend = 1.0f - timeAlphaBlend;
                        lightResult.SunFogStrength = lastLdRes.SunFogStrength;
                        lightResult.SunFogAngle = lastLdRes.SunFogAngle;
                    } else {
                        lightResult.SunAngleBlend = timeAlphaBlend;
                        lightResult.SunFogAngle = SunFogAngle2;
                        lightResult.SunFogStrength = currLdRes.SunFogStrength;
                    }
                }
                if (lastLdRes.time == -1) {
                    addOnlyOne(lightResult, currLdRes);
                } else {
                    blendTwoAndAdd(lightResult, lastLdRes, currLdRes, timeAlphaBlend);
                }
                break;
            }
            lastLdRes = currLdRes;
        }

        if (!assigned) {
            addOnlyOne(lightResult, lastLdRes);
        }

        //Hack for LegacyFogScaler
        if (
            arr4SqrLength(lightResult.MainFogCoefficients) > 0.00000011920929f ||
            arr4SqrLength(lightResult.MainFogCoefficients) > 0.00000011920929f
            ) {
            lightResult.LegacyFogScalar = 0.0f;
        } else {
            lightResult.LegacyFogScalar = 1.0f;
        }

        lightResults.push_back(lightResult);
    }
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
