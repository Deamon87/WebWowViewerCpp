#ifndef WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H

#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <locale>
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


class M2Object;

class WoWSceneImpl: public WoWScene, public IWoWInnerApi {

public:
    WoWSceneImpl(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight);
    void draw(int deltaTime);

    virtual void provideFile(const char* fileName, unsigned char* data, int fileLength){
        std::vector<unsigned char> fileData;
        fileData.assign(data, data+fileLength);
        std::string s_fileName(fileName);

        m2GeomCache.provideFile(s_fileName, fileData);
        skinGeomCache.provideFile(s_fileName, fileData);
        textureCache.provideFile(s_fileName, fileData);
    };
    virtual void rejectFile(const char* fileName) {
        std::string s_fileName(fileName);

        m2GeomCache.reject(s_fileName);
        textureCache.reject(s_fileName);
    }
    void setFileRequestProcessor(IFileRequest*){};

    IControllable* getCurrentContollable() {
        return &this->m_firstCamera;
    };
public:
    virtual Cache<M2Geom> *getM2GeomCache() {
        return &m2GeomCache;
    };
    virtual Cache<SkinGeom> *getSkinGeomCache() {
        return &skinGeomCache;
    };
    virtual Cache<BlpTexture> *getTextureCache() {
        return &textureCache;
    };
    virtual ShaderRuntimeData *getM2Shader() {
        return m2Shader;
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
    mathfu::mat4 m_perspectiveMatrix;

    FirstPersonCamera m_firstCamera;
    FirstPersonCamera m_secondCamera;

    int canvWidth;
    int canvHeight;
    float canvAspect;

    float uFogStart = -1;
    float uFogEnd = -1;

    GLuint frameBuffer = 0;
    GLuint frameBufferColorTexture = 0;
    GLuint frameBufferDepthTexture = 0;
    GLuint vertBuffer = 0;


    Cache<M2Geom> m2GeomCache;
    Cache<SkinGeom> skinGeomCache;
    Cache<BlpTexture> textureCache;

    M2Object *m2Object = nullptr;

    void activateRenderFrameShader();

    void activateRenderDepthShader();

    void activateReadDepthBuffer();

    void activateAdtShader();

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

    void activateFrustumBoxShader();

    void activateDrawLinesShader();

    void activateDrawPortalShader();

    void activateTextureCompositionShader(GLuint texture);
};

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
};

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
};

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
};

// trim from start (copying)
static inline std::string ltrimmed(std::string s) {
    ltrim(s);
    return s;
};

// trim from end (copying)
static inline std::string rtrimmed(std::string s) {
    rtrim(s);
    return s;
};

// trim from both ends (copying)
static inline std::string trimmed(std::string s) {
    trim(s);
    return s;
};

#endif //WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
