//
// Created by deamon on 03.07.17.
//

#include "wmoGroupGeom.h"

chunkDef<WmoGroupGeom> WmoGroupGeom::wmoGroupTable = {
        handler : [](WmoGroupGeom& object, ChunkData& chunkData){},
        subChunks : {
                {
                        'MVER',
                        {
                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                }
                        }
                },
                {
                        'MOGP',
                        {
                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                    chunkData.readValue(object.mogp);
                                },
                                subChunks: {
                                        {
                                            'MOPY', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MOVI', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MONR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MOTV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MOCV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MODR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MOBA', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MOBN', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                },
                                            }
                                        },
                                        {
                                            'MOBR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

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
}
