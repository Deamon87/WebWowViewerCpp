//
// Created by Deamon on 1/22/2021.
//

#include "CMinimapDataDB.h"
#include <SQLiteCpp/Transaction.h>
#include <SQLiteCpp/Database.h>
#include <sqlite3.h>

CMinimapDataDB::CMinimapDataDB(std::string fileName) : m_sqliteDatabase(fileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    char *sErrMsg = "";
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA schema.journal_mode = MEMORY", NULL, NULL, &sErrMsg);

    //-------------------------------
    // Create Scenario table
    // ------------------------------
    {
        SQLite::Transaction transaction(m_sqliteDatabase);

        m_sqliteDatabase.exec("CREATE TABLE IF NOT EXISTS scenarios\n"
                              "(\n"
                              "    id                   INTEGER PRIMARY KEY,\n"
                              "    map_id               INTEGER,\n"
                              "    orientation          INTEGER,\n"
                              "    ocean_color_0        FLOAT,\n"
                              "    ocean_color_1        FLOAT,\n"
                              "    ocean_color_2        FLOAT,\n"
                              "    ocean_color_3        FLOAT\n"
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
        SQLite::Transaction transaction(m_sqliteDatabase);

        m_sqliteDatabase.exec("CREATE TABLE IF NOT EXISTS adt_bounding_boxes (\n"
                              "    id INTEGER PRIMARY KEY,\n"
                              "    map_id INTEGER,\n"
                              "    adt_x INTEGER,\n"
                              "    adt_y INTEGER,\n"
                              "    min_x FLOAT,\n"
                              "    min_y FLOAT,\n"
                              "    min_z FLOAT,\n"
                              "    max_x FLOAT,\n"
                              "    max_y FLOAT,\n"
                              "    max_z FLOAT\n"
                              ")");

        transaction.commit();
    }

    //-------------------------------
    // Create river color overrides
    // ------------------------------

    {
        SQLite::Transaction transaction(m_sqliteDatabase);

        m_sqliteDatabase.exec("CREATE TABLE IF NOT EXISTS river_color_overrides\n"
                              "(\n"
                              "    id      INTEGER PRIMARY KEY,\n"
                              "    map_id  INTEGER,\n"
                              "    area_id INTEGER,\n"
                              "    color_0 FLOAT,\n"
                              "    color_1 FLOAT,\n"
                              "    color_2 FLOAT,\n"
                              "    color_3 FLOAT\n"
                              ")");

        transaction.commit();
    }

    //-------------------------------
    // Create WMO aabb overrides
    // ------------------------------
    {
        SQLite::Transaction transaction(m_sqliteDatabase);

        m_sqliteDatabase.exec("CREATE TABLE IF NOT EXISTS wmo_aabb_overrides\n"
                              "(\n"
                              "    id        INTEGER PRIMARY KEY,\n"
                              "    map_id    INTEGER,\n"
                              "    unique_id INTEGER,\n"
                              "    min_x     FLOAT,\n"
                              "    min_y     FLOAT,\n"
                              "    min_z     FLOAT,\n"
                              "    max_x     FLOAT,\n"
                              "    max_y     FLOAT,\n"
                              "    max_z     FLOAT\n"
                              ")");

        transaction.commit();
    }
}

void CMinimapDataDB::getScenarios(std::vector<ScenarioDef> &scenarioList) {
    SQLite::Statement getScenarioList(m_sqliteDatabase,
    "select s.id, s.map_id, s.orientation,\n"
    "       s.ocean_color_0, s.ocean_color_1, s.ocean_color_2, s.ocean_color_3,\n"
    "       s.world_coord_min_x, s.world_coord_min_y, s.world_coord_max_x, s.world_coord_max_y,\n"
    "       s.image_width, s.image_height, s.zoom, s.folderToSave from scenarios s;"
    );
    getScenarioList.reset();

    while (getScenarioList.executeStep())
    {
        ScenarioDef scenarioDef;

        scenarioDef.id = getScenarioList.getColumn(0).getInt();
        scenarioDef.mapId = getScenarioList.getColumn(1).getInt();
        scenarioDef.orientation = static_cast<ScenarioOrientation>(getScenarioList.getColumn(3).getInt());
        scenarioDef.closeOceanColor = mathfu::vec4(
            getScenarioList.getColumn(4).getDouble(),
            getScenarioList.getColumn(5).getDouble(),
            getScenarioList.getColumn(6).getDouble(),
            getScenarioList.getColumn(7).getDouble()
        );
        scenarioDef.minWowWorldCoord = mathfu::vec2(getScenarioList.getColumn(8).getDouble(),
                                                    getScenarioList.getColumn(9).getDouble());
        scenarioDef.maxWowWorldCoord = mathfu::vec2(getScenarioList.getColumn(10).getDouble(),
                                                    getScenarioList.getColumn(11).getDouble());

        scenarioDef.imageWidth = getScenarioList.getColumn(12).getInt();
        scenarioDef.imageHeight = getScenarioList.getColumn(13).getInt();

        scenarioDef.zoom = getScenarioList.getColumn(14).getDouble();
        scenarioDef.folderToSave = getScenarioList.getColumn(15).getDouble();

        scenarioList.push_back(scenarioDef);
    }
}

void CMinimapDataDB::getScenario(int id, ScenarioDef &scenario) {
    SQLite::Statement getScenarioById(m_sqliteDatabase,
                                      "select s.id, s.map_id, s.orientation,\n"
                                      "       s.ocean_color_0, s.ocean_color_1, s.ocean_color_2, s.ocean_color_3,\n"
                                      "       s.world_coord_min_x, s.world_coord_min_y, s.world_coord_max_x, s.world_coord_max_y,\n"
                                      "       s.image_width, s.image_height, s.zoom, s.folderToSave from scenarios s"
                                      "       where s.id = ?;"
    );
    getScenarioById.reset();
    getScenarioById.bind(1, id);

    while (getScenarioById.executeStep())
    {
        ScenarioDef scenarioDef;

        scenarioDef.id = getScenarioById.getColumn(0).getInt();
        scenarioDef.mapId = getScenarioById.getColumn(1).getInt();
        scenarioDef.orientation = static_cast<ScenarioOrientation>(getScenarioById.getColumn(3).getInt());
        scenarioDef.closeOceanColor = mathfu::vec4(
            getScenarioById.getColumn(4).getDouble(),
            getScenarioById.getColumn(5).getDouble(),
            getScenarioById.getColumn(6).getDouble(),
            getScenarioById.getColumn(7).getDouble()
        );
        scenarioDef.minWowWorldCoord = mathfu::vec2(getScenarioById.getColumn(8).getDouble(),
                                                    getScenarioById.getColumn(9).getDouble());
        scenarioDef.maxWowWorldCoord = mathfu::vec2(getScenarioById.getColumn(10).getDouble(),
                                                    getScenarioById.getColumn(11).getDouble());

        scenarioDef.imageWidth = getScenarioById.getColumn(12).getInt();
        scenarioDef.imageHeight = getScenarioById.getColumn(13).getInt();

        scenarioDef.zoom = getScenarioById.getColumn(14).getDouble();
        scenarioDef.folderToSave = getScenarioById.getColumn(15).getString();
    }
}

void CMinimapDataDB::saveScenario(ScenarioDef &scenario) {
    SQLite::Transaction transaction(m_sqliteDatabase);

    if (scenario.id == -1) {
        SQLite::Statement insertScenario(m_sqliteDatabase,
            "insert into scenarios (map_id, orientation, \n"
            "                      ocean_color_0, ocean_color_1, ocean_color_2, ocean_color_3, \n"
            "                      world_coord_min_x, world_coord_min_y, world_coord_max_x, world_coord_max_y, \n"
            "                      image_width, image_height, zoom, folderToSave) \n"
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
        );

        insertScenario.bind(1, scenario.mapId);
        insertScenario.bind(2, (int)scenario.orientation);
        insertScenario.bind(3, scenario.closeOceanColor.x);
        insertScenario.bind(4, scenario.closeOceanColor.y);
        insertScenario.bind(5, scenario.closeOceanColor.z);
        insertScenario.bind(6, scenario.closeOceanColor.w);
        insertScenario.bind(7, scenario.minWowWorldCoord.x);
        insertScenario.bind(8, scenario.minWowWorldCoord.y);
        insertScenario.bind(9, scenario.maxWowWorldCoord.x);
        insertScenario.bind(10, scenario.maxWowWorldCoord.y);
        insertScenario.bind(11, scenario.imageWidth);
        insertScenario.bind(12, scenario.imageHeight);
        insertScenario.bind(13, scenario.zoom);
        insertScenario.bind(14, scenario.folderToSave.c_str());
        insertScenario.exec();
    } else {
        SQLite::Transaction transaction(m_sqliteDatabase);

        SQLite::Statement updateScenario(m_sqliteDatabase,
                                         "update scenarios set map_id = ?, orientation = ?,\n"
                                         "    ocean_color_0 = ?, ocean_color_1 = ?, ocean_color_2 = ?, ocean_color_3 = ?,\n"
                                         "    world_coord_min_x = ?, world_coord_min_y = ?, world_coord_max_x = ?, world_coord_max_y = ?,\n"
                                         "    image_width = ?, image_height = ?, zoom = ?, folderToSave = ?\n"
                                         "    where id = ?;"
        );

        updateScenario.bind(1, scenario.mapId);
        updateScenario.bind(2, (int)scenario.orientation);
        updateScenario.bind(3, scenario.closeOceanColor.x);
        updateScenario.bind(4, scenario.closeOceanColor.y);
        updateScenario.bind(5, scenario.closeOceanColor.z);
        updateScenario.bind(6, scenario.closeOceanColor.w);
        updateScenario.bind(7, scenario.minWowWorldCoord.x);
        updateScenario.bind(8, scenario.minWowWorldCoord.y);
        updateScenario.bind(9, scenario.maxWowWorldCoord.x);
        updateScenario.bind(10, scenario.maxWowWorldCoord.y);
        updateScenario.bind(11, scenario.imageWidth);
        updateScenario.bind(12, scenario.imageHeight);
        updateScenario.bind(13, scenario.zoom);
        updateScenario.bind(14, scenario.folderToSave.c_str());
        updateScenario.bind(15, scenario.id);
        updateScenario.exec();
    }

    // Commit transaction
    transaction.commit();
}

void CMinimapDataDB::getAdtBoundingBoxes(int mapId, ADTBoundingBoxHolder &boundingBoxHolder) {
    SQLite::Statement getAdtBB(m_sqliteDatabase,
                                      "select abb.adt_x, abb.adt_y, \n"
                                      "       abb.min_x, abb.min_y, abb.min_z, \n"
                                      "       abb.max_x, abb.max_y, abb.max_z from adt_bounding_boxes abb \n"
                                      "   where map_id = ?"
    );
    getAdtBB.reset();
    getAdtBB.bind(1, mapId);

    while (getAdtBB.executeStep())
    {
        int adt_x = getAdtBB.getColumn(0).getInt();
        int adt_y = getAdtBB.getColumn(1).getInt();
        CAaBox aaBox = CAaBox(
            C3Vector(mathfu::vec3(
                getAdtBB.getColumn(2).getDouble(),
                getAdtBB.getColumn(3).getDouble(),
                getAdtBB.getColumn(4).getDouble()
            )),
            C3Vector(mathfu::vec3(
                getAdtBB.getColumn(5).getDouble(),
                getAdtBB.getColumn(6).getDouble(),
                getAdtBB.getColumn(7).getDouble()
            ))
        );

        boundingBoxHolder[adt_x][adt_y] = aaBox;
    }
}

void CMinimapDataDB::saveAdtBoundingBoxes(int mapId, ADTBoundingBoxHolder &boundingBoxHolder) {
    SQLite::Transaction transaction(m_sqliteDatabase);

    //1. Clear all records for that mapId from DB
    {
        SQLite::Statement cleanABB(m_sqliteDatabase,
                                          "delete from adt_bounding_boxes where map_id = ?"
        );
        cleanABB.exec();
    }

    //2. Insert into database
    SQLite::Statement insertBB(m_sqliteDatabase, "insert into adt_bounding_boxes(map_id, adt_x, adt_y, min_x, min_y, min_z, max_x, max_y, max_z)\n"
                                                 "values (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            auto aabb = boundingBoxHolder[i][j];
            insertBB.reset();

            insertBB.bind(1, mapId);
            insertBB.bind(2, i);
            insertBB.bind(3, j);
            insertBB.bind(4, aabb.min.x);
            insertBB.bind(5, aabb.min.y);
            insertBB.bind(6, aabb.min.z);
            insertBB.bind(7, aabb.max.x);
            insertBB.bind(8, aabb.max.y);
            insertBB.bind(9, aabb.max.z);

            insertBB.exec();
        }
    }

    //3. Commit transaction
    transaction.commit();
}

