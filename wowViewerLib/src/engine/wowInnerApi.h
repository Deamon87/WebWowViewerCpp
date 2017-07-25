//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_WOWINNERAPI_H
#define WOWVIEWERLIB_WOWINNERAPI_H

class M2Geom;
class SkinGeom;
class BlpTexture;
class WmoMainGeom;
class WmoGroupGeom;

#include "opengl/header.h"
#include "cache/cache.h"
#include "shader/ShaderRuntimeData.h"
#include "objects/adtObject.h"

class IWoWInnerApi {
public:
    virtual Cache<AdtObject>* getAdtGeomCache() = 0;
    virtual Cache<M2Geom>* getM2GeomCache() = 0;
    virtual Cache<SkinGeom>* getSkinGeomCache() = 0;
    virtual Cache<BlpTexture>* getTextureCache() = 0;
    virtual Cache<WmoMainGeom>* getWmoMainCache() = 0;
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() = 0;

    virtual ShaderRuntimeData *getM2Shader() = 0;
    virtual ShaderRuntimeData *getWmoShader() = 0;
    virtual ShaderRuntimeData *getAdtShader() = 0;

    virtual GLuint getBlackPixelTexture() = 0;

};

#include "geometry/m2Geom.h"

#include "geometry/wmoMainGeom.h"
#include "geometry/wmoGroupGeom.h"

#endif //WOWVIEWERLIB_WOWINNERAPI_H
