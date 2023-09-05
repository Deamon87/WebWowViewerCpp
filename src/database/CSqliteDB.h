//
// Created by deamon on 24.12.19.
//

#ifndef AWEBWOWVIEWERCPP_CSQLITEDB_H
#define AWEBWOWVIEWERCPP_CSQLITEDB_H

#include <vector>
#include <functional>
#include <unordered_map>
#include <SQLiteCpp/Database.h>
#include "../../wowViewerLib/src/include/databaseHandler.h"
#include "../../wowViewerLib/src/engine/algorithms/hashString.h"


class CSqliteDB : public IClientDatabase {
public:
    explicit CSqliteDB(std::string dbFileName);
    void getMapArray(std::vector<MapRecord> &mapRecords) override;
    bool getMapById(int mapId, MapRecord &mapRecord) override;
    AreaRecord getArea(int areaId) override;
    AreaRecord getWmoArea(int wmoId, int nameId, int groupId) override;

    void getEnvInfo(int mapId, float x, float y, float z, int time, std::vector<LightResult> &lightResults, float farClip) override;
    void getLightById(int lightId, int time, LightResult &lightResult, float farClip) override;
    void getLiquidObjectData(int liquidObjectId, int fallbackliquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) override;
    void getLiquidTypeData(int liquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) override;
    void getLiquidTexture(int liquidTypeId, std::vector<LiquidTextureData> &textures);
    void getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) override;

private:
    std::string generateSimpleSelectSQL(const std::string &tableName,
                                        const std::vector<std::string> &skipColumnNames,
                                        const std::string &whereClause);

    class StatementFieldHolder {
    public:
        StatementFieldHolder(SQLite::Database &database, const std::string &query);

        template <class ... Ts>
        void setInputs(Ts && ... inputs);

        void setInputs();
        bool execute();
        SQLite::Column getField(HashedString fieldName);
        inline int getFieldIndex(HashedString fieldName) {
            auto it = fieldToIndex.find(fieldName.Hash());
            if(it != fieldToIndex.end()) {
                return it->second;
            } else {
                return -1;
            }
        }
        template<int N, typename T, char const* chars>
        inline void readArray(std::array<T, N> &data, const std::function<T (SQLite::Column &)> &fieldToValue) {
            constexpr std::string_view fieldName = std::string(chars) + std::to_string(N);
            constexpr const char *c_fieldName = fieldName.data();
            constexpr auto hashedString = HashedString(c_fieldName);

            data[N - 1] = fieldToValue(this->getField(hashedString));
            if constexpr (N > 0) {
                this->readArray<N - 1>(data, fieldToValue);
            } else {
                this->readArrayZero<chars>(data, fieldToValue);
            }
        }
    private:
        SQLite::Statement m_query;
        std::unordered_map<size_t, int> fieldToIndex;


        template<typename T, char... chars>
        inline void readArrayZero(std::vector<T> &data, const std::function<T (SQLite::Column &)> &fieldToValue) {
            constexpr std::string_view fieldName = std::string(chars...) + std::to_string(0);
            constexpr const char *c_fieldName = fieldName.data();
            constexpr auto hashedString = HashedString(c_fieldName);

            data[0] = fieldToValue(this->getField(hashedString));
        }

   };

    SQLite::Database m_sqliteDatabase;

    StatementFieldHolder getWmoAreaAreaName;
    StatementFieldHolder getAreaNameStatement;
    StatementFieldHolder getLightStatement;
    StatementFieldHolder getLightByIdStatement;
    StatementFieldHolder getLightData;
    StatementFieldHolder getLiquidObjectInfo;
    StatementFieldHolder getLiquidTypeInfo;
    StatementFieldHolder getLiquidTextureFileDataIds;

    StatementFieldHolder getZoneLightInfo;
    StatementFieldHolder getZoneLightPointsInfo;
    StatementFieldHolder getMapList;
    StatementFieldHolder getMapByIdStatement;

    bool getHasLiquidTypeXTexture(SQLite::Database &sqliteDatabase) {
        bool m_hasLiquidTypeXTexture = sqliteDatabase.tableExists("LiquidTypeXTexture") ? 1 : 0;

        return m_hasLiquidTypeXTexture;
    }

    bool getHasWDTId(SQLite::Database &sqliteDatabase) {
        bool m_hasWdtId;
        try {
            sqliteDatabase.execAndGet("select WdtFileDataID from Map");
            m_hasWdtId = true;
        } catch (...) {
            m_hasWdtId = false;
        }

        return m_hasWdtId;
    }

    std::vector<std::string> getTableFields(SQLite::Database &sqliteDatabase, const std::string &tableName) {
        std::vector<std::string> result;
        SQLite::Statement statement(sqliteDatabase, "PRAGMA table_info('"+tableName+"') ");
        try {
            while (statement.executeStep()) {
                result.push_back(statement.getColumn("name").getString());
            }
        } catch (...) {

        }

        return result;
    }

    struct InnerLightResult {
        float pos[3];
        float fallbackStart;
        float fallbackEnd;
        float lightDistSQR = 0;
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
        int horizontAmbientColor = 0;
        int groundAmbientColor = 0;

        int directLight;
        int closeRiverColor;
        int farRiverColor;
        int closeOceanColor;
        int farOceanColor;

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
        float FogZScalar;
        float MainFogStartDist;
        float MainFogEndDist;
        float SunFogAngle;
        int EndFogColor;
        float EndFogColorDistance;
        float FogStartOffset;
        int SunFogColor;
        float SunFogStrength;
        int FogHeightColor;
        int EndFogHeightColor;
        float FogHeightCoefficients[4];
        float MainFogCoefficients[4];
        float HeightDensityFogCoeff[4];

        int time;
    };

    void convertInnerResultsToPublic(int ptime, std::vector<LightResult> &lightResults,
                                     std::vector<InnerLightResult> &innerResults, float farClip);

    void
    addOnlyOne(LightResult &lightResult, const CSqliteDB::InnerLightDataRes &currLdRes) const;

    void
    blendTwoAndAdd(LightResult &lightResult, const InnerLightDataRes &lastLdRes,
                   const InnerLightDataRes &currLdRes,
                   float timeAlphaBlend) const;

};

#endif //AWEBWOWVIEWERCPP_CSQLITEDB_H
