//
// Created by Deamon on 1/22/2021.
//

#include "CMinimapDataDB.h"
#include <SQLiteCpp/Transaction.h>
#include <SQLiteCpp/Database.h>
#include <sqlite3.h>


CMinimapDataDB::StatementFieldHolder::StatementFieldHolder(SQLite::Database &database, const std::string &query) :
    m_query(database, query) {

    for (int i = 0; i < m_query.getColumnCount(); i++) {
        fieldToIndex[CalculateFNV(m_query.getColumnName(i))] = i;
    }
}

void CMinimapDataDB::StatementFieldHolder::setInputs() {
    this->m_query.reset();
}

template<class... Ts>
void CMinimapDataDB::StatementFieldHolder::setInputs(Ts &&... inputs) {
    int i = 1;
    this->m_query.reset();
    ((this->m_query.bind(i++, inputs)), ...);
}

bool CMinimapDataDB::StatementFieldHolder::execute() {
    return m_query.executeStep();
}
SQLite::Column CMinimapDataDB::StatementFieldHolder::getField(const HashedString fieldName) {
    int index = getFieldIndex(fieldName);

    if (index >= 0) {
        return m_query.getColumn(index);
    } else {
        throw SQLite::Exception("Field was not found in query");
    }
}

const std::string getAllScenariosSQL =
R"===(
        select s.id, s.orientation, s.name,
           s.ocean_color_0, s.ocean_color_1, s.ocean_color_2, s.ocean_color_3,
           s.world_coord_min_x, s.world_coord_min_y, s.world_coord_max_x, s.world_coord_max_y,
           s.image_width, s.image_height, s.zoom, s.folderToSave from scenarios s;
)===";

const std::string insertNewScenariosSQL =
R"===(
    insert into scenarios (orientation,
                          ocean_color_0, ocean_color_1, ocean_color_2, ocean_color_3,
                          world_coord_min_x, world_coord_min_y, world_coord_max_x, world_coord_max_y,
                          image_width, image_height, zoom, folderToSave)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
)===";

const std::string updateScenarioSQL =
R"===(
     update scenarios set map_id = ?, orientation = ?,
         ocean_color_0 = ?, ocean_color_1 = ?, ocean_color_2 = ?, ocean_color_3 = ?,
         world_coord_min_x = ?, world_coord_min_y = ?, world_coord_max_x = ?, world_coord_max_y = ?,
         image_width = ?, image_height = ?, zoom = ?, folderToSave = ?
         where id = ?;
)===";

const std::string getADTBoundingBoxSQL =
R"===(
select abb.adt_x, abb.adt_y,
       abb.min_z,
       abb.max_z from adt_bounding_boxes abb
   where map_id = ?
)===";

const std::string insertADTBoundingBoxesSQL =
    R"===(
    insert into adt_bounding_boxes(map_id,
       adt_x, adt_y,
       min_z, max_z)
    values (?, ?, ?, ?, ?)
)===";

const std::string getMapDefSQL =
    R"===(
    select
        smd.map_id,
        smd.deltaX, smd.deltaY, smd.deltaZ
        from scenario_map_def smd
        where smd.scenario_id = ?
)===";

const std::string insertMapDefSQL =
    R"===(
    insert into scenario_map_def(
       scenario_id,
       map_id,
       deltaX, deltaY, deltaZ)
    values (?, ?, ?, ?, ?)
)===";

const std::string updateMapDefSQL =
    R"===(
    update scenario_map_def set map_id = ?,
       deltaX = ?, deltaY = ?, deltaZ = ?
    where scenario_id = ?
)===";

const std::string getAdtExcludedSQL =
R"===(
    select
        exl.adt_x, exl.adt_y,
        exl.chunk_x, exl.chunk_y
        from scenario_map_def_excluded_adt exl
        where exl.map_definition_id = ?
)===";

const std::string insertAdtExcludedSQL =
    R"===(
    insert into scenario_map_def_excluded_adt(map_definition_id,
       adt_x, adt_y,
       min_z, max_z)
    values (?, ?, ?, ?, ?)
)===";

