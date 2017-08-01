//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_BLPTEXTURE_H
#define WOWVIEWERLIB_BLPTEXTURE_H

#include "../opengl/header.h"
#include "../persistance/header/blpFileHeader.h"
#include <vector>

class BlpTexture {
public:
    void process(std::vector<unsigned char> &blpFile);
    bool getIsLoaded() { return m_isLoaded; };
    GLuint getGlTexture() {return texture;}
private:
    GLuint texture;
    bool m_isLoaded = false;

};


#endif //WOWVIEWERLIB_BLPTEXTURE_H
