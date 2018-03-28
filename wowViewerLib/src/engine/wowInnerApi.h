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
#include "objects/adt/adtObject.h"
#include "persistance/adtFile.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"

class IWoWInnerApi {
public:
    virtual Cache<AdtFile>* getAdtGeomCache() = 0;
    virtual Cache<M2Geom>* getM2GeomCache() = 0;
    virtual Cache<SkinGeom>* getSkinGeomCache() = 0;
    virtual Cache<BlpTexture>* getTextureCache() = 0;
    virtual Cache<WmoMainGeom>* getWmoMainCache() = 0;
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() = 0;
    virtual Cache<WdtFile>* getWdtFileCache() = 0;
    virtual Cache<WdlFile>* getWdlFileCache() = 0;

    virtual ShaderRuntimeData *getM2Shader() = 0;
    virtual ShaderRuntimeData *getM2ParticleShader() = 0;
    virtual ShaderRuntimeData *getM2InstancingShader() = 0;
    virtual ShaderRuntimeData *getBBShader() = 0;
    virtual ShaderRuntimeData *getWmoShader() = 0;
    virtual ShaderRuntimeData *getPortalShader() = 0;
    virtual ShaderRuntimeData *getAdtShader() = 0;
    virtual ShaderRuntimeData *getDrawPointsShader() = 0;

    virtual void activateM2Shader() = 0;
    virtual void deactivateM2Shader() = 0;
    virtual void activateM2ParticleShader() = 0;
    virtual void deactivateM2ParticleShader() = 0;
    virtual void activateM2InstancingShader() = 0;
    virtual void deactivateM2InstancingShader() = 0;
    virtual void activateAdtShader() = 0;
    virtual void deactivateAdtShader() = 0;
    virtual void activateDrawPointShader() = 0;
    virtual void deactivateDrawPointShader() = 0;
    virtual void activateWMOShader() = 0;
    virtual void deactivateWMOShader() = 0;
    virtual void activateBoundingBoxShader() = 0;
    virtual void deactivateBoundingBoxShader() = 0;
    virtual void activateFrustumBoxShader() = 0;
    virtual void activateDrawPortalShader() = 0;
    virtual bool getIsDebugCamera() = 0;
    virtual void drawCamera() = 0;

    virtual mathfu::mat4& getViewMat() = 0;

    virtual GLuint getBlackPixelTexture() = 0;
    virtual Config *getConfig() = 0;

};

#include "geometry/m2Geom.h"
#include "geometry/wmoMainGeom.h"
#include "geometry/wmoGroupGeom.h"



#endif //WOWVIEWERLIB_WOWINNERAPI_H
