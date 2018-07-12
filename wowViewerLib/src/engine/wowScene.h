#ifndef WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H

#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <locale>
#include "wowInnerApi.h"

#include "shader/ShaderRuntimeData.h"
#include "../include/wowScene.h"
#include "../include/config.h"
#include "mathfu/glsl_mappings.h"
#include "camera/firstPersonOrthoCamera.h"
#include "camera/firstPersonCamera.h"
#include "texture/BlpTexture.h"
#include "geometry/skinGeom.h"
#include "geometry/m2Geom.h"
#include "cache/cache.h"
#include "stringTrim.h"
#include "geometry/wmoGroupGeom.h"
#include "geometry/wmoMainGeom.h"
#include "objects/scenes/map.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"
#include "persistance/db2/base/DB2Base.h"
#include "persistance/db2/DB2Light.h"
#include "../gapi/GDevice.h"

class WoWSceneImpl: public WoWScene, public IWoWInnerApi {

public:
    WoWSceneImpl(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight);
    void draw(animTime_t deltaTime) override;
    void setScreenSize(int canvWidth, int canvHeight) override;

    virtual void provideFile(const char* fileName, unsigned char* data, int fileLength) override {
        std::vector<unsigned char> fileData;
        fileData.assign(data, data+fileLength);
        std::string s_fileName(fileName);

        adtObjectCache.provideFile(s_fileName, fileData);
        wmoGeomCache.provideFile(s_fileName, fileData);
        wmoMainCache.provideFile(s_fileName, fileData);
        m2GeomCache.provideFile(s_fileName, fileData);
        skinGeomCache.provideFile(s_fileName, fileData);
        textureCache.provideFile(s_fileName, fileData);
        wdtCache.provideFile(s_fileName, fileData);
        wdlCache.provideFile(s_fileName, fileData);
        db2Cache.provideFile(s_fileName, fileData);
    };
    virtual void rejectFile(const char* fileName) override {
        std::string s_fileName(fileName);

        adtObjectCache.reject(s_fileName);
        wmoGeomCache.reject(s_fileName);
        wmoMainCache.reject(s_fileName);
        m2GeomCache.reject(s_fileName);
        skinGeomCache.reject(s_fileName);
        textureCache.reject(s_fileName);
        wdtCache.reject(s_fileName);
        wdlCache.reject(s_fileName);
        db2Cache.reject(s_fileName);
    }
    void setFileRequestProcessor(IFileRequest*) override {} ;

    IControllable* controllable = &m_firstCamera;

    IControllable* getCurrentCamera() override {
        return this->controllable;
    };

    void switchCameras() override {
        if (controllable == &m_firstCamera) {
            this->m_config->setUseSecondCamera(true);
            controllable = &m_secondCamera;
            mathfu::vec3 camPos = m_firstCamera.getCameraPosition();
            m_secondCamera.setCameraPos(camPos.x, camPos.y, camPos.z);
        } else {
            this->m_config->setUseSecondCamera(false);
            controllable = &m_firstCamera;
        }
    }
public:
    virtual GDevice * getDevice() override {
        return &m_gdevice;
    }

    virtual HGUniformBuffer getSceneWideUniformBuffer() override {
        return m_sceneWideUniformBuffer;
    }

    virtual Cache<AdtFile> *getAdtGeomCache() override {
        return &adtObjectCache;
    }
    virtual Cache<M2Geom> *getM2GeomCache() override {
        return &m2GeomCache;
    };
    virtual Cache<SkinGeom> *getSkinGeomCache() override {
        return &skinGeomCache;
    };
    virtual Cache<BlpTexture> *getTextureCache() override {
        return &textureCache;
    };
    virtual Cache<WmoMainGeom>* getWmoMainCache() override {
        return &wmoMainCache;
    };
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() override {
        return &wmoGeomCache;
    };
    virtual Cache<WdtFile>* getWdtFileCache() override {
        return &wdtCache;
    };

    virtual Cache<WdlFile>* getWdlFileCache() override {
        return &wdlCache;
    };

    virtual mathfu::mat4& getViewMat() override {
        return m_lookAtMat4;
    }
    virtual mathfu::vec4 getGlobalAmbientColor() override {
        return m_globalAmbientColor ;
    }
    virtual mathfu::vec3 getGlobalSunDir() override {
        return m_sunDir;
    };
    virtual mathfu::vec4 getGlobalSunColor() override {
        return m_globalSunColor;
    }
    virtual mathfu::vec4 getGlobalFogColor() override {
        return m_fogColor;
    }
    virtual float getGlobalFogStart() override { return uFogStart; };
    virtual float getGlobalFogEnd() override { return uFogEnd; };
    virtual mathfu::vec3 getViewUp() override { return m_upVector; };


