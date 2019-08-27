//
// Created by Deamon on 9/4/2018.
//

#include "IDeviceFactory.h"

#include "ogl3.3/GDeviceGL33.h"
#include "ogl4.x/GDeviceGL4x.h"
#include "vulkan/GDeviceVulkan.h"

void initOGLPointers(){
#ifdef _WIN32
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
    }
#endif
}

IDevice *IDeviceFactory::createDevice(std::string gapiName, void * data) {
    if (gapiName == "ogl3") {
        initOGLPointers();
        //return new GDeviceGL33();
    } else if (gapiName == "ogl4") {
        initOGLPointers();
//        return new GDeviceGL4x();
    } else if (gapiName == "vulkan") {
        return new GDeviceVLK((vkCallInitCallback *)data);
    }

    return nullptr;
}

