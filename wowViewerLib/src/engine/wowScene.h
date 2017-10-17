#ifndef WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H

#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <locale>
#include "wowInnerApi.h"

#include "shadersStructures.h"
#include "shader/ShaderRuntimeData.h"
#include "../include/wowScene.h"
#include "../include/config.h"
#include "mathfu/glsl_mappings.h"
#include "camera/firstPersonCamera.h"
#include "texture/BlpTexture.h"
#include "geometry/skinGeom.h"
#include "geometry/m2Geom.h"
#include "cache/cache.h"
#include "stringTrim.h"
#include "geometry/wmoGroupGeom.h"
#include "geometry/wmoMainGeom.h"
#include "objects/adtObject.h"
#include "objects/wmoObject.h"
#include "objects/m2Object.h"
#include "objects/map.h"


class WoWSceneImpl: public WoWScene, public IWoWInnerApi {

public:
    WoWSceneImpl(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight);
    void draw(double deltaTime);
    void setScreenSize(int canvWidth, int canvHeight);

    virtual void provideFile(const char* fileName, unsigned char* data, int fileLength){
        std::vector<unsigned char> fileData;
        fileData.assign(data, data+fileLength);
        std::string s_fileName(fileName);

        adtObjectCache.provideFile(s_fileName, fileData);
        wmoGeomCache.provideFile(s_fileName, fileData);
        wmoMainCache.provideFile(s_fileName, fileData);
        m2GeomCache.provideFile(s_fileName, fileData);
        skinGeomCache.provideFile(s_fileName, fileData);
        textureCache.provideFile(s_fileName, fileData);
    };
    virtual void rejectFile(const char* fileName) {
        std::string s_fileName(fileName);

        adtObjectCache.reject(s_fileName);
        wmoGeomCache.reject(s_fileName);
        wmoMainCache.reject(s_fileName);
        m2GeomCache.reject(s_fileName);
        skinGeomCache.reject(s_fileName);
        textureCache.reject(s_fileName);
    }
    void setFileRequestProcessor(IFileRequest*){};

    IControllable* controllable = &m_firstCamera;

    IControllable* getCurrentContollable() {
        return this->controllable;
    };

    void switchCameras() {
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
    virtual Cache<AdtObject> *getAdtGeomCache() {
        return &adtObjectCache;
    }
    virtual Cache<M2Geom> *getM2GeomCache() {
        return &m2GeomCache;
    };
    virtual Cache<SkinGeom> *getSkinGeomCache() {
        return &skinGeomCache;
    };
    virtual Cache<BlpTexture> *getTextureCache() {
        return &textureCache;
    };
    virtual Cache<WmoMainGeom>* getWmoMainCache() {
        return &wmoMainCache;
    };
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() {
        return &wmoGeomCache;
    };

    virtual ShaderRuntimeData *getM2Shader() {
        return m2Shader;
    };
    virtual ShaderRuntimeData *getM2InstancingShader() {
        return m2InstancingShader;
    };
    virtual ShaderRuntimeData *getBBShader() {
        return bbShader;
    };
    virtual ShaderRuntimeData *getPortalShader() {
        return drawPortalShader;
    }
    virtual ShaderRuntimeData *getWmoShader() {
        return wmoShader;
    }
    virtual ShaderRuntimeData *getAdtShader() {
        return adtShader;
    }
    virtual GLuint getBlackPixelTexture();
    virtual Config *getConfig() {
        return m_config;
    }

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
    void initShaders();
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
    ShaderRuntimeData *textureCompositionShader = nullptr;
    ShaderRuntimeData *renderFrameShader = nullptr;
    ShaderRuntimeData *drawDepthBuffer = nullptr;
    ShaderRuntimeData *readDepthBuffer = nullptr;
    ShaderRuntimeData *wmoShader = nullptr;
    ShaderRuntimeData *wmoInstancingShader = nullptr;
    ShaderRuntimeData *m2Shader = nullptr;
    ShaderRuntimeData *m2InstancingShader = nullptr;
    ShaderRuntimeData *bbShader = nullptr;
    ShaderRuntimeData *adtShader = nullptr;
    ShaderRuntimeData *drawPortalShader = nullptr;
    ShaderRuntimeData *drawFrustumShader = nullptr;
    ShaderRuntimeData *drawLinesShader = nullptr;

    Config * m_config;

    mathfu::mat4 m_lookAtMat4;
    mathfu::mat4 m_viewCameraForRender;
    mathfu::mat4 m_perspectiveMatrix;

    FirstPersonCamera m_firstCamera;
    FirstPersonCamera m_secondCamera;

    GLuint vbo_vertices;
    GLuint ibo_elements;

    int canvWidth;
    int canvHeight;
    float canvAspect;

    float uFogStart = -1;
    float uFogEnd = -1;
    float m_fogColor[4] = {1.0, 1.0, 1.0, 1.0};

    bool m_isDebugCamera = false;

    GLuint frameBuffer = -1;
    GLuint frameBufferColorTexture = -1;
    GLuint frameBufferDepthTexture = -1;
    GLuint vertBuffer = 0;
    GLuint blackPixel = 0;


    Cache<AdtObject> adtObjectCache;
    Cache<WmoGroupGeom> wmoGeomCache;
    Cache<WmoMainGeom> wmoMainCache;
    Cache<M2Geom> m2GeomCache;
    Cache<SkinGeom> skinGeomCache;
    Cache<BlpTexture> textureCache;

    M2Object *m2Object = nullptr;
    WmoObject *wmoObject = nullptr;
    AdtObject *adtObject = nullptr;
    iInnerSceneApi *currentScene;

    void activateRenderFrameShader();
    void activateRenderDepthShader();
    void activateReadDepthBuffer();
    void activateAdtShader();
    void deactivateAdtShader();
    void activateWMOShader();
    void deactivateWMOShader();
    void deactivateTextureCompositionShader();
    void activateM2ShaderAttribs();
    void deactivateM2ShaderAttribs();
    void activateM2Shader();
    void deactivateM2Shader();
    void activateM2InstancingShader();
    void deactivateM2InstancingShader();
    void activateBoundingBoxShader();
    void deactivateBoundingBoxShader();
    void activateFrustumBoxShader();
    void activateDrawLinesShader();
    void activateDrawPortalShader();

    void activateTextureCompositionShader(GLuint texture);


    void initVertBuffer();

    void drawTexturedQuad(GLuint texture, float x, float y, float width, float height, float canv_width, float canv_height,
                          bool drawDepth);

    void drawCamera();
    bool getIsDebugCamera() {
        return m_isDebugCamera;
    }
};



#endif //WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
