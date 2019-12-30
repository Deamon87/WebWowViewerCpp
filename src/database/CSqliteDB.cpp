//
// Created by deamon on 24.12.19.
//

#include <sqlite3.h>
#include "CSqliteDB.h"
CSqliteDB::CSqliteDB(std::string dbFileName) :
    m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getWmoAreaAreaName(m_sqliteDatabase,
        "select wat.AreaName_lang as wmoAreaName, at.AreaName_lang from WMOAreaTable wat "
        "left join AreaTable at on at.id = wat.AreaTableID "
        "where wat.WMOID = ? and wat.NameSetID = ? and (wat.WMOGroupID = -1 or wat.WMOGroupID = ?) ORDER BY wat.WMOGroupID DESC"),
    getAreaNameStatement(m_sqliteDatabase,
        "select at.AreaName_lang from AreaTable at where at.ID = ?")

{
    char *sErrMsg = "";
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);

}

void CSqliteDB::getMapArray(std::vector<MapRecord> &mapRecords) {
    SQLite::Statement getMapList(m_sqliteDatabase, "select m.ID, m.Directory, m.WdtFileDataID, m.MapType from Map m where m.WdtFileDataID > 0");

    while (getMapList.executeStep())
    {
        MapRecord mapRecord;

        // Demonstrate how to get some typed column value
        mapRecord.ID = getMapList.getColumn(0);
        mapRecord.MapName = std::string((const char*) getMapList.getColumn(1));
        mapRecord.WdtFileID = getMapList.getColumn(2);
        mapRecord.MapType = getMapList.getColumn(3);

        mapRecords.push_back(mapRecord);
    }

}

std::string CSqliteDB::getAreaName(int areaId) {
    getAreaNameStatement.reset();

    getAreaNameStatement.bind(1, areaId);
    while (getAreaNameStatement.executeStep()) {
        std::string areaName = getAreaNameStatement.getColumn(0);

        return areaName;
    }


    return std::string();
}

std::string CSqliteDB::getWmoAreaName(int wmoId, int nameId, int groupId) {
    getWmoAreaAreaName.reset();

    getWmoAreaAreaName.bind(1, wmoId);
    getWmoAreaAreaName.bind(2, nameId);
    getWmoAreaAreaName.bind(3, groupId);

    while (getWmoAreaAreaName.executeStep()) {
        std::string wmoAreaName = getWmoAreaAreaName.getColumn(0);
        std::string areaName = getWmoAreaAreaName.getColumn(1);

        if (wmoAreaName == "") {
            return areaName;
        } else {
            return wmoAreaName;
        }
    }


    return "";
}
