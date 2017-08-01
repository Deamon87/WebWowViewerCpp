//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_ADTOBJECT_H
#define WEBWOWVIEWERCPP_ADTOBJECT_H

class IWoWInnerApi;
class AdtObject;

#include <vector>

#include "../persistance/header/adtFileHeader.h"
#include "../opengl/header.h"
#include "../wowInnerApi.h"

#include "../persistance/adtFile.h"

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
