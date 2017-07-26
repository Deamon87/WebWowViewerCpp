//
// Created by deamon on 10.07.17.
//

#include "adtObject.h"
#include "../shaderDefinitions.h"

chunkDef<AdtObject> AdtObject::adtObjectTable = {

    handler : [](AdtObject& object, ChunkData& chunkData){},
    subChunks : {
        {
            'MVER',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MVER"<<std::endl;
                }
            }
        },
        {
            'MHDR',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MHDR"<<std::endl;
                  chunkData.readValue(object.mhdr);
                }
            }
        },
        {
            'MCIN',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MCIN"<<std::endl;
                  chunkData.readValue(object.mcins);
                }
            }
        },
        {
            'MTEX',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MTEX"<<std::endl;
                  char *textureNamesField;
                  int textureNamesFieldLen = chunkData.chunkLen;
                  chunkData.readValues(textureNamesField, textureNamesFieldLen);

                  int i = 0;
                  while (i < textureNamesFieldLen) {
                    object.textureNames.push_back(std::string(textureNamesField+i));
                    i+= object.textureNames[object.textureNames.size()-1].size()+1;
                  }

                }
            }
        },
        {
            'MMDX',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MMDX"<<std::endl;
                  object.doodadNamesFieldLen = chunkData.chunkLen;
                  chunkData.readValues(object.doodadNamesField, object.doodadNamesFieldLen);
                }
            }
        },
        {
            'MMID',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MMID"<<std::endl;
                }
            }
        },
        {
            'MWMO',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MWMO"<<std::endl;
                  object.wmoNamesFieldLen = chunkData.chunkLen;
                  chunkData.readValues(object.wmoNamesField, object.wmoNamesFieldLen);
                }
            }
        },
        {
            'MWID',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MWID"<<std::endl;
                }
            }
        },
        {
            'MDDF',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MDDF"<<std::endl;
                }
            }
        },
        {
            'MODF',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                  std::cout<<"Entered MODF"<<std::endl;
                }
            }
        },
        {
            'MCNK',
            {
                handler: [](AdtObject& object, ChunkData& chunkData){
                    std::cout<<"Entered MCNK"<<std::endl;
                    object.mcnkRead++;
                    chunkData.readValue(object.mapTile[object.mcnkRead]);

                    SMChunk chunk = object.mapTile[object.mcnkRead];
                    chunkDef<AdtObject> *def = &AdtObject::adtObjectTable.subChunks['MCNK'];

                    //1. Load MCVT
                    if (chunk.ofsHeight > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsHeight - 8;
                        chunkData.processChunkAtOffs(offset, object, def);
                    }
//                    2. Load MCNR
                    if (chunk.ofsNormal > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsNormal - 8;
                        chunkData.processChunkAtOffs(offset, object, def);
                    }
//                    3. Load MCLY
                    if (chunk.ofsLayer > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsLayer - 8;
                        chunkData.processChunkAtOffs(offset, object, def);
                    }
//                    4. Load MCAL
                    if (chunk.ofsAlpha > 0) {
                        int offset = chunkData.dataOffset + chunk.ofsAlpha - 8;
                        chunkData.processChunkAtOffsWithSize(offset, chunk.sizeAlpha, object, def);
                    }
//                    5. Load MCRF
                    if (chunk.ofsRefs >  0) {
                        int offset = chunkData.dataOffset + chunk.ofsRefs - 8;
                        chunkData.processChunkAtOffs(offset, object, def);
                    }

                    //Force stop the parsing of chunk
                    chunkData.bytesRead = chunkData.chunkLen;

                },
                subChunks : {
                    {
                        'MCVT',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCVT"<<std::endl;
                              chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcvt);
                            }
                        }
                    },
                    {
                        'MCLV',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCLV"<<std::endl;
                            }
                        }
                    },
                    {
                        'MCCV',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCCV"<<std::endl;
                              chunkData.readValue(object.mcnkStructs[object.mcnkRead].mccv);
                            }
                        }
                    },
                    {
                        'MCNR',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCNR"<<std::endl;
                              chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcnr);
                            }
                        }
                    },
                    {
                        'MCLY',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCLY"<<std::endl;
//                              object.mcnkStructs[object.mcnkRead].mcly_count =
//                                  chunkData.chunkLen/sizeof(SMLayer);
                              chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcly);
                            }
                        }
                    },
                    {
                        'MCRF',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCRF"<<std::endl;
                              chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcrf);
                            }
                        }
                    },
                    {
                        'MCAL',
                        {
                            handler: [](AdtObject& object, ChunkData& chunkData){
                              std::cout<<"Entered MCAL"<<std::endl;
                              chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcal);
                            }
                        }
                    }
                }
            }
        },
    }
};

void AdtObject::process(std::vector<unsigned char> &adtFile) {
    CChunkFileReader reader(adtFile);
    reader.processFile(*this, &AdtObject::adtObjectTable);

    createTriangleStrip();
    createVBO();
    loadAlphaTextures(256);
//    createIndexVBO();
    m_loaded = true;
}

