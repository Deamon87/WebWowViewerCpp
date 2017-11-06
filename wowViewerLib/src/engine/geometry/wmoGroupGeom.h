//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOGROUPGEOM_H
#define WOWVIEWERLIB_WMOGROUPGEOM_H


#include <vector>
#include "../persistance/header/wmoFileHeader.h"
#include "../persistance/helper/ChunkFileReader.h"
#include "../opengl/header.h"
#include "../wowInnerApi.h"

class WmoGroupGeom {
public:
    WmoGroupGeom() : indicesLen(0), verticesLen(0),
        normalsLen(0), textureCoordsLen(0), cvLen(0), doodadRefsLen(0),
        batchesLen(0), nodesLen(0), bpsIndiciesLen(0) {

    }

    void process(std::vector<unsigned char> &wmoGroupFile);

    static chunkDef<WmoGroupGeom> wmoGroupTable;

    void setMOHD(SMOHeader *mohd) {this->mohd = mohd; };
    void createVBO();
    bool isLoaded() { return m_loaded; };
    void createIndexVBO();

    void draw(IWoWInnerApi *api, SMOMaterial *materials, std::function <BlpTexture* (int materialId, bool isSpec)> getTextureFunc);

private:
    bool m_loaded = false;

    int normalOffset = 0;
    int textOffset = 0;
    int textOffset2 = 0;
    int textOffset3 = 0;
    int colorOffset = 0;
    int colorOffset2 = 0;

public:
    std::vector<uint8_t> m_wmoGroupFile;

    SMOHeader *mohd = nullptr;

    MOGP *mogp;

    uint16_t *indicies = nullptr;
    int indicesLen = 0;

    C3Vector *verticles = nullptr;
    int verticesLen;

    C3Vector *normals = nullptr;
    int normalsLen = 0;

    int textureCoordsRead = 0;
    C2Vector *textCoords = nullptr;
    int textureCoordsLen = 0;

    C2Vector *textCoords2 = nullptr;
    int textureCoordsLen2 = 0;

    C2Vector *textCoords3 = nullptr;
    int textureCoordsLen3 = 0;

    CImVector *colorArray = nullptr;
    int cvLen = 0;

    CImVector *colorArray2 = nullptr;
    int cvLen2 = 0;
    int mocvRead = 0;

    uint16_t *doodadRefs = nullptr;
    int doodadRefsLen = 0;

    SMOLight * lights = nullptr;
    int lightsLen = 0;

    SMOBatch *batches = nullptr;
    int batchesLen = 0;

    t_BSP_NODE * bsp_nodes = nullptr;
    int nodesLen = 0;

    uint16_t *bpsIndicies = nullptr;
    int bpsIndiciesLen = 0;

    GLuint combinedVBO;
    GLuint indexVBO;

    void fixColorVertexAlpha(SMOHeader *mohd);
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
