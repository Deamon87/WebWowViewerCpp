//
// Created by deamon on 04.02.20.
//

#ifndef AWEBWOWVIEWERCPP_APICONTAINER_H
#define AWEBWOWVIEWERCPP_APICONTAINER_H

#include <memory>
class ApiContainer;
typedef std::shared_ptr<ApiContainer> HApiContainer;

#include <memory>
#include "WowFilesCacheStorage.h"
#include "camera/CameraInterface.h"
#include "../include/config.h"
#include "../include/databaseHandler.h"



class ApiContainer {
private:
    Config config;
public:
    HWoWFilesCacheStorage cacheStorage = nullptr;
    std::shared_ptr<IDevice> hDevice = nullptr;
    IClientDatabase *databaseHandler = nullptr;
    std::shared_ptr<ICamera> camera = nullptr;
    std::shared_ptr<ICamera> debugCamera = nullptr;

    Config *getConfig() {
        return &config;
    }
};

typedef std::shared_ptr<ApiContainer> HApiContainer;


#endif //AWEBWOWVIEWERCPP_APICONTAINER_H