void AdtObject::createVBO() {
    /* 1. help index + Heights + texCoords +  */
    std::vector<float> vboArray ;

    /* 1.1 help index */
    this->indexOffset = vboArray.size();
    for (int i = 0; i < 9 * 9 + 8 * 8; i++) {
        vboArray.push_back((float)i);
    }

    /* 1.2 Heights */
    this->heightOffset = vboArray.size();
    for (int i = 0; i <= mcnkRead; i++) {
        for (int j = 0; j < 145; j++) {
            vboArray.push_back(mcnkStructs[i].mcvt->height[j]);
        }
    }

    /* 1.3 Make combinedVbo */
    glGenBuffers(1, &combinedVbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);
    glBufferData(GL_ARRAY_BUFFER, vboArray.size()*sizeof(float), &vboArray[0], GL_STATIC_DRAW);

    /* 2. Strips */
    glGenBuffers(1, &stripVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, strips.size()*sizeof(int16_t), &strips[0], GL_STATIC_DRAW);
}

bool isHole(int hole, int i, int j) {
    static int holetab_h[4] = {0x1111, 0x2222, 0x4444, 0x8888};
    static int holetab_v[4] = {0x000F, 0x00F0, 0x0F00, 0xF000};

    return (hole & holetab_h[i] & holetab_v[j]) != 0;
}

int indexMapBuf(int x, int y) {
    int result = ((y + 1) >> 1) * 9 + (y >> 1) * 8 + x;
    return result;
}

void AdtObject::createTriangleStrip() {
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


std::vector<uint8_t> AdtObject::processTexture(int wdtObjFlags, int i) {
    mcnkStruct_t &mcnkObj = mcnkStructs[i];
    uint8_t* alphaArray = mcnkObj.mcal;
    SMLayer* layers = mcnkObj.mcly;

    std::vector<uint8_t> currentLayer = std::vector<uint8_t>((64*4) * 64);
    if (layers == nullptr || alphaArray == nullptr) return currentLayer;
    for (int j = 0; j < mapTile[i].nLayers; j++ ) {
        int alphaOffs = layers[j].offsetInMCAL;
        int offO = j;
        int readCnt = 0;
        int readForThisLayer = 0;

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

void AdtObject::loadAlphaTextures(int limit) {
    if (this->alphaTexturesLoaded>=256) return;

    int chunkCount = mcnkRead+1;
    int maxAlphaTexPerChunk = 4;
    int alphaTexSize = 64;

    int texWidth = alphaTexSize;
    int texHeight = alphaTexSize;

    int createdThisRun = 0;
    for (int i = this->alphaTexturesLoaded; i < chunkCount; i++) {
        GLuint alphaTexture;
        glGenTextures(1, &alphaTexture);
        std::vector<uint8_t> alphaTextureData = processTexture(0, i);

        glBindTexture(GL_TEXTURE_2D, alphaTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &alphaTextureData[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        alphaTextures.push_back(alphaTexture);

        createdThisRun++;
        if (createdThisRun >= limit) {
            break;
        }
    }
    this->alphaTexturesLoaded += createdThisRun;
}



void AdtObject::draw(std::vector<bool> &drawChunks) {
    if (!m_loaded) return;
    GLuint blackPixelTexture = this->m_api->getBlackPixelTexture();
    ShaderRuntimeData *adtShader = this->m_api->getAdtShader();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVbo);

    glVertexAttribPointer(+adtShader::Attribute::aIndex, 1, GL_FLOAT, false, 0, (void *)(this->indexOffset * 4));

//Draw
    for (int i = 0; i < 256; i++) {
        if (!drawChunks[i]) continue;

        glVertexAttribPointer(+adtShader::Attribute::aHeight, 1, GL_FLOAT, false, 0, (void *)((this->heightOffset + i * 145) * 4));
        glUniform3f(adtShader->getUnf("uPos"), mapTile[i].position.x, mapTile[i].position.y, mapTile[i].position.z);

        if (mapTile[i].nLayers <= 0) continue;

        BlpTexture &layer0 = getAdtTexture(mcnkStructs[i].mcly[0].textureId);
        if (layer0.getIsLoaded()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layer0.getGlTexture());

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, alphaTextures[i]);

            //Bind layer textures
            for (int j = 1; j < mapTile[i].nLayers; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                BlpTexture &layer_x = getAdtTexture(mcnkStructs[i].mcly[j].textureId);
                if (layer_x.getIsLoaded()) {
                    //gl.enable(gl.TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, layer_x.getGlTexture());
                } else {
                    glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
                }
            }
            for (int j = mapTile[i].nLayers; j < 4; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
            }

            int stripLength = stripOffsets[i + 1] - stripOffsets[i];
            glDrawElements(GL_TRIANGLE_STRIP, stripLength, GL_UNSIGNED_SHORT, (void *)(stripOffsets[i] * 2));
        }
    }
}

BlpTexture &AdtObject::getAdtTexture(int textureId) {
    std::string &materialTexture = textureNames[textureId];

    return *m_api->getTextureCache()->get(materialTexture);
}
