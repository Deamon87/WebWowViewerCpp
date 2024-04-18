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
#include "../../../src/persistance/RequestProcessor.h"


class ApiContainer {
private:
    std::shared_ptr<Config> m_config = nullptr;
public:
    ApiContainer() : m_config(std::make_shared<Config>()){
    };

    HApiContainer clone() {
        auto newApi = std::make_shared<ApiContainer>();
        newApi->m_config = this->m_config;
        newApi->requestProcessor = this->requestProcessor;
        newApi->cacheStorage = this->cacheStorage;
        newApi->hDevice = this->hDevice;
        newApi->databaseHandler = this->databaseHandler;

        return newApi;
    }


    HWoWFilesCacheStorage cacheStorage = nullptr;
    HRequestProcessor requestProcessor = nullptr;
    HGDevice hDevice = nullptr;
    std::shared_ptr<IClientDatabase> databaseHandler = nullptr;

    Config *getConfig() {
        return m_config.get();
    }
};

typedef std::shared_ptr<ApiContainer> HApiContainer;

//typedef std::array<std::array<CAaBox, 64>, 64> ADTBoundingBoxHolder;
//typedef std::shared_ptr<ADTBoundingBoxHolder> HADTBoundingBoxHolder;

#endif //AWEBWOWVIEWERCPP_APICONTAINER_H
