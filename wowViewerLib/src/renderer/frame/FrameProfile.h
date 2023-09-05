//
// Created by Deamon on 7/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_FRAMEPROFILE_H
#define AWEBWOWVIEWERCPP_FRAMEPROFILE_H

#ifdef LINK_TRACY
#include "Tracy.hpp"

#ifdef LINK_VULKAN
    #include "../../gapi/vulkan/context/vulkan_context.h"
    #include "tracy/TracyVulkan.hpp"
#endif

#define setThreadName(a)  tracy::SetThreadName(a);
#else
#define setThreadName(a)
#define ZoneScoped
#define ZoneScopedN(a)
#define TracyVkContext(x,y,z,w)

#endif


#endif //AWEBWOWVIEWERCPP_FRAMEPROFILE_H
