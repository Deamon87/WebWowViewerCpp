#include "adtFile.h"
#include "header/adtFileHeader.h"
#include "header/wdtFileHeader.h"

chunkDef<AdtFile> AdtFile::adtFileTable = {
    [](AdtFile& file, ChunkData& chunkData){},
    {
        {
            'MVER',
            {
                [](AdtFile& file, ChunkData& chunkData){
                  debuglog("Entered MVER");
                }
            }
        },
        {
            'MHDR',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MHDR");
                    chunkData.readValue(file.mhdr);
                }
            }
        },
        {
            'MCIN',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MCIN");
                    chunkData.readValue(file.mcins);
                }
            }
        },
        {
            'MDID',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MDID");
                    file.mdid_len = chunkData.chunkLen / 4;
                    chunkData.readValues(file.mdid, file.mdid_len);
                }
            }
        },
        {
            'MHID',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MHID");
                    file.mhid_len = chunkData.chunkLen / 4;
                    chunkData.readValues(file.mhid, file.mhid_len);
                }
            }
        },
        {
            'MTEX',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MTEX");
                    char *textureNamesField;
                    int textureNamesFieldLen = chunkData.chunkLen;
                    chunkData.readValues(textureNamesField, textureNamesFieldLen);

                    int i = 0;
                    while (i < textureNamesFieldLen) {
                        file.textureNames.push_back(std::string(textureNamesField+i));
                        i+= file.textureNames[file.textureNames.size()-1].size()+1;
                    }
                }
            }
        },
        {
            'MTXP',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MTXP");
                    file.mtxp_len = chunkData.chunkLen / sizeof(SMTextureParams);
                    chunkData.readValues(file.mtxp, file.mtxp_len);
//                    for (int i = 0; i < file.mtxp_len; i++) {
//                        std::cout << "file.mtxp[" << i << "].padding = " << file.mtxp[i].padding << std::endl;
//                    }
                }
            }
        },
        {
            'MTXF',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MTXF");
                    std::cout << "Found MTXF" << std::endl;
                    file.mtxf_len = chunkData.chunkLen / sizeof(SMTextureFlags);
                    chunkData.readValues(file.mtxf, file.mtxf_len);
                }
            }
        },
        {
            'MAMP',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MAMP");
//                    std::cout << "Found MAMP" << std::endl;
//                    std::cout << "mamp chunkData.chunkLen = " << chunkData.chunkLen << std::endl;

                    chunkData.readValue(file.mamp_val);
//                    std::cout << "mamp value = " << (int)file.mamp_val << std::endl;
                }
            }
        },
        {
            'MMDX',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MMDX");
                    file.doodadNamesFieldLen = chunkData.chunkLen;
                    chunkData.readValues(file.doodadNamesField, file.doodadNamesFieldLen);
                }
            }
        },
        {
            'MMID',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MMID");

                    file.mmid_length = chunkData.chunkLen / sizeof(uint32_t);
                    chunkData.readValues(file.mmid, file.mmid_length);
                }
            }
        },
        {
            'MWMO',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MWMO");

                    file.wmoNamesFieldLen = chunkData.chunkLen;
                    chunkData.readValues(file.wmoNamesField, file.wmoNamesFieldLen);
                }
            }
        },
        {
            'MWID',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MWID");

                    file.mwid_length = chunkData.chunkLen / sizeof(uint32_t);
                    chunkData.readValues(file.mwid, file.mwid_length);
                }
            }
        },
        {
            'MDDF',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MDDF");

                    file.doodadDef_len = chunkData.chunkLen / sizeof(SMDoodadDef);
                    chunkData.readValues(file.doodadDef, file.doodadDef_len);
                }
            }
        },
        {
            'MLDD',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLDD");

                    file.doodadDefObj1_len = chunkData.chunkLen / sizeof(SMDoodadDef);
                    chunkData.readValues(file.doodadDefObj1, file.doodadDefObj1_len);
                }
            }
        },
        {
            'MLMX',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLMX");

                    file.wmoLodExtents_len = chunkData.chunkLen / sizeof(LodExtents);
                    chunkData.readValues(file.wmoLodExtents, file.wmoLodExtents_len);
                }
            }
        },

        {
            'MLDL',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLDL");

                    file.mldd_len = chunkData.chunkLen / sizeof(uint32_t);
                    chunkData.readValues(file.mldd, file.mldd_len);
                }
            }
        },
        {
            'MLMD',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLMD");

                    file.mapObjDefObj1_len = chunkData.chunkLen / sizeof(SMMapObjDefObj1);
                    chunkData.readValues(file.mapObjDefObj1, file.mapObjDefObj1_len);
                }
            }
        },
        {
            'MLFD',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLFD");

                    chunkData.readValue(file.lod_levels_for_objects);


                }
            }
        },
        {
            'MODF',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MODF");

                    file.mapObjDef_len = chunkData.chunkLen / sizeof(SMMapObjDef);
                    chunkData.readValues(file.mapObjDef, file.mapObjDef_len);
                }
            }
        },
        {
            'MH2O',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MH2O");
                    //Read everything into blob and parse in ADTObject
                    file.mH2OblobOffset = chunkData.bytesRead;
                    int byteSize = chunkData.chunkLen - chunkData.bytesRead;
                    file.mH2OBlob_len = byteSize;
                    chunkData.readValues(file.mH2OBlob, byteSize);

                    file.mH2OHeader = (M2HOHeader *)(&file.mH2OBlob[0]);
                }
            }
        },
        {
            'MCNK',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MCNK");

                    file.mcnkRead++;
                    if (file.mcnkRead >= 256) {
                        std::cout << "ALARM!"<< std::endl;
                    }

                    SMChunk &chunk = file.mapTile[file.mcnkRead];
                    chunkDef<AdtFile> *def = &AdtFile::adtFileTable.subChunks['MCNK'];

                    if (file.m_mainAdt) {
                        chunkData.readValue(chunk);
                        file.mcnkMap[chunk.IndexX][chunk.IndexY] = file.mcnkRead;

//1. Load MCVT
//                        if (chunk.ofsHeight > 0) {
//                            int offset = chunkData.dataOffset + chunk.ofsHeight - 8;
//                            chunkData.processChunkAtOffs(offset, file, def);
//                        }
////2. Load MCNR
//                        if (chunk.ofsNormal > 0) {
//                            int offset = chunkData.dataOffset + chunk.ofsNormal - 8;
//                            chunkData.processChunkAtOffs(offset, file, def);
//                        }
////3. Load MCLY
//                        if (chunk.ofsLayer > 0) {
//                            int offset = chunkData.dataOffset + chunk.ofsLayer - 8;
//                            chunkData.processChunkAtOffs(offset, file, def);
//                        }
////4. Load MCAL
//                        if (chunk.ofsAlpha > 0) {
//                            int offset = chunkData.dataOffset + chunk.ofsAlpha - 8;
//                            chunkData.processChunkAtOffsWithSize(offset, chunk.sizeAlpha, file, def);
//                        }
////5. Load MCRF
//                        if (chunk.ofsRefs > 0) {
//                            int offset = chunkData.dataOffset + chunk.ofsRefs - 8;
//                            chunkData.processChunkAtOffs(offset, file, def);
//                        }

//Force stop the parsing of chunk
                        //TODO: Turn on this for Wotlk

                        //chunkData.bytesRead = chunkData.chunkLen;
                    }
                },
                {
                    {
                        'MCVT',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCVT");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mcvt);
                            }
                        }
                    },
                    {
                        'MCLV',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCLV");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mclv);
                            }
                        }
                    },
                    {
                        'MCCV',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCCV");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mccv);
                            }
                        }
                    },
                    {
                        'MCNR',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCNR");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mcnr);
                            }
                        }
                    },
                    {
                        'MCLY',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCLY");

                                int mclyCnt = chunkData.chunkLen/sizeof(SMLayer);

                                file.mcnkStructs[file.mcnkRead].mclyCnt = mclyCnt;

                                chunkData.readValues(
                                        file.mcnkStructs[file.mcnkRead].mcly,
                                        mclyCnt
                                );
                            }
                        }
                    },
                    {
                        'MCRF',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCRF");

                                chunkData.readValues(
                                        file.mcnkStructs[file.mcnkRead].mcrf.doodad_refs,
                                        file.mapTile[file.mcnkRead].nDoodadRefs
                                );
                                chunkData.readValues(
                                        file.mcnkStructs[file.mcnkRead].mcrf.object_refs,
                                        file.mapTile[file.mcnkRead].nMapObjRefs
                                );
                            }
                        }
                    },
                    {
                        'MCRD',
                        {
                            [](AdtFile& file, ChunkData& chunkData) {
                                debuglog("Entered MCRD");

                                file.mcnkStructs[file.mcnkRead].mcrd_doodad_refs_len =
                                    chunkData.chunkLen / sizeof(uint32_t);
                                chunkData.readValues(
                                    file.mcnkStructs[file.mcnkRead].mcrd_doodad_refs,
                                    file.mcnkStructs[file.mcnkRead].mcrd_doodad_refs_len
                                );
                            }
                        }
                    },
                    {
                        'MCRW',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCRW");

                                file.mcnkStructs[file.mcnkRead].mcrw_object_refs_len = chunkData.chunkLen / sizeof(uint32_t);
                                chunkData.readValues(
                                    file.mcnkStructs[file.mcnkRead].mcrw_object_refs,
                                    file.mcnkStructs[file.mcnkRead].mcrw_object_refs_len
                                );
                            }
                        }
                    },
                    {
                        'MCAL',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCAL");
                                chunkData.readValues(file.mcnkStructs[file.mcnkRead].mcal, chunkData.chunkLen);
                            }
                        }
                    },
                    {
                        'MCLQ',
                        {
                            [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCQL");
                                std::cout << "Entered MCQL" << std::endl;


                                file.mcnkStructs[file.mcnkRead].mcqlLen = chunkData.chunkLen;
                                chunkData.readValues(
                                    file.mcnkStructs[file.mcnkRead].mcql,
                                    file.mcnkStructs[file.mcnkRead].mcqlLen
                                );
                            }
                        }
                    }
                }
            }
        },
        {
            'MLHD',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLHD ");

                    chunkData.readValue(file.mlHeader);
                }
            }
        },
        {
            'MLVH',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLVH");

                    file.floatDataBlob_len = chunkData.chunkLen / sizeof(float);
                    chunkData.readValues(file.floatDataBlob, file.floatDataBlob_len);
                }
            }
        },
        {
            'MLVI',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLVI");

                    file.mvli_len = chunkData.chunkLen / sizeof(uint16_t);
                    chunkData.readValues(file.mvli_indicies, file.mvli_len);
                }
            }
        },
        {
            'MLLL',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLLL");

                    file.mlll_len = chunkData.chunkLen / sizeof(MLLL);
                    chunkData.readValues(file.mllls, file.mlll_len);
                }
            }
        },
        {
            'MLND',
            {
                [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLND");

                    file.mlnd_len = chunkData.chunkLen / sizeof(MLND);
                    chunkData.readValues(file.mlnds, file.mlnd_len);
                }
            }
        },
        {
            'MLSI',
                {
                    [](AdtFile& file, ChunkData& chunkData){
                        debuglog("Entered MLSI");

                        file.mlsi_len = chunkData.chunkLen / sizeof(uint16_t);
                        chunkData.readValues(file.mlsi_indicies, file.mlsi_len);
                    }
                }
        }
    }
};

