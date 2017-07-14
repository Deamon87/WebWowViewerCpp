//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_M2GEOM_H
#define WOWVIEWERLIB_M2GEOM_H

class M2SkinProfile;
#include <vector>
#include "../wowInnerApi.h"
#include "../persistance/M2File.h"
#include "../objects/m2Helpers/M2MaterialInst.h"
#include "../objects/m2Object.h"

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

    void drawMesh(
            IWoWInnerApi *api,
            M2MaterialInst &materialData,
            M2SkinProfile &skinData,
            mathfu::vec4 &meshColor,
            float transparency,
            mathfu::mat4 &textureMatrix1,
            mathfu::mat4 &textureMatrix2,
            int pixelShaderIndex,
            mathfu::vec4 &originalFogColor,
            int instanceCount);
private:
    std::vector<uint8_t> m2File;
    M2Data *m_m2Data;

    bool m_loaded = false;
    GLuint vertexVbo;

};


#endif //WOWVIEWERLIB_M2GEOM_H
