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

    void createVBO();
    bool isLoaded() { return m_loaded; };
    void createIndexVBO();

    void draw(IWoWInnerApi *api, SMOMaterial *materials, std::function <BlpTexture* (int materialId, bool isSpec)> getTextureFunc);

private:
    bool m_loaded = false;

    int normalOffset;
    int textOffset = 0;
    int textOffset2 = 0;
    int textOffset3 = 0;
    int colorOffset;
    int colorOffset2;

public:
    std::vector<uint8_t> m_wmoGroupFile;

    MOGP *mogp;

    uint16_t *indicies;
    int indicesLen;

    C3Vector *verticles;
    int verticesLen;

    C3Vector *normals;
    int normalsLen;

    int textureCoordsRead = 0;
    C2Vector *textCoords;
    int textureCoordsLen = 0;

    C2Vector *textCoords2;
    int textureCoordsLen2 = 0;

    C2Vector *textCoords3;
    int textureCoordsLen3 = 0;

    CImVector *colorArray;
    int cvLen = 0;

    CImVector *colorArray2;
    int cvLen2 = 0;
    int mocvRead = 0;

    uint16_t *doodadRefs;
    int doodadRefsLen;

    SMOBatch *batches;
    int batchesLen;

    t_BSP_NODE * bsp_nodes = nullptr;
    int nodesLen;

    uint16_t *bpsIndicies;
    int bpsIndiciesLen;

    GLuint combinedVBO;
    GLuint indexVBO;
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