MCAL_Offsets_Runtime AdtFile::createAlphaTextureRuntime(int mcnkChunkIndex) {
    mcnkStruct_t &mcnkObj = mcnkStructs[mcnkChunkIndex];
    uint8_t* mcal = mcnkObj.mcal;
    auto &layers = mcnkObj.mcly;

    uint32_t uniqueTextureIds[4] = {0,0,0,0};
    uint32_t uniqTextCnt = 0;
    if ( mcnkObj.mclyCnt) {
        uniqueTextureIds[0] = layers[0].textureId;
        uniqTextCnt = 1;
    }
    for (int j = 1; j < mcnkObj.mclyCnt; j++ ) {
        auto &layerDef = layers[j];

        bool alreadyIncluded = false;
        for (int k = 0; k < uniqTextCnt; k++) {
            if (uniqueTextureIds[k] == layerDef.textureId) {
                alreadyIncluded = true;
                break;
            }
        }
        if (!alreadyIncluded) {
            uniqueTextureIds[uniqTextCnt++] = layerDef.textureId;
        }
    }


    MCAL_Offsets_Runtime result;
    for (int j = 0; j < mcnkObj.mclyCnt; j++ ) {
        auto &layerDef = layers[j];
        uint32_t alphaOffs = layerDef.offsetInMCAL;

        for (int k = 0; k < uniqTextCnt; k++) {
            if (uniqueTextureIds[k] == layerDef.textureId) {
                if (layerDef.flags.use_alpha_map) {
                    result.alphaPtrs[k] = &mcal[alphaOffs];
                } else {
                    result.uncompressedIndex = k;
                }
                result.alphaFlags[k] = layerDef.flags;

                break;
            }
        }
    }

    return result;
}

