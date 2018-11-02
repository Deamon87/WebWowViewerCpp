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

    void process(const std::vector<unsigned char> &wmoGroupFile, const std::string &fileName);

    static chunkDef<WmoGroupGeom> wmoGroupTable;

    void setMOHD(SMOHeader *mohd) {this->mohd = mohd; };
    void setAttenuateFunction(std::function<void (WmoGroupGeom& wmoGroupGeom)> attenuateFunc) {this->m_attenuateFunc = attenuateFunc; };
    bool isLoaded() const { return m_loaded; };
    bool hasWater() const {return m_mliq != nullptr; };


    HGVertexBufferBindings getVertexBindings(IDevice &device);
    HGVertexBufferBindings getWaterVertexBindings(IDevice &device);
private:
    bool m_loaded = false;

    int normalOffset = 0;
    int textOffset = 0;
    int textOffset2 = 0;
    int textOffset3 = 0;
    int colorOffset = 0;
    int colorOffset2 = 0;

    std::function<void (WmoGroupGeom& wmoGroupGeom)> m_attenuateFunc;

    HGVertexBuffer getVBO(IDevice &device);
    HGIndexBuffer getIBO(IDevice &device);
public:
    std::vector<uint8_t> m_wmoGroupFile;

    SMOHeader *mohd = nullptr;

    MOGP *mogp;

    uint16_t *indicies = nullptr;
    int indicesLen = 0;

    SMOPoly *mopy = nullptr;
    int mopyLen = 0;

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

    uint16_t * lightRefList = nullptr;
    int lightRefListLen = 0;

    SMOBatch *batches = nullptr;
    int batchesLen = 0;

    SMOBatch *prePassbatches = nullptr;
    int prePassBatchesLen = 0;


    t_BSP_NODE * bsp_nodes = nullptr;
    int nodesLen = 0;

    uint16_t *bpsIndicies = nullptr;
    int bpsIndiciesLen = 0;

    int use_replacement_for_header_color = 0;
    CArgb replacement_for_header_color = {};

    MOLP *molp = nullptr;
    int molpCnt = 0;

    MLIQ *m_mliq = nullptr;


    SMOLVert * m_liquidVerticles = nullptr;
    int m_liquidVerticles_len = -1;

    SMOLTile * m_liquidTiles = nullptr;
    int m_liquidTiles_len = -1;

    HGVertexBuffer combinedVBO;
    HGIndexBuffer indexVBO;
    HGVertexBufferBindings vertexBufferBindings;
    HGVertexBufferBindings vertexWaterBufferBindings;

    HGVertexBuffer waterVBO;
    HGIndexBuffer waterIBO;

private:
    void fixColorVertexAlpha(SMOHeader *mohd);
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
