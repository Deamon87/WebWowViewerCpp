#include "adtFile.h"
#include "header/adtFileHeader.h"
#include "header/wdtFileHeader.h"

chunkDef<AdtFile> AdtFile::adtFileTable = {
    handler : [](AdtFile& file, ChunkData& chunkData){},
    subChunks : {
        {
            'MVER',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                  debuglog("Entered MVER");
                }
            }
        },
        {
            'MHDR',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MHDR");
                    chunkData.readValue(file.mhdr);
                }
            }
        },
        {
            'MCIN',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MCIN");
                    chunkData.readValue(file.mcins);
                }
            }
        },
        {
            'MTEX',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
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
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MTXP");
                    file.mtxp_len = chunkData.chunkLen / sizeof(SMTextureParams);
                    chunkData.readValues(file.mtxp, file.mtxp_len);
                }
            }
        },
        {
            'MMDX',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MMDX");
                    file.doodadNamesFieldLen = chunkData.chunkLen;
                    chunkData.readValues(file.doodadNamesField, file.doodadNamesFieldLen);
                }
            }
        },
        {
            'MMID',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MMID");

                    file.mmid_length = chunkData.chunkLen / sizeof(uint32_t);
                    chunkData.readValues(file.mmid, file.mmid_length);
                }
            }
        },
        {
            'MWMO',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MWMO");

                    file.wmoNamesFieldLen = chunkData.chunkLen;
                    chunkData.readValues(file.wmoNamesField, file.wmoNamesFieldLen);
                }
            }
        },
        {
            'MWID',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MWID");

                    file.mwid_length = chunkData.chunkLen / sizeof(uint32_t);
                    chunkData.readValues(file.mwid, file.mwid_length);
                }
            }
        },
        {
            'MDDF',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MDDF");

                    file.doodadDef_len = chunkData.chunkLen / sizeof(SMDoodadDef);
                    chunkData.readValues(file.doodadDef, file.doodadDef_len);
                }
            }
        },
        {
            'MLDD',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MLDD");

//                    file.doodadDef_len = chunkData.chunkLen / sizeof(SMDoodadDef);
//                    chunkData.readValues(file.doodadDef, file.doodadDef_len);
                }
            }
        },
        {
            'MODF',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MODF");

                    file.mapObjDef_len = chunkData.chunkLen / sizeof(SMMapObjDef);
                    chunkData.readValues(file.mapObjDef, file.mapObjDef_len);
                }
            }
        },
        {
            'MCNK',
            {
                handler: [](AdtFile& file, ChunkData& chunkData){
                    debuglog("Entered MCNK");

                    file.mcnkRead++;

                    SMChunk chunk = file.mapTile[file.mcnkRead];
                    chunkDef<AdtFile> *def = &AdtFile::adtFileTable.subChunks['MCNK'];

                    if (file.m_mainAdt) {
                        chunkData.readValue(file.mapTile[file.mcnkRead]);

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
                subChunks : {
                    {
                        'MCVT',
                        {
                            handler: [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCVT");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mcvt);
                            }
                        }
                    },
                    {
                        'MCLV',
                        {
                            handler: [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCLV");
                            }
                        }
                    },
                    {
                        'MCCV',
                        {
                            handler: [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCCV");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mccv);
                            }
                        }
                    },
                    {
                        'MCNR',
                        {
                            handler: [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCNR");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mcnr);
                            }
                        }
                    },
                    {
                        'MCLY',
                        {
                            handler: [](AdtFile& file, ChunkData& chunkData){
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
                            handler: [](AdtFile& file, ChunkData& chunkData){
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
                        'MCAL',
                        {
                            handler: [](AdtFile& file, ChunkData& chunkData){
                                debuglog("Entered MCAL");
                                chunkData.readValue(file.mcnkStructs[file.mcnkRead].mcal);
                            }
                        }
                    }
                }
            }
        },
    }
};



std::vector<uint8_t> AdtFile::processTexture(const MPHDFlags &wdtObjFlags, int i) {
    mcnkStruct_t &mcnkObj = mcnkStructs[i];
    uint8_t* alphaArray = mcnkObj.mcal;
    SMLayer* layers = mcnkObj.mcly;

    std::vector<uint8_t> currentLayer = std::vector<uint8_t>((64*4) * 64, 0);
    if (layers == nullptr || alphaArray == nullptr) return currentLayer;

//    for (int j = 0; j < mapTile[i].nLayers; j++ ) {
    for (int j = 0; j < mcnkStructs[i].mclyCnt; j++ ) {
        int alphaOffs = layers[j].offsetInMCAL;
        int offO = j;
        int readForThisLayer = 0;

        if (!layers[j].flags.use_alpha_map) {
            for (int k = 0; k < 4096; k++) {
                currentLayer[offO+k*4] = 255;
            }
        } else if (layers[j].flags.alpha_map_compressed) {
            //Compressed
            //http://www.pxr.dk/wowdev/wiki/index.php?title=ADT/v18
            while( readForThisLayer < 4096 )
            {
                // fill or copy mode
                bool fill = (alphaArray[alphaOffs] & 0x80 ) > 0;
                int n = alphaArray[alphaOffs] & 0x7F;
                alphaOffs++;

                for ( int k = 0; k < n; k++ )
                {
                    currentLayer[offO] = alphaArray[alphaOffs];
                    readForThisLayer++;
                    offO += 4;

                    if( !fill ) alphaOffs++;
                }
                if( fill ) alphaOffs++;
            }
        } else {
            //Uncompressed
            if (((wdtObjFlags.adt_has_big_alpha) > 0) || ((wdtObjFlags.adt_has_height_texturing) > 0)) {
                //Uncompressed (4096)
                for (int iX =0; iX < 64; iX++) {
                    for (int iY = 0; iY < 64; iY++){
                        currentLayer[offO] = alphaArray[alphaOffs];

                        offO += 4;
                        readForThisLayer+=1;
                        alphaOffs++;
                    }
                }
            } else {
                //Uncompressed (2048)
                for (int iX =0; iX < 64; iX++) {
                    for (int iY = 0; iY < 32; iY++){
                        //Old world
                        currentLayer[offO] = (alphaArray[alphaOffs] & 0x0f ) * 17;
                        offO += 4;
                        currentLayer[offO] =  ((alphaArray[alphaOffs] & 0xf0 ) >> 4) * 17;
                        offO += 4;
                        readForThisLayer+=2; alphaOffs++;
                    }
                }
            }
        }

        //Fix alpha depending on flag
//        if (((wdtObjFlags.adt_has_big_alpha) > 0) || ((wdtObjFlags.adt_has_height_texturing) > 0)) {
//            int offO = j;
//            for (int k = 0; k < 4096; k++) {
//                currentLayer[offO+k*4] = (unsigned char) (178 * currentLayer[offO + k * 4] >> 8);
//            }
//        }
    }
    return currentLayer;
}

static bool isHoleLowRes(int hole, int i, int j) {
    static int holetab_h[4] = {0x1111, 0x2222, 0x4444, 0x8888};
    static int holetab_v[4] = {0x000F, 0x00F0, 0x0F00, 0xF000};

    i = i >> 1;
    j = j >> 1;

    return (hole & holetab_h[i] & holetab_v[j]) != 0;
}
static bool isHoleHighRes(uint64_t hole, int i, int j) {
    uint8_t * holeAsUint8 = (uint8_t *) &hole;
    return ((holeAsUint8[j] >> i) & 1) > 0;

//    return ((hole >> ((7-j) * 8  + i)) & 1) > 0;
}

void AdtFile::createTriangleStrip() {
    if (mcnkRead < 0) return;

    const int stripLenght = 9;
    static uint8_t squareIndsStrip[stripLenght] = {17, 0, 9, 1, 18, 18, 9, 17, 17};

    for (int i = 0; i <= mcnkRead; i++) {
        SMChunk &mcnkObj = mapTile[i];
        int holeLow = mcnkObj.holes_low_res;
        uint64_t holeHigh = mcnkObj.postMop.holes_high_res;

        stripOffsets.push_back(strips.size());

        int j = 0;
        bool first = true;
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                bool isHole = (!mcnkObj.flags.high_res_holes) ?
                              isHoleLowRes(holeLow, x, y) :
                              isHoleHighRes(holeHigh, x, y);
                if (!isHole) {
                    //There are 8 squares in width and 8 square in height.
                    //Each square is 4 triangles
                    if (!first) {
                        strips.push_back(squareIndsStrip[0] + 17 * y + x);
                    }
                    first = false;
                    for (int k = 0; k < stripLenght; k++) {
                        strips.push_back(squareIndsStrip[k] + 17 * y + x);
                    }
                }
            }
        }
    }
    stripOffsets.push_back(strips.size());
}

void AdtFile::process(std::vector<unsigned char> &adtFile) {
    m_adtFile = std::vector<uint8_t>(adtFile);
    CChunkFileReader reader(m_adtFile);
    reader.processFile(*this, &AdtFile::adtFileTable);

    createTriangleStrip();

    m_loaded = true;
}