const std::array<uint8_t, 64> noAlphaArray = {};

void AdtFile::processAlphaTextureRow(MCAL_Offsets_Runtime &mcalRuntime, const MPHDFlags &wdtObjFlags, int i, uint8_t* __restrict currentLayer, uint32_t textureSize) {
    mcnkStruct_t &mcnkObj = mcnkStructs[i];
    uint8_t* mcal = mcnkObj.mcal;
    auto &layers = mcnkObj.mcly;

    assert(mcnkObj.mclyCnt <= 4);
    if (layers == nullptr || mcal == nullptr) return;


    for (int j = 0; j < mcnkObj.mclyCnt; j++ ) {
        auto &layerDef = layers[j];

        auto &alphaArray = mcalRuntime.alphaPtrs[j];
        auto alphaFlag = mcalRuntime.alphaFlags[j];

        int readForThisLayer = 0;

        if (!alphaFlag.use_alpha_map) {
            std::copy(noAlphaArray.data(), noAlphaArray.data() + textureSize, currentLayer);
            currentLayer += textureSize;
            readForThisLayer += textureSize;
        } else if (alphaFlag.alpha_map_compressed) {
            //Compressed
            //http://www.pxr.dk/wowdev/wiki/index.php?title=ADT/v18
            while (readForThisLayer < textureSize) {
                // fill or copy mode
                uint8_t codeVal = *alphaArray++;
                bool fill = (codeVal & 0x80) != 0;

                int n = fill ?
                        codeVal & 0x7F :
                        codeVal;

                if (fill) {
                    uint8_t alphaVal = *alphaArray++;

                    for (int k = 0; (k < n) && (readForThisLayer < textureSize); k++, readForThisLayer++) {
                        *currentLayer++ = alphaVal;
                    }
                } else {
                    for (int k = 0; (k < n) && (readForThisLayer < textureSize); k++, readForThisLayer++) {
                        *currentLayer++ = *alphaArray;
                        alphaArray++;
                    }
                }
            }
        } else {
            //Uncompressed
            if (((wdtObjFlags.adt_has_big_alpha) > 0) || ((wdtObjFlags.adt_has_height_texturing) > 0)) {
                //Uncompressed (4096)
                for (int iY = 0; iY < textureSize; iY++) {
                    *currentLayer++ = *alphaArray++;
                }

            } else {
                //Uncompressed (2048)
                for (int iY = 0; iY < textureSize; iY += 2) {
                    //Old world
                    uint8_t alphaVal = *alphaArray++;
                    *currentLayer++ = (alphaVal & 0x0f) * 17;
                    *currentLayer++ = ((alphaVal & 0xf0) >> 4) * 17;
                }
            }
        }
    }

//    if (mcalRuntime.uncompressedIndex) {
//        auto pCurrentLayer = currentLayer;
//        auto pCurrentLayerInt = (uint32_t *)currentLayer;
//        for ( int i = 0; i < 64; i++ ) {
//            uint8_t layer0 = *pCurrentLayer++;
//            uint8_t layer1 = *pCurrentLayer++;
//            uint8_t layer2 = *pCurrentLayer++;
//            uint8_t layer3 = *pCurrentLayer++;
//
//            *pCurrentLayerInt++ = 255 - layer0 - layer1 - layer2 - layer3;
//        }
//    }
}

