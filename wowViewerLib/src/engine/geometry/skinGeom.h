//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_SKINGEOM_H
#define WOWVIEWERLIB_SKINGEOM_H

#include <vector>
#include "../opengl/header.h"
#include "../persistance/skinFile.h"

class SkinGeom {
public:
    void process(std::vector<unsigned char> &skinFile);
    void createVBO();
    bool isLoaded() { return m_loaded; };
    void setupAttributes();
private:
    std::vector<uint8_t> m2Skin;
    M2SkinProfile *m_skinData;

    GLuint indexVbo = 0;
    bool m_loaded = false;
};


#endif //WOWVIEWERLIB_SKINGEOM_H
