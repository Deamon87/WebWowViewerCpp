//
// Created by Deamon on 12/30/2019.
//

#ifndef AWEBWOWVIEWERCPP_DATABASEHANDLER_H
#define AWEBWOWVIEWERCPP_DATABASEHANDLER_H

#include <vector>
#include <string>
#include "database/dbStructs.h"

class IClientDatabase {
public:
    virtual void getMapArray(std::vector<MapRecord> &mapRecords) = 0;
    virtual std::string getAreaName(int areaId) = 0;
    virtual std::string getWmoAreaName(int wmoId, int nameId, int groupId) = 0;
};

#endif //AWEBWOWVIEWERCPP_DATABASEHANDLER_H
