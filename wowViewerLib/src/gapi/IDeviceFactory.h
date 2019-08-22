//
// Created by Deamon on 9/4/2018.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICEFACTORY_H
#define AWEBWOWVIEWERCPP_IDEVICEFACTORY_H

#include <string>
#include "interface/IDevice.h"



class IDeviceFactory {
public:
    static IDevice *createDevice(std::string gapiName, void* data);

};


#endif //AWEBWOWVIEWERCPP_IDEVICEFACTORY_H