CMinimapDataDB::CMinimapDataDB(const std::string &fileName) :
    m_sqliteDatabase(fileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getAllScenarios(InitDB(m_sqliteDatabase), getAllScenariosSQL),
    insertNewScenario(InitDB(m_sqliteDatabase), insertNewScenariosSQL),
    updateScenario(InitDB(m_sqliteDatabase), updateScenarioSQL),
    getADTBoundingBoxes(InitDB(m_sqliteDatabase), getADTBoundingBoxSQL),
    insertADTBoundingBoxes(InitDB(m_sqliteDatabase), insertADTBoundingBoxesSQL),

    getMapDef(InitDB(m_sqliteDatabase), getMapDefSQL),
    insertMapDef(InitDB(m_sqliteDatabase), insertMapDefSQL),
    updateMapDef(InitDB(m_sqliteDatabase), updateMapDefSQL),

    getADTExcluded(InitDB(m_sqliteDatabase), getAdtExcludedSQL),
    insertADTExcluded(InitDB(m_sqliteDatabase), insertAdtExcludedSQL)
{
    char *sErrMsg = "";
//    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
//    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA schema.journal_mode = MEMORY", NULL, NULL, &sErrMsg);
}

void CMinimapDataDB::getScenarios(std::vector<ScenarioDef> &scenarioList) {
    if (!m_sqliteDatabase.tableExists("scenarios"))
        return;

    getAllScenarios.setInputs();

    while (getAllScenarios.execute())
    {
        ScenarioDef scenarioDef;

        scenarioDef.id = getAllScenarios.getField("id").getInt();

        scenarioDef.orientation = static_cast<ScenarioOrientation>(getAllScenarios.getField("orientation").getInt());
        scenarioDef.name = getAllScenarios.getField("name").getString();
        scenarioDef.closeOceanColor = mathfu::vec4(
            getAllScenarios.getField("ocean_color_0").getDouble(),
            getAllScenarios.getField("ocean_color_1").getDouble(),
            getAllScenarios.getField("ocean_color_2").getDouble(),
            getAllScenarios.getField("ocean_color_3").getDouble()
        );
        scenarioDef.minWowWorldCoord = mathfu::vec2(getAllScenarios.getField("world_coord_min_x").getDouble(),
                                                    getAllScenarios.getField("world_coord_min_y").getDouble());
        scenarioDef.maxWowWorldCoord = mathfu::vec2(getAllScenarios.getField("world_coord_max_x").getDouble(),
                                                    getAllScenarios.getField("world_coord_max_y").getDouble());

        scenarioDef.imageWidth = getAllScenarios.getField("image_width").getInt();
        scenarioDef.imageHeight = getAllScenarios.getField("image_height").getInt();

        scenarioDef.zoom = getAllScenarios.getField("zoom").getDouble();
        scenarioDef.folderToSave = getAllScenarios.getField("folderToSave").getDouble();

        scenarioList.push_back(scenarioDef);
    }

    for (auto &scenario : scenarioList) {
        this->getMapRenderDef(scenario.id, scenario.maps);
    }
}

void CMinimapDataDB::saveScenarios(std::vector<ScenarioDef> &scenarios) {
    SQLite::Transaction transaction(m_sqliteDatabase);

    for (auto &scenario : scenarios) {
        if (scenario.id == -1) {
            insertNewScenario.setInputs(
                (int) scenario.orientation,
                scenario.closeOceanColor.x,
                scenario.closeOceanColor.y,
                scenario.closeOceanColor.z,
                scenario.closeOceanColor.w,
                scenario.minWowWorldCoord.x,
                scenario.minWowWorldCoord.y,
                scenario.maxWowWorldCoord.x,
                scenario.maxWowWorldCoord.y,
                scenario.imageWidth,
                scenario.imageHeight,
                scenario.zoom,
                scenario.folderToSave.c_str()
            );

            if (insertNewScenario.execute()) {
                scenario.id = m_sqliteDatabase.getLastInsertRowid();
            }
        } else {
            updateScenario.setInputs(
                (int) scenario.orientation,
                scenario.closeOceanColor.x,
                scenario.closeOceanColor.y,
                scenario.closeOceanColor.z,
                scenario.closeOceanColor.w,
                scenario.minWowWorldCoord.x,
                scenario.minWowWorldCoord.y,
                scenario.maxWowWorldCoord.x,
                scenario.maxWowWorldCoord.y,
                scenario.imageWidth,
                scenario.imageHeight,
                scenario.zoom,
                scenario.folderToSave.c_str(),
                scenario.id
            );

            updateScenario.execute();
        }
    }

    // Commit transaction
    transaction.commit();
}

void CMinimapDataDB::getAdtBoundingBoxes(MapRenderDef& mapRenderDef) {
    getADTBoundingBoxes.setInputs(mapRenderDef.mapId);

    while (getADTBoundingBoxes.execute())
    {
        int adt_x = getADTBoundingBoxes.getField("adt_x").getInt();
        int adt_y = getADTBoundingBoxes.getField("adt_y").getInt();

        mapRenderDef.adtConfigHolder->adtMinZ[adt_x][adt_y] =  getADTBoundingBoxes.getField("min_z").getDouble();
        mapRenderDef.adtConfigHolder->adtMaxZ[adt_x][adt_y] =  getADTBoundingBoxes.getField("max_z").getDouble();
    }
}

void CMinimapDataDB::saveAdtBoundingBoxes(MapRenderDef& mapRenderDef) {
    SQLite::Transaction transaction(m_sqliteDatabase);

    //1. Clear all records for that mapId from DB
    {
        SQLite::Statement cleanABB(m_sqliteDatabase,
                                          "delete from adt_bounding_boxes where map_id = ?"
        );
        cleanABB.exec();
    }

    //2. Insert into database
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            insertADTBoundingBoxes.setInputs(mapRenderDef.mapId, i, j,
                                             mapRenderDef.adtConfigHolder->adtMinZ[i][j],
                                             mapRenderDef.adtConfigHolder->adtMaxZ[i][j]);
            insertADTBoundingBoxes.execute();
        }
    }

    //3. Commit transaction
    transaction.commit();
}

