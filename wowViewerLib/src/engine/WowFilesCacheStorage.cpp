//
// Created by Deamon on 12/21/2019.
//

#include "WowFilesCacheStorage.h"

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
        case CacheHolderType::CACHE_DB2:
            db2Cache.reject(s_fileName);
            break;
    }
}

WoWFilesCacheStorage::WoWFilesCacheStorage(IFileRequest *requestProcessor) :
    wmoMainCache(requestProcessor, CacheHolderType::CACHE_MAIN_WMO),
    wmoGeomCache(requestProcessor, CacheHolderType::CACHE_GROUP_WMO),
    wdtCache(requestProcessor, CacheHolderType::CACHE_WDT),
    wdtLightCache(requestProcessor, CacheHolderType::CACHE_WDT_LIGHT),
    wdlCache(requestProcessor, CacheHolderType::CACHE_WDL),
    m2GeomCache(requestProcessor, CacheHolderType::CACHE_M2),
    skinGeomCache(requestProcessor, CacheHolderType::CACHE_SKIN),
    textureCache(requestProcessor, CacheHolderType::CACHE_BLP),
    adtObjectCache(requestProcessor, CacheHolderType::CACHE_ADT),
    animCache(requestProcessor, CacheHolderType::CACHE_ANIM),
    skelCache(requestProcessor, CacheHolderType::CACHE_SKEL),
    db2Cache(requestProcessor, CacheHolderType::CACHE_DB2) {
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
    this->db2Cache.clear();
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

Cache<WdtFile>* WoWFilesCacheStorage::getWdtFileCache() {
    return &wdtCache;
}

Cache<WdtLightFile> *WoWFilesCacheStorage::getWdtLightFileCache() {
    return &wdtLightCache;
};

Cache<WdlFile> *WoWFilesCacheStorage::getWdlFileCache() {
    return &wdlCache;
}

Cache<Db2File> *WoWFilesCacheStorage::getDb2Cache() {
    return &db2Cache;
};