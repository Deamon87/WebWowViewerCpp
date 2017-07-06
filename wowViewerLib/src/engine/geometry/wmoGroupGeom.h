//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOGROUPGEOM_H
#define WOWVIEWERLIB_WMOGROUPGEOM_H


#include <vector>
#include "../persistance/wmoFile.h"
#include "../persistance/ChunkFileReader.h"
#include "../opengl/header.h"

class WmoGroupGeom {
public:
    WmoGroupGeom() : indicesLen(0), verticesLen(0),
        normalsLen(0), textureCoordsLen(0), cvLen(0), doodadRefsLen(0),
        batchesLen(0), nodesLen(0), bpsIndiciesLen(0) {

    }

    void process(std::vector<unsigned char> &wmoGroupFile);

    static chunkDef<WmoGroupGeom> wmoGroupTable;

    void createVBO();
    void createIndexVBO();

private:
    MOGP *mogp;

    int16_t *indicies;
    int indicesLen;

    C3Vector *verticles;
    int verticesLen;

    C3Vector *normals;
    int normalsLen;

    C2Vector *textCoords;
    int textureCoordsLen;

    CImVector *colorArray;
    int cvLen;

    uint16_t *doodadRefs;
    int doodadRefsLen;

    SMOBatch *batches;
    int batchesLen;

    t_BSP_NODE * bsp_nodes;
    int nodesLen;

    uint16_t *bpsIndicies;
    int bpsIndiciesLen;

    GLuint combinedVBO;
    GLuint indexVBO;
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
