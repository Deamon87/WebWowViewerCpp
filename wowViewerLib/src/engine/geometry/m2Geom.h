//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_M2GEOM_H
#define WOWVIEWERLIB_M2GEOM_H

#include <vector>
#include "../opengl/header.h"
#include "../persistance/M2File.h"
#include "../wowInnerApi.h"

class M2Geom {
public:
    void process(std::vector<unsigned char> &m2File);
    void loadTextures();
    void createVBO();
    void setupAttributes();
    bool isLoaded() { return m_loaded; };

    M2Data * getM2Data(){ if (m_loaded) {return m_m2Data;} else {return nullptr;}};

    void setupUniforms(
            IWoWInnerApi *api,
            mathfu::mat4 &placementMatrix,
            std::vector<mathfu::mat4> &boneMatrices,
            mathfu::vec4 &diffuseColor,
            bool drawTransparent);

private:
    std::vector<uint8_t> m2File;
    M2Data *m_m2Data;

    bool m_loaded = false;
    GLuint vertexVbo;

};


#endif //WOWVIEWERLIB_M2GEOM_H
