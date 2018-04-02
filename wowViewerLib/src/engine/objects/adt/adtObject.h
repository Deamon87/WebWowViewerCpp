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

#include "../../persistance/header/adtFileHeader.h"
#include "../../opengl/header.h"
#include "../../wowInnerApi.h"

#include "../../persistance/adtFile.h"
#include "../../persistance/wdtFile.h"
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"
#include "../iMapApi.h"

class AdtObject {
public:
    AdtObject(IWoWInnerApi *api, std::string &adtFileTemplate, WdtFile * wdtfile);
    void setMapApi(IMapApi *api) {
        m_mapApi = api;
    }


    void draw();

    bool checkFrustumCulling(
            mathfu::vec4 &cameraPos,
            std::vector<mathfu::vec4> &frustumPlanes,
            std::vector<mathfu::vec3> &frustumPoints,
            std::vector<mathfu::vec3> &hullLines,
            mathfu::mat4 &lookAtMat4,
            std::set<M2Object*> &m2ObjectsCandidates,
            std::set<WmoObject*> &wmoCandidates);

private:
    void loadingFinished();
    void createVBO();
    void loadAlphaTextures(int limit);

    IWoWInnerApi *m_api;
    IMapApi *m_mapApi;
    WdtFile *m_wdtFile;

    AdtFile *m_adtFile;
    AdtFile *m_adtFileTex;
    AdtFile *m_adtFileObj;
    AdtFile *m_adtFileObjLod;

    int alphaTexturesLoaded = 0;
    bool m_loaded = false;

    int indexOffset = 0;
    int heightOffset = 0;
    int normalOffset = 0;
    int colorOffset = 0;
    int lightingOffset = 0;

    std::unordered_map<int, BlpTexture*> m_requestedTextures;
    std::unordered_map<int, BlpTexture*> m_requestedTexturesHeight;
    std::unordered_map<int, BlpTexture*> m_requestedTexturesSpec;

    GLuint combinedVbo = 0;
    GLuint stripVBO = 0;

private:
    std::vector<GLuint> alphaTextures;
    std::vector<CAaBox> tileAabb;

    std::string m_adtFileTemplate;

    bool drawChunk[256] = {};

    std::vector<M2Object*> m2Objects;
    std::vector<WmoObject*> wmoObjects;

    BlpTexture & getAdtTexture(int textureId);
    BlpTexture & getAdtHeightTexture(int textureId);
    BlpTexture & getAdtSpecularTexture(int textureId);

    void calcBoundingBoxes();
    void loadM2s();
    void loadWmos();
};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
