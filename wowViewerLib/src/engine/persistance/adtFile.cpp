#include "adtFile.h"
#include "header/adtFileHeader.h"

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
                    chunkData.readValue(file.mapTile[file.mcnkRead]);

                    SMChunk chunk = file.mapTile[file.mcnkRead];
                    chunkDef<AdtFile> *def = &AdtFile::adtFileTable.subChunks['MCNK'];

//1. Load MCVT
                    if (chunk.ofsHeight > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsHeight - 8;
                        chunkData.processChunkAtOffs(offset, file, def);
                    }
//2. Load MCNR
                    if (chunk.ofsNormal > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsNormal - 8;
                        chunkData.processChunkAtOffs(offset, file, def);
                    }
//3. Load MCLY
                    if (chunk.ofsLayer > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsLayer - 8;
                        chunkData.processChunkAtOffs(offset, file, def);
                    }
//4. Load MCAL
                    if (chunk.ofsAlpha > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsAlpha - 8;
                        chunkData.processChunkAtOffsWithSize(offset, chunk.sizeAlpha, file, def);
                    }
//5. Load MCRF
                    if (chunk.ofsRefs > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsRefs - 8;
                        chunkData.processChunkAtOffs(offset, file, def);
                    }

//Force stop the parsing of chunk
                    chunkData.bytesRead = chunkData.chunkLen;

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

                                chunkData.readValues(
                                        file.mcnkStructs[file.mcnkRead].mcly,
                                        chunkData.chunkLen/sizeof(SMLayer)
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

AdtFile::AdtFile(std::vector<unsigned char> &adtFile) {
    m_adtFile = std::vector<uint8_t>(adtFile);
    CChunkFileReader reader(m_adtFile);
    reader.processFile(*this, &AdtFile::adtFileTable);

    createTriangleStrip();
}

static bool isHole(int hole, int i, int j) {
    static int holetab_h[4] = {0x1111, 0x2222, 0x4444, 0x8888};
    static int holetab_v[4] = {0x000F, 0x00F0, 0x0F00, 0xF000};

    return (hole & holetab_h[i] & holetab_v[j]) != 0;
}

static int indexMapBuf(int x, int y) {
    int result = ((y + 1) >> 1) * 9 + (y >> 1) * 8 + x;
    return result;
}


std::vector<uint8_t> AdtFile::processTexture(int wdtObjFlags, int i) {
    mcnkStruct_t &mcnkObj = mcnkStructs[i];
    uint8_t* alphaArray = mcnkObj.mcal;
    SMLayer* layers = mcnkObj.mcly;

    std::vector<uint8_t> currentLayer = std::vector<uint8_t>((64*4) * 64, 1.0);
    if (layers == nullptr || alphaArray == nullptr) return currentLayer;
    for (int j = 0; j < mapTile[i].nLayers; j++ ) {
        int alphaOffs = layers[j].offsetInMCAL;
        int offO = j;
        int readCnt = 0;
        int readForThisLayer = 0;

        if (!layers[j].flags.use_alpha_map) continue;

        if (layers[j].flags.alpha_map_compressed) {
            //Compressed
            //http://www.pxr.dk/wowdev/wiki/index.php?title=ADT/v18
            while( readForThisLayer < 4096 )
            {
                // fill or copy mode
                int fill = (alphaArray[alphaOffs] & 0x80 );
                int n = alphaArray[alphaOffs] & 0x7F;
                alphaOffs++;

                for ( int k = 0; k < n; k++ )
                {
                    if (readForThisLayer == 4096) break;

                    currentLayer[offO] = alphaArray[alphaOffs];
                    readCnt++; readForThisLayer++;
                    offO += 4;

                    if (readCnt >=64) {
                        readCnt = 0;
                    }

                    if( !fill ) alphaOffs++;
                }
                if( fill ) alphaOffs++;
            }
        } else {
            //Uncompressed
            if (((wdtObjFlags & 0x4) > 0) || ((wdtObjFlags & 0x80) > 0)) {
                //Uncompressed (4096)
                for (int iX =0; iX < 64; iX++) {
                    for (int iY = 0; iY < 64; iY++){
                        currentLayer[offO] = alphaArray[alphaOffs];

                        offO += 4; readCnt+=1; readForThisLayer+=1; alphaOffs++;
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
                        readCnt+=2; readForThisLayer+=2; alphaOffs++;
                    }
                }
            }
        }
    }
    return currentLayer;
}

void AdtFile::createTriangleStrip() {
    if (mcnkRead < 0) return;

    for (int i = 0; i <= mcnkRead; i++) {
        SMChunk &mcnkObj = mapTile[i];
        int hole = mcnkObj.holes_low_res;
        stripOffsets.push_back(strips.size());

        int j = 0;
        bool first = true;
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                if (!isHole(hole, x, y)) {
                    int _i = x * 2;
                    int _j = y * 4;

                    for (int k = 0; k < 2; k++) {
                        if (!first) {
                            strips.push_back(indexMapBuf(_i, _j + k * 2));
                        } else
                            first = false;

                        for (int l = 0; l < 3; l++) {

                            strips.push_back(indexMapBuf(_i + l, _j + k * 2));
                            strips.push_back(indexMapBuf(_i + l, _j + k * 2 + 2));
                        }
                        strips.push_back(indexMapBuf(_i + 2, _j + k * 2 + 2));
                    }
                }
            }
        }
    }
    stripOffsets.push_back(strips.size());
}
