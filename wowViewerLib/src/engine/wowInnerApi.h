//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_WOWINNERAPI_H
#define WOWVIEWERLIB_WOWINNERAPI_H

#include "engineClassList.h"

#include "./wowCommonClasses.h"
#include "./../include/databaseHandler.h"

//Functional classes

#include "cache/cache.h"

#include "objects/adt/adtObject.h"
#include "persistance/adtFile.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"
#include "../gapi/interface/IDevice.h"
#include "persistance/animFile.h"

class IWoWInnerApi {
public:
    virtual ~IWoWInnerApi() = default;

    //WoWFilesCacheStorage
    virtual Cache<AdtFile>* getAdtGeomCache() = 0;
    virtual Cache<M2Geom>* getM2GeomCache() = 0;
    virtual Cache<SkinGeom>* getSkinGeomCache() = 0;
    virtual Cache<AnimFile>* getAnimCache() = 0;
    virtual Cache<SkelFile>* getSkelCache() = 0;
    virtual Cache<BlpTexture>* getTextureCache() = 0;
    virtual Cache<WmoMainGeom>* getWmoMainCache() = 0;
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() = 0;
    virtual Cache<WdtFile>* getWdtFileCache() = 0;
    virtual Cache<WdlFile>* getWdlFileCache() = 0;

    //Device
    virtual IDevice * getDevice()= 0;

    //Config
    virtual Config *getConfig() = 0;
    //DB
    virtual IClientDatabase *getDatabaseHandler() = 0;

//    virtual void setCameraPosition(float x, float y, float z) = 0;
//    virtual void setCameraOffset(float x, float y, float z) = 0;
};

#include "geometry/m2Geom.h"
#include "geometry/wmoMainGeom.h"
#include "geometry/wmoGroupGeom.h"



#endif //WOWVIEWERLIB_WOWINNERAPI_H
