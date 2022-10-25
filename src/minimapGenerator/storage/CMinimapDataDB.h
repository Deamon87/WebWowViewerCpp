//
// Created by Deamon on 1/22/2021.
//

#ifndef AWEBWOWVIEWERCPP_CMINIMAPDATADB_H
#define AWEBWOWVIEWERCPP_CMINIMAPDATADB_H

#include <SQLiteCpp/Database.h>
#include "../entities.h"
#include "../minimapGenerator.h"
#include "../../../wowViewerLib/src/engine/algorithms/hashString.h"


class CMinimapDataDB {
private:
    SQLite::Database m_sqliteDatabase;
public:
    explicit CMinimapDataDB(const std::string &fileName);

    //Scenarios
    void getScenarios(std::vector<ScenarioDef> &scenarioList);
    void saveScenarios(std::vector<ScenarioDef> &scenarios);

    //ADT Bounding boxes
    void getMapRenderDef(int scenarioId, std::vector<MapRenderDef> &mapRenderDef);
    void saveMapRenderDef(int scenarioId, MapRenderDef& mapRenderDef);

    void getAdtBoundingBoxes(MapRenderDef& mapRenderDef);
    void saveAdtBoundingBoxes(MapRenderDef& mapRenderDef);

    void getAdtExcluded(MapRenderDef& mapRenderDef);
    void saveAdtExcluded(MapRenderDef& mapRenderDef);

    //River color overrides
    void getRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides);
    void saveRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides);

    //Get WMO ADT aabb override
    bool getWmoAABBOverride(int mapId, int uniqueId, CAaBox &aaBox);
    void saveWmoAABBOverride(int mapId, int uniqueId, const CAaBox &aaBox);

private:
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
    private:
        SQLite::Statement m_query;
        std::unordered_map<size_t, int> fieldToIndex;
    };

    SQLite::Database &InitDB(SQLite::Database &database);

    StatementFieldHolder getAllScenarios;
    StatementFieldHolder insertNewScenario;
    StatementFieldHolder updateScenario;
    StatementFieldHolder getADTBoundingBoxes;
    StatementFieldHolder insertADTBoundingBoxes;

    StatementFieldHolder getMapDef;
    StatementFieldHolder insertMapDef;
    StatementFieldHolder updateMapDef;

    StatementFieldHolder getADTExcluded;
    StatementFieldHolder insertADTExcluded;
};


#endif //AWEBWOWVIEWERCPP_CMINIMAPDATADB_H
