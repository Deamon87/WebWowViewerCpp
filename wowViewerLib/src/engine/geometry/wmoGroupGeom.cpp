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
//                                    chunkData.readValue(object.mogp);
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
                                                    std::cout<<"Entered MOVI"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MONR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MONR"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOTV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MOTV"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOCV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MOCV"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MODR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MODR"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBA', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MOBA"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBN', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MOBN"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
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
}
