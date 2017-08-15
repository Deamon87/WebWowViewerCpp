//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_ADTOBJECT_H
#define WEBWOWVIEWERCPP_ADTOBJECT_H

class IWoWInnerApi;
class AdtObject;
class M2Object;

#include <vector>
#include <set>

#include "../persistance/header/adtFileHeader.h"
#include "../opengl/header.h"
#include "../wowInnerApi.h"

#include "../persistance/adtFile.h"
#include "m2Object.h"
#include "wmoObject.h"

class AdtObject {
public:
    AdtObject() : alphaTextures(){
        m_api = nullptr;
    }
    void setApi(IWoWInnerApi *api) {
        m_api = api;
    }
    AdtObject(IWoWInnerApi *api) : m_api(api), alphaTextures()  {}
    void process(std::vector<unsigned char> &adtFile);
    void draw(std::vector<bool> &drawChunks);

    bool checkFrustumCulling(
            mathfu::vec4 &cameraPos,
            std::vector<mathfu::vec4> &frustumPlanes,
            std::vector<mathfu::vec3> &frustumPoints,
            std::vector<mathfu::vec3> &hullLines,
            mathfu::mat4 &lookAtMat4,
            std::set<M2Object*> &m2ObjectsCandidates,
            std::set<WmoObject*> &wmoCandidates);

private:
    void createVBO();
    void loadAlphaTextures(int limit);

    IWoWInnerApi *m_api;
    AdtFile *m_adtFile;
    int alphaTexturesLoaded = 0;
    bool m_loaded = false;

    int indexOffset = 0;
    int heightOffset = 0;

    GLuint combinedVbo = 0;
    GLuint stripVBO = 0;

private:
    std::vector<GLuint> alphaTextures;

    BlpTexture & getAdtTexture(int textureId);

};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
