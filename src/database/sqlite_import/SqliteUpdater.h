//
// Created by Deamon on 8/24/2022.
//

#ifndef AWEBWOWVIEWERCPP_SQLITEUPDATER_H
#define AWEBWOWVIEWERCPP_SQLITEUPDATER_H


#include "../../../wowViewerLib/src/engine/WowFilesCacheStorage.h"

class SqliteUpdater {
private:


public:
    SqliteUpdater(HWoWFilesCacheStorage cacheStorage);


    void process(std::string currentMessage);

private:

};


#endif //AWEBWOWVIEWERCPP_SQLITEUPDATER_H
