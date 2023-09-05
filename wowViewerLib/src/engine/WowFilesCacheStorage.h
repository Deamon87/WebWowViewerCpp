//
// Created by Deamon on 12/21/2019.
//

#ifndef AWEBWOWVIEWERCPP_WOWFILESCACHESTORAGE_H
#define AWEBWOWVIEWERCPP_WOWFILESCACHESTORAGE_H

#include <memory>
class WoWFilesCacheStorage;
typedef std::shared_ptr<WoWFilesCacheStorage> HWoWFilesCacheStorage;

#include <memory>
#include "../include/iostuff.h"
#include "../gapi/interface/IDevice.h"
#include "persistance/adtFile.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"
#include "persistance/skelFile.h"
#include "cache/cache.h"


#include "geometry/wmoGroupGeom.h"
#include "geometry/wmoMainGeom.h"
#include "geometry/m2Geom.h"
#include "geometry/skinGeom.h"
#include "texture/BlpTexture.h"
#include "persistance/db2File.h"

class WoWFilesCacheStorage : public IFileRequester {
private:
    Cache<AdtFile> adtObjectCache;
    Cache<WdtFile> wdtCache;
    Cache<WdlFile> wdlCache;
    Cache<WmoGroupGeom> wmoGeomCache;
    Cache<WmoMainGeom> wmoMainCache;
    Cache<M2Geom> m2GeomCache;
    Cache<SkinGeom> skinGeomCache;
    Cache<BlpTexture> textureCache;
    Cache<AnimFile> animCache;
    Cache<SkelFile> skelCache;
    Cache<Db2File> db2Cache;
public:
    WoWFilesCacheStorage(IFileRequest * requestProcessor);
    void rejectFile(CacheHolderType holderType, const char* fileName) override ;

    void actuallDropCache();

    Cache<AdtFile> *getAdtGeomCache();
    Cache<M2Geom> *getM2GeomCache();
    Cache<SkinGeom>* getSkinGeomCache();
    Cache<AnimFile>* getAnimCache();
    Cache<SkelFile>* getSkelCache();
    Cache<BlpTexture> *getTextureCache();
    Cache<WmoMainGeom>* getWmoMainCache();
    Cache<WmoGroupGeom>* getWmoGroupGeomCache();
    Cache<WdtFile>* getWdtFileCache();
    Cache<WdlFile>* getWdlFileCache();
    Cache<Db2File>* getDb2Cache();
};



#endif //AWEBWOWVIEWERCPP_WOWFILESCACHESTORAGE_H
