//
// Created by Deamon on 9/4/2018.
//

#include "IDeviceFactory.h"

#include "ogl3.3/GDeviceGL33.h"
//#include "ogl4.x/GDeviceGL4x.h"

IDevice *IDeviceFactory::createDevice(std::string gapiName) {
    if (gapiName == "ogl3") {
        return new GDeviceGL33();
    }// else if (gapiName == "ogl4") {
//        return new GDeviceGL4x();
//    }

    return nullptr;
}
