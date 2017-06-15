#ifndef WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H

#include <string>
#include "shadersStructures.h"
#include "shader/ShaderRuntimeData.h"
#include "../include/wowScene.h"
#include "../include/config.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include "mathfu/glsl_mappings.h"
#include "camera/firstPersonCamera.h"


class WoWSceneImpl: public WoWScene {

public:
    WoWSceneImpl(Config *config);

    void draw(int deltaTime);
    void provideFile(int requestId, char* fileName, unsigned char* data, int fileLength){};
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

    float uFogStart = -1;
    float uFogEnd = -1;



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
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrimmed(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrimmed(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trimmed(std::string s) {
    trim(s);
    return s;
}

#endif //WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
