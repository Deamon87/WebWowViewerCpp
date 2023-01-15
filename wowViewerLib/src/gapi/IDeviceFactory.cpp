//
// Created by Deamon on 9/4/2018.
//


#include "IDeviceFactory.h"

#ifdef LINK_OGL2
#include "ogl2.0/GDeviceGL20.h"
#endif

#ifdef LINK_OGL3
#include "ogl3.3/GDeviceGL33.h"
#endif
#ifdef LINK_OGL4
#include "ogl4.x/GDeviceGL4x.h"
#endif
#ifndef SKIP_VULKAN
#include "vulkan/GDeviceVulkan.h"
#endif

#if defined(LINK_OGL2) || defined(LINK_OGL3) || defined(LINK_OGL4)
void initOGLPointers(){
#if defined(_WIN32) && (!defined(WITH_GLESv2) && !defined(__EMSCRIPTEN__))
    glewExperimental = true; // Needed in core profile
    auto result = glewInit();

    if (result != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
    }
#endif
}
#endif

HGDevice IDeviceFactory::createDevice(std::string gapiName, void * data) {
#ifdef LINK_OGL2
    if (gapiName == "ogl2") {
        initOGLPointers();

        auto device = std::make_shared<GDeviceGL20>();
        device->initialize();
        return device;
    } else
#endif

#ifdef LINK_OGL3
    if (gapiName == "ogl3") {
        initOGLPointers();

        auto device = std::make_shared<GDeviceGL33>();
        device->initialize();
        return device;
    } else
#endif
#ifdef LINK_OGL4
    if (gapiName == "ogl4") {
        initOGLPointers();
//        return std::make_shared<GDeviceGL4x>();
    } else
#endif


#ifndef SKIP_VULKAN
    if (gapiName == "vulkan") {
        return std::make_shared<GDeviceVLK>((vkCallInitCallback *) data);
    } else
#endif
    {}

    return nullptr;
}

