//
// Created by deamon on 24.12.19.
//

#ifndef AWEBWOWVIEWERCPP_CSQLITEDB_H
#define AWEBWOWVIEWERCPP_CSQLITEDB_H

#include <vector>
#include <SQLiteCpp/Database.h>
#include "dbStructs.h"



class CSqliteDB {
public:
    CSqliteDB(std::string dbFileName);
    void getMapArray(std::vector<MapRecord> &mapRecords);
private:
    SQLite::Database m_sqliteDatabase;
};


#endif //AWEBWOWVIEWERCPP_CSQLITEDB_H
