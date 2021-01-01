//
// Created by Deamon on 12/21/2019.
//

#ifndef AWEBWOWVIEWERCPP_IOSTUFF_H
#define AWEBWOWVIEWERCPP_IOSTUFF_H

#include "sharedFile.h"
#include "../engine/persistance/PersistentFile.h"

#include <cstdint>
#include <vector>
#ifdef WITH_GLESv2
typedef float animTime_t;
#else
typedef double animTime_t;
#endif

enum class CacheHolderType {
    CACHE_M2,
    CACHE_SKIN,
    CACHE_ANIM,
    CACHE_BONE,
    CACHE_SKEL,
    CACHE_MAIN_WMO,
    CACHE_GROUP_WMO,
    CACHE_ADT,
    CACHE_WDT,
    CACHE_WDL,
    CACHE_BLP,
    CACHE_DB2,
};

class IFileRequest {
public:
    virtual void requestFile(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) = 0;
    virtual ~IFileRequest()= default;
};

class IFileRequester {
public:
    virtual void rejectFile(CacheHolderType holderType, const char* fileName) = 0;
};
#endif //AWEBWOWVIEWERCPP_IOSTUFF_H
