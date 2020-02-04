//
// Created by deamon on 04.02.20.
//

#ifndef AWEBWOWVIEWERCPP_APICONTAINER_H
#define AWEBWOWVIEWERCPP_APICONTAINER_H


#include "WowFilesCacheStorage.h"

class ApiContainer {
private:
    Config config;
public:
    HWoWFilesCacheStorage cacheStorage = nullptr;
    std::shared_ptr<IDevice> hDevice = nullptr;
    IClientDatabase *databaseHandler = nullptr;

    Config *getConfig() {
        return &config;
    }
};


#endif //AWEBWOWVIEWERCPP_APICONTAINER_H