void CMinimapDataDB::getAdtExcluded(MapRenderDef& mapRenderDef) {
    getADTExcluded.setInputs(mapRenderDef.id);

    while (getADTExcluded.execute())
    {
        int adt_x = getADTExcluded.getField("adt_x").getInt();
        int adt_y = getADTExcluded.getField("adt_y").getInt();
        int chunk_x = getADTExcluded.getField("chunk_x").getInt();
        int chunk_y = getADTExcluded.getField("chunk_y").getInt();

        AdtCell adtCell = {adt_x, adt_y};
        AdtCell chunkCell = {chunk_x, chunk_y};
        if (chunk_x == -1 || chunk_y == -1) {\

            mapRenderDef.adtConfigHolder->excludedADTs.insert(adtCell);
        } else {
            mapRenderDef.adtConfigHolder->excludedChunksPerADTs[adtCell].insert(chunkCell);
        }
    }
}
void CMinimapDataDB::saveAdtExcluded(MapRenderDef& mapRenderDef){

}

void CMinimapDataDB::getRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides) {
//    SQLite::Statement getRiverColor(m_sqliteDatabase,
//                               "select rco.liquid_id, rco.color_0, rco.color_1, rco.color_2, rco.color_3 "
//                               " from river_color_overrides rco where rco.map_id = ?;"
//    );
//    getRiverColor.reset();
//    getRiverColor.bind(1, mapId);
//
//    while (getRiverColor.executeStep())
//    {
//        RiverColorOverride colorOverride;
//        colorOverride.liquidId = getRiverColor.getColumn(0).getInt();
//        colorOverride.color = mathfu::vec4(
//            getRiverColor.getColumn(1).getDouble(),
//            getRiverColor.getColumn(2).getDouble(),
//            getRiverColor.getColumn(3).getDouble(),
//            getRiverColor.getColumn(4).getDouble()
//        );
//        riverOverrides.push_back(colorOverride);
//    }
}

void CMinimapDataDB::saveRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides) {
//    SQLite::Transaction transaction(m_sqliteDatabase);
//
//    //1. Clear all records for that mapId from DB
//    {
//        SQLite::Statement cleanRiverColor(m_sqliteDatabase,
//                                   "delete from river_color_overrides where map_id = ?"
//        );
//        cleanRiverColor.bind(1, mapId);
//        cleanRiverColor.exec();
//    }
//
//    SQLite::Statement saveRiverColor(m_sqliteDatabase, "insert into river_color_overrides(map_id, liquid_id, "
//                                                       "color_0, color_1, color_2, color_3) \n"
//                                                       "values (?, ?, ?, ?, ?, ?);");
//
//    for (int j = 0; j < riverOverrides.size(); j++) {
//        auto &roRec = riverOverrides[j];
//        saveRiverColor.reset();
//
//        saveRiverColor.bind(1, mapId);
//        saveRiverColor.bind(2, roRec.liquidId);
//        saveRiverColor.bind(3, roRec.color.x);
//        saveRiverColor.bind(4, roRec.color.y);
//        saveRiverColor.bind(5, roRec.color.z);
//        saveRiverColor.bind(6, roRec.color.w);
//
//        saveRiverColor.exec();
//    }
//    transaction.commit();
}

