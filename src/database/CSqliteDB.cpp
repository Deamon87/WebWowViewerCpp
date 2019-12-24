//
// Created by deamon on 24.12.19.
//

#include <sqlite3.h>
#include "CSqliteDB.h"
CSqliteDB::CSqliteDB(std::string dbFileName) : m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE){
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
