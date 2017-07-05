//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOGROUPGEOM_H
#define WOWVIEWERLIB_WMOGROUPGEOM_H


#include <vector>
#include "../persistance/wmoFile.h"
#include "../persistance/ChunkFileReader.h"

class WmoGroupGeom {
public:
    void process(std::vector<unsigned char> &wmoGroupFile);

    static chunkDef<WmoGroupGeom> wmoGroupTable;
private:
    MOGP *mogp;

    int16_t *indicies;
    int indicesLen;

    C3Vector *verticles;
    int verticesLen;

    C3Vector *normals;
    int normalsLen;

    SMOBatch *batches;
    int batchesLen;

    t_BSP_NODE * bsp_nodes;
    int nodesLen;

    uint16_t *bpsIndicies;
    int bpsIndiciesLen;
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
