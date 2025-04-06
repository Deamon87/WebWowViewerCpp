//
// Created by Deamon on 1/22/2021.
//

#include "CMinimapDataDB.h"
#include <SQLiteCpp/Transaction.h>
#include <SQLiteCpp/Database.h>
#include <sqlite3.h>

CMinimapDataDB::DataBaseClass::DataBaseClass(const std::string &fileName) : m_storage(makeStorage(fileName)) {
    char *sErrMsg = "";
//    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
//    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA schema.journal_mode = MEMORY", NULL, NULL, &sErrMsg);

    m_storage.sync_schema(true);
}

void CMinimapDataDB::DataBaseClass::getScenarios(std::vector<ScenarioDef> &scenarioList) {
    scenarioList = m_storage.get_all<ScenarioDef>();

    for (auto &scenario : scenarioList) {
        this->getMapRenderDef(scenario.id, scenario.maps);
    }
}

void CMinimapDataDB::DataBaseClass::saveScenarios(std::vector<ScenarioDef> &scenarios) {
    for (auto &scenario : scenarios) {
        saveScenario(scenario);
    }
}

void CMinimapDataDB::DataBaseClass::saveScenario(ScenarioDef &scenario) {
    using namespace sqlite_orm;

    auto &scenarioId = scenario.id;
    if(m_storage.count<ScenarioDef>(where(c(&ScenarioDef::id) == scenarioId))) {
        m_storage.update(scenario);
    } else {
        scenarioId = m_storage.insert(scenario);
    }

    //Save arrays
    for (auto &mapDef : scenario.maps) {
        saveMapRenderDef(scenarioId, mapDef);
    }
}

void CMinimapDataDB::DataBaseClass::getAdtBoundingBoxes(MapRenderDef& mapRenderDef) {
    using namespace sqlite_orm;
    auto adtBoundingBoxesDB = m_storage.get_all<ADTBoundingBoxDB>(where(c(&ADTBoundingBoxDB::map_id) == mapRenderDef.mapId));

    for (auto &adtBoundingBoxeDB : adtBoundingBoxesDB) {
        mapRenderDef.adtConfigHolder->adtMin[adtBoundingBoxeDB.adt_x][adtBoundingBoxeDB.adt_y] = {adtBoundingBoxeDB.adtMin_x, adtBoundingBoxeDB.adtMin_y, adtBoundingBoxeDB.adtMin_z} ;
        mapRenderDef.adtConfigHolder->adtMax[adtBoundingBoxeDB.adt_x][adtBoundingBoxeDB.adt_y] = {adtBoundingBoxeDB.adtMax_x, adtBoundingBoxeDB.adtMax_y, adtBoundingBoxeDB.adtMax_z};
    }
}

void CMinimapDataDB::DataBaseClass::saveAdtBoundingBoxes(MapRenderDef& mapRenderDef) {
    using namespace sqlite_orm;

    m_storage.remove_all<ADTBoundingBoxDB>(where(c(&ADTBoundingBoxDB::map_id) == mapRenderDef.mapId));

    //2. Insert into database
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            ADTBoundingBoxDB boxDb;
            boxDb.map_id = mapRenderDef.mapId;
            boxDb.adt_x = i;
            boxDb.adt_y = j;
            boxDb.adtMin_x = mapRenderDef.adtConfigHolder->adtMin[i][j].x;
            boxDb.adtMin_y = mapRenderDef.adtConfigHolder->adtMin[i][j].y;
            boxDb.adtMin_z = mapRenderDef.adtConfigHolder->adtMin[i][j].z;
            boxDb.adtMax_x = mapRenderDef.adtConfigHolder->adtMax[i][j].x;
            boxDb.adtMax_y = mapRenderDef.adtConfigHolder->adtMax[i][j].y;
            boxDb.adtMax_z = mapRenderDef.adtConfigHolder->adtMax[i][j].z;

            m_storage.insert(boxDb);
        }
    }
}

