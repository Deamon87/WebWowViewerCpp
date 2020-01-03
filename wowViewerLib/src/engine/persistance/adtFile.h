//
// Created by deamon on 01.08.17.
//

#ifndef WOWVIEWERLIB_ADTFILE_H
#define WOWVIEWERLIB_ADTFILE_H
#include "helper/ChunkFileReader.h"
#include "header/adtFileHeader.h"
#include "header/wdtFileHeader.h"
#include "../../include/wowScene.h"

struct mcnkStruct_t {
    MCVT *mcvt = nullptr;
    MCLV *mclv = nullptr;
    MCCV *mccv = nullptr;
    SMNormal *mcnr = nullptr;
    PointerChecker<SMLayer> mcly = mclyCnt;
    int mclyCnt = 0;

    struct {uint32_t *doodad_refs; uint32_t *object_refs; } mcrf;

    PointerChecker<uint32_t> mcrd_doodad_refs = mcrd_doodad_refs_len;
    int mcrd_doodad_refs_len = 0;

    PointerChecker<uint32_t> mcrw_object_refs = mcrw_object_refs_len;
    int mcrw_object_refs_len = 0;


    uint8_t *mcal = nullptr;
};

class AdtFile {
public:
    AdtFile(std::string fileName){};
    AdtFile(int fileDataId){};

    void processTexture(const MPHDFlags &wdtObjFlags, int i, std::vector<uint8_t> &currentLayer);
    void process(HFileContent adtFile, const std::string &fileName);
    bool getIsLoaded() { return m_loaded; };
    void setIsMain(bool isMain) { m_mainAdt = isMain; };
public:
    SMMapHeader* mhdr;

    struct {
        SMChunkInfo chunkInfo[16][16];
    } *mcins;

    std::vector<std::string> textureNames;

    PointerChecker<uint32_t> mdid = mdid_len;
    int mdid_len = 0;

    PointerChecker<uint32_t> mhid = mhid_len;
    int mhid_len = 0;

    PointerChecker<char> doodadNamesField = doodadNamesFieldLen;
    int doodadNamesFieldLen;

    PointerChecker<char> wmoNamesField = wmoNamesFieldLen;
    int wmoNamesFieldLen;

    PointerChecker<SMTextureParams> mtxp = mtxp_len;
    int mtxp_len = 0;

    PointerChecker<SMDoodadDef> doodadDef = doodadDef_len;
    int doodadDef_len = 0;

    PointerChecker<SMMapObjDef> mapObjDef = mapObjDef_len;
    int mapObjDef_len = 0;

    PointerChecker<SMDoodadDef> doodadDefObj1 = doodadDefObj1_len;
    int doodadDefObj1_len = 0;

    PointerChecker<SMMapObjDefObj1> mapObjDefObj1 = mapObjDefObj1_len;
    int mapObjDefObj1_len = 0;

    PointerChecker<uint32_t> mldd = mldd_len;
    int mldd_len = 0;

    PointerChecker<float> floatDataBlob = floatDataBlob_len;
    int floatDataBlob_len = 0;

    PointerChecker<uint16_t> mvli_indicies = mvli_len;
    int mvli_len = 0;

    PointerChecker<uint16_t> mlsi_indicies = mlsi_len;
    int mlsi_len = 0;

    PointerChecker<MLLL> mllls = mlll_len;
    int mlll_len = 0;

    PointerChecker<MLND> mlnds = mlnd_len;
    int mlnd_len = 0;

    SMLodLevelPerObject * lod_levels_for_objects = nullptr;

    PointerChecker<uint32_t> mmid = mmid_length;
    int mmid_length;

    PointerChecker<uint32_t> mwid = mwid_length;
    int mwid_length;

    int mcnkRead = -1;
    std::array<SMChunk, 16*16> mapTile;
    std::array<mcnkStruct_t, 16*16> mcnkStructs;
    int mcnkMap[16][16] = {{-1}};

    std::vector<int16_t> strips;
    std::vector<int> stripOffsets;
private:
    bool m_loaded = false;
    bool m_mainAdt = false;

    void createTriangleStrip();
    static chunkDef<AdtFile> adtFileTable;
    HFileContent m_adtFile;
};

#endif //WOWVIEWERLIB_ADTFILE_H