bool isHoleLowRes(int hole, int i, int j) {
    assert(i >= 0 && i < 8);
    assert(j >= 0 && j < 8);

    static int holetab_h[4] = {0x1111, 0x2222, 0x4444, 0x8888};
    static int holetab_v[4] = {0x000F, 0x00F0, 0x0F00, 0xF000};

    i = i >> 1;
    j = j >> 1;

    return (hole & holetab_h[i] & holetab_v[j]) != 0;
}
bool isHoleHighRes(uint64_t hole, int i, int j) {
    assert(i >= 0 && i < 8);
    assert(j >= 0 && j < 8);

    uint8_t * holeAsUint8 = (uint8_t *) &hole;
    return ((holeAsUint8[j] >> i) & 1) > 0;

//    return ((hole >> ((7-j) * 8  + i)) & 1) > 0;
}

const int stripLenght = 9;
const int vertCountPerMCNK= 9 * 9 + 8 * 8;
static uint8_t squareIndsStrip[stripLenght] = {17, 0, 9, 1, 18, 18, 9, 17, 17};
const int triangleLength = 12;
//static uint8_t squareIndsTriangle[triangleLength] = {17, 9, 0, 0, 9, 1, 1, 9, 18, 18, 9, 17};
static uint8_t squareIndsTriangle[triangleLength] = {9, 0, 17, 9, 1, 0, 9, 18, 1, 9, 17, 18};