void CMinimapDataDB::DataBaseClass::getAdtExcluded(MapRenderDef& mapRenderDef) {
    using namespace sqlite_orm;
    auto excludedADTAndChunks = m_storage.get_all<ExcludedADT_and_chunk_DB>(
        where(c(&ExcludedADT_and_chunk_DB::map_definition_id) == mapRenderDef.id)
    );

    for (auto &excludedADTAndChunk : excludedADTAndChunks) {
        if (excludedADTAndChunk.chunk_x == -1 && excludedADTAndChunk.chunk_y == -1) {
            mapRenderDef.adtConfigHolder->excludedADTs.insert({excludedADTAndChunk.adt_x, excludedADTAndChunk.adt_y});
        } else {
            mapRenderDef.adtConfigHolder->
                excludedChunksPerADTs[{excludedADTAndChunk.adt_x, excludedADTAndChunk.adt_y}]
                    .insert({excludedADTAndChunk.chunk_x, excludedADTAndChunk.chunk_y});
        }
    }
}
void CMinimapDataDB::DataBaseClass::saveAdtExcluded(MapRenderDef& mapRenderDef){
    using namespace sqlite_orm;
    m_storage.remove_all<ExcludedADT_and_chunk_DB>(where(c(&ExcludedADT_and_chunk_DB::map_definition_id) == mapRenderDef.id));

    for (auto &excludedADT : mapRenderDef.adtConfigHolder->excludedADTs) {
        ExcludedADT_and_chunk_DB adtAndChunkDb;
        adtAndChunkDb.map_definition_id = mapRenderDef.id;
        adtAndChunkDb.adt_x = excludedADT[0];
        adtAndChunkDb.adt_y = excludedADT[1];
        adtAndChunkDb.chunk_x = -1;
        adtAndChunkDb.chunk_y = -1;

        m_storage.insert(adtAndChunkDb);
    }

    for (auto &excludedADTChunk : mapRenderDef.adtConfigHolder->excludedChunksPerADTs) {
        ExcludedADT_and_chunk_DB adtAndChunkDb;
        adtAndChunkDb.map_definition_id = mapRenderDef.id;
        adtAndChunkDb.adt_x = excludedADTChunk.first[0];
        adtAndChunkDb.adt_y = excludedADTChunk.first[1];
        for (auto &chunk : excludedADTChunk.second) {
            adtAndChunkDb.chunk_x = chunk[0];
            adtAndChunkDb.chunk_y = chunk[1];

            m_storage.insert(adtAndChunkDb);
        }
    }
}

void CMinimapDataDB::DataBaseClass::getMapRenderDef(int scenarioId, std::vector<MapRenderDef> &mapRenderDefs) {
    using namespace sqlite_orm;

    mapRenderDefs.clear();
    auto mapRenderDefsDB = m_storage.get_all<MapRenderDefDB>(where(c(&MapRenderDefDB::scenarioId) == scenarioId));
    for (auto const &mapRenderDefDB : mapRenderDefsDB) {
        auto &mapRenderDef = mapRenderDefs.emplace_back();
        mapRenderDef.id = mapRenderDefDB.id;
        mapRenderDef.mapId = mapRenderDefDB.mapId;
        mapRenderDef.deltaX = mapRenderDefDB.deltaX;
        mapRenderDef.deltaY = mapRenderDefDB.deltaY;
        mapRenderDef.deltaZ = mapRenderDefDB.deltaZ;
    }

    for (auto &mapRenderDef : mapRenderDefs) {
        this->getAdtBoundingBoxes(mapRenderDef);
        this->getAdtExcluded(mapRenderDef);
    }
}

void CMinimapDataDB::DataBaseClass::saveMapRenderDef(int scenarioId, MapRenderDef& mapRenderDef) {
    MapRenderDefDB defDb;
    defDb.id = mapRenderDef.id;
    defDb.mapId = mapRenderDef.mapId;
    defDb.scenarioId = scenarioId;
    defDb.deltaX = mapRenderDef.deltaX;
    defDb.deltaY = mapRenderDef.deltaY;
    defDb.deltaZ = mapRenderDef.deltaZ;

    if(defDb.id >= 0) {
        m_storage.update(defDb);
    } else {
        mapRenderDef.id = m_storage.insert(defDb);
    }

    saveAdtExcluded(mapRenderDef);
    saveAdtBoundingBoxes(mapRenderDef);
}