SQLite::Database &CMinimapDataDB::InitDB(SQLite::Database &database) {

    //-------------------------------
    // Create Scenario table
    // ------------------------------
    {
        SQLite::Transaction transaction(database);

        database.exec("CREATE TABLE IF NOT EXISTS scenarios\n"
                              "(\n"
                              "    id                   INTEGER PRIMARY KEY,\n"
                              "    name                 VARCHAR(256),\n"
                              "    orientation          INTEGER,\n"
                              "    ocean_color_0        FLOAT,\n"
                              "    ocean_color_1        FLOAT,\n"
                              "    ocean_color_2        FLOAT,\n"
                              "    ocean_color_3        FLOAT,\n"
                              "    world_coord_min_x    FLOAT,\n"
                              "    world_coord_min_y    FLOAT,\n"
                              "    world_coord_max_x    FLOAT,\n"
                              "    world_coord_max_y    FLOAT,\n"
                              "    image_height         INTEGER,\n"
                              "    image_width          INTEGER,\n"
                              "    zoom                 FLOAT,\n"
                              "    folderToSave         VARCHAR(256)\n"
                              ");");

        transaction.commit();
    }

    //-------------------------------
    // Create Bounding boxes table
    // ------------------------------
    {
        SQLite::Transaction transaction(database);

        database.exec("CREATE TABLE IF NOT EXISTS adt_bounding_boxes (\n"
                              "    id INTEGER PRIMARY KEY,\n"
                              "    map_id INTEGER,\n"
                              "    adt_x INTEGER,\n"
                              "    adt_y INTEGER,\n"
                              "    min_z FLOAT,\n"
                              "    max_z FLOAT\n"
                              ")");

        transaction.commit();
    }

    //-------------------------------
    // Create map definition per scenario
    // ------------------------------
    {
        SQLite::Transaction transaction(database);

        database.exec("CREATE TABLE IF NOT EXISTS scenario_map_def (\n"
                              "    scenario_id INTEGER,\n"
                              "    map_id INTEGER,\n"
                              "    deltaX DOUBLE,\n"
                              "    deltaY DOUBLE,\n"
                              "    deltaZ DOUBLE\n"
                              ")");

        transaction.commit();
    }

    //-------------------------------
    // Create excluded ADTs per map_def
    // ------------------------------
    {
        SQLite::Transaction transaction(database);

        database.exec("CREATE TABLE IF NOT EXISTS scenario_map_def (\n"
                              "    id INTEGER PRIMARY KEY,\n"
                              "    map_definition_id INTEGER,\n"
                              "    adt_x INTEGER,\n"
                              "    adt_y INTEGER\n"
                              ")");

        transaction.commit();
    }

    //-------------------------------
    // Create excluded ADT CELLS/chunks per map_def
    // ------------------------------
    {
        SQLite::Transaction transaction(database);

        database.exec("CREATE TABLE IF NOT EXISTS scenario_map_def_excluded_adt (\n"
                              "    map_definition_id INTEGER,\n"
                              "    adt_x INTEGER,\n"
                              "    adt_y INTEGER,\n"
                              "    chunk_x INTEGER,\n"
                              "    chunk_y INTEGER\n"
                              ")");

        transaction.commit();
    }


    //-------------------------------
    // Create river color overrides
    // ------------------------------

    {
        SQLite::Transaction transaction(database);

        database.exec("CREATE TABLE IF NOT EXISTS river_color_overrides\n"
                              "(\n"
                              "    id      INTEGER PRIMARY KEY,\n"
                              "    map_id  INTEGER,\n"
                              "    liquid_id INTEGER,\n"
                              "    color_0 FLOAT,\n"
                              "    color_1 FLOAT,\n"
                              "    color_2 FLOAT,\n"
                              "    color_3 FLOAT\n"
                              ")");

        transaction.commit();
    }

    return database;
}

void CMinimapDataDB::getMapRenderDef(int scenarioId, std::vector<MapRenderDef> &mapRenderDefs) {
    getMapDef.setInputs(scenarioId);

    while (getMapDef.execute()) {
        auto &mapRenderDef = mapRenderDefs.emplace_back();
        mapRenderDef.mapId = getMapDef.getField("map_id").getInt();
        mapRenderDef.deltaX = getMapDef.getField("deltaX").getDouble();
        mapRenderDef.deltaY = getMapDef.getField("deltaY").getDouble();
        mapRenderDef.deltaZ = getMapDef.getField("deltaZ").getDouble();
    }

    for (auto &mapRenderDef : mapRenderDefs) {
        this->getAdtBoundingBoxes(mapRenderDef);
        this->getAdtExcluded(mapRenderDef);
    }
}

void CMinimapDataDB::saveMapRenderDef(int scenarioId, MapRenderDef& mapRenderDef) {

}