inline void addSquare(int offset, int x, int y, std::vector<int16_t> &strips) {
    for (int k = 0; k < triangleLength; k++) {
        strips.push_back(offset + squareIndsTriangle[k] + 17 * y + x);
    }
}

void AdtFile::createTriangleStrip() {
    if (mcnkRead < 0) return;

    strips = std::vector<int16_t>();
    stripOffsets = std::vector<int>();

    stripsNoHoles = std::vector<int16_t>();
    stripOffsetsNoHoles = std::vector<int>();

    for (int i = 0; i <= mcnkRead; i++) {
        SMChunk &mcnkObj = mapTile[i];
        int holeLow = mcnkObj.holes_low_res;
        uint64_t holeHigh = mcnkObj.postMop.holes_high_res;

        stripOffsets.push_back(strips.size());
        stripOffsetsNoHoles.push_back(stripsNoHoles.size());

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                bool isHole = (!mcnkObj.flags.high_res_holes) ?
                              isHoleLowRes(holeLow, x, y) :
                              isHoleHighRes(holeHigh, x, y);
                if (!isHole) {
                    //There are 8 squares in width and 8 square in height.
                    //Each square is 4 triangles
                    addSquare(i * vertCountPerMCNK, x, y, strips);
                }
                addSquare(i * vertCountPerMCNK, x, y, stripsNoHoles);
            }
        }
    }
    stripOffsets.push_back(strips.size());
    stripOffsetsNoHoles.push_back(stripsNoHoles.size());
}

void AdtFile::process(HFileContent adtFile, const std::string &fileName) {
    m_adtFile = adtFile;
    CChunkFileReader reader(*m_adtFile.get(), fileName);
    reader.processFile(*this, &AdtFile::adtFileTable);

    createTriangleStrip();

    fsStatus = FileStatus::FSLoaded;
}
