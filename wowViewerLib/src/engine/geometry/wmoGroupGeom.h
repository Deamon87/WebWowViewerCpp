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

    void draw(IWoWInnerApi *api, SMOMaterial *materials, std::function <BlpTexture&(int materialId)> getTextureFunc);

private:
    bool m_loaded = false;

    int normalOffset;
    int textOffset;
    int textOffset2;
    int colorOffset;
    int colorOffset2;

public:
    std::vector<uint8_t> m_wmoGroupFile;

    MOGP *mogp;

    int16_t *indicies;
    int indicesLen;

    C3Vector *verticles;
    int verticesLen;

    C3Vector *normals;
    int normalsLen;

    int textureCoordsRead = 0;
    C2Vector *textCoords;
    int textureCoordsLen;

    C2Vector *textCoords2;
    int textureCoordsLen2;

    C2Vector *textCoords3;
    int textureCoordsLen3;

    CImVector *colorArray;
    int cvLen;

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
