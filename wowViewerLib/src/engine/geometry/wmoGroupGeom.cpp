//
// Created by deamon on 03.07.17.
//

#include "wmoGroupGeom.h"
#include <iostream>

chunkDef<WmoGroupGeom> WmoGroupGeom::wmoGroupTable = {
        handler : [](WmoGroupGeom& object, ChunkData& chunkData){},
        subChunks : {
                {
                        'MVER',
                        {
                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                    std::cout<<"Entered MVER"<<std::endl;
                                }
                        }
                },
                {
                        'MOGP',
                        {
                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                    chunkData.readValue(object.mogp);
                                    std::cout<<"Entered MOGP"<<std::endl;
                                },
                                subChunks: {
                                        {
                                            'MOPY', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MOPY"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOVI', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.indicesLen = chunkData.chunkLen / 2;
                                                    chunkData.readValues(object.indicies, object.indicesLen);
                                                    std::cout<<"Entered MOVI"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOVT', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                    object.verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                                                    chunkData.readValues(object.verticles, object.verticesLen);
                                                    std::cout<<"Entered MOVT"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MONR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.normalsLen = chunkData.chunkLen / sizeof(C3Vector);
                                                    chunkData.readValues(object.normals, object.normalsLen);
                                                    std::cout<<"Entered MONR"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOTV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.textureCoordsLen = chunkData.chunkLen / sizeof(C2Vector);
                                                    chunkData.readValues(object.textCoords, object.textureCoordsLen);

                                                    std::cout<<"Entered MOTV"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOCV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.cvLen = chunkData.chunkLen / 4;
                                                    chunkData.readValues(object.colorArray, object.cvLen);
                                                    std::cout<<"Entered MOCV"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MODR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.doodadRefsLen = chunkData.chunkLen / 2;
                                                    chunkData.readValues(object.doodadRefs, object.doodadRefsLen);
                                                    std::cout<<"Entered MODR"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBA', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.batchesLen = chunkData.chunkLen / sizeof(SMOBatch);
                                                    chunkData.readValues(object.batches, object.batchesLen);
                                                    std::cout<<"Entered MOBA"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBN', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.nodesLen = chunkData.chunkLen / sizeof(t_BSP_NODE);
                                                    chunkData.readValues(object.bsp_nodes, object.nodesLen);
                                                    std::cout<<"Entered MOBN"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.bpsIndiciesLen = chunkData.chunkLen / sizeof(uint16_t);
                                                    chunkData.readValues(object.bpsIndicies, object.bpsIndiciesLen);
                                                    std::cout<<"Entered MOBR"<<std::endl;
                                                },
                                            }
                                        },
                                }
                        }
                }
        }
};



void WmoGroupGeom::process(std::vector<unsigned char> &wmoGroupFile) {
    CChunkFileReader<WmoGroupGeom> reader(wmoGroupFile, &WmoGroupGeom::wmoGroupTable);
    reader.processFile(*this);

    createVBO();
    createIndexVBO();
}

#define size_of_pvar(x) sizeof(std::remove_pointer<decltype(x)>::type)
#define makePtr(x) (unsigned char *) x
void WmoGroupGeom::createVBO() {

    int vboSizeInBytes =
            size_of_pvar(verticles) * verticesLen+
            size_of_pvar(normals) * normalsLen+
            size_of_pvar(textCoords) * textureCoordsLen+
            size_of_pvar(colorArray) * cvLen;

    std::vector<unsigned char> buffer (vboSizeInBytes);

    unsigned char *nextOffset = &buffer[0];
    nextOffset = std::copy(makePtr(verticles),  makePtr(verticles+verticesLen), nextOffset);
    nextOffset = std::copy(makePtr(normals),    makePtr(normals+normalsLen), nextOffset);
    nextOffset = std::copy(makePtr(textCoords), makePtr(textCoords+textureCoordsLen), nextOffset);
    nextOffset = std::copy(makePtr(colorArray), makePtr(colorArray+cvLen), nextOffset);

    glGenBuffers(1, &combinedVBO);
    glBindBuffer( GL_ARRAY_BUFFER, combinedVBO);
    glBufferData( GL_ARRAY_BUFFER, vboSizeInBytes, &buffer[0], GL_STATIC_DRAW );
}

void WmoGroupGeom::createIndexVBO() {
    glGenBuffers(1, &indexVBO);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indicesLen, indicies, GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
}
