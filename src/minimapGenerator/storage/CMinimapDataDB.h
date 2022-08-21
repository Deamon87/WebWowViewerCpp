//
// Created by Deamon on 1/22/2021.
//

#ifndef AWEBWOWVIEWERCPP_CMINIMAPDATADB_H
#define AWEBWOWVIEWERCPP_CMINIMAPDATADB_H

#include <SQLiteCpp/Database.h>
#include "../entities.h"
#include "../minimapGenerator.h"


class CMinimapDataDB {
private:
    SQLite::Database m_sqliteDatabase;

public:
    explicit CMinimapDataDB(std::string fileName);

    //Scenarios
    void getScenarios(std::vector<ScenarioDef> &scenarioList);
    void getScenario(int id, ScenarioDef &scenario);
    void saveScenario(ScenarioDef &scenario);

    //ADT Bounding boxes
    void getAdtBoundingBoxes(int mapId, ADTBoundingBoxHolder &boundingBoxHolder);
    void saveAdtBoundingBoxes(int mapId, ADTBoundingBoxHolder &boundingBoxHolder);

    //River color overrides
    void getRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides);
    void saveRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides);

    //Get WMO ADT aabb override
    bool getWmoAABBOverride(int mapId, int uniqueId, CAaBox &aaBox);
    void saveWmoAABBOverride(int mapId, int uniqueId, const CAaBox &aaBox);
};


#endif //AWEBWOWVIEWERCPP_CMINIMAPDATADB_H
