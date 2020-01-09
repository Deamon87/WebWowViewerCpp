//
// Created by Deamon on 12/21/2019.
//

#ifndef AWEBWOWVIEWERCPP_WOWFILESCACHESTORAGE_H
#define AWEBWOWVIEWERCPP_WOWFILESCACHESTORAGE_H

#include "../include/iostuff.h"
#include "../gapi/interface/IDevice.h"
#include "persistance/adtFile.h"
#include "persistance/wdtFile.h"
#include "cache/cache.h"


#include "geometry/wmoGroupGeom.h"
#include "geometry/wmoMainGeom.h"
#include "geometry/m2Geom.h"
#include "geometry/skinGeom.h"
#include "texture/BlpTexture.h"

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
public:
    WoWFilesCacheStorage(IFileRequest * requestProcessor);
    void provideFile(CacheHolderType holderType, const char *fileName, const HFileContent &data) override ;
    void rejectFile(CacheHolderType holderType, const char* fileName) override ;

    void actuallDropCache();

    void processCaches(int limit);

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
};
#endif //AWEBWOWVIEWERCPP_WOWFILESCACHESTORAGE_H
