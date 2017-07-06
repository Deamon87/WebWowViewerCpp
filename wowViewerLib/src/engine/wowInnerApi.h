//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_WOWINNERAPI_H
#define WOWVIEWERLIB_WOWINNERAPI_H

class M2Geom;
class SkinGeom;
class BlpTexture;
#include "opengl/header.h"
#include "cache/cache.h"
#include "geometry/m2Geom.h"
#include "shader/ShaderRuntimeData.h"



class IWoWInnerApi {
public:
    virtual Cache<M2Geom>* getM2GeomCache() = 0;
    virtual Cache<SkinGeom>* getSkinGeomCache() = 0;
    virtual Cache<BlpTexture>* getTextureCache() = 0;

    virtual ShaderRuntimeData *getM2Shader() = 0;

    virtual GLuint getBlackPixelTexture() = 0;

};

#endif //WOWVIEWERLIB_WOWINNERAPI_H