    virtual GLuint getBlackPixelTexture() override ;
    virtual Config *getConfig() override {
        return m_config;
    }
    virtual DB2Light *getDB2Light() override {
        return db2Light;
    };
    virtual DB2LightData *getDB2LightData() override {
        return db2LightData;
    };
    virtual DB2WmoAreaTable *getDB2WmoAreaTable() override {
        return db2WmoAreaTable;
    };

private:
    ShaderRuntimeData *compileShader (std::string shaderName, std::string vertShaderString, std::string fragmentShaderString,
                                      std::string *vertExtraDefStrings = nullptr, std::string *fragExtraDefStrings = nullptr);
    void createBlackPixelTexture();
    void initGlContext (/*canvas*/);
    void initArrayInstancedExt();
    void initAnisotropicExt ();
    void initVertexArrayObjectExt ();
    void initCompressedTextureS3tcExt();
    void initDepthTextureExt ();
    void initDeferredRendering ();
    void initCaches();
    void initDrawBuffers (int frameBuffer);
    void initRenderBuffers ();
    void initSceneGraph ();
    void initSceneApi ();
    void initCamera ();
    void initBoxVBO ();
    void initTextureCompVBO ();
private:
    bool m_enable;

    Config * m_config;
    GDevice m_gdevice;
    HGUniformBuffer m_sceneWideUniformBuffer;


    mathfu::mat4 m_lookAtMat4;
    mathfu::mat4 m_viewCameraForRender;
    mathfu::mat4 m_perspectiveMatrix;

    mathfu::vec3 m_upVector;
    mathfu::vec3 m_sunDir;
    mathfu::vec4 m_globalAmbientColor;
    mathfu::vec4 m_globalSunColor;

    FirstPersonCamera m_firstCamera;
    FirstPersonCamera m_secondCamera;

    GLuint vbo_vertices;
    GLuint ibo_elements;

    DB2Light *db2Light;
    DB2LightData *db2LightData;
    DB2WmoAreaTable *db2WmoAreaTable;

    int canvWidth;
    int canvHeight;
    float canvAspect;

    GLuint vao;

    float uFogStart = -1;
    float uFogEnd = -1;
    mathfu::vec4 m_fogColor = mathfu::vec4(1.0, 1.0, 1.0, 1.0);

    bool m_isDebugCamera = false;

    GLuint frameBuffer = -1;
    GLuint frameBufferColorTexture = -1;
    GLuint frameBufferDepthTexture = -1;
    GLuint vertBuffer = 0;
    GLuint blackPixel = 0;


    Cache<AdtFile> adtObjectCache;
    Cache<WdtFile> wdtCache;
    Cache<WdlFile> wdlCache;
    Cache<WmoGroupGeom> wmoGeomCache;
    Cache<WmoMainGeom> wmoMainCache;
    Cache<M2Geom> m2GeomCache;
    Cache<SkinGeom> skinGeomCache;
    Cache<BlpTexture> textureCache;
    Cache<DB2Base> db2Cache;

    iInnerSceneApi *currentScene;

    void activateRenderFrameShader();
    void activateRenderDepthShader();
    void activateReadDepthBuffer();
    void activateAdtShader();
    void deactivateAdtShader();
    void activateAdtLodShader();
    void deactivateAdtLodShader();
    void activateWMOShader();
    void deactivateWMOShader();
    void deactivateTextureCompositionShader();
    void activateM2ShaderAttribs() ;
    void deactivateM2ShaderAttribs();
    void activateM2Shader();
    void activateM2ParticleShader();
    void deactivateM2ParticleShader();
    void deactivateM2Shader() ;
    void activateM2InstancingShader();
    void deactivateM2InstancingShader();
    void activateBoundingBoxShader();
    void deactivateBoundingBoxShader();
    void activateFrustumBoxShader();
    void activateDrawLinesShader() ;
    void activateDrawPortalShader();
    void activateDrawPointShader();
    void deactivateDrawPointShader();

    void activateTextureCompositionShader(GLuint texture);


    void initVertBuffer();

    void drawTexturedQuad(GLuint texture, float x, float y, float width, float height, float canv_width, float canv_height,
                          bool drawDepth);

    void drawCamera() override ;
    bool getIsDebugCamera() override {
        return m_isDebugCamera;
    }



    void SetDirection();
};



#endif //WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
