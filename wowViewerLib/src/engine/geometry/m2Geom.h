//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_M2GEOM_H
#define WOWVIEWERLIB_M2GEOM_H

#include <vector>
#include "../wowInnerApi.h"
#include "../persistance/header/M2FileHeader.h"
#include "../objects/m2/m2Helpers/M2MaterialInst.h"
//#include "../objects/m2Object.h"


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
            mathfu::vec4 &ambientColor,
            bool drawTransparent,
            std::vector<M2LightResult> &lights,
            bool instanced);

    void setupPlacementAttribute(GLuint placementVBO);

    void drawMesh(
            IWoWInnerApi *api,
            M2MaterialInst &materialData,
            M2SkinProfile &skinData,
            mathfu::vec4 &meshColor,
            float transparency,
            mathfu::mat4 &textureMatrix1,
            mathfu::mat4 &textureMatrix2,
            int vertexShaderIndex,
            int pixelShaderIndex,
            mathfu::vec4 &originalFogColor,
            int instanceCount);

    M2Data *m_m2Data;
    std::vector<uint32_t> skinFileDataIDs;
    std::vector<uint32_t> textureFileDataIDs;
private:
    std::vector<uint8_t> m2File;

    bool m_loaded = false;
    GLuint vertexVbo;

    static chunkDef<M2Geom> m2FileTable;

};
typedef std::shared_ptr<M2Geom> HM2Geom;

#endif //WOWVIEWERLIB_M2GEOM_H
