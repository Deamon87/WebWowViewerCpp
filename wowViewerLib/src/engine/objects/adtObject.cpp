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
                                    object.textureNamesFieldLen = chunkData.chunkLen;
                                    chunkData.readValues(object.textureNamesField, object.textureNamesFieldLen);
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
                                                            chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcly);
                                                        }
                                                }
                                        },
                                        {
                                                'MCRF',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCRF"<<std::endl;
                                                            chunkData.readValue(object.mcnkStructs[object.mcnkRead].mcly);
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
    CChunkFileReader<AdtObject> reader(adtFile, &AdtObject::adtObjectTable);
    reader.processFile(*this);

    createTriangleStrip();
    createVBO();
//    createIndexVBO();
    loaded = true;
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

void AdtObject::loadAlphaTextures(int limit) {

}

void AdtObject::draw(std::vector<bool> &drawChunks) {

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

        if ((textureArray[i]) && (textureArray[i][0])) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureArray[i][0].texture);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, this.alphaTextures[i]);

//Bind layer textures
            for (int j = 1; j < textureArray[i].length; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                if ((textureArray[i][j]) && (textureArray[i][j].texture)) {
//gl.enable(gl.TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, textureArray[i][j].texture);
                } else {
                    glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
                }
            }
            for (int j = textureArray[i].length; j < 4; j++) {
                glActiveTexture(GL_TEXTURE1 + j);
                glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
            }

            int stripLength = stripOffsets[i + 1] - stripOffsets[i];
            glDrawElements(GL_TRIANGLE_STRIP, stripLength, GL_UNSIGNED_SHORT, (void *)(stripOffsets[i] * 2));
        }
    }
}