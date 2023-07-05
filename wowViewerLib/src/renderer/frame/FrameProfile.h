//
// Created by Deamon on 7/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_FRAMEPROFILE_H
#define AWEBWOWVIEWERCPP_FRAMEPROFILE_H

#ifdef LINK_TRACY
#include "Tracy.hpp"

#define setThreadName(a)  tracy::SetThreadName(a);
#else
#define setThreadName(a)
#define ZoneScoped
#define ZoneScopedN(a)

#endif


#endif //AWEBWOWVIEWERCPP_FRAMEPROFILE_H