void CMinimapDataDB::getRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides) {
    SQLite::Statement getRiverColor(m_sqliteDatabase,
                               "select rco.area_id, rco.color_0, rco.color_1, rco.color_2, rco.color_3 "
                               " from river_color_overrides rco where rco.map_id = ?;"
    );
    getRiverColor.reset();
    getRiverColor.bind(1, mapId);

    while (getRiverColor.executeStep())
    {
        RiverColorOverride colorOverride;
        colorOverride.areaId = getRiverColor.getColumn(0).getInt();
        colorOverride.color = mathfu::vec4(
            getRiverColor.getColumn(1).getDouble(),
            getRiverColor.getColumn(2).getDouble(),
            getRiverColor.getColumn(3).getDouble(),
            getRiverColor.getColumn(4).getDouble()
        );
        riverOverrides.push_back(colorOverride);
    }
}

void CMinimapDataDB::saveRiverColorOverrides(int mapId, std::vector<RiverColorOverride> &riverOverrides) {
    SQLite::Transaction transaction(m_sqliteDatabase);

    //1. Clear all records for that mapId from DB
    {
        SQLite::Statement cleanABB(m_sqliteDatabase,
                                   "delete from river_color_overrides where map_id = ?"
        );
        cleanABB.exec();
    }

    SQLite::Statement saveRiverColor(m_sqliteDatabase, "insert into river_color_overrides(map_id, area_id, "
                                                       "color_0, color_1, color_2, color_3) \n"
                                                       "values (?, ?, ?, ?, ?, ?);");

    for (int j = 0; j < riverOverrides.size(); j++) {
        auto &roRec = riverOverrides[j];
        saveRiverColor.reset();

        saveRiverColor.bind(1, mapId);
        saveRiverColor.bind(2, roRec.areaId);
        saveRiverColor.bind(3, roRec.color.x);
        saveRiverColor.bind(4, roRec.color.y);
        saveRiverColor.bind(5, roRec.color.z);
        saveRiverColor.bind(6, roRec.color.w);

        saveRiverColor.exec();
    }

}

bool CMinimapDataDB::getWmoAABBOverride(int mapId, int uniqueId, CAaBox &aaBox) {
    return false;
}

void CMinimapDataDB::saveWmoAABBOverride(int mapId, int uniqueId, const CAaBox &aaBox) {

}


