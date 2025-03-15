//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOGROUPGEOM_H
#define WOWVIEWERLIB_WMOGROUPGEOM_H


#include <vector>
#include "../persistance/header/wmoFileHeader.h"
#include "../persistance/helper/ChunkFileReader.h"
#include "../../include/sharedFile.h"
#include "../../renderer/mapScene/IMapSceneBufferCreate.h"
enum class LiquidTypes : int
{
    LIQUID_NONE = -1,
    // ...
    LIQUID_WMO_Water = 13,
    LIQUID_WMO_Ocean = 14,
    LIQUID_Green_Lava = 15,
    LIQUID_WMO_Magma = 19,
    LIQUID_WMO_Slime = 20,

    LIQUID_END_BASIC_LIQUIDS = 20,
    LIQUID_FIRST_NONBASIC_LIQUID_TYPE = 21,

    LIQUID_NAXX_SLIME = 21,
    // ...

};

class WmoGroupGeom : public PersistentFile {
public:
    WmoGroupGeom(std::string fileName){ m_fileName = fileName; };
    WmoGroupGeom(int fileDataId){ m_fileDataId = fileDataId;};

    void process(HFileContent wmoGroupFile, const std::string &fileName) override;

    static chunkDef<WmoGroupGeom> wmoGroupTable;

    void setAttenuateFunction(std::function<void (WmoGroupGeom& wmoGroupGeom)> attenuateFunc) {this->m_attenuateFunc = attenuateFunc; };
    bool hasWater() const {return m_mliq != nullptr; };

    HGVertexBufferBindings getVertexBindings(const HMapSceneBufferCreate &sceneRenderer, SMOHeader *mohd);
    HGVertexBufferBindings getWaterVertexBindings(const HMapSceneBufferCreate &sceneRenderer, LiquidTypes liquid_type, CAaBox &waterAaBB);

    int getFileDataId() const {return m_fileDataId;}
    const std::string &getFileName() const {return m_fileName;}
private:
    std::function<void (WmoGroupGeom& wmoGroupGeom)> m_attenuateFunc;

    LiquidTypes getLegacyWaterType(int a);
    HGVertexBuffer getVBO(const HMapSceneBufferCreate &sceneRenderer);
    HGIndexBuffer getIBO(const HMapSceneBufferCreate &sceneRenderer);
public:
    std::string m_fileName = "";
    int m_fileDataId = 0;

    HFileContent m_wmoGroupFile;

    MOGP *mogp = nullptr;

    PointerChecker<uint16_t> indicies = PointerChecker<uint16_t>(indicesLen);
    int indicesLen = 0;

    PointerChecker<uint16_t> mopy = (mopyLen);
    int mopyLen = 0;

    PointerChecker<C3Vector> verticles = (verticesLen);
    int verticesLen = 0;

    PointerChecker<C3Vector> normals = (normalsLen);
    int normalsLen = 0;

    int textureCoordsRead = 0;
    PointerChecker<C2Vector> textCoords = (textureCoordsLen);
    int textureCoordsLen = 0;

    PointerChecker<C2Vector> textCoords2 = (textureCoordsLen2);
    int textureCoordsLen2 = 0;

    PointerChecker<C2Vector> textCoords3 = (textureCoordsLen3);
    int textureCoordsLen3 = 0;

    PointerChecker<C2Vector> textCoords4 = (textureCoordsLen4);
    int textureCoordsLen4 = 0;

    PointerChecker<CImVector> colorArray = cvLen;
    int cvLen = 0;



    PointerChecker<CImVector> colorArray2 = cvLen2;
    int cvLen2 = 0;
    int mocvRead = 0;

    PointerChecker<CImVector> colorSecondArray = cvSecondLen;
    int cvSecondLen = 0;

    PointerChecker<uint16_t> doodadRefs = (doodadRefsLen);
    int doodadRefsLen = 0;

    PointerChecker<uint16_t> lightRefList = (lightRefListLen);
    int lightRefListLen = 0;

    PointerChecker<SMOBatch> batches = (batchesLen);
    int batchesLen = 0;

    PointerChecker<SMOBatch> prePassbatches = (prePassBatchesLen);
    int prePassBatchesLen = 0;


    PointerChecker<t_BSP_NODE> bsp_nodes = (nodesLen);
    int nodesLen = 0;


    PointerChecker<uint16_t> bpsIndicies = (bpsIndiciesLen);
    int bpsIndiciesLen = 0;

    int use_replacement_for_header_color = 0;
    CArgb replacement_for_header_color = {};



    MLIQ *m_mliq = nullptr;

    PointerChecker<SMOLVert> m_liquidVerticles = (m_liquidVerticles_len);
    int m_liquidVerticles_len = -1;

    PointerChecker<SMOLTile> m_liquidTiles = (m_liquidTiles_len);
    int m_liquidTiles_len = -1;



    PointerChecker<LightRecPerSet> mapobject_pointlight_animsets = (mapobject_pointlight_animsetsLen);
    int mapobject_pointlight_animsetsLen = -1;

    PointerChecker<LightRecPerSet> mapobject_spotlight_animsets = (mapobject_spotlight_animsetsLen);
    int mapobject_spotlight_animsetsLen = -1;

    PointerChecker<LightRecPerSet> map_object_lightset_pointlights = (map_object_lightset_pointlightsLen);
    int map_object_lightset_pointlightsLen = -1;

    PointerChecker<map_object_point_light> map_object_point_lights = (map_object_point_lightLen);
    int map_object_point_lightLen = 0;

    PointerChecker<map_object_pointlight_anim> map_object_pointlight_anims = (map_object_pointlight_animLen);
    int map_object_pointlight_animLen = 0;

    PointerChecker<uint16_t > mapobject_new_light_refs = (mapobject_new_light_refsLen);
    int mapobject_new_light_refsLen = 0;

    HGVertexBuffer combinedVBO;
    HGIndexBuffer indexVBO;
    HGVertexBufferBindings vertexBufferBindings;
    HGVertexBufferBindings vertexWaterBufferBindings;
    int waterIndexSize = 0;

    LiquidTypes m_legacyLiquidType = LiquidTypes::LIQUID_NONE;

    HGVertexBuffer waterVBO;
    HGIndexBuffer waterIBO;

private:
    void fixColorVertexAlpha(SMOHeader *mohd);
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
