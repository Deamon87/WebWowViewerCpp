//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_WOWINNERAPI_H
#define WOWVIEWERLIB_WOWINNERAPI_H

#include "engineClassList.h"

#include "./wowCommonClasses.h"
#include "objects/wmo/wmoObject.h"

//Functional classes


#include "opengl/header.h"
#include "cache/cache.h"
#include "shader/ShaderRuntimeData.h"
#include "objects/adt/adtObject.h"
#include "persistance/adtFile.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"
#include "persistance/db2/DB2Light.h"
#include "persistance/db2/DB2LightData.h"
#include "persistance/db2/DB2WmoAreaTable.h"
#include "../gapi/interface/IDevice.h"

class IWoWInnerApi {
public:
    virtual ~IWoWInnerApi() {};

    virtual Cache<AdtFile>* getAdtGeomCache() = 0;
    virtual Cache<M2Geom>* getM2GeomCache() = 0;
    virtual Cache<SkinGeom>* getSkinGeomCache() = 0;
    virtual Cache<BlpTexture>* getTextureCache() = 0;
    virtual Cache<WmoMainGeom>* getWmoMainCache() = 0;
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() = 0;
    virtual Cache<WdtFile>* getWdtFileCache() = 0;
    virtual Cache<WdlFile>* getWdlFileCache() = 0;

    virtual bool getIsDebugCamera() = 0;
    virtual void drawCamera() = 0;

    virtual IDevice * getDevice()= 0;
    virtual HGUniformBuffer getSceneWideUniformBuffer() = 0;

    virtual mathfu::mat4& getViewMat() = 0;
    virtual mathfu::vec4 getGlobalAmbientColor() = 0;
    virtual mathfu::vec3 getGlobalSunDir() = 0;
    virtual mathfu::vec4 getGlobalSunColor() = 0;
    virtual mathfu::vec4 getGlobalFogColor() = 0;
    virtual float getGlobalFogStart() = 0;
    virtual float getGlobalFogEnd() = 0;
    virtual mathfu::vec3 getViewUp() = 0;

    virtual Config *getConfig() = 0;

    virtual DB2Light *getDB2Light() = 0;
    virtual DB2LightData *getDB2LightData() = 0;
    virtual DB2WmoAreaTable *getDB2WmoAreaTable() = 0;

};

#include "geometry/m2Geom.h"
#include "geometry/wmoMainGeom.h"
#include "geometry/wmoGroupGeom.h"



#endif //WOWVIEWERLIB_WOWINNERAPI_H
