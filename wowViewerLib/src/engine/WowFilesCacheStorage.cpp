//
// Created by Deamon on 12/21/2019.
//

#include "WowFilesCacheStorage.h"

void WoWFilesCacheStorage::provideFile(CacheHolderType holderType, const char *fileName, const HFileContent &data) {
//    std::cout << "data.use_count() = " << data.use_count() << std::endl << std::flush;
    std::string s_fileName(fileName);

    switch (holderType) {
        case CacheHolderType::CACHE_M2:
            m2GeomCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_SKIN:
            skinGeomCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_MAIN_WMO:
            wmoMainCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_GROUP_WMO:
            wmoGeomCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_ADT:
            adtObjectCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_WDT:
            wdtCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_WDL:
            wdlCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_BLP:
            textureCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_ANIM:
            animCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_SKEL:
            skelCache.provideFile(s_fileName, data);
            break;
    }
}

void WoWFilesCacheStorage::processCaches(int limit) {
    this->adtObjectCache.processCacheQueue(limit);
    this->wdtCache.processCacheQueue(limit);
    this->wdlCache.processCacheQueue(limit);
    this->wmoGeomCache.processCacheQueue(limit);
    this->wmoMainCache.processCacheQueue(limit);
    this->skinGeomCache.processCacheQueue(limit);
    this->animCache.processCacheQueue(limit);
    this->skelCache.processCacheQueue(limit);
    this->m2GeomCache.processCacheQueue(limit);
    this->textureCache.processCacheQueue(limit);
}

void WoWFilesCacheStorage::rejectFile(CacheHolderType holderType, const char* fileName) {
    std::string s_fileName(fileName);

    switch (holderType) {
        case CacheHolderType::CACHE_M2:
            m2GeomCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_SKIN:
            skinGeomCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_MAIN_WMO:
            wmoMainCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_GROUP_WMO:
            wmoGeomCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_ADT:
            adtObjectCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_WDT:
            wdtCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_WDL:
            wdlCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_BLP:
            textureCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_ANIM:
            animCache.reject(s_fileName);
            break;
        case CacheHolderType::CACHE_SKEL:
            skelCache.reject(s_fileName);
            break;
    }
}

WoWFilesCacheStorage::WoWFilesCacheStorage(IFileRequest *requestProcessor) :
    wmoMainCache(requestProcessor, CacheHolderType::CACHE_MAIN_WMO),
    wmoGeomCache(requestProcessor, CacheHolderType::CACHE_GROUP_WMO),
    wdtCache(requestProcessor, CacheHolderType::CACHE_WDT),
    wdlCache(requestProcessor, CacheHolderType::CACHE_WDL),
    m2GeomCache(requestProcessor, CacheHolderType::CACHE_M2),
    skinGeomCache(requestProcessor, CacheHolderType::CACHE_SKIN),
    textureCache(requestProcessor, CacheHolderType::CACHE_BLP),
    adtObjectCache(requestProcessor, CacheHolderType::CACHE_ADT),
    animCache(requestProcessor, CacheHolderType::CACHE_ANIM),
    skelCache(requestProcessor, CacheHolderType::CACHE_SKEL) {
}

void WoWFilesCacheStorage::actuallDropCache() {
//    std::cout << "Called " << __PRETTY_FUNCTION__ << std::endl;
    this->adtObjectCache.clear();
    this->wdtCache.clear();
    this->wdlCache.clear();
    this->wmoGeomCache.clear();
    this->wmoMainCache.clear();
    this->m2GeomCache.clear();
    this->skinGeomCache.clear();
    this->animCache.clear();
    this->textureCache.clear();
}

Cache<AdtFile> *WoWFilesCacheStorage::getAdtGeomCache() {
    return &adtObjectCache;
}

Cache<M2Geom> *WoWFilesCacheStorage::getM2GeomCache() {
    return &m2GeomCache;
};

Cache<SkinGeom> *WoWFilesCacheStorage::getSkinGeomCache() {
    return &skinGeomCache;
};

Cache<AnimFile> *WoWFilesCacheStorage::getAnimCache() {
    return &animCache;
};

Cache<SkelFile> *WoWFilesCacheStorage::getSkelCache() {
    return &skelCache;
};

Cache<BlpTexture> *WoWFilesCacheStorage::getTextureCache() {
    return &textureCache;
};

Cache<WmoMainGeom> *WoWFilesCacheStorage::getWmoMainCache() {
    return &wmoMainCache;
};

Cache<WmoGroupGeom> *WoWFilesCacheStorage::getWmoGroupGeomCache() {
    return &wmoGeomCache;
};

Cache<WdtFile> *WoWFilesCacheStorage::getWdtFileCache() {
    return &wdtCache;
};

Cache<WdlFile> *WoWFilesCacheStorage::getWdlFileCache() {
    return &wdlCache;
